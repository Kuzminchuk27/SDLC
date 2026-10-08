#ifndef INPUTDIALOG_H
#define INPUTDIALOG_H

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include "model/pressuremodel.h"

// Отдельное модальное окно ввода данных.
// Ничего не вычисляет и не проверяет — только собирает введённый текст.
class InputDialog
{
public:
    explicit InputDialog(HWND owner);

    InputDialog(const InputDialog &) = delete;
    InputDialog &operator=(const InputDialog &) = delete;

    void setInput(const PressureInput &input); // восстановление прошлого ввода
    PressureInput input() const { return m_input; } // выдача введённого контроллеру

    // Показывает окно модально. true — пользователь нажал «Рассчитать».
    bool exec();

private:
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT handleMessage(UINT message, WPARAM wParam, LPARAM lParam);

    void createControls();
    void readInput();

    HWND m_owner;
    HWND m_hwnd = nullptr;
    HFONT m_font = nullptr;

    // Дата рождения вводится тремя отдельными полями:
    // стандартный элемент ввода даты по условию использовать нельзя.
    HWND m_day       = nullptr;
    HWND m_month     = nullptr;
    HWND m_year      = nullptr;
    HWND m_weight    = nullptr;
    HWND m_systolic  = nullptr;
    HWND m_diastolic = nullptr;

    PressureInput m_input;
    bool m_accepted = false;
    bool m_finished = false;
};

#endif // INPUTDIALOG_H
