#include "view/mainwindow.h"

#include "view/uihelpers.h"

namespace {

const wchar_t kClassName[] = L"PressureCalculatorMainWindow";
constexpr int kEnterButtonId = 101;

// Склонение слова «год» после числа: 1 год, 2 года, 5 лет.
std::wstring yearsWord(int n)
{
    const int mod100 = n % 100;
    const int mod10  = n % 10;
    if (mod100 >= 11 && mod100 <= 14) return L"лет";
    if (mod10 == 1)                   return L"год";
    if (mod10 >= 2 && mod10 <= 4)     return L"года";
    return L"лет";
}

// Число со знаком: +5, −3, 0.
std::wstring signedNumber(int value)
{
    if (value > 0) return L"+" + std::to_wstring(value);
    if (value < 0) return L"−" + std::to_wstring(-value);
    return L"0";
}

std::wstring pressureText(int systolic, int diastolic)
{
    return std::to_wstring(systolic) + L"/" + std::to_wstring(diastolic) + L" мм рт. ст.";
}

} // namespace

MainWindow::MainWindow(PressureModel *model)
    : m_model(model)
{
    // Подписка представления на активную модель.
    m_model->subscribe(this);
}

MainWindow::~MainWindow()
{
    m_model->unsubscribe(this);
    if (m_font)        DeleteObject(m_font);
    if (m_verdictFont) DeleteObject(m_verdictFont);
}

bool MainWindow::create(HINSTANCE instance, int showCommand)
{
    WNDCLASSEXW wc{};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = &MainWindow::windowProc;
    wc.hInstance     = instance;
    wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
    wc.hIcon         = LoadIconW(nullptr, IDI_APPLICATION);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = kClassName;
    RegisterClassExW(&wc);

    const DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    RECT rect{ 0, 0, ui::scale(460), ui::scale(398) };
    AdjustWindowRect(&rect, style, FALSE);

    m_hwnd = CreateWindowExW(0, kClassName, L"Нормальное артериальное давление", style,
                             CW_USEDEFAULT, CW_USEDEFAULT,
                             rect.right - rect.left, rect.bottom - rect.top,
                             nullptr, nullptr, instance, this);
    if (!m_hwnd)
        return false;

    ShowWindow(m_hwnd, showCommand);
    UpdateWindow(m_hwnd);
    return true;
}

void MainWindow::createControls()
{
    m_font        = ui::createUiFont();
    m_verdictFont = ui::createUiFont(true, 1.35);

    ui::createChild(m_hwnd, L"BUTTON", L"Ввести данные",
                    WS_TABSTOP | BS_PUSHBUTTON, 12, 12, 436, 32, kEnterButtonId);

    // Группа «Последние введённые данные»
    ui::createChild(m_hwnd, L"BUTTON", L"Последние введённые данные",
                    BS_GROUPBOX, 12, 54, 436, 104);
    ui::createChild(m_hwnd, L"STATIC", L"Дата рождения:",  0, 28, 78, 200, 20);
    ui::createChild(m_hwnd, L"STATIC", L"Вес:",            0, 28, 102, 200, 20);
    ui::createChild(m_hwnd, L"STATIC", L"Текущее давление:", 0, 28, 126, 200, 20);
    m_birthDate = ui::createChild(m_hwnd, L"STATIC", ui::kEmpty, 0, 236, 78, 200, 20);
    m_weight    = ui::createChild(m_hwnd, L"STATIC", ui::kEmpty, 0, 236, 102, 200, 20);
    m_pressure  = ui::createChild(m_hwnd, L"STATIC", ui::kEmpty, 0, 236, 126, 200, 20);

    // Группа «Результат»
    ui::createChild(m_hwnd, L"BUTTON", L"Результат",
                    BS_GROUPBOX, 12, 168, 436, 218);
    ui::createChild(m_hwnd, L"STATIC", L"Возраст:",                  0, 28, 192, 200, 20);
    ui::createChild(m_hwnd, L"STATIC", L"Идеальное давление:",       0, 28, 216, 200, 20);
    ui::createChild(m_hwnd, L"STATIC", L"Текущее давление:",         0, 28, 240, 200, 20);
    ui::createChild(m_hwnd, L"STATIC", L"Отклонение от идеального:", 0, 28, 264, 200, 20);
    ui::createChild(m_hwnd, L"STATIC", L"Оценка давления:",          0, 28, 292, 200, 20);
    m_age           = ui::createChild(m_hwnd, L"STATIC", ui::kEmpty, 0, 236, 192, 200, 20);
    m_idealPressure = ui::createChild(m_hwnd, L"STATIC", ui::kEmpty, 0, 236, 216, 200, 20);
    m_current       = ui::createChild(m_hwnd, L"STATIC", ui::kEmpty, 0, 236, 240, 200, 20);
    m_deviation     = ui::createChild(m_hwnd, L"STATIC", ui::kEmpty, 0, 236, 264, 200, 20);
    m_verdict       = ui::createChild(m_hwnd, L"STATIC", ui::kEmpty, 0, 236, 288, 200, 26);
    m_advice        = ui::createChild(m_hwnd, L"STATIC", L"", 0, 28, 324, 404, 54);

    ui::applyFontToChildren(m_hwnd, m_font);
    SendMessageW(m_verdict, WM_SETFONT, reinterpret_cast<WPARAM>(m_verdictFont), TRUE);
}

// ---------------------------------------------------------------------------
// Обработчики уведомлений активной модели
// ---------------------------------------------------------------------------

void MainWindow::onInputChanged(const PressureInput &input)
{
    SetWindowTextW(m_birthDate, (input.day + L"." + input.month + L"." + input.year).c_str());
    SetWindowTextW(m_weight,    (input.weight + L" кг").c_str());
    SetWindowTextW(m_pressure,  (input.systolic + L"/" + input.diastolic + L" мм рт. ст.").c_str());
}

void MainWindow::onResultChanged(const PressureResult &result)
{
    if (!result.valid) {
        // Прошлый результат перестал соответствовать введённым данным.
        clearResult();
        return;
    }

    m_level = result.level;
    SetWindowTextW(m_age, (std::to_wstring(result.age) + L" " + yearsWord(result.age)).c_str());
    SetWindowTextW(m_idealPressure, pressureText(result.idealSystolic, result.idealDiastolic).c_str());
    SetWindowTextW(m_current, pressureText(result.systolic, result.diastolic).c_str());
    SetWindowTextW(m_deviation, (signedNumber(result.deltaSystolic) + L" / "
                                 + signedNumber(result.deltaDiastolic)).c_str());
    SetWindowTextW(m_verdict, result.verdict.c_str());
    SetWindowTextW(m_advice, result.advice.c_str());
    InvalidateRect(m_verdict, nullptr, TRUE);
}

void MainWindow::onValidationFailed(const std::wstring &message)
{
    MessageBoxW(m_hwnd, message.c_str(), L"Некорректные данные", MB_OK | MB_ICONWARNING);
}

void MainWindow::clearResult()
{
    m_level = PressureLevel::Unknown;
    for (HWND label : { m_age, m_idealPressure, m_current, m_deviation, m_verdict })
        SetWindowTextW(label, ui::kEmpty);
    SetWindowTextW(m_advice, L"");
}

// ---------------------------------------------------------------------------
// Оконная процедура
// ---------------------------------------------------------------------------

LRESULT CALLBACK MainWindow::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    MainWindow *self = nullptr;
    if (message == WM_NCCREATE) {
        auto *cs = reinterpret_cast<CREATESTRUCTW *>(lParam);
        self = static_cast<MainWindow *>(cs->lpCreateParams);
        self->m_hwnd = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<MainWindow *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self)
        return self->handleMessage(message, wParam, lParam);
    return DefWindowProcW(hwnd, message, wParam, lParam);
}

LRESULT MainWindow::handleMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_CREATE:
        createControls();
        return 0;

    case WM_COMMAND:
        if (LOWORD(wParam) == kEnterButtonId && HIWORD(wParam) == BN_CLICKED) {
            if (onEnterDataRequested)
                onEnterDataRequested();
            return 0;
        }
        break;

    case WM_CTLCOLORSTATIC: {
        // Белый фон для меток; цвет оценки зависит от уровня давления.
        HDC dc = reinterpret_cast<HDC>(wParam);
        SetBkMode(dc, TRANSPARENT);
        if (reinterpret_cast<HWND>(lParam) == m_verdict) {
            switch (m_level) {
            case PressureLevel::High:   SetTextColor(dc, RGB(196, 30, 30));  break;
            case PressureLevel::Low:    SetTextColor(dc, RGB(25, 90, 200));  break;
            case PressureLevel::Normal: SetTextColor(dc, RGB(20, 135, 60));  break;
            default: break;
            }
        }
        return reinterpret_cast<LRESULT>(GetSysColorBrush(COLOR_WINDOW));
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(m_hwnd, message, wParam, lParam);
}
