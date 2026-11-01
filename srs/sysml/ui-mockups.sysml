// Макеты окон «ПаркУчёт» на языке SysML v2: окно – часть (part), элемент окна – вложенная часть.
// Изображения: images/mockup-main.png, mockup-exit.png, mockup-history.png, mockup-settings.png
package ParkUchetUi {
    private import ScalarValues::*;

    // Элементы интерфейса
    part def Window { attribute title : String; }
    part def Label { attribute text : String; }
    part def TextField { attribute caption : String; }
    part def Button { attribute caption : String; }
    part def Grid { attribute columns : String[1..*] ordered; }

    // Рисунок mockup-main.png
    part mainWindow : Window {
        :>> title = "ПаркУчёт";
        part capacityInfo : Label { :>> text = "Всего мест"; }
        part occupiedInfo : Label { :>> text = "Занято"; }
        part freeInfo : Label { :>> text = "Свободно"; }
        part plateField : TextField { :>> caption = "Госномер"; }
        part brandField : TextField { :>> caption = "Марка"; }
        part timeField : TextField { :>> caption = "Время въезда"; }
        part entryButton : Button { :>> caption = "Зарегистрировать въезд"; }
        part searchField : TextField { :>> caption = "Поиск"; }
        part parkedGrid : Grid {
            :>> columns = ("Госномер", "Марка", "Въезд", "Длительность");
        }
        part exitButton : Button { :>> caption = "Оформить выезд"; }
        part annulButton : Button { :>> caption = "Аннулировать"; }
        part historyButton : Button { :>> caption = "История"; }
        part settingsButton : Button { :>> caption = "Настройки"; }
    }

    // Рисунок mockup-exit.png
    part exitWindow : Window {
        :>> title = "Оформление выезда";
        part plateInfo : Label { :>> text = "Госномер"; }
        part brandInfo : Label { :>> text = "Марка"; }
        part entryInfo : Label { :>> text = "Въезд"; }
        part exitField : TextField { :>> caption = "Выезд"; }
        part durationInfo : Label { :>> text = "Длительность"; }
        part tariffInfo : Label { :>> text = "Тариф"; }
        part costInfo : Label { :>> text = "К оплате"; }
        part confirmButton : Button { :>> caption = "Оплата принята"; }
        part cancelButton : Button { :>> caption = "Отмена"; }
    }

    // Рисунок mockup-history.png
    part historyWindow : Window {
        :>> title = "История стоянок";
        part fromField : TextField { :>> caption = "С"; }
        part toField : TextField { :>> caption = "По"; }
        part plateFilter : TextField { :>> caption = "Номер"; }
        part findButton : Button { :>> caption = "Найти"; }
        part historyGrid : Grid {
            :>> columns = ("Госномер", "Въезд", "Выезд", "Сумма", "Статус");
        }
        part countInfo : Label { :>> text = "Записей"; }
        part totalInfo : Label { :>> text = "Итого"; }
        part exportButton : Button { :>> caption = "Выгрузить в CSV"; }
        part closeButton : Button { :>> caption = "Закрыть"; }
    }

    // Рисунок mockup-settings.png
    part settingsWindow : Window {
        :>> title = "Настройки";
        part capacityField : TextField { :>> caption = "Вместимость, мест"; }
        part rateField : TextField { :>> caption = "Ставка за час, BYN"; }
        part freeField : TextField { :>> caption = "Льготный период, мин"; }
        part saveButton : Button { :>> caption = "Сохранить"; }
        part cancelButton : Button { :>> caption = "Отмена"; }
    }
}
