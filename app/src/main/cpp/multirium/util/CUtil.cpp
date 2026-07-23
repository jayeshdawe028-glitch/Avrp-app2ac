//
// Created by viski on 15.02.2025.
//

#include "CUtil.h"

// что то новенькое
#include "gui/gui.h"

extern CGUI *pGUI;

void cp1251_to_utf8(char* out, const char* in, unsigned int len)
{
    static const int table[128] =
            {
                    // 80
                    0x82D0,     0x83D0,     0x9A80E2,   0x93D1,     0x9E80E2,   0xA680E2,   0xA080E2,   0xA180E2,
                    0xAC82E2,   0xB080E2,   0x89D0,     0xB980E2,   0x8AD0,     0x8CD0,     0x8BD0,     0x8FD0,
                    // 90
                    0x92D1,     0x9880E2,   0x9980E2,   0x9C80E2,   0x9D80E2,   0xA280E2,   0x9380E2,   0x9480E2,
                    0,          0xA284E2,   0x99D1,     0xBA80E2,   0x9AD1,     0x9CD1,     0x9BD1,     0x9FD1,
                    // A0
                    0xA0C2,     0x8ED0,     0x9ED1,     0x88D0,     0xA4C2,     0x90D2,     0xA6C2,     0xA7C2,
                    0x81D0,     0xA9C2,     0x84D0,     0xABC2,     0xACC2,     0xADC2,     0xAEC2,     0x87D0,
                    // B0
                    0xB0C2,     0xB1C2,     0x86D0,     0x96D1,     0x91D2,     0xB5C2,     0xB6C2,     0xB7C2,
                    0x91D1,     0x9684E2,   0x94D1,     0xBBC2,     0x98D1,     0x85D0,     0x95D1,     0x97D1,
                    // C0
                    0x90D0,     0x91D0,     0x92D0,     0x93D0,     0x94D0,     0x95D0,     0x96D0,     0x97D0,
                    0x98D0,     0x99D0,     0x9AD0,     0x9BD0,     0x9CD0,     0x9DD0,     0x9ED0,     0x9FD0,
                    // D0
                    0xA0D0,     0xA1D0,     0xA2D0,     0xA3D0,     0xA4D0,     0xA5D0,     0xA6D0,     0xA7D0,
                    0xA8D0,     0xA9D0,     0xAAD0,     0xABD0,     0xACD0,     0xADD0,     0xAED0,     0xAFD0,
                    // E0
                    0xB0D0,     0xB1D0,     0xB2D0,     0xB3D0,     0xB4D0,     0xB5D0,     0xB6D0,     0xB7D0,
                    0xB8D0,     0xB9D0,     0xBAD0,     0xBBD0,     0xBCD0,     0xBDD0,     0xBED0,     0xBFD0,
                    // F0
                    0x80D1,     0x81D1,     0x82D1,     0x83D1,     0x84D1,     0x85D1,     0x86D1,     0x87D1,
                    0x88D1,     0x89D1,     0x8AD1,     0x8BD1,     0x8CD1,     0x8DD1,     0x8ED1,     0x8FD1
            };

    int count = 0;

    while (*in)
    {
        if (len && (count >= len)) break;

        if (*in & 0x80)
        {
            int v = table[(int)(0x7f & *in++)];
            if (!v)
                continue;
            *out++ = (char)v;
            *out++ = (char)(v >> 8);
            if (v >>= 16)
                * out++ = (char)v;
        }
        else // ASCII
            *out++ = *in++;

        count++;
    }

    *out = 0;
}

uintptr_t CUtil::FindLib(const char* libname)
{
    void* handle = dlopen(libname, RTLD_LAZY);
    if (handle) {
        void* symbol = dlsym(handle, "JNI_OnLoad");
        if (symbol) {
            Dl_info info;
            if (dladdr(symbol, &info) != 0) {
                return reinterpret_cast<uintptr_t>(info.dli_fbase);
            }
        }
        dlclose(handle);
    }

    return 0;
}

RwTexture* CUtil::LoadTextureFromDB(const char* dbname, const char* texture)
{
    TextureDatabaseRuntime* db_handle = TextureDatabaseRuntime::GetDatabase(dbname);
    if(!db_handle)
    {
        Log("Error: Database not found! (%s)", dbname);
        return nullptr;
    }

    TextureDatabaseRuntime::Register(db_handle);

    auto tex = CUtil::GetTexture(texture);
    if(!tex)
    {
        Log("Error: Texture (%s) not found in database (%s)", dbname, texture);
        return nullptr;
    }

    TextureDatabaseRuntime::Unregister(db_handle);

    return tex;
}

RwTexture* CUtil::GetTexture(const char* name)
{
    auto tex = TextureDatabaseRuntime::GetTexture(name);
    if (!tex)
    {
        //tex = CUtil::LoadTextureFromDB("gta3", "ahoodfence2");
        Log("WARNING! No tex = %s", name);
        return nullptr;
    }
    ++tex->refCount;

    return tex;
}
CVector CUtil::TransformPoint(const CSimpleTransform &t, const RwV3d &v)
{
    float cos_heading = cosf(t.m_fHeading);
    float sin_heading = sinf(t.m_fHeading);

    return {
            t.m_vPosn.x + cos_heading * v.x - sin_heading * v.y,
            t.m_vPosn.y + sin_heading * v.x + cos_heading * v.y,
            v.z + t.m_vPosn.z
    };
}

char* ConvertColorToHtml(std::string format) {
    bool first_time_flag = true;
    bool color_used = false;

    for (size_t i = 0; i < format.length(); i++) {
        if (format[i] == '{' && i + 7 < format.length() && format[i + 7] == '}') {
            format.insert(++i, first_time_flag ? "<font color='#" : "</font><font color='#");
            first_time_flag = false;
            color_used = true;
        }
        if (format[i] == '}') {
            format.insert(++i, "'>");
        }
    }

    format.erase(std::remove(format.begin(), format.end(), '{'), format.end());
    format.erase(std::remove(format.begin(), format.end(), '}'), format.end());

    if (color_used) {
        format.append("</font>");
    }

    for (size_t i = 0; i < format.size(); i++) {
        if (format[i] == '\n') {
            format.replace(i, 1, "<br>");
            i += 3;
        }
    }

    char* result = new char[format.length() + 1];
    strcpy(result, format.c_str());

    return result;
}

ImVec2 CUtil::CalcTextSizeWithoutTags(char* szStr, float font_size)
{
    if(!szStr) return ImVec2(0, 0);

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

    ImVec2 text_size;

    if(font_size)
    {
        text_size = pGUI->GetFont()->CalcTextSizeA(font_size, FLT_MAX, -1.0f, szNonColored);
        text_size.x = (float)(int)(text_size.x + 0.95f);
    }
    else
        text_size = ImGui::CalcTextSize(szNonColored);

    return text_size;
}

void CUtil::drawText(const char* fmt, ...)
{
    char tempStr[4096];

    va_list argPtr;
    va_start(argPtr, fmt);
    vsnprintf(tempStr, sizeof(tempStr), fmt, argPtr);
    va_end(argPtr);
    tempStr[sizeof(tempStr) - 1] = '\0';

    bool pushedColorStyle = false;
    const char* textStart = tempStr;
    const char* textCur = tempStr;
    while(textCur < (tempStr + sizeof(tempStr)) && *textCur != '\0')
    {
        if (*textCur == '{')
        {
            if (textCur != textStart)
            {
                ImGui::TextUnformatted(textStart, textCur);
                ImGui::SameLine(0.0f, 0.0f);
            }

            const char* colorStart = textCur + 1;
            do
            {
                ++textCur;
            } while (*textCur != '\0' && *textCur != '}');

            if (pushedColorStyle)
            {
                ImGui::PopStyleColor();
                pushedColorStyle = false;
            }

            ImVec4 textColor;
            if (CUtil::ProcessInlineHexColor(colorStart, textCur, textColor))
            {
                ImGui::PushStyleColor(ImGuiCol_Text, textColor);
                pushedColorStyle = true;
            }

            textStart = textCur + 1;
        }
        else if (*textCur == '\n')
        {
            ImGui::TextUnformatted(textStart, textCur);
            textStart = textCur + 1;
        }

        ++textCur;
    }

    if (textCur != textStart)
        ImGui::TextUnformatted(textStart, textCur);
    else
        ImGui::NewLine();

    if(pushedColorStyle)
        ImGui::PopStyleColor();
}

void CUtil::drawText(ImVec2 pos, ImColor col, const char* szStr, const float font_size)
{
    char tempStr[4096];

    ImVec2 vecPos = pos;

    strcpy(tempStr, szStr);
    tempStr[sizeof(tempStr) - 1] = '\0';

    bool pushedColorStyle = false;
    const char* textStart = tempStr;
    const char* textCur = tempStr;
    while(textCur < (tempStr + sizeof(tempStr)) && *textCur != '\0')
    {
        if (*textCur == '{')
        {
            if (textCur != textStart)
            {
                pGUI->RenderTextWithSize(vecPos, col, true, textStart, textCur, font_size);
                vecPos.x += ImGui::CalcTextSizeByFontSize(font_size, textStart, textCur).x;
            }

            const char* colorStart = textCur + 1;
            do
            {
                ++textCur;
            } while (*textCur != '\0' && *textCur != '}');

            if (pushedColorStyle)
                pushedColorStyle = false;

            ImVec4 textColor;
            if (CUtil::ProcessInlineHexColor(colorStart, textCur, textColor))
            {
                col = textColor;
                pushedColorStyle = true;
            }

            textStart = textCur + 1;
        }
        else if (*textCur == '\n')
        {
            pGUI->RenderTextWithSize(vecPos, col, true, textStart, textCur, font_size);
            vecPos.x = pos.x;
            vecPos.y += font_size;
            textStart = textCur + 1;
        }

        ++textCur;
    }

    if (textCur != textStart)
    {
        pGUI->RenderTextWithSize(vecPos, col, true, textStart, textCur, font_size);
        vecPos.x += ImGui::CalcTextSizeByFontSize(font_size, textStart, textCur).x;
    }
    else
        vecPos.y += font_size;
}

bool CUtil::ProcessInlineHexColor(const char* start, const char* end, ImVec4& color)
{
    const int hexCount = (int)(end-start);
    if(hexCount == 6 || hexCount == 8)
    {
        char hex[9];
        strncpy(hex, start, hexCount);
        hex[hexCount] = 0;

        unsigned int hexColor = 0;
        if(sscanf(hex, "%x", &hexColor)	> 0)
        {
            color.x = static_cast< float >((hexColor & 0x00FF0000) >> 16) / 255.0f;
            color.y = static_cast< float >((hexColor & 0x0000FF00) >> 8) / 255.0f;
            color.z = static_cast< float >((hexColor & 0x000000FF)) / 255.0f;
            color.w = 1.0f;

            if(hexCount == 8)
                color.w = static_cast< float >((hexColor & 0xFF000000) >> 24) / 255.0f;

            return true;
        }
    }

    return false;
}