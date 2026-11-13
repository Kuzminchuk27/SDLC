// Макеты окон «ПаркУчёт» на языке SysML v2.
// Окно и группа элементов – части (part), элементы окна – вложенные части;
// имя части – надпись, которую видит оператор, определение – вид элемента.
// Изображения: images/mockup-main.png, mockup-exit.png, mockup-history.png, mockup-settings.png
package ParkUchetUi {

    // Виды элементов интерфейса
    part def Window;
    part def Group;
    part def Label;
    part def TextField;
    part def Button;
    part def Table;

    // Рисунок mockup-main.png
    part 'Главное окно «ПаркУчёт»' : Window {
        part 'Места' : Group {
            part 'Всего мест' : Label;
            part 'Занято' : Label;
            part 'Свободно' : Label;
        }
        part 'Въезд автомобиля' : Group {
            part 'Госномер' : TextField;
            part 'Марка' : TextField;
            part 'Время въезда' : TextField;
            part 'Зарегистрировать въезд' : Button;
        }
        part 'Автомобили на стоянке' : Group {
            part 'Госномер, марка, въезд, длительность' : Table;
        }
        part 'Действия' : Group {
            part 'Оформить выезд' : Button;
            part 'История' : Button;
            part 'Настройки' : Button;
        }
        part 'Действующий тариф' : Label;
    }

    // Рисунок mockup-exit.png
    part 'Окно «Оформление выезда»' : Window {
        part 'Автомобиль' : Group {
            part 'Госномер' : Label;
            part 'Марка' : Label;
            part 'Время въезда' : Label;
        }
        part 'Выезд' : Group {
            part 'Время выезда' : TextField;
            part 'Длительность' : Label;
            part 'Тариф' : Label;
            part 'К оплате, BYN' : Label;
        }
        part 'Кнопки' : Group {
            part 'Оплата принята' : Button;
            part 'Отмена' : Button;
        }
    }

    // Рисунок mockup-history.png
    part 'Окно «История стоянок»' : Window {
        part 'Госномер, въезд, выезд, сумма' : Table;
        part 'Закрыть' : Button;
    }

    // Рисунок mockup-settings.png
    part 'Окно «Настройки»' : Window {
        part 'Параметры' : Group {
            part 'Вместимость, мест' : TextField;
            part 'Ставка за час, BYN' : TextField;
            part 'Льготный период, мин' : TextField;
        }
        part 'Кнопки' : Group {
            part 'Сохранить' : Button;
            part 'Отмена' : Button;
        }
    }
}
