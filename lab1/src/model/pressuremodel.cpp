#include "model/pressuremodel.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <cwctype>

namespace {

// Ограничения на вводимые значения.
constexpr int    kMinYear      = 1900;
constexpr int    kMinAge       = 18;    // формула рассчитана на взрослых
constexpr int    kMaxAge       = 120;
constexpr double kMinWeight    = 30.0;
constexpr double kMaxWeight    = 300.0;
constexpr int    kMinSystolic  = 60;
constexpr int    kMaxSystolic  = 260;
constexpr int    kMinDiastolic = 30;
constexpr int    kMaxDiastolic = 160;

// Допустимое отклонение текущего давления от «идеального», мм рт. ст.
constexpr int kSystolicTolerance  = 15;
constexpr int kDiastolicTolerance = 10;

// Общепринятые границы гипертонии и гипотонии, мм рт. ст.
constexpr int kHighSystolic  = 140;
constexpr int kHighDiastolic = 90;
constexpr int kLowSystolic   = 90;
constexpr int kLowDiastolic  = 60;

// Удаление пробелов по краям строки.
std::wstring trimmed(const std::wstring &text)
{
    const auto begin = std::find_if_not(text.begin(), text.end(),
                                        [](wchar_t c) { return std::iswspace(c); });
    const auto end = std::find_if_not(text.rbegin(), text.rend(),
                                      [](wchar_t c) { return std::iswspace(c); }).base();
    return begin < end ? std::wstring(begin, end) : std::wstring();
}

// Разбор целого числа с проверкой диапазона.
bool parseInt(const std::wstring &text, int minValue, int maxValue, int &out)
{
    const std::wstring s = trimmed(text);
    if (s.empty() || s.size() > 9)
        return false;
    for (wchar_t c : s)
        if (c < L'0' || c > L'9')
            return false;

    const long value = std::wcstol(s.c_str(), nullptr, 10);
    if (value < minValue || value > maxValue)
        return false;
    out = static_cast<int>(value);
    return true;
}

// Разбор вещественного числа (запятая допускается как разделитель).
bool parseDouble(const std::wstring &text, double minValue, double maxValue, double &out)
{
    std::wstring s = trimmed(text);
    std::replace(s.begin(), s.end(), L',', L'.');
    if (s.empty())
        return false;

    wchar_t *end = nullptr;
    const double value = std::wcstod(s.c_str(), &end);
    if (end == s.c_str() || *end != L'\0' || !std::isfinite(value))
        return false;
    if (value < minValue || value > maxValue)
        return false;
    out = value;
    return true;
}

bool isLeapYear(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

// Количество дней в месяце.
// В феврале 28 дней, и только в високосный год — 29.
int daysInMonth(int month, int year)
{
    switch (month) {
    case 2:
        return isLeapYear(year) ? 29 : 28;
    case 4: case 6: case 9: case 11:
        return 30;
    default:
        return 31;
    }
}

// Сравнение дат: <0, 0, >0.
int compareDates(const SimpleDate &a, const SimpleDate &b)
{
    if (a.year != b.year)   return a.year  - b.year;
    if (a.month != b.month) return a.month - b.month;
    return a.day - b.day;
}

// Количество полных лет на дату today.
int fullYears(const SimpleDate &birth, const SimpleDate &today)
{
    int age = today.year - birth.year;
    if (today.month < birth.month
        || (today.month == birth.month && today.day < birth.day))
        --age;
    return age;
}

} // namespace

// ---------------------------------------------------------------------------
// Подписка наблюдателей
// ---------------------------------------------------------------------------

void PressureModel::subscribe(IPressureObserver *observer)
{
    if (observer && std::find(m_observers.begin(), m_observers.end(), observer) == m_observers.end())
        m_observers.push_back(observer);
}

void PressureModel::unsubscribe(IPressureObserver *observer)
{
    m_observers.erase(std::remove(m_observers.begin(), m_observers.end(), observer),
                      m_observers.end());
}

void PressureModel::notifyInputChanged()
{
    const auto observers = m_observers; // копия: подписчик может отписаться в обработчике
    for (IPressureObserver *o : observers)
        o->onInputChanged(m_input);
}

void PressureModel::notifyResultChanged()
{
    const auto observers = m_observers;
    for (IPressureObserver *o : observers)
        o->onResultChanged(m_result);
}

void PressureModel::notifyValidationFailed(const std::wstring &message)
{
    const auto observers = m_observers;
    for (IPressureObserver *o : observers)
        o->onValidationFailed(message);
}

// ---------------------------------------------------------------------------
// Текущая дата
// ---------------------------------------------------------------------------

void PressureModel::setToday(const SimpleDate &today)
{
    m_fixedToday = today;
    m_hasFixedToday = true;
}

SimpleDate PressureModel::today() const
{
    if (m_hasFixedToday)
        return m_fixedToday;

    const std::time_t now = std::time(nullptr);
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif
    return SimpleDate{ local.tm_mday, local.tm_mon + 1, local.tm_year + 1900 };
}

// ---------------------------------------------------------------------------
// Формулы
// ---------------------------------------------------------------------------

// Верхнее (систолическое): 109 + 0,5 * возраст + 0,1 * вес
int PressureModel::idealSystolic(int age, double weight)
{
    return static_cast<int>(std::lround(109.0 + 0.5 * age + 0.1 * weight));
}

// Нижнее (диастолическое): 63 + 0,1 * возраст + 0,15 * вес
int PressureModel::idealDiastolic(int age, double weight)
{
    return static_cast<int>(std::lround(63.0 + 0.1 * age + 0.15 * weight));
}

// ---------------------------------------------------------------------------
// Приём данных от контроллера
// ---------------------------------------------------------------------------

void PressureModel::submit(const PressureInput &input)
{
    // 1. Введённые данные запоминаются в любом случае, даже некорректные:
    //    при повторном вводе окно будет ими заполнено.
    m_input = input;
    notifyInputChanged();

    // 2. Проверка корректности. При ошибке прежний результат сбрасывается,
    //    подписчики получают пустой результат и сообщение об ошибке.
    auto fail = [this](const std::wstring &message) {
        m_result = PressureResult{};
        notifyResultChanged();
        notifyValidationFailed(message);
    };

    const SimpleDate now = today();
    int day = 0, month = 0, year = 0;

    // Любая ошибка в дне или месяце, а также несуществующая дата
    // (например, 30.02 или 29.02 в невисокосный год) — одно сообщение.
    const std::wstring kNoSuchDate = L"Такой даты не существует.";

    if (!parseInt(input.day, 1, 31, day)) {
        fail(kNoSuchDate);
        return;
    }
    if (!parseInt(input.month, 1, 12, month)) {
        fail(kNoSuchDate);
        return;
    }
    if (!parseInt(input.year, kMinYear, now.year, year)) {
        fail(L"Год рождения должен быть целым числом от " + std::to_wstring(kMinYear)
             + L" до " + std::to_wstring(now.year) + L".");
        return;
    }
    if (day > daysInMonth(month, year)) {
        fail(kNoSuchDate);
        return;
    }

    const SimpleDate birth{ day, month, year };
    if (compareDates(birth, now) > 0) {
        fail(L"Дата рождения не может быть в будущем.");
        return;
    }

    const int age = fullYears(birth, now);
    if (age < kMinAge || age > kMaxAge) {
        fail(L"Расчёт выполняется для взрослых: возраст должен быть от "
             + std::to_wstring(kMinAge) + L" до " + std::to_wstring(kMaxAge)
             + L" лет (сейчас " + std::to_wstring(age) + L").");
        return;
    }

    double weight = 0.0;
    if (!parseDouble(input.weight, kMinWeight, kMaxWeight, weight)) {
        fail(L"Вес должен быть числом от 30 до 300 кг.");
        return;
    }

    int systolic = 0, diastolic = 0;
    if (!parseInt(input.systolic, kMinSystolic, kMaxSystolic, systolic)) {
        fail(L"Верхнее давление должно быть целым числом от "
             + std::to_wstring(kMinSystolic) + L" до " + std::to_wstring(kMaxSystolic)
             + L" мм рт. ст.");
        return;
    }
    if (!parseInt(input.diastolic, kMinDiastolic, kMaxDiastolic, diastolic)) {
        fail(L"Нижнее давление должно быть целым числом от "
             + std::to_wstring(kMinDiastolic) + L" до " + std::to_wstring(kMaxDiastolic)
             + L" мм рт. ст.");
        return;
    }
    if (systolic <= diastolic) {
        fail(L"Верхнее давление должно быть больше нижнего.");
        return;
    }

    // 3. Расчёт и уведомление подписчиков.
    m_result = calculate(age, weight, systolic, diastolic);
    notifyResultChanged();
}

// ---------------------------------------------------------------------------
// Расчёт
// ---------------------------------------------------------------------------

PressureResult PressureModel::calculate(int age, double weight,
                                        int systolic, int diastolic) const
{
    PressureResult r;
    r.valid  = true;
    r.age    = age;
    r.weight = weight;

    // Шаг 1. «Идеальное» давление для возраста и веса.
    r.idealSystolic  = idealSystolic(age, weight);
    r.idealDiastolic = idealDiastolic(age, weight);

    // Шаг 2. Отклонение текущего давления от идеального.
    r.systolic       = systolic;
    r.diastolic      = diastolic;
    r.deltaSystolic  = systolic  - r.idealSystolic;
    r.deltaDiastolic = diastolic - r.idealDiastolic;

    // Шаг 3. Оценка. Повышенное давление проверяется первым,
    // так как оно опаснее (например, при изолированной систолической гипертонии).
    const bool high = systolic >= kHighSystolic || diastolic >= kHighDiastolic
                   || r.deltaSystolic  >  kSystolicTolerance
                   || r.deltaDiastolic >  kDiastolicTolerance;
    const bool low  = systolic <  kLowSystolic  || diastolic <  kLowDiastolic
                   || r.deltaSystolic  < -kSystolicTolerance
                   || r.deltaDiastolic < -kDiastolicTolerance;

    if (high) {
        r.level   = PressureLevel::High;
        r.verdict = L"Повышенное";
        r.advice  = L"Давление выше нормы. Отдохните 10–15 минут и повторите измерение. "
                    L"Ограничьте соль и кофе. При стойком повышении обратитесь к врачу.";
    } else if (low) {
        r.level   = PressureLevel::Low;
        r.verdict = L"Пониженное";
        r.advice  = L"Давление ниже нормы. Выпейте воды, отдохните. "
                    L"При слабости и головокружении обратитесь к врачу.";
    } else {
        r.level   = PressureLevel::Normal;
        r.verdict = L"Норма";
        r.advice  = L"Давление в пределах нормы для вашего возраста и веса.";
    }
    return r;
}
