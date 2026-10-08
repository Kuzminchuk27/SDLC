#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <functional>

#include "model/pressuremodel.h"

// Главное окно. В MVC с активной моделью представление само подписывается
// на модель (реализует IPressureObserver) и обновляет себя при уведомлениях,
// без участия контроллера.
class MainWindow : public IPressureObserver
{
public:
    explicit MainWindow(PressureModel *model);
    ~MainWindow() override;

    MainWindow(const MainWindow &) = delete;
    MainWindow &operator=(const MainWindow &) = delete;

    bool create(HINSTANCE instance, int showCommand);
    HWND hwnd() const { return m_hwnd; }

    // Намерение пользователя «ввести данные». Обрабатывает контроллер.
    std::function<void()> onEnterDataRequested;

    // IPressureObserver
    void onInputChanged(const PressureInput &input) override;
    void onResultChanged(const PressureResult &result) override;
    void onValidationFailed(const std::wstring &message) override;

private:
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT handleMessage(UINT message, WPARAM wParam, LPARAM lParam);

    void createControls();
    void clearResult();

    PressureModel *m_model;
    HWND m_hwnd = nullptr;

    HFONT m_font        = nullptr;
    HFONT m_verdictFont = nullptr;

    // Последние введённые данные
    HWND m_birthDate = nullptr;
    HWND m_weight    = nullptr;
    HWND m_pressure  = nullptr;

    // Результат расчёта
    HWND m_age           = nullptr;
    HWND m_idealPressure = nullptr;
    HWND m_current       = nullptr;
    HWND m_deviation     = nullptr;
    HWND m_verdict       = nullptr;
    HWND m_advice        = nullptr;

    PressureLevel m_level = PressureLevel::Unknown;
};

#endif // MAINWINDOW_H
