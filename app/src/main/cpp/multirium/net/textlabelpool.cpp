#include "../main.h"
#include "netgame.h"
#include "game/control/scripting.h"
#include "settings.h"
#include "../util/CUtil.h"
#include "game/peds/playerped.h"

extern CNetGame* pNetGame;
extern CGUI *pGUI;

C3DTextLabelPool::C3DTextLabelPool()
{
    for(int i = 0; i < TEXT_LABEL_POOL_SIZE; i++) {
        m_pTextLabels[i] = nullptr;
        m_bSlotState[i] = false;
    }
}
C3DTextLabelPool::~C3DTextLabelPool()
{
    for (int i = 0; i < TEXT_LABEL_POOL_SIZE; i++) {
        if (m_pTextLabels[i] && m_bSlotState[i]) {
            m_bSlotState[i] = false;

            m_pTextLabels[i]->text[0] = '\0';

            delete m_pTextLabels[i];
            m_pTextLabels[i] = nullptr;
        }
    }
}

void TextWithColors(ImVec2 pos, ImColor col, const char* szStr, const char* szStrWithoutColors);

void C3DTextLabelPool::DrawNonAttached(TEXT_LABEL* pLabel)
{
    if (!pLabel)
    {
        return;
    }
    if (!pNetGame)
    {
        return;
    }
    if (!pNetGame->GetPlayerPool())
    {
        return;
    }
    CVector pos;
    memcpy((void*)&pos, (const void*)&(pLabel->pos), sizeof(CVector));
    DrawTextLabel(pLabel, &pos);
}

extern CSettings *pSettings;

void C3DTextLabelPool::DrawAttachedToPlayer(TEXT_LABEL* pLabel)
{
    if (!pNetGame->GetPlayerPool()->GetLocalPlayer())
    {
        return;
    }
    CPlayerPed* pPed = pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed();
    if (!pPed)
    {
        return;
    }

    if(!/*pSettings->Get().iIsEnable3dTextInVehicle && */pPed->IsInVehicle())
    {
        return;
    }

    PLAYERID playerId = pLabel->attachedToPlayerID;

    CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();

    CRemotePlayer* pPlayer = pPlayerPool->GetAt(playerId);
    if (!pPlayer)
    {
        return;
    }

    CPlayerPed* pPlayerPed = pPlayer->GetPlayerPed();
    if (!pPlayerPed)
    {
        return;
    }
    CVector boneOut;
    pPlayerPed->GetBonePosition(5, &boneOut);

    boneOut.x += pLabel->offsetCoords.x;
    boneOut.y += pLabel->offsetCoords.y;
    boneOut.z += pLabel->offsetCoords.z;

    float fDist = pPlayerPed->m_pPed->GetDistanceFromCamera();
    boneOut.z += fDist * 0.055f;

    DrawTextLabel(pLabel, &boneOut);
}

void C3DTextLabelPool::DrawAttachedToVehicle(TEXT_LABEL* pLabel)
{

    /*CVector pos;
    memset((void*)&pos, 0, sizeof(CVector));

    auto pVehicle = pNetGame->GetVehiclePool()->m_pVehicles[ pLabel->attachedToVehicleID ];

    if(pVehicle == nullptr || pVehicle->m_pVehicle == nullptr)
        return;

    RwMatrix mat;
    memset((void*)& mat, 0, sizeof(RwMatrix));

    pVehicle->GetMatrix(&mat);

    memcpy((void*)& pos, (const void*)& mat.pos, sizeof(CVector));

    pos.x += pLabel->offsetCoords.x;
    pos.y += pLabel->offsetCoords.y;
    pos.z += pLabel->offsetCoords.z;

    DrawTextLabel(pLabel, &pos);*/
}

void C3DTextLabelPool::DrawVehiclesInfo() {}

void C3DTextLabelPool::DrawTextLabel(TEXT_LABEL* pLabel, CVector* pPos)
{

    CPlayerPed* pPed = pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed();
    if (!pPed)
    {
        return;
    }

    if (!pPed->m_pPed->IsAdded())
    {
        return;
    }
    bool hitEntity = false;
    if (pLabel->useLineOfSight)
    {
        CAMERA_AIM* pCam = GameGetInternalAim();
        if (!pCam)
        {
            return;
        }

        hitEntity = ScriptCommand(&get_line_of_sight,
                                  pPos->x, pPos->y, pPos->z,
                                  pCam->pos1x, pCam->pos1y, pCam->pos1z,
                                  1, 0, 0, 0, 0);
    }
    if (!pLabel->useLineOfSight || hitEntity)
    {
        if (pPed->m_pPed->GetDistanceFromPoint(pPos->x, pPos->y, pPos->z) <= pLabel->drawDistance)
        {
            CVector Out;
            memset((void*)&Out, 0, sizeof(CVector));
            // CSprite::CalcScreenCoors
#if !VER_x32
            ((void (*)(CVector*, CVector*, float*, float*, bool, bool))(g_libGTASA + 0x006E9DF8))(pPos, &Out, 0, 0, 0, 0);
#else
            ((void (*)(CVector*, CVector*, float*, float*, bool, bool))(g_libGTASA + 0x005C57E8 + 1))(pPos, &Out, 0, 0, 0, 0);
#endif
            if (Out.z < 1.0f)
            {
                return;
            }

            ImVec2 pos = ImVec2(Out.x, Out.y);

            if (pLabel->m_fTrueX < 0.0f)
            {
                char* curBegin = pLabel->textWithoutColors;
                char* curPos = pLabel->textWithoutColors;
                while (*curPos != '\0')
                {
                    if (*curPos == '\n')
                    {
                        float width = ImGui::CalcTextSize(curBegin, (char*)(curPos - 1)).x;
                        if (width > pLabel->m_fTrueX)
                        {
                            pLabel->m_fTrueX = width;
                        }

                        curBegin = curPos + 1;
                    }

                    curPos++;
                }

                if (pLabel->m_fTrueX < 0.0f)
                {
                    pLabel->m_fTrueX = ImGui::CalcTextSize(pLabel->textWithoutColors).x;
                }

            }

            TextWithColors(pos, __builtin_bswap32(pLabel->color), pLabel->text, pLabel->textWithoutColors);
        }
    }
}


void FilterColors(char* szStr)
{
    if(!szStr) return;

    char szNonColored[2048+1];
    int iNonColoredMsgLen = 0;

    for(int pos = 0; pos < strlen(szStr) && szStr[pos] != '\0'; pos++)
    {
        if(pos+7 < strlen(szStr))
        {
            if(szStr[pos] == '{' && szStr[pos+7] == '}')
            {
                pos += 7;
                continue;
            }
        }

        szNonColored[iNonColoredMsgLen] = szStr[pos];
        iNonColoredMsgLen++;
    }

    szNonColored[iNonColoredMsgLen] = 0;
    strcpy(szStr, szNonColored);
}

void C3DTextLabelPool::CreateTextLabel(int labelID, char* text, uint32_t color,
                                       float posX, float posY, float posZ, float drawDistance, bool useLOS, PLAYERID attachedToPlayerID, VEHICLEID attachedToVehicleID)
{
    if (labelID < 0 || labelID >= TEXT_LABEL_POOL_SIZE)
    {
        return;
    }
    if (m_pTextLabels[labelID])
    {
        delete m_pTextLabels[labelID];
        m_pTextLabels[labelID] = nullptr;
        m_bSlotState[labelID] = false;
    }

    TEXT_LABEL* pTextLabel = new TEXT_LABEL;

    if (pTextLabel)
    {
        //pTextLabel->text = text;
        cp1251_to_utf8(pTextLabel->text, text);
        cp1251_to_utf8(pTextLabel->textWithoutColors, text);
        FilterColors(pTextLabel->textWithoutColors);

        pTextLabel->color = color;
        pTextLabel->pos.x = posX;
        pTextLabel->pos.y = posY;
        pTextLabel->pos.z = posZ;
        pTextLabel->drawDistance = drawDistance;
        pTextLabel->useLineOfSight = useLOS;
        pTextLabel->attachedToPlayerID = attachedToPlayerID;
        pTextLabel->attachedToVehicleID = attachedToVehicleID;

        pTextLabel->m_fTrueX = -1.0f;

        if (attachedToVehicleID != INVALID_VEHICLE_ID || attachedToPlayerID != INVALID_PLAYER_ID)
        {
            pTextLabel->offsetCoords.x = posX;
            pTextLabel->offsetCoords.y = posY;
            pTextLabel->offsetCoords.z = posZ;
        }

        m_pTextLabels[labelID] = pTextLabel;
        m_bSlotState[labelID] = true;
    }
}

void C3DTextLabelPool::Delete(int labelID)
{
    if (labelID < 0 || labelID >= TEXT_LABEL_POOL_SIZE)
    {
        return;
    }
    m_bSlotState[labelID] = false;
    if (m_pTextLabels[labelID])
    {
        delete m_pTextLabels[labelID];
        m_pTextLabels[labelID] = nullptr;
    }
}

void C3DTextLabelPool::AttachToPlayer(int labelID, PLAYERID playerID, CVector pos)
{
    if (labelID < 0 || labelID >= TEXT_LABEL_POOL_SIZE)
    {
        return;
    }
    if (m_bSlotState[labelID] == true && m_pTextLabels[labelID])
    {
        m_pTextLabels[labelID]->attachedToPlayerID = playerID;
        m_pTextLabels[labelID]->pos = pos;
        m_pTextLabels[labelID]->offsetCoords = pos;
    }
}

void C3DTextLabelPool::AttachToVehicle(int labelID, VEHICLEID vehicleID, CVector pos)
{
    if (labelID < 0 || labelID >= TEXT_LABEL_POOL_SIZE)
    {
        return;
    }
    if (m_bSlotState[labelID] == true && m_pTextLabels[labelID])
    {
        m_pTextLabels[labelID]->attachedToVehicleID = vehicleID;
        m_pTextLabels[labelID]->pos = pos;
        m_pTextLabels[labelID]->offsetCoords = pos;
    }
}

void C3DTextLabelPool::Update3DLabel(int labelID, uint32_t color, char* text)
{
    if (labelID < 0 || labelID >= TEXT_LABEL_POOL_SIZE)
    {
        return;
    }
    if (m_bSlotState[labelID] == true && m_pTextLabels[labelID])
    {
        m_pTextLabels[labelID]->color = color;
        cp1251_to_utf8(m_pTextLabels[labelID]->text, text);
    }
}

void C3DTextLabelPool::Draw()
{
    //if(pGame->m_bDl_enabled) DrawVehiclesInfo();
    for (int i = 0; i < TEXT_LABEL_POOL_SIZE; i++)
    {
        if (!m_pTextLabels[i] || !m_bSlotState[i])
        {
            continue;
        }
        TEXT_LABEL* pLabel = m_pTextLabels[i];
        if (!pNetGame->GetPlayerPool())
        {
            continue;
        }
        if (pLabel->attachedToPlayerID != INVALID_PLAYER_ID)
        {
            if (pLabel->attachedToPlayerID == pNetGame->GetPlayerPool()->GetLocalPlayerID())
            {
                continue;
            }
            DrawAttachedToPlayer(pLabel);
            continue;
        }
        else if (pLabel->attachedToVehicleID != INVALID_VEHICLE_ID)
        {
            DrawAttachedToVehicle(pLabel);
            continue;
        }
        else
        {
            DrawNonAttached(pLabel);
            continue;
        }

    }
}

void TextWithColors(ImVec2 pos, ImColor col, const char* szStr, const char* szStrWithoutColors = nullptr)
{
    if (pNetGame) {
        if (pNetGame->GetPlayerPool()) {
            if (pNetGame->GetPlayerPool()->GetLocalPlayer()) {
                CLocalPlayer* pPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
                if (pPlayer->GetPlayerPed()) {
                    if (pPlayer->GetPlayerPed()->GetActionTrigger() == ACTION_DEATH || pPlayer->GetPlayerPed()->IsDead()) return;
                }
            }
        }
    }
    float fLineOffsets[100];

    int iCounter = 0;
    if (szStrWithoutColors)
    {
        const char* curBegin = szStrWithoutColors;
        const char* curPos = szStrWithoutColors;
        while (*curPos != '\0')
        {
            if (*curPos == '\n')
            {
                float width = ImGui::CalcTextSize(curBegin, (char*)(curPos - 1)).x;
                width /= 2.0f;

                fLineOffsets[iCounter] = width;
                iCounter++;

                curBegin = curPos + 1;
            }
            curPos++;
            if (*curPos == '\0')
            {
                float width = ImGui::CalcTextSize(curBegin, (char*)(curPos - 1)).x;
                width /= 2.0f;

                fLineOffsets[iCounter] = width;
                iCounter++;
            }
        }
    }

    iCounter = 0;
    char tempStr[4096];

    ImVec2 vecPos = pos;

    strcpy(tempStr, szStr);
    tempStr[sizeof(tempStr) - 1] = '\0';

    bool pushedColorStyle = false;
    const char* textStart = tempStr;
    const char* textCur = tempStr;

    if (szStrWithoutColors)
    {
        vecPos.x -= fLineOffsets[0];
    }

    while(textCur < (tempStr + sizeof(tempStr)) && *textCur != '\0') {
        if (*textCur == '{') {
            if (textCur != textStart) {
                pGUI->RenderText(vecPos, col, true, textStart, textCur);
                vecPos.x += ImGui::CalcTextSize(textStart, textCur).x;
            }

            const char* colorStart = textCur + 1;
            do {
                ++textCur;
            } while (*textCur != '\0' && *textCur != '}');

            if (pushedColorStyle) {
                pushedColorStyle = false;
            }

            ImVec4 textColor;
            if (CUtil::ProcessInlineHexColor(colorStart, textCur, textColor)) {
                col = textColor;
                pushedColorStyle = true;
            }

            textStart = textCur + 1;
        } else if (*textCur == '\n') {
            pGUI->RenderText(vecPos, col, true, textStart, textCur);
            iCounter++;

            vecPos.x = pos.x;
            if (szStrWithoutColors)
            {
                vecPos.x -= fLineOffsets[iCounter];
            }

            vecPos.y += pGUI->GetFontSize();
            textStart = textCur + 1;
        }

        ++textCur;
    }

    if (textCur != textStart) {
        pGUI->RenderText(vecPos, col, true, textStart, textCur);
        vecPos.x += ImGui::CalcTextSize(textStart, textCur).x;
    }
    else
        vecPos.y += pGUI->GetFontSize();
}