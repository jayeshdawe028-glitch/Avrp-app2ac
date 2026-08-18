#include "../main.h"
#include "game/core/game.h"
#include "../net/netgame.h"
#include "game/fakerw/RenderWare.h"
#include "../playertags.h"
#include "../java/jniutil.h"
#include "game/vehicles/vehicle.h"
#include "CFontRenderer.h"
#include "gui.h"
#include "settings.h"
#include "../java/CHud.h"
#include "../java/CChat.h"

#include "game/core/textures/TextureDatabaseRuntime.h"
#include "game/core/Streaming.h"
#include "game/pools/Pools.h"
#include "game/custom/DebugInfo.h"
#include "game/core/widgets/WidgetRadar.h"
#include "game/custom/GPS.h"

extern CPlayerTags *pPlayerTags;
extern CNetGame *pNetGame;
extern CJavaWrapper *g_pJavaWrapper;
extern CSettings *pSettings;

/* imgui_impl_renderware.h */
void ImGui_ImplRenderWare_RenderDrawData(ImDrawData* draw_data);
bool ImGui_ImplRenderWare_Init();
void ImGui_ImplRenderWare_ShutDown();

#define MULT_X	0.00052083333f	// 1/1920
#define MULT_Y	0.00092592592f 	// 1/1080

CGUI::CGUI()
{
	Log(OBFUSCATE("Initializing GUI.."));

    m_dwTextColor = 0xFFFFFFFF;
    m_dwInfoColor = 0x00C8C8FF;
    m_dwDebugColor = 0xBEBEBEFF;

	m_bMouseDown = false;
	m_vTouchPos = ImVec2(-1, -1);
	m_bNextClear = false;
	m_bNeedClearMousePos = false;

	// setup ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();

	ImGui_ImplRenderWare_Init();

	// scale
	m_vecScale.x = io.DisplaySize.x * MULT_X;
	m_vecScale.y = io.DisplaySize.y * MULT_Y;
	// font Size
	m_fFontSize = ScaleY( pSettings->Get().fFontSize );

	Log(OBFUSCATE("GUI | Scale factor: %f, %f Font size: %f"), m_vecScale.x, m_vecScale.y, m_fFontSize);

	// setup style
	ImGuiStyle& style = ImGui::GetStyle();
	style.ScrollbarSize = ScaleY(55.0f);
	style.WindowBorderSize = 0.0f;
	ImGui::StyleColorsDark();

	// load fonts
	char path[0xFF];
	sprintf(path, "%sSAMP/fonts/%s", g_pszStorage, pSettings->Get().szFont);
	// cp1251 ranges
	static const ImWchar ranges[] = 
	{
		0x0020, 0x0080,
		0x00A0, 0x00C0,
		0x0400, 0x0460,
		0x0490, 0x04A0,
		0x2010, 0x2040,
		0x20A0, 0x20B0,
		0x2110, 0x2130,
		0
	};
	Log("GUI | Loading font: %s", pSettings->Get().szFont);
	m_pFont = io.Fonts->AddFontFromFileTTF(path, m_fFontSize, nullptr, ranges);
	Log("GUI | ImFont pointer = 0x%X", m_pFont);
	style.WindowRounding = 0.0f;

	m_bKeysStatus = false;
}

ImFont* CGUI::LoadFont(char *font, float fontsize)
{
	ImGuiIO &io = ImGui::GetIO();

	// load fonts
	char path[0xFF];

    sprintf(path, "%sSAMP/fonts/%s", g_pszStorage, font);

	// ranges
	static const ImWchar ranges[] = 
	{
		0x0020, 0x00FF, // Basic Latin + Latin Supplement
		0x0400, 0x04FF, // Russia
		0x0E00, 0x0E7F, // Thai
                                    0x2DE0, 0x2DFF, // Cyrillic Extended-A
                                    0xA640, 0xA69F, // Cyrillic Extended-B
		0xF020, 0xF0FF, // Half-width characters
		0
	};
	
	ImFont* pFont = io.Fonts->AddFontFromFileTTF(path, m_fFontSize, nullptr, ranges);
	return pFont;
}

CGUI::~CGUI()
{
	ImGui_ImplRenderWare_ShutDown();
	ImGui::DestroyContext();
}

#include "game/custom/DebugInfo.h"
#include "game/custom/GPS.h"
#include "java/CDialog.h"
#include "java/CSpeedometr.h"
#include "game/custom/debugtools/DebugModules/DebugModules.h"

void CGUI::PreProcessInput()
{
    ImGuiIO& io = ImGui::GetIO();

    io.MousePos = m_vTouchPos;
    io.MouseDown[0] = m_bMouseDown;
    if (!m_bNeedClearMousePos && m_bNextClear)
    {
        m_bNextClear = false;
    }
    if (m_bNeedClearMousePos && m_bNextClear)
    {
        io.MousePos = ImVec2(-1, -1);
        m_bNextClear = true;
    }
}

void CGUI::PostProcessInput()
{
    ImGuiIO& io = ImGui::GetIO();

    if (m_bNeedClearMousePos && io.MouseDown[0])
    {
        return;
    }

    if (m_bNeedClearMousePos && !io.MouseDown[0])
    {
        io.MousePos = ImVec2(-1, -1);
        m_bNextClear = true;
    }
}
void RenderBackgroundHud();
extern CGame* pGame;

void ImGui_ImplRenderWare_NewFrame();

void CGUI::Render()
{
    if(pNetGame) {
        PreProcessInput();
        ImGui_ImplRenderWare_NewFrame();

        ImGui::NewFrame();

        if (pPlayerTags) pPlayerTags->Render();

        if (pNetGame && pNetGame->GetTextLabelPool()) {
            pNetGame->GetTextLabelPool()->Draw();
        }

        CWidgetRadar::RenderRadarTexture();
		GPS::DoPathDraw();
        CDebugInfo::ToggleDebugDraw(true);
        CDebugInfo::Draw();
        DebugModules::PreRenderUpdate();

        if (pGame && pGame->FindPlayerPed() && pGame->FindPlayerPed()->IsInVehicle() && !pGame->FindPlayerPed()->IsAPassenger() && !CDialog::bIsShow) {
            if (!CSpeedometr::bIsShow) {
                CSpeedometr::show();
            }
            CSpeedometr::update();
        } else {
            if (CSpeedometr::bIsShow) {
                CSpeedometr::hide();
            }
        }

        ImGui::EndFrame();

        ImGui::Render();
        ImGui_ImplRenderWare_RenderDrawData(ImGui::GetDrawData());
        PostProcessInput();
    }
}

bool CGUI::OnTouchEvent(int type, bool multi, int x, int y)
{
	bool bFalse = true;
    ImGuiIO &io = ImGui::GetIO();

	switch(type)
	{
		case TOUCH_PUSH:
		{
			m_vTouchPos = ImVec2(x, y);
			m_bMouseDown = true;
			m_bNeedClearMousePos = false;
			break;
		}

		case TOUCH_POP:
		{
			m_bMouseDown = false;
			m_bNeedClearMousePos = true;
			break;
		}

		case TOUCH_MOVE:
		{
			m_bNeedClearMousePos = false;
			m_vTouchPos = ImVec2(x, y);
			break;
		}
	}
	if (!bFalse)
	{
		return false;
	}
	return true;
}


void CGUI::RenderTextWithSize(ImVec2& posCur, ImU32 col, bool bOutline, const char* text_begin, const char* text_end, float font_size)
{
	int iOffset = pSettings->Get().iFontOutline;

	if (bOutline)
	{
		// left
		posCur.x -= iOffset;
		ImGui::GetBackgroundDrawList()->AddText(m_pFont, font_size, posCur, col, text_begin, text_end);
		posCur.x += iOffset;
		// right
		posCur.x += iOffset;
		ImGui::GetBackgroundDrawList()->AddText(m_pFont, font_size, posCur, col, text_begin, text_end);
		posCur.x -= iOffset;
		// above
		posCur.y -= iOffset;
		ImGui::GetBackgroundDrawList()->AddText(m_pFont, font_size, posCur, col, text_begin, text_end);
		posCur.y += iOffset;
		// below
		posCur.y += iOffset;
		ImGui::GetBackgroundDrawList()->AddText(m_pFont, font_size, posCur, col, text_begin, text_end);
		posCur.y -= iOffset;
	}

	ImGui::GetBackgroundDrawList()->AddText(m_pFont, font_size, posCur, col, text_begin, text_end);
}

void CGUI::RenderText(ImVec2& posCur, ImU32 col, bool bOutline, const char* text_begin, const char* text_end)
{
	int iOffset = pSettings->Get().iFontOutline;

	if (bOutline)
	{
		if (true)
		{
			posCur.x -= iOffset;
			ImGui::GetBackgroundDrawList()->AddText(posCur, ImColor(IM_COL32_BLACK), text_begin, text_end);
			posCur.x += iOffset;
			// right 
			posCur.x += iOffset;
			ImGui::GetBackgroundDrawList()->AddText(posCur, ImColor(IM_COL32_BLACK), text_begin, text_end);
			posCur.x -= iOffset;
			// above
			posCur.y -= iOffset;
			ImGui::GetBackgroundDrawList()->AddText(posCur, ImColor(IM_COL32_BLACK), text_begin, text_end);
			posCur.y += iOffset;
			// below
			posCur.y += iOffset;
			ImGui::GetBackgroundDrawList()->AddText(posCur, ImColor(IM_COL32_BLACK), text_begin, text_end);
			posCur.y -= iOffset;
		}
		else
		{
			ImVec2 b(posCur.x + ImGui::CalcTextSize(text_begin, text_end).x, posCur.y + GetFontSize());

			ImColor co(0.0f, 0.0f, 0.0f, 0.4f);
			ImGui::GetBackgroundDrawList()->AddRectFilled(posCur, b, co);

		}
	}

	ImGui::GetBackgroundDrawList()->AddText(posCur, col, text_begin, text_end);
}

void CGUI::FilterInvalidChars(char *szString)
{
    while(*szString)
    {
        if(*szString > 0 && *szString < ' ')
            *szString = ' ';

        szString++;
    }
}

void CGUI::addChatMessage(char* message, const std::string& nick, const ImColor& nick_color)
{
    FilterInvalidChars(message);
    CChat::AddChatMessage(message, m_dwTextColor);
}

void CGUI::addInfoMessage(const std::string& format, ...)
{
    char tmp_buf[512];

    va_list args;
    va_start(args, format);
    vsprintf(tmp_buf, format.c_str(), args);
    va_end(args);

    FilterInvalidChars(tmp_buf);
    CChat::AddChatMessage(tmp_buf, m_dwInfoColor);
}

void CGUI::AddDebugMessage(char* szFormat, ...)
{
    char tmp_buf[512];
    memset(tmp_buf, 0, sizeof(tmp_buf));

    va_list args;
    va_start(args, szFormat);
    vsprintf(tmp_buf, szFormat, args);
    va_end(args);

    FilterInvalidChars(tmp_buf);
    CChat::AddChatMessage(tmp_buf, m_dwInfoColor);
}

void CGUI::addClientMessage(char* message, const ImColor& color)
{
    FilterInvalidChars(message);
    CChat::AddChatMessage(message, color);
}

void CGUI::AddDebugMessageNonFormatted(char* szStr)
{
    FilterInvalidChars(szStr);
    CChat::AddChatMessage(szStr, m_dwInfoColor);
}