#ifndef PRESSUREMODEL_H
#define PRESSUREMODEL_H

#include <string>
#include <vector>

// Данные, введённые пользователем в окне ввода.
// Хранятся строками: так модель может запомнить и вернуть в окно
// ровно то, что набрал пользователь, даже если это некорректный ввод.
struct PressureInput
{
    std::wstring day;        // день рождения
    std::wstring month;      // месяц рождения
    std::wstring year;       // год рождения
    std::wstring weight;     // вес, кг
    std::wstring systolic;   // верхнее (систолическое) давление, мм рт. ст.
    std::wstring diastolic;  // нижнее (диастолическое) давление, мм рт. ст.
};

// Оценка текущего давления.
enum class PressureLevel
{
    Unknown,  // расчёт не выполнялся
    Low,      // пониженное
    Normal,   // норма
    High      // повышенное
};

// Результат расчёта.
struct PressureResult
{
    bool          valid          = false;
    int           age            = 0;    // полных лет
    double        weight         = 0.0;  // кг
    int           idealSystolic  = 0;    // «идеальное» верхнее давление
    int           idealDiastolic = 0;    // «идеальное» нижнее давление
    int           systolic       = 0;    // текущее верхнее давление
    int           diastolic      = 0;    // текущее нижнее давление
    int           deltaSystolic  = 0;    // отклонение верхнего от идеального
    int           deltaDiastolic = 0;    // отклонение нижнего от идеального
    PressureLevel level          = PressureLevel::Unknown;
    std::wstring  verdict;               // «Повышенное» / «Пониженное» / «Норма»
    std::wstring  advice;                // рекомендация
};

// Календарная дата (модель не зависит от WinAPI, поэтому своя структура).
struct SimpleDate
{
    int day   = 1;
    int month = 1;
    int year  = 2000;
};

// Интерфейс наблюдателя (шаблон Observer).
// Его реализует любое представление, которое хочет узнавать
// об изменениях модели.
class IPressureObserver
{
public:
    virtual ~IPressureObserver() = default;

    virtual void onInputChanged(const PressureInput &input) = 0;
    virtual void onResultChanged(const PressureResult &result) = 0;
    virtual void onValidationFailed(const std::wstring &message) = 0;
};

// АКТИВНАЯ модель: хранит данные, проверяет их, выполняет расчёт и при любом
// изменении своего состояния сама рассылает уведомления подписчикам.
// О конкретных окнах и о контроллере модель ничего не знает.
class PressureModel
{
public:
    PressureModel() = default;

    // Подписка / отписка наблюдателей.
    void subscribe(IPressureObserver *observer);
    void unsubscribe(IPressureObserver *observer);

    const PressureInput  &input()  const { return m_input; }
    const PressureResult &result() const { return m_result; }

    // Единственная точка входа для контроллера.
    void submit(const PressureInput &input);

    // Текущая дата (по умолчанию — системная). Можно подменить для тестов.
    void setToday(const SimpleDate &today);
    SimpleDate today() const;

    // Формулы расчёта «идеального» давления.
    static int idealSystolic(int age, double weight);
    static int idealDiastolic(int age, double weight);

private:
    PressureResult calculate(int age, double weight,
                             int systolic, int diastolic) const;

    void notifyInputChanged();
    void notifyResultChanged();
    void notifyValidationFailed(const std::wstring &message);

    std::vector<IPressureObserver *> m_observers;

    PressureInput  m_input;
    PressureResult m_result;

    bool       m_hasFixedToday = false;
    SimpleDate m_fixedToday;
};

#endif // PRESSUREMODEL_H
