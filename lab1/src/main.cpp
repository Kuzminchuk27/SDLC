#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <commctrl.h>

#include "controller/pressurecontroller.h"
#include "model/pressuremodel.h"
#include "view/mainwindow.h"

#ifdef _MSC_VER
#pragma comment(lib, "comctl32.lib")
#endif

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand)
{
    // Подключение современных элементов управления Windows (Common Controls 6).
    INITCOMMONCONTROLSEX icc{ sizeof(icc), ICC_STANDARD_CLASSES };
    InitCommonControlsEx(&icc);

    PressureModel model;
    MainWindow window(&model);
    if (!window.create(instance, showCommand))
        return 1;

    PressureController controller(&model, window.hwnd());

    // Единственная связь View -> Controller: намерение пользователя.
    window.onEnterDataRequested = [&controller] { controller.onEnterDataRequested(); };

    // Цикл обработки сообщений.
    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        if (!IsDialogMessageW(window.hwnd(), &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    return static_cast<int>(msg.wParam);
}
