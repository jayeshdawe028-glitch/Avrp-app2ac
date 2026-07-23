// voice/Plugin.cpp

#include "../main.h"
#include "../gui/gui.h"

#include "Plugin.h"

#include "include/util/Timer.h"

#include "Record.h"
#include "Playback.h"
#include "Network.h"
#include "PluginConfig.h"
#include "MicroIcon.h"
#include "SpeakerList.h"
#include "GlobalStream.h"
#include "StreamAtPoint.h"
#include "StreamAtVehicle.h"
#include "StreamAtPlayer.h"
#include "StreamAtObject.h"

#include "../audiostream.h"
extern CAudioStream* pAudioStream;

uint32_t lasttickmicro = GetTickCount();
extern CGUI* pGUI;

static int MicType = 1;

bool Plugin::muteStatus { false };
bool Plugin::recordStatus { false };
bool Plugin::recordBusy { false };
bool Plugin::voiceReady { false };

std::map<uint32_t, StreamPtr> Plugin::streamTable;

bool Plugin::OnPluginLoad() noexcept
{
    if(!Render::Init())
    {
        LogVoice("[sv:err:plugin] : failed to init render module");
        return false;
    }

    Plugin::OnDeviceInit();
    Render::AddDeviceInitCallback(Plugin::OnDeviceInit);
    Render::AddRenderCallback(Plugin::OnRender);
    Render::AddDeviceFreeCallback(Plugin::OnDeviceFree);

    return true;
}

bool Plugin::OnSampLoad() noexcept
{
    if(!Samp::Init())
    {
        LogVoice("[sv:err:plugin] : failed to init samp");
        Render::Free();
        return false;
    }

    Samp::AddLoadCallback(Plugin::OnInitGame);
    Samp::AddExitCallback(Plugin::OnExitGame);

    if(!Network::Init())
    {
        LogVoice("[sv:err:plugin] : failed to init network");
        Render::Free();
        Samp::Free();
        return false;
    }

    Network::AddConnectCallback(Plugin::ConnectHandler);
    Network::AddSvConnectCallback(Plugin::PluginConnectHandler);
    Network::AddSvInitCallback(Plugin::PluginInitHandler);
    Network::AddDisconnectCallback(Plugin::DisconnectHandler);

    if(!Playback::Init())
    {
        LogVoice("[sv:err:plugin] : failed to init playback");
        Render::Free();
        Samp::Free();
        Network::Free();
        return false;
    }

    return true;
}

void Plugin::OnInitGame() noexcept {}

void Plugin::OnExitGame() noexcept
{
    Plugin::voiceReady = false;

    Network::Free();
    Plugin::streamTable.clear();

    Record::Free();
    Playback::Free();
}

void Plugin::MainLoop()
{
    if(!Samp::IsLoaded()) return;

    while(const auto controlPacket = Network::ReceiveControlPacket())
    {
        Plugin::ControlPacketHandler(*&*controlPacket);
    }

    while(const auto voicePacket = Network::ReceiveVoicePacket())
    {
        const auto& voicePacketRef = *voicePacket;

        const auto iter = Plugin::streamTable.find(voicePacketRef->stream);
        if(iter == Plugin::streamTable.end()) continue;

        iter->second->Push(*&voicePacketRef);
    }

    for(const auto& stream : Plugin::streamTable)
        stream.second->Tick();

    Playback::Tick();
    Record::Tick();

    const int defaultKey = 0x42;

    if (MicType == 2)
        Plugin::RenderVoiceButton();

    if(Plugin::voiceReady && !Plugin::muteStatus)
    {
        if(Plugin::recordStatus)
        {
            if(GetTickCount() - lasttickmicro >= 20000)
            {
                LogVoice("[sv:dbg:plugin] : activation key(%hhu) released timeout", defaultKey);

                if (!Plugin::recordBusy)
                    Plugin::recordStatus = false;

                SV::ReleaseKeyPacket releaseKeyPacket{};
                releaseKeyPacket.keyId = (uint8_t)defaultKey;

                if (!Network::SendControlPacket(SV::ControlPacketType::releaseKey,
                                                &releaseKeyPacket, sizeof(releaseKeyPacket)))
                {
                    LogVoice("[sv:err:plugin] : failed to send ReleaseKey packet (timeout)");
                }

                lasttickmicro = GetTickCount();
            }
        }
    }

    uint8_t frameBuffer[Network::kMaxVoiceDataSize];
    if(const auto frameSize = Record::GetFrame(frameBuffer, sizeof(frameBuffer)))
    {
        if(Plugin::voiceReady)
        {
            if(!Network::SendVoicePacket(frameBuffer, frameSize))
                LogVoice("[sv:err:plugin] : failed to send voice packet");
        }

        if(!Plugin::recordStatus)
        {
            Record::StopRecording();
            Network::EndSequence();
        }
    }
}

void Plugin::ConnectHandler(const std::string&, const uint16_t) {}

void Plugin::PluginConnectHandler(SV::ConnectPacket& connectStruct)
{
    connectStruct.signature = SV::kSignature;
    connectStruct.version = SV::kVersion;
    connectStruct.micro = Record::HasMicro();
}

bool Plugin::PluginInitHandler(const SV::PluginInitPacket& initPacket)
{
    Plugin::muteStatus = initPacket.mute;

    if(!Record::Init(initPacket.bitrate))
        LogVoice("[sv:inf:plugin:packet:init] : failed init record");

    Plugin::voiceReady = true;
    LogVoice("[sv:dbg:plugin] : voiceReady = true");

    return true;
}

void Plugin::ControlPacketHandler(const ControlPacket& controlPacket)
{
    switch(controlPacket.packet)
    {
        case SV::ControlPacketType::muteEnable:
        {
            if(controlPacket.length != 0) break;

            LogVoice("[sv:dbg:plugin:muteenable]");
            Plugin::muteStatus = true;
            Plugin::recordStatus = false;
            Plugin::recordBusy = false;
        }
            break;

        case SV::ControlPacketType::muteDisable:
        {
            if(controlPacket.length != 0) break;

            LogVoice("[sv:dbg:plugin:mutedisable]");
            Plugin::muteStatus = false;
        }
            break;

        case SV::ControlPacketType::startRecord:
        {
            if(controlPacket.length != 0) break;

            LogVoice("[sv:dbg:plugin:startrecord]");
            if(Plugin::muteStatus) break;

            Plugin::recordBusy = true;
            Plugin::recordStatus = true;
            Record::StartRecording();
        }
            break;

        case SV::ControlPacketType::stopRecord:
        {
            if(controlPacket.length != 0) break;

            LogVoice("[sv:dbg:plugin:stoprecord]");
            if(Plugin::muteStatus) break;

            Plugin::recordStatus = false;
            Plugin::recordBusy = false;
        }
            break;

        case SV::ControlPacketType::addKey:
        {
            const auto& stData = *reinterpret_cast<const SV::AddKeyPacket*>(controlPacket.data);
            if(controlPacket.length != sizeof(stData)) break;

            LogVoice("[sv:dbg:plugin:addkey] : keyid(0x%hhx)", stData.keyId);
        }
            break;

        case SV::ControlPacketType::removeKey:
        {
            const auto& stData = *reinterpret_cast<const SV::RemoveKeyPacket*>(controlPacket.data);
            if(controlPacket.length != sizeof(stData)) break;

            LogVoice("[sv:dbg:plugin:removekey] : keyid(0x%hhx)", stData.keyId);
        }
            break;

        case SV::ControlPacketType::removeAllKeys:
        {
            if(controlPacket.length) break;
            LogVoice("[sv:dbg:plugin:removeallkeys]");
        }
            break;

        case SV::ControlPacketType::createGStream:
        {
            const auto& stData = *reinterpret_cast<const SV::CreateGStreamPacket*>(controlPacket.data);
            if(controlPacket.length < sizeof(stData)) break;

            LogVoice("[sv:dbg:plugin:creategstream] : stream(%p), color(0x%x), name(%s)",
                     stData.stream, stData.color, stData.color ? stData.name : "");

            const auto& streamPtr = Plugin::streamTable[stData.stream] =
                                            MakeGlobalStream(stData.color, stData.name);

            streamPtr->AddPlayCallback(SpeakerList::OnSpeakerPlay);
            streamPtr->AddStopCallback(SpeakerList::OnSpeakerStop);
        }
            break;

        case SV::ControlPacketType::createLPStream:
        {
            const auto& stData = *reinterpret_cast<const SV::CreateLPStreamPacket*>(controlPacket.data);
            if(controlPacket.length < sizeof(stData)) break;

            LogVoice("[sv:dbg:plugin:createlpstream] : stream(%p), dist(%.2f), pos(%.2f;%.2f;%.2f), color(0x%x), name(%s)",
                     stData.stream, stData.distance, stData.position.x, stData.position.y, stData.position.z,
                     stData.color, stData.color ? stData.name : "");

            const auto& streamPtr = Plugin::streamTable[stData.stream] =
                                            MakeStreamAtPoint(stData.color, stData.name, stData.distance, stData.position);

            streamPtr->AddPlayCallback(SpeakerList::OnSpeakerPlay);
            streamPtr->AddStopCallback(SpeakerList::OnSpeakerStop);
        }
            break;

        case SV::ControlPacketType::createLStreamAtVehicle:
        {
            const auto& stData = *reinterpret_cast<const SV::CreateLStreamAtPacket*>(controlPacket.data);
            if(controlPacket.length < sizeof(stData)) break;

            LogVoice("[sv:dbg:plugin:createlstreamatvehicle] : stream(%p), dist(%.2f), vehicle(%hu), color(0x%x), name(%s)",
                     stData.stream, stData.distance, stData.target,
                     stData.color, stData.color ? stData.name : "");

            const auto& streamPtr = Plugin::streamTable[stData.stream] =
                                            MakeStreamAtVehicle(stData.color, stData.name, stData.distance, stData.target);

            streamPtr->AddPlayCallback(SpeakerList::OnSpeakerPlay);
            streamPtr->AddStopCallback(SpeakerList::OnSpeakerStop);
        }
            break;

        case SV::ControlPacketType::createLStreamAtPlayer:
        {
            const auto& stData = *reinterpret_cast<const SV::CreateLStreamAtPacket*>(controlPacket.data);
            if(controlPacket.length < sizeof(stData)) break;

            LogVoice("[sv:dbg:plugin:createlstreamatplayer] : stream(%p), dist(%.2f), player(%hu), color(0x%x), name(%s)",
                     stData.stream, stData.distance, stData.target,
                     stData.color, stData.color ? stData.name : "");

            const auto& streamPtr = Plugin::streamTable[stData.stream] =
                                            MakeStreamAtPlayer(stData.color, stData.name, stData.distance, stData.target);

            streamPtr->AddPlayCallback(SpeakerList::OnSpeakerPlay);
            streamPtr->AddStopCallback(SpeakerList::OnSpeakerStop);
        }
            break;

        case SV::ControlPacketType::createLStreamAtObject:
        {
            const auto& stData = *reinterpret_cast<const SV::CreateLStreamAtPacket*>(controlPacket.data);
            if(controlPacket.length < sizeof(stData)) break;

            LogVoice("[sv:dbg:plugin:createlstreamatobject] : stream(%p), dist(%.2f), object(%hu), color(0x%x), name(%s)",
                     stData.stream, stData.distance, stData.target,
                     stData.color, stData.color ? stData.name : "");

            const auto& streamPtr = Plugin::streamTable[stData.stream] =
                                            MakeStreamAtObject(stData.color, stData.name, stData.distance, stData.target);

            streamPtr->AddPlayCallback(SpeakerList::OnSpeakerPlay);
            streamPtr->AddStopCallback(SpeakerList::OnSpeakerStop);
        }
            break;

        case SV::ControlPacketType::updateLStreamDistance:
        {
            const auto& stData = *reinterpret_cast<const SV::UpdateLStreamDistancePacket*>(controlPacket.data);
            if(controlPacket.length != sizeof(stData)) break;

            const auto iter = Plugin::streamTable.find(stData.stream);
            if(iter == Plugin::streamTable.end()) break;

            static_cast<LocalStream*>(iter->second.get())->SetDistance(stData.distance);
        }
            break;

        case SV::ControlPacketType::updateLPStreamPosition:
        {
            const auto& stData = *reinterpret_cast<const SV::UpdateLPStreamPositionPacket*>(controlPacket.data);
            if(controlPacket.length != sizeof(stData)) break;

            const auto iter = Plugin::streamTable.find(stData.stream);
            if(iter == Plugin::streamTable.end()) break;

            static_cast<StreamAtPoint*>(iter->second.get())->SetPosition(stData.position);
        }
            break;

        case SV::ControlPacketType::deleteStream:
        {
            const auto& stData = *reinterpret_cast<const SV::DeleteStreamPacket*>(controlPacket.data);
            if (controlPacket.length != sizeof(stData)) break;

            Plugin::streamTable.erase(stData.stream);
        }
            break;

        case SV::ControlPacketType::setStreamParameter:
        {
            const auto& stData = *reinterpret_cast<const SV::SetStreamParameterPacket*>(controlPacket.data);
            if(controlPacket.length != sizeof(stData)) break;

            const auto iter = Plugin::streamTable.find(stData.stream);
            if(iter == Plugin::streamTable.end()) break;

            iter->second->SetParameter(stData.parameter, stData.value);
        }
            break;

        case SV::ControlPacketType::slideStreamParameter:
        {
            const auto& stData = *reinterpret_cast<const SV::SlideStreamParameterPacket*>(controlPacket.data);
            if(controlPacket.length != sizeof(stData)) break;

            const auto iter = Plugin::streamTable.find(stData.stream);
            if(iter == Plugin::streamTable.end()) break;

            iter->second->SlideParameter(stData.parameter, stData.startvalue, stData.endvalue, stData.time);
        }
            break;

        case SV::ControlPacketType::createEffect:
        {
            const auto& stData = *reinterpret_cast<const SV::CreateEffectPacket*>(controlPacket.data);
            if(controlPacket.length < sizeof(stData)) break;

            const auto iter = Plugin::streamTable.find(stData.stream);
            if(iter == Plugin::streamTable.end()) break;

            iter->second->EffectCreate(stData.effect, stData.number, stData.priority,
                                       stData.params, controlPacket.length - sizeof(stData));
        }
            break;

        case SV::ControlPacketType::deleteEffect:
        {
            const auto& stData = *reinterpret_cast<const SV::DeleteEffectPacket*>(controlPacket.data);
            if(controlPacket.length != sizeof(stData)) break;

            const auto iter = Plugin::streamTable.find(stData.stream);
            if(iter == Plugin::streamTable.end()) break;

            iter->second->EffectDelete(stData.effect);
        }
            break;
    }
}

void Plugin::DisconnectHandler()
{
    Plugin::voiceReady = false;

    Plugin::streamTable.clear();

    Plugin::muteStatus = false;
    Plugin::recordStatus = false;
    Plugin::recordBusy = false;

    Record::Free();
}

void Plugin::OnDeviceInit()
{
    SpeakerList::Init();
    MicroIcon::Init();
}

void Plugin::OnRender()
{
    Timer::Tick();
    Plugin::MainLoop();
    SpeakerList::Render();
}

void Plugin::OnDeviceFree()
{
    SpeakerList::Free();
    MicroIcon::Free();
}

void Plugin::RenderVoiceButton()
{
    const int defaultKey = 0x42;
    ImGuiIO &io = ImGui::GetIO();

    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0x00, 0x00, 0x00, 0x00).Value);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(0x00, 0x00, 0x00, 0x00).Value);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(0x00, 0x00, 0x00, 0x00).Value);

    ImGuiStyle style;
    style.FrameBorderSize = ImGui::GetStyle().FrameBorderSize;
    ImGui::GetStyle().FrameBorderSize = 0.0f;

    float screenWidth = io.DisplaySize.x;
    float screenHeight = io.DisplaySize.y;

    ImVec2 pos2 = ImVec2(screenWidth * 0.085f, screenHeight * 0.38f);
    ImVec2 size2 = ImVec2(screenWidth * 0.055f, screenWidth * 0.055f);

    ImGui::Begin("VoiceButton", nullptr,
                 ImGuiWindowFlags_NoTitleBar |
                 ImGuiWindowFlags_NoBackground |
                 ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoScrollbar |
                 ImGuiWindowFlags_NoSavedSettings |
                 ImGuiWindowFlags_AlwaysAutoResize);

    if (!Plugin::voiceReady)
    {
        ImGui::ImageButton("##mic_notready",
                           (ImTextureRef)MicroIcon::tMutedIcon->raster,
                           size2);
    }
    else if (!Plugin::muteStatus)
    {
        if (!Plugin::recordStatus)
        {
            if (ImGui::ImageButton("##mic_passive",
                                   (ImTextureRef)MicroIcon::tPassiveIcon->raster,
                                   size2))
            {
                lasttickmicro = GetTickCount();

                if (!Plugin::recordBusy)
                {
                    Plugin::recordStatus = true;
                    Record::StartRecording();
                }

                SV::PressKeyPacket pressKeyPacket{};
                pressKeyPacket.keyId = (uint8_t)defaultKey;

                if (!Network::SendControlPacket(SV::ControlPacketType::pressKey,
                                                &pressKeyPacket, sizeof(pressKeyPacket)))
                {
                    LogVoice("[sv:err:imgui] : failed to send PressKey packet");
                }
            }
        }
        else
        {
            if (ImGui::ImageButton("##mic_active",
                                   (ImTextureRef)MicroIcon::tActiveIcon->raster,
                                   size2))
            {
                if (!Plugin::recordBusy)
                    Plugin::recordStatus = false;

                SV::ReleaseKeyPacket releaseKeyPacket{};
                releaseKeyPacket.keyId = (uint8_t)defaultKey;

                if (!Network::SendControlPacket(SV::ControlPacketType::releaseKey,
                                                &releaseKeyPacket, sizeof(releaseKeyPacket)))
                {
                    LogVoice("[sv:err:imgui] : failed to send ReleaseKey packet");
                }
            }
        }
    }
    else
    {
        ImGui::ImageButton("##mic_muted",
                           (ImTextureRef)MicroIcon::tMutedIcon->raster,
                           size2);
    }

    ImGui::SetWindowPos(pos2);
    ImGui::SetWindowSize(size2);
    ImGui::End();

    ImGui::PopStyleColor(3);
    ImGui::GetStyle().FrameBorderSize = style.FrameBorderSize;
}

extern "C" void Voice_HudStart()
{
    LogVoice("[sv:dbg:hudvoice] : Voice_HudStart ENTER");

    const int defaultKey = 0x42;

    if (!Plugin::voiceReady) { LogVoice("[sv:dbg:hudvoice] : not ready"); return; }
    if (Plugin::muteStatus)  { LogVoice("[sv:dbg:hudvoice] : muted"); return; }

    lasttickmicro = GetTickCount();

    if (Plugin::recordStatus) { LogVoice("[sv:dbg:hudvoice] : already recording"); return; }

    LogVoice("[sv:dbg:hudvoice] : activation key(%hhu) pressed", defaultKey);

    if (!Plugin::recordBusy)
    {
        Plugin::recordStatus = true;
        Record::StartRecording();
    }

    SV::PressKeyPacket pressKeyPacket{};
    pressKeyPacket.keyId = (uint8_t)defaultKey;

    if (!Network::SendControlPacket(SV::ControlPacketType::pressKey,
                                    &pressKeyPacket, sizeof(pressKeyPacket)))
    {
        LogVoice("[sv:err:hudvoice] : failed to send PressKey packet");
    }
}

extern "C" void Voice_HudStop()
{
    LogVoice("[sv:dbg:hudvoice] : Voice_HudStop ENTER");

    const int defaultKey = 0x42;

    if (!Plugin::voiceReady) { LogVoice("[sv:dbg:hudvoice] : not ready"); return; }
    if (Plugin::muteStatus)  { LogVoice("[sv:dbg:hudvoice] : muted"); return; }

    LogVoice("[sv:dbg:hudvoice] : activation key(%hhu) released", defaultKey);

    if (!Plugin::recordBusy)
        Plugin::recordStatus = false;

    SV::ReleaseKeyPacket releaseKeyPacket{};
    releaseKeyPacket.keyId = (uint8_t)defaultKey;

    if (!Network::SendControlPacket(SV::ControlPacketType::releaseKey,
                                    &releaseKeyPacket, sizeof(releaseKeyPacket)))
    {
        LogVoice("[sv:err:hudvoice] : failed to send ReleaseKey packet");
    }
}
