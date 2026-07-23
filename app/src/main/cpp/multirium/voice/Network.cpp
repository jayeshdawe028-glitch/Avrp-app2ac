/* ============================================================================

    Echo Cross
    Telegram: @tasjJson

============================================================================ */

#include "../main.h"
#include "game/core/game.h"
#include "../net/netgame.h"

#include "Network.h"

extern CNetGame* pNetGame;

bool Network::Init() noexcept
{
    if (Network::initStatus)
        return false;

    Log("[sv:dbg:network:init] : module initializing...");

    Network::socketHandle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (Network::socketHandle == INVALID_SOCKET)
        return false;

    const int sendBufferSize{ (int)kSendBufferSize };
    const int recvBufferSize{ (int)kRecvBufferSize };

    if (setsockopt(Network::socketHandle, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<const void*>(&sendBufferSize), sizeof(sendBufferSize)) == SOCKET_ERROR ||
        setsockopt(Network::socketHandle, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const void*>(&recvBufferSize), sizeof(recvBufferSize)) == SOCKET_ERROR)
    {
        return false;
    }

    Network::connectionStatus = ConnectionStatus::Disconnected;

    memset(Network::inputVoicePacket.GetData(), 0, Network::inputVoicePacket.GetSize());
    memset(Network::outputVoicePacket.GetData(), 0, Network::outputVoicePacket.GetSize());

    Log("[sv:dbg:network:init] : module initialized");

    Network::initStatus = true;
    return true;
}

void Network::Free() noexcept
{
    if (!Network::initStatus)
        return;

    Log("[sv:dbg:network:free] : module releasing...");

    Network::connectionStatus = ConnectionStatus::Disconnected;

    if (Network::voiceThread.joinable())
        Network::voiceThread.detach();

    close(Network::socketHandle);
    Network::socketHandle = INVALID_SOCKET;

    Network::serverIp.clear();
    Network::serverKey = NULL;

    memset(Network::inputVoicePacket.GetData(), 0, Network::inputVoicePacket.GetSize());
    memset(Network::outputVoicePacket.GetData(), 0, Network::outputVoicePacket.GetSize());

    {
        std::lock_guard<std::mutex> lock(controlQueueMutex);
        for (auto& packet : Network::controlQueue)
            delete packet;
        Network::controlQueue.clear();
    }

    while (!Network::voiceQueue.empty())
        Network::voiceQueue.pop();

    Log("[sv:dbg:network:free] : module released");

    Network::initStatus = false;
}

bool Network::SendControlPacket(const uint16_t packet, const void* dataAddr, const uint16_t dataSize) noexcept
{
    if (Network::connectionStatus != ConnectionStatus::Connected)
        return false;

    if (!pNetGame || !pNetGame->GetRakClient())
        return false;

    RakNet::BitStream bsSend;
    bsSend.Write<uint8_t>(kRaknetPacketId);
    bsSend.Write<uint16_t>(packet);
    bsSend.Write<uint16_t>(dataSize);

    if (dataAddr != nullptr && dataSize != 0)
        bsSend.Write(static_cast<const char*>(dataAddr), dataSize);

    return pNetGame->GetRakClient()->Send(&bsSend, HIGH_PRIORITY, RELIABLE, 0);
}

bool Network::SendVoicePacket(const void* dataAddr, const uint16_t dataSize) noexcept
{
    if (dataAddr == nullptr || dataSize == 0 || dataSize > kMaxVoiceDataSize)
        return false;

    if (Network::connectionStatus != ConnectionStatus::Connected)
        return false;

    Network::outputVoicePacket->length = dataSize;
    Network::outputVoicePacket->CalcHash();

    std::memcpy(Network::outputVoicePacket->data, dataAddr, dataSize);

    const auto voicePacketAddr = reinterpret_cast<const char*>(Network::outputVoicePacket.GetData());
    const auto voicePacketSize = static_cast<int>(Network::outputVoicePacket->GetFullSize());

    const auto sentBytes = send(Network::socketHandle, voicePacketAddr, voicePacketSize, NULL);

    ++Network::outputVoicePacket->packid;

    return sentBytes == voicePacketSize;
}

void Network::EndSequence() noexcept
{
    if (!Network::initStatus)
        return;

    Network::outputVoicePacket->packid = NULL;
}

ControlPacket* Network::ReceiveControlPacket() noexcept
{
    std::lock_guard<std::mutex> lock(controlQueueMutex);

    if (!Network::initStatus || Network::controlQueue.empty())
        return nullptr;

    auto controlPacket = Network::controlQueue.front();
    Network::controlQueue.pop_front();

    return controlPacket;
}

VoicePacketContainerPtr Network::ReceiveVoicePacket() noexcept
{
    if (!Network::initStatus || Network::voiceQueue.empty())
        return nullptr;

    auto voicePacket = std::move(*Network::voiceQueue.front());
    Network::voiceQueue.pop();

    return voicePacket;
}

std::size_t Network::AddConnectCallback(ConnectCallback callback) noexcept
{
    if (!Network::initStatus) return (std::size_t)-1;
    Network::connectCallbacks.emplace_back(std::move(callback));
    return Network::connectCallbacks.size() - 1;
}

std::size_t Network::AddSvConnectCallback(SvConnectCallback callback) noexcept
{
    if (!Network::initStatus) return (std::size_t)-1;
    Network::svConnectCallbacks.emplace_back(std::move(callback));
    return Network::svConnectCallbacks.size() - 1;
}

std::size_t Network::AddSvInitCallback(SvInitCallback callback) noexcept
{
    if (!Network::initStatus) return (std::size_t)-1;
    Network::svInitCallbacks.emplace_back(std::move(callback));
    return Network::svInitCallbacks.size() - 1;
}

std::size_t Network::AddDisconnectCallback(DisconnectCallback callback) noexcept
{
    if (!Network::initStatus) return (std::size_t)-1;
    Network::disconnectCallbacks.emplace_back(std::move(callback));
    return Network::disconnectCallbacks.size() - 1;
}

void Network::RemoveConnectCallback(std::size_t callback) noexcept {}
void Network::RemoveSvConnectCallback(std::size_t callback) noexcept {}
void Network::RemoveSvInitCallback(std::size_t callback) noexcept {}
void Network::RemoveDisconnectCallback(std::size_t callback) noexcept {}

void Network::VoiceThread() noexcept
{
    VoicePacket keepAlivePacket;
    Timer::time_t keepAliveLastTime{ NULL };

    memset(&keepAlivePacket, 0, sizeof(keepAlivePacket));

    keepAlivePacket.svrkey = Network::serverKey;
    keepAlivePacket.packet = SV::VoicePacketType::keepAlive;
    keepAlivePacket.packid = NULL;
    keepAlivePacket.length = NULL;
    keepAlivePacket.sender = NULL;
    keepAlivePacket.stream = NULL;
    keepAlivePacket.CalcHash();

    while (true)
    {
        const auto curStatus = Network::connectionStatus;
        if (curStatus == ConnectionStatus::Disconnected) break;

        const auto curTime = Timer::Get();
        if (curTime - keepAliveLastTime >= kKeepAliveInterval)
        {
            send(Network::socketHandle, reinterpret_cast<const char*>(&keepAlivePacket), sizeof(keepAlivePacket), NULL);
            keepAliveLastTime = curTime;
        }

        if (curStatus != ConnectionStatus::Connected)
        {
            SleepForMilliseconds(100);
            continue;
        }

        const auto received = recv(Network::socketHandle,
                                   static_cast<char*>(Network::inputVoicePacket.GetData()),
                                   Network::inputVoicePacket.GetSize(), NULL);

        if (received == SOCKET_ERROR)
            break;

        if (received < (int)sizeof(VoicePacket))
            continue;

        if (!Network::inputVoicePacket->CheckHeader()) continue;
        if (received != (int)Network::inputVoicePacket->GetFullSize()) continue;
        if (Network::inputVoicePacket->packet == SV::VoicePacketType::keepAlive) continue;

        Network::voiceQueue.try_emplace(MakeVoicePacketContainer(Network::inputVoicePacket));
    }
}

void Network::OnRaknetConnect(const char* ip, const uint32_t port) noexcept
{
    if (!Network::initStatus)
        return;

    Network::serverIp = ip;

    if (Network::connectionStatus == ConnectionStatus::Disconnected)
    {
        for (const auto& connectCallback : Network::connectCallbacks)
            if (connectCallback) connectCallback(Network::serverIp, (uint16_t)port);
    }

    Network::connectionStatus = ConnectionStatus::RNConnecting;
}

bool Network::OnRaknetRpc(const int id, RakNet::BitStream& parameters) noexcept
{
    if (!Network::initStatus || id != kRaknetConnectRcpId)
        return true;

    if (Network::svConnectCallbacks.empty())
        return true;

    SV::ConnectPacket stData{};
    for (const auto& svConnectCallback : Network::svConnectCallbacks)
        if (svConnectCallback) svConnectCallback(stData);

    parameters.Write(reinterpret_cast<const char*>(&stData), sizeof(stData));

    Log("[sv:dbg:network:connect] : append ConnectPacket (sign=0x%X ver=%u mic=%u)",
        (unsigned)stData.signature, (unsigned)stData.version, (unsigned)stData.micro);

    return true;
}

bool Network::OnRaknetReceive(Packet* packet) noexcept
{
    if (!Network::initStatus || !packet || packet->length < 1)
        return true;

    auto controlPacketPtr = new ControlPacket();

    RakNet::BitStream bs((unsigned char*)packet->data, packet->length, false);
    bs.IgnoreBits(8);

    bs.Read(controlPacketPtr->packet);
    bs.Read(controlPacketPtr->length);

    controlPacketPtr->data = new uint8_t[controlPacketPtr->length];
    if (controlPacketPtr->length)
        bs.Read(reinterpret_cast<char*>(controlPacketPtr->data), controlPacketPtr->length);

    switch (controlPacketPtr->packet)
    {
        case SV::ControlPacketType::serverInfo:
        {
            const auto& stData = *reinterpret_cast<SV::ServerInfoPacket*>(controlPacketPtr->data);
            if (controlPacketPtr->length != sizeof(stData)) { delete controlPacketPtr; return false; }

            Log("[sv:dbg:network:serverInfo] : voiceserver '%s:%hu' key=0x%X",
                Network::serverIp.c_str(), stData.serverPort, (unsigned)stData.serverKey);

            sockaddr_in serverAddress{};
            serverAddress.sin_family = AF_INET;
            serverAddress.sin_addr.s_addr = inet_addr(Network::serverIp.c_str());
            serverAddress.sin_port = htons(stData.serverPort);

            if (connect(Network::socketHandle, reinterpret_cast<const sockaddr*>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR)
            {
                Log("[sv:err:network:serverInfo] : connect error");
                delete controlPacketPtr;
                return false;
            }

            Network::serverKey = stData.serverKey;

            Network::outputVoicePacket->svrkey = Network::serverKey;
            Network::outputVoicePacket->packet = SV::VoicePacketType::voicePacket;
            Network::outputVoicePacket->packid = NULL;
            Network::outputVoicePacket->length = NULL;
            Network::outputVoicePacket->sender = NULL;
            Network::outputVoicePacket->stream = NULL;
            Network::outputVoicePacket->CalcHash();

            Network::connectionStatus = ConnectionStatus::SVConnecting;
            Network::voiceThread = std::thread(Network::VoiceThread);

            delete controlPacketPtr;
            return false;
        }

        case SV::ControlPacketType::pluginInit:
        {
            const auto& stData = *reinterpret_cast<SV::PluginInitPacket*>(controlPacketPtr->data);
            if (controlPacketPtr->length != sizeof(stData)) { delete controlPacketPtr; return false; }

            Log("[sv:dbg:network:pluginInit] : bitrate=%u mute=%u", (unsigned)stData.bitrate, (unsigned)stData.mute);

            for (const auto& svInitCallback : Network::svInitCallbacks)
                if (svInitCallback) svInitCallback(stData);

            Network::connectionStatus = ConnectionStatus::Connected;

            delete controlPacketPtr;
            return false;
        }

        default:
        {
            std::lock_guard<std::mutex> lock(controlQueueMutex);
            Network::controlQueue.push_back(controlPacketPtr);
            return false;
        }
    }
}

void Network::OnRaknetDisconnect() noexcept
{
    if (!Network::initStatus)
        return;

    Log("[sv:dbg:network:disconnect] : raknet disconnected");

    if (Network::connectionStatus != ConnectionStatus::Disconnected)
    {
        for (const auto& disconnectCallback : Network::disconnectCallbacks)
            if (disconnectCallback) disconnectCallback();
    }

    Network::connectionStatus = ConnectionStatus::Disconnected;

    if (Network::voiceThread.joinable())
        Network::voiceThread.detach();

    Network::serverIp.clear();
    Network::serverKey = NULL;

    memset(Network::inputVoicePacket.GetData(), 0, Network::inputVoicePacket.GetSize());
    memset(Network::outputVoicePacket.GetData(), 0, Network::outputVoicePacket.GetSize());

    {
        std::lock_guard<std::mutex> lock(controlQueueMutex);
        for (auto& packet : Network::controlQueue)
            delete packet;
        Network::controlQueue.clear();
    }

    while (!Network::voiceQueue.empty())
        Network::voiceQueue.pop();
}
