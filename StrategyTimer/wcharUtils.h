#pragma once

#include <array>
#include <stdexcept>

// error C4996: 'wctomb': This function or variable may be unsafe. Consider using wctomb_s instead.
// To disable deprecation, use _CRT_SECURE_NO_WARNINGS.
#pragma warning(disable : 4996) //_CRT_SECURE_NO_WARNINGS 

static wchar_t* StrToWCHAR(std::string str);
static std::string WCHARToStr(const std::wstring& wStr);

static wchar_t* StrToWCHAR(std::string str)
{
    int wchars_num = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    wchar_t* wstrResultWStr = new wchar_t[wchars_num];
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wstrResultWStr, wchars_num);
    return wstrResultWStr;
}

static std::string WCHARToStr(const std::wstring& wStr)
{
    const int BUFF_SIZE = 7;
    if (MB_CUR_MAX >= BUFF_SIZE) throw std::invalid_argument("BUFF_SIZE too small");
    std::string result;
    bool shifts = wctomb(nullptr, 0);  // reset the conversion state
    for (const wchar_t wc : wStr)
    {
        std::array<char, BUFF_SIZE> buffer;
        const int ret = wctomb(buffer.data(), wc);
        if (ret < 0) throw std::invalid_argument("inconvertible wide characters in the current locale");
        buffer[ret] = '\0';  // make 'buffer' contain a C-style string
        result = result + std::string(buffer.data());
    }
    return result;
}
