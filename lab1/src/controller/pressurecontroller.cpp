#include "controller/pressurecontroller.h"

#include "model/pressuremodel.h"
#include "view/inputdialog.h"

PressureController::PressureController(PressureModel *model, HWND dialogOwner)
    : m_model(model)
    , m_dialogOwner(dialogOwner)
{
}

void PressureController::onEnterDataRequested()
{
    InputDialog dialog(m_dialogOwner);

    // Восстановление последних введённых данных из модели.
    dialog.setInput(m_model->input());

    if (!dialog.exec())
        return; // пользователь нажал «Отмена»

    m_model->submit(dialog.input());
}
