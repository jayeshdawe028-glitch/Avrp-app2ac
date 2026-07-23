#pragma once

// круто
#include <jni.h>
#include "game/core/game.h"
#include "util/util.h"
#include "../../main.h"
#include "../../settings.h"
#include "util/CUtil.h"

#include "../vendor/encoding/encoding.h"

#include "..//vendor/raknet/SingleProducerConsumer.h"
#include "../vendor/imgui/imgui.h"
#include "vendor/armhook/patch.h"

extern CSettings* pSettings;

#pragma pack(push, 1)
struct BUFFERED_COMMAND_TEXTDRAW
{
    uint16_t textdrawId;
};
#pragma pack(pop)

enum eTouchType
{
	TOUCH_POP = 1,
	TOUCH_PUSH = 2,
	TOUCH_MOVE = 3
};

class CGUI
{
public:
	CGUI();
	~CGUI();

	void Render();

    static const ImColor fixcolor(uint32_t color) {
        return ImColor(
                (int)((color & 0xFF000000) >> 24),
                (int)((color & 0x00FF0000) >> 16),
                (int)((color & 0x0000FF00) >> 8));
    }

	float ScaleX(float x) { return m_vecScale.x * x; }
	float ScaleY(float y) { return m_vecScale.y * y; }
	ImFont* GetFont() { return m_pFont; }
	ImFont* GetSampFont() { return m_pSampFont; }
	ImFont* LoadFont(char *font, float fontsize);
	float GetFontSize() { return m_fFontSize; }

	bool OnTouchEvent(int type, bool multi, int x, int y);

	void RenderText(ImVec2& pos, ImU32 col, bool bOutline, const char* text_begin, const char* text_end = nullptr);

	RwRaster* g_FontRaster = nullptr;

	void RenderTextWithSize(ImVec2 &posCur, ImU32 col, bool bOutline, const char *text_begin,
							const char *text_end, float font_size);

    ImFont* 	m_pFont;

    // chat
    void addChatMessage(char* message, const std::string& nick, const ImColor& nick_color);
    void addInfoMessage(const std::string& format, ...);
    void AddDebugMessage(char* szFormat, ...);
    void addClientMessage(char* message, const ImColor& color);
    void AddDebugMessageNonFormatted(char *szStr);
    void FilterInvalidChars(char *szString);
private:
	void PreProcessInput();
	void PostProcessInput();

private:
    ImFont*		m_pSampFont;

	ImVec2		m_vecScale;
	float 		m_fFontSize;

	bool		m_bNeedClearMousePos;
	bool		m_bMouseDown;
	ImVec2		m_vTouchPos;
	bool		m_bNextClear;

    bool 		m_bKeysStatus;

    uint32_t m_dwTextColor;
    uint32_t m_dwInfoColor;
    uint32_t m_dwDebugColor;
};