#include "view/inputdialog.h"

#include <commctrl.h>

#include "view/uihelpers.h"

namespace {

const wchar_t kClassName[] = L"PressureCalculatorInputDialog";

HWND createEdit(HWND parent, int x, int y, int w, int maxLength, const wchar_t *cue)
{
    HWND edit = ui::createChild(parent, L"EDIT", L"",
                                WS_TABSTOP | ES_AUTOHSCROLL, x, y, w, 23, 0, WS_EX_CLIENTEDGE);
    SendMessageW(edit, EM_SETLIMITTEXT, static_cast<WPARAM>(maxLength), 0);
    if (cue)
        SendMessageW(edit, EM_SETCUEBANNER, FALSE, reinterpret_cast<LPARAM>(cue));
    return edit;
}

} // namespace

InputDialog::InputDialog(HWND owner)
    : m_owner(owner)
{
}

void InputDialog::setInput(const PressureInput &input)
{
    m_input = input;
}

bool InputDialog::exec()
{
    static bool registered = false;
    if (!registered) {
        WNDCLASSEXW wc{};
        wc.cbSize        = sizeof(wc);
        wc.lpfnWndProc   = &InputDialog::windowProc;
        wc.hInstance     = GetModuleHandleW(nullptr);
        wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wc.lpszClassName = kClassName;
        RegisterClassExW(&wc);
        registered = true;
    }

    const DWORD style   = WS_POPUP | WS_CAPTION | WS_SYSMENU;
    const DWORD exStyle = WS_EX_DLGMODALFRAME | WS_EX_CONTROLPARENT;
    RECT rect{ 0, 0, ui::scale(426), ui::scale(220) };
    AdjustWindowRectEx(&rect, style, FALSE, exStyle);
    const int width  = rect.right - rect.left;
    const int height = rect.bottom - rect.top;

    // Окно размещается по центру главного окна.
    RECT ownerRect{};
    GetWindowRect(m_owner, &ownerRect);
    const int x = ownerRect.left + (ownerRect.right - ownerRect.left - width) / 2;
    const int y = ownerRect.top + (ownerRect.bottom - ownerRect.top - height) / 2;

    m_accepted = false;
    m_finished = false;
    m_hwnd = CreateWindowExW(exStyle, kClassName, L"Ввод данных", style,
                             x, y, width, height, m_owner, nullptr,
                             GetModuleHandleW(nullptr), this);
    if (!m_hwnd)
        return false;

    // Модальность: главное окно блокируется, пока открыт диалог.
    EnableWindow(m_owner, FALSE);
    ShowWindow(m_hwnd, SW_SHOW);
    SetFocus(m_day);
    SendMessageW(m_day, EM_SETSEL, 0, -1);

    MSG msg{};
    while (!m_finished) {
        const BOOL got = GetMessageW(&msg, nullptr, 0, 0);
        if (got == 0) {                       // пришёл WM_QUIT — вернуть его в очередь
            PostQuitMessage(static_cast<int>(msg.wParam));
            break;
        }
        if (got == -1)
            break;
        if (!IsDialogMessageW(m_hwnd, &msg)) { // Tab, Enter, Esc как в обычном диалоге
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    EnableWindow(m_owner, TRUE);
    SetActiveWindow(m_owner);
    DestroyWindow(m_hwnd);
    m_hwnd = nullptr;
    if (m_font) {
        DeleteObject(m_font);
        m_font = nullptr;
    }
    return m_accepted;
}

void InputDialog::createControls()
{
    m_font = ui::createUiFont();

    ui::createChild(m_hwnd, L"STATIC", L"Дата рождения (дд.мм.гггг):", 0, 16, 19, 228, 20);
    m_day   = createEdit(m_hwnd, 250, 16, 40, 2, L"дд");
    ui::createChild(m_hwnd, L"STATIC", L".", SS_CENTER, 290, 19, 12, 20);
    m_month = createEdit(m_hwnd, 302, 16, 40, 2, L"мм");
    ui::createChild(m_hwnd, L"STATIC", L".", SS_CENTER, 342, 19, 12, 20);
    m_year  = createEdit(m_hwnd, 354, 16, 56, 4, L"гггг");

    ui::createChild(m_hwnd, L"STATIC", L"Вес, кг:", 0, 16, 51, 228, 20);
    m_weight = createEdit(m_hwnd, 250, 48, 160, 6, L"например, 75");

    ui::createChild(m_hwnd, L"STATIC", L"Верхнее (систолическое) давление:", 0, 16, 83, 228, 20);
    m_systolic = createEdit(m_hwnd, 250, 80, 160, 3, L"например, 120");

    ui::createChild(m_hwnd, L"STATIC", L"Нижнее (диастолическое) давление:", 0, 16, 115, 228, 20);
    m_diastolic = createEdit(m_hwnd, 250, 112, 160, 3, L"например, 80");

    ui::createChild(m_hwnd, L"STATIC", L"Давление указывается в мм рт. ст.", 0, 16, 148, 394, 20);

    ui::createChild(m_hwnd, L"BUTTON", L"Рассчитать",
                    WS_TABSTOP | BS_DEFPUSHBUTTON, 226, 178, 90, 28, IDOK);
    ui::createChild(m_hwnd, L"BUTTON", L"Отмена",
                    WS_TABSTOP | BS_PUSHBUTTON, 322, 178, 88, 28, IDCANCEL);

    ui::applyFontToChildren(m_hwnd, m_font);

    // Восстановление последних введённых данных.
    SetWindowTextW(m_day,       m_input.day.c_str());
    SetWindowTextW(m_month,     m_input.month.c_str());
    SetWindowTextW(m_year,      m_input.year.c_str());
    SetWindowTextW(m_weight,    m_input.weight.c_str());
    SetWindowTextW(m_systolic,  m_input.systolic.c_str());
    SetWindowTextW(m_diastolic, m_input.diastolic.c_str());
}

void InputDialog::readInput()
{
    m_input.day       = ui::windowText(m_day);
    m_input.month     = ui::windowText(m_month);
    m_input.year      = ui::windowText(m_year);
    m_input.weight    = ui::windowText(m_weight);
    m_input.systolic  = ui::windowText(m_systolic);
    m_input.diastolic = ui::windowText(m_diastolic);
}

LRESULT CALLBACK InputDialog::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    InputDialog *self = nullptr;
    if (message == WM_NCCREATE) {
        auto *cs = reinterpret_cast<CREATESTRUCTW *>(lParam);
        self = static_cast<InputDialog *>(cs->lpCreateParams);
        self->m_hwnd = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<InputDialog *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self && self->m_hwnd == hwnd)
        return self->handleMessage(message, wParam, lParam);
    return DefWindowProcW(hwnd, message, wParam, lParam);
}

LRESULT InputDialog::handleMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_CREATE:
        createControls();
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:          // «Рассчитать» или Enter
            readInput();
            m_accepted = true;
            m_finished = true;
            return 0;
        case IDCANCEL:      // «Отмена» или Esc
            m_finished = true;
            return 0;
        }
        break;

    case WM_CLOSE:          // крестик в заголовке
        m_finished = true;
        return 0;

    case WM_CTLCOLORSTATIC: {
        HDC dc = reinterpret_cast<HDC>(wParam);
        SetBkMode(dc, TRANSPARENT);
        return reinterpret_cast<LRESULT>(GetSysColorBrush(COLOR_WINDOW));
    }
    }
    return DefWindowProcW(m_hwnd, message, wParam, lParam);
}
