// Модель системы «ПаркУчёт» на языке SysML v2 (текстовая нотация).
// Диаграммы: images/use-cases.png, images/exit-flow.png, images/session-states.png
package ParkUchet {
    private import ScalarValues::*;

    // Участники и система
    part def Operator;
    part def Administrator;

    part def ParkingSystem {
        attribute capacity : Integer;
        attribute hourlyRate : Real;
        attribute freeMinutes : Integer;
    }

    // Сессия стоянки
    item def ParkingSession {
        attribute plate : String;
        attribute brand : String;
        attribute entryTime : String;
        attribute exitTime : String;
        attribute cost : Real;
    }

    // Варианты использования (рисунок use-cases.png)
    use case def 'Регистрация въезда' {
        subject parking : ParkingSystem;
        actor operator : Operator;
        objective {
            doc /* Создать сессию с госномером, маркой и временем въезда. */
        }
    }

    use case def 'Расчёт стоимости' {
        subject parking : ParkingSystem;
        objective {
            doc /* Рассчитать стоимость стоянки по тарифу на момент въезда. */
        }
    }

    use case def 'Оформление выезда' {
        subject parking : ParkingSystem;
        actor operator : Operator;
        objective {
            doc /* Показать стоимость и завершить сессию после подтверждения. */
        }
        include use case calculate : 'Расчёт стоимости';
    }

    use case def 'Аннулирование записи' {
        subject parking : ParkingSystem;
        actor operator : Operator;
    }

    use case def 'Просмотр свободных мест' {
        subject parking : ParkingSystem;
        actor operator : Operator;
    }

    use case def 'Просмотр истории' {
        subject parking : ParkingSystem;
        actor operator : Operator;
    }

    use case def 'Настройка тарифа и вместимости' {
        subject parking : ParkingSystem;
        actor administrator : Administrator;
    }

    // Оформление выезда (рисунок exit-flow.png)
    action def 'Выбрать автомобиль';
    action def 'Указать время выезда';
    action def 'Показать ошибку времени';
    action def 'Рассчитать стоимость';
    action def 'Показать стоимость';
    action def 'Завершить сессию';
    action def 'Обновить свободные места';

    action def 'Оформить выезд' {
        action select : 'Выбрать автомобиль';
        action setTime : 'Указать время выезда';
        action showError : 'Показать ошибку времени';
        action calculate : 'Рассчитать стоимость';
        action show : 'Показать стоимость';
        action close : 'Завершить сессию';
        action update : 'Обновить свободные места';

        attribute timeValid : Boolean;
        attribute confirmed : Boolean;

        first start;
        then select;
        then setTime;
        then decide checkTime;
            if timeValid then calculate;
            else showError;
        first showError then setTime;
        first calculate then show;
        then decide checkConfirm;
            if confirmed then close;
            else done;
        first close then update;
        first update then done;
    }

    // Состояния сессии стоянки (рисунок session-states.png)
    attribute def EntryRegistered;
    attribute def ExitStarted;
    attribute def ExitCancelled;
    attribute def PaymentConfirmed;
    attribute def EntryAnnulled;

    state def 'Состояния сессии' {
        entry; then 'Новая';

        state 'Новая';
        accept EntryRegistered then 'На стоянке';

        state 'На стоянке';
        transition first 'На стоянке' accept ExitStarted then 'Расчёт стоимости';
        transition first 'На стоянке' accept EntryAnnulled then 'Аннулирована';

        state 'Расчёт стоимости';
        transition first 'Расчёт стоимости' accept ExitCancelled then 'На стоянке';
        transition first 'Расчёт стоимости' accept PaymentConfirmed then 'Завершена';

        state 'Завершена';
        state 'Аннулирована';
    }

    // Основные требования к расчёту и учёту мест (полный перечень в srs/srs.md)
    requirement def <'REQ-FUNC-005'> 'Запрет въезда при отсутствии мест' {
        doc /* При нуле свободных мест въезд отклоняется. */
    }

    requirement def <'REQ-FUNC-007'> 'Расчёт стоимости' {
        doc /* Стоимость рассчитывается по тарифу на момент въезда. */
    }

    requirement def <'REQ-FUNC-009'> 'Показ свободных мест' {
        doc /* Свободные места равны вместимости минус занятые места. */
    }
}
