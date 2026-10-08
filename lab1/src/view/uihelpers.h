#ifndef UIHELPERS_H
#define UIHELPERS_H

// Вспомогательные функции для построения интерфейса на WinAPI.

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <string>

namespace ui {

// Масштабирование размеров под текущий DPI экрана (96 DPI = 100 %).
inline int scale(int value)
{
    static int dpi = [] {
        HDC dc = GetDC(nullptr);
        const int d = GetDeviceCaps(dc, LOGPIXELSY);
        ReleaseDC(nullptr, dc);
        return d > 0 ? d : 96;
    }();
    return MulDiv(value, dpi, 96);
}

// Системный шрифт интерфейса (Segoe UI). bold и sizeFactor — для выделенных меток.
inline HFONT createUiFont(bool bold = false, double sizeFactor = 1.0)
{
    NONCLIENTMETRICSW metrics{};
    metrics.cbSize = sizeof(metrics);
    SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, 0);

    LOGFONTW font = metrics.lfMessageFont;
    font.lfHeight = static_cast<LONG>(font.lfHeight * sizeFactor);
    if (bold)
        font.lfWeight = FW_SEMIBOLD;
    return CreateFontIndirectW(&font);
}

// Текст поля ввода.
inline std::wstring windowText(HWND hwnd)
{
    const int length = GetWindowTextLengthW(hwnd);
    std::wstring text(static_cast<size_t>(length) + 1, L'\0');
    GetWindowTextW(hwnd, &text[0], length + 1);
    text.resize(static_cast<size_t>(length));
    return text;
}

// Создание дочернего элемента управления.
inline HWND createChild(HWND parent, const wchar_t *className, const wchar_t *text,
                        DWORD style, int x, int y, int w, int h,
                        int id = 0, DWORD exStyle = 0)
{
    return CreateWindowExW(exStyle, className, text,
                           WS_CHILD | WS_VISIBLE | style,
                           scale(x), scale(y), scale(w), scale(h),
                           parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
                           GetModuleHandleW(nullptr), nullptr);
}

// Установка шрифта всем дочерним окнам.
inline void applyFontToChildren(HWND parent, HFONT font)
{
    EnumChildWindows(parent, [](HWND child, LPARAM lParam) -> BOOL {
        SendMessageW(child, WM_SETFONT, static_cast<WPARAM>(lParam), TRUE);
        return TRUE;
    }, reinterpret_cast<LPARAM>(font));
}

const wchar_t kEmpty[] = L"—"; // длинное тире — «нет данных»

} // namespace ui

#endif // UIHELPERS_H
