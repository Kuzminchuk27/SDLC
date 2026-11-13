// Модель системы «ПаркУчёт» на языке SysML v2 (текстовая нотация).
// Диаграммы: images/use-cases.png, images/exit-flow.png, images/session-states.png
package ParkUchet {
    private import ScalarValues::*;

    // Участник и система
    part def Operator;

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
    use case 'Работа оператора' {
        subject 'ПаркУчёт' : ParkingSystem;
        actor 'Оператор' : Operator;

        include use case 'Регистрация въезда';
        include use case 'Оформление выезда' {
            include use case 'Расчёт стоимости';
        }
        include use case 'Просмотр свободных мест';
        include use case 'Просмотр истории';
        include use case 'Настройка тарифа и вместимости';
    }

    // Оформление выезда (рисунок exit-flow.png)
    action def 'Оформить выезд' {
        attribute timeValid : Boolean;
        attribute confirmed : Boolean;

        first start;
        then action 'Выбрать автомобиль';
        then merge 'Ввод времени';
        then action 'Указать время выезда';
        then decide 'Проверка времени';
            if timeValid then 'Рассчитать стоимость';
            else 'Показать ошибку времени';

        action 'Показать ошибку времени';
        then 'Ввод времени';

        action 'Рассчитать стоимость';
        then action 'Показать стоимость';
        then decide 'Проверка оплаты';
            if confirmed then 'Завершить сессию';
            else done;

        action 'Завершить сессию';
        then action 'Обновить свободные места';
        then done;
    }

    // Состояния сессии стоянки (рисунок session-states.png)
    attribute def 'Въезд';
    attribute def 'Начат выезд';
    attribute def 'Отмена';
    attribute def 'Оплата принята';

    state def 'Состояния сессии' {
        entry; then 'Новая';

        state 'Новая';
        accept 'Въезд' then 'На стоянке';

        state 'На стоянке';
        accept 'Начат выезд' then 'Расчёт стоимости';

        state 'Расчёт стоимости';
        transition first 'Расчёт стоимости' accept 'Отмена' then 'На стоянке';
        transition first 'Расчёт стоимости' accept 'Оплата принята' then 'Завершена';

        state 'Завершена';
    }

    // Основные требования к расчёту и учёту мест (полный перечень в srs/srs.md)
    requirement def <'REQ-FUNC-005'> 'Запрет въезда при отсутствии мест' {
        doc /* При нуле свободных мест въезд отклоняется. */
    }

    requirement def <'REQ-FUNC-007'> 'Правило расчёта стоимости' {
        doc /* Стоимость рассчитывается по тарифу на момент въезда. */
    }

    requirement def <'REQ-FUNC-009'> 'Показ свободных мест' {
        doc /* Свободные места равны вместимости минус занятые места. */
    }
}
