# Модели «ПаркУчёт» на SysML v2

| Файл | Содержание |
|---|---|
| `parking-system.sysml` | Варианты использования, действие «Оформить выезд», состояния сессии, основные требования |
| `ui-mockups.sysml` | Макеты четырёх окон: окно и группы – части, элементы окна – вложенные части |
| `parkuchet.ipynb` | Ноутбук для ядра Jupyter SysML: загрузка моделей и команды `%viz` для рисунков 1 – 7 |
| `viz/jobs.txt` | Перечень диаграмм: элемент модели, вид, стиль, имя файла в `../images` |
| `viz/Viz.java` | Запуск той же визуализации без Jupyter, через API эталонной реализации |

Модели проверены и диаграммы построены эталонной реализацией SysML v2 Pilot Implementation,
выпуск 2026-09.1, ядро Jupyter SysML 0.63.1 (https://github.com/Systems-Modeling/SysML-v2-Pilot-Implementation).
Визуализация `%viz` выдаёт SVG, файлы PNG в `../images` получены из него без изменений.

## Построение диаграмм в Jupyter

Установить ядро по инструкции `install/jupyter` выпуска 2026-09.1, открыть `parkuchet.ipynb`
и выполнить все ячейки.

## Построение диаграмм без Jupyter

Нужны Java 21 и распакованный архив `jupyter-sysml-kernel-0.63.1.zip` из выпуска 2026-09.1:

    java -cp "<kernel>/sysml/*" viz/Viz.java "<kernel>/sysml/sysml.library/" <каталог-SVG> parking-system.sysml ui-mockups.sysml -- @viz/jobs.txt

Путь к библиотеке указывается абсолютным, с прямыми косыми чертами.
