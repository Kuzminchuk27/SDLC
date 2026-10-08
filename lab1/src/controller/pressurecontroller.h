#ifndef PRESSURECONTROLLER_H
#define PRESSURECONTROLLER_H

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

class PressureModel;

// Контроллер: принимает намерения пользователя, показывает окно ввода
// и передаёт собранные данные в модель. Результат не отображает —
// это делает представление по уведомлениям активной модели.
class PressureController
{
public:
    PressureController(PressureModel *model, HWND dialogOwner);

    void onEnterDataRequested();

private:
    PressureModel *m_model;
    HWND           m_dialogOwner;
};

#endif // PRESSURECONTROLLER_H
