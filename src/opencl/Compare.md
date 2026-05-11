# Сравнение библиотек для создания механизма конфигурирования
Вся информация взята из репозиториев соответствующих проектов. Дата обращения: 21.04.26.

## Критерии сравнения

| Критерий | Метрика | Единицы измерения |
|:---|:---|:---|
| **Зависимости** | наличие | +/- |
| **Источники** | CLI (argv) / env / файлы | +/-, форматы |
| **Стандарт C++** | минимальная версия | C++хх |
| **Поддерживает стандарт иерархии** | Linux/Windows/MacOS | +/- |
| **Способ распространения** | header-only / статическая / динамическая | — |
| **ОС** | Linux / Windows / macOS | +/-, компиляторы |
| **Активность** | среднее число коммитов | коммитов/мес (за последний мес) |
| **Issues** | open, closed/open | n, коэффициент |
| **Сообщество** | GitHub Stars | n |
| **Проходит ли CI** | CI | +/- |

> ? - не указано в README.


## Сравнение парсеров командной строки (CLI)

| Критерий | CLI11 | cxxopts | 
|:---|:---:|:---:|
| Зависимости | - | - |
| **Источники** | | |
| &emsp;CLI (argv) | + | + | 
| &emsp;env | + | - | 
| &emsp;Файлы | - | - | 
| Стандарт C++ | &ge; C++11 | &ge; C++11 | 
| **Поддерживает стандарт иерархии** | | |
| &emsp;Linux | + | + |
| &emsp;Windows | + | + | 
| &emsp;MacOS | + | + |  
| Способ распространения | header-only | header-only 
| **ОС** | | |
| &emsp;Linux | + (GCC 4.8+, Clang 3.5+) | + (GCC ≥ 4.9, Clang ≥ 3.1) |
| &emsp;Windows | + (MSVC ≥ 2015) | + (MSVC ≥ 2015) |
| &emsp;macOS | + (AppleClang 7+) | + (Clang ≥ 3.1 с libc++) |
| **Сообщество** | | | 
| &emsp;Активность (коммитов/мес) | 4 | 1 |
| &emsp;Issues всего | 506 | 307 | 
| &emsp;Issues closed/open | 9 | 6 | 
| &emsp;Stars | 4.3k | 4.7k | 
| Проходит ли CI | + | + | + |


## Парсеры файлов

| Критерий | nlohmann/json  | yaml-cpp | toml++ | toml11 | config-cxx | taocpp/config
|:---|:---:|:---:|:---:|:---:|:---:|:---:|
| Зависимости | - | - | - | - | - | - |
| **Источники** |
| &emsp;CLI (argv) | - | - | - | - | - | - |
| &emsp;env | - | - | - | - | + | + |
| &emsp;Файлы | JSON | YAML | TOML | TOML | JSON, YAML, XML | JSON, JAXN |
| Стандарт C++ | &ge; C++11 | &ge; C++11 | &ge; C++17 | &ge; C++11 | &ge; C++20 | &ge; C++17 |
| **Поддерживает стандарт иерархии** |
| &emsp;Linux | + | + | + | + | + | + |
| &emsp;Windows | + | + | + | + | + | + |
| &emsp;MacOS | + | + | + | + | + | + |
| Способ распространения | header-only | статическая/динамическая¹ | header-only/статическая/динамическая¹ | header-only/статическая/динамическая¹ | header-only | header-only |
| **ОС** | | |
| &emsp;Linux | + (GCC 4.8–14.2, Clang 3.4–21.0) | + (GCC, Clang) | + (Clang 8+, GCC 8+) | + (GCC, Clang) | + (GCC 13+, Clang 16+) | +
| &emsp;Windows | + (MSVC 2015–2022) | + (MSVC) | + (MSVC VS2019+) | + (MSVC, MinGW) | + (MSVC 143+ (VS 2022)) | +
| &emsp;macOS | + (AppleClang 9.1–16.0) | + (Xcode, AppleClang) | + (AppleClang) | + (AppleClang) | + (AppleClang 16+) | +
| **Сообщество** | | | 
| &emsp;Активность (коммитов/мес) | 2 | 9 | 2 | 4 | 2 | 13 |
| &emsp;Issues всего | 3271 | 905 | 193 | 194 | 24 | 5 |
| &emsp;Issues closed/open | 68 | 3 | 8 | 5 | 0 open | 0 open |
| &emsp;Stars | 49.4k | 15k | 2k | 1.3k | 31 | 194 |
| Проходит ли CI | + | + | + | - | + | ? |

> ¹ Опционально.

## Сравнение универсальных инструментов

| Критерий | Boost.Program_options
|:---|:---:|
| Зависимости | +¹ |
| **Источники** | | |
| &emsp;CLI (argv) | + | + |
| &emsp;env | + | + |
| &emsp;Файлы | INI |
| Стандарт C++ | &ge; C++3 | 
| **Поддерживает стандарт иерархии** | | |
| &emsp;Linux | + | 
| &emsp;Windows | + | 
| &emsp;MacOS | + |
| Способ распространения | статическая/динамическая² | 
| **ОС** | | |
| &emsp;Linux | + (GCC 5+, Clang 3.6+) | 
| &emsp;Windows | + (MSVC 2015 (vc140)+) |
| &emsp;macOS | + (AppleClang) |
| **Сообщество** | | | 
| &emsp;Активность (коммитов/мес) | 12 | 
| &emsp;Issues всего | 400 | 
| &emsp;Issues closed/open | 1 | 
| &emsp;Stars | 9.4k |
| Проходит ли CI | + | + |

> ¹ Список зависимостей: 
> 1. Boost.Any
> 2. Boost.Bind
> 3. Boost.Config
> 4. Boost.Core
> 5. Boost.Detail
> 6. Boost.Function
> 7. Boost.Iterator
> 8. Boost.Lexical Cast
> 9. Boost.Smart Ptr
> 10. Boost.ThrowException
> 11. Boost.Tokenizer
> 12. Boost.Type Traits 

> ² Опционально.


## Итог:
По результатам сравнения, вероятно, лучшим вариантом будет использование `CLI11` и `nlohmann/json`, так как они, в отличие от `Boost.program_options`, не требуют зависимостей и являются header-only.

# Сценарии использования spla:
**Что важно всем:** возможность выбрать платформу и устройство 

1. Пользователь хочет решить свою задачу 
    - Неподготовленный пользователь
        - важно: 
            - быстрая настройка через флаги
        - что конфигурировать:
            - уменьшить количество отладочной информации
            - остальное - дефолтные параметры
    - Опытный пользователь: 
        - важно:
            - детальная ручная конфигурация параметров через конфигурационный файл
            - ускорение вычислений с разреженными данными
        - что конфигурировать:
            - управление кэшем
            - кастомные аллокаторы для разных форматов данных
            - управление очередями команд 
            - Wave size override
            - Work-group size
            - оптимизация доступа к глобальной памяти
            - управление логированием
2. Разработчик spla
    - Отладчик, тестировик
        - важно: 
            - верификация корректности работы на разных устройствах 
            - воспроизводимость ошибок
        - что конфигурировать:
            - возможность работать только на 1 гпу
            - подробное логирование
            - отключение кэша для принудительной перекомпиляции
            - тестирование на неподдерживаемых вендорах без падения с ошибкой
    - Бенчмаркер
        - важно:
            - минимизация шума при измерениях
            - воспроизводимость результатов
            - сравнение разных конфигураций
        - что конфигурировать:
            - профайлинг очередей `CL_QUEUE_PROFILING_ENABLE`
            - сбор статистики и уменьшение шума `CLCounterPool`
            - количество очередей для измерения параллелизма
            - линейный аллокатор для предсказуемости памяти
            - Wave size override
            - Work-group size
            - оптимизация доступа к глобальной памяти
    - Расширяющий функциональность
        - важно: 
            - возможность встроить код в существующую инфраструктуру
            - чтобы новый код работал на всех устройтсвах
        - что конфигурировать:
            - включение кэша
            - кастомные аллокаторы для разных форматов данных
            - управление очередями
            - Wave size override
            - Work-group size

# Что нужно конфигурировать 
1. Выбор платформы
2. Выбор устройства 
3. Количество очередей: сейчас создается только 1 очередь 
4. Приоритеты очередей
5. Размер линейного аллокатора `DEFAULT_SIZE`
6. Стратегия выбора аллокатора: \
    NVIDIA используют общий аллокатор \
    Все остальные используем линейный аллокатор
7. Характеристики устройтсв:\
    `m_vendor_id` \
    `m_max_cu` \
    `m_default_wgs` \
    `m_max_local_mem` \
    `m_addr_align` \
    пороги `m_max_wgs`, `m_max_cu`, `m_wave_size`
8. Строковые паттерны проверки девайса: возможно только 3 вариаета `m_vendor_name`
6. Дефолтные значения характеристик \
    `m_addr_align` \
    `m_default_wgs` \
    `uint m_wave_size` \
    `uint m_num_of_mem_banks`
7. Суффикс и имя \
    `m_name` \
    `m_suffix `
8. Тип устройства (чтобы можно было использовать CPU)
    `getDevices`

# Структура конфига (JSON)

```bash
config:

    "platform":
        "selection":
            "user_alias": string
            "official_name": string
            "vendor": "nvidia" | "amd" | "intel" | "img" | ""
        "index": int
        "priority": int
        "fallback": (???)                           # Нужен ли fallback
        "device":
            "selection": 
                "user_alias": string
                "official_name": string
                "type": "gpu" | "cpu" 
            "index": int
            "priority": int
            "naming": 
                "accelerator_name"                      # m_name
                "cache_suffix"                          # m_suffix
            "tuning": 
                "default_work_group_size": int          # m_default_wgs
                "wave_size": int                        # m_wave_size
                "memory_banks": int                     # m_num_of_mem_banks
                "memory_alignment": int                 # m_addr_align
            "fallback": (???)                           # Нужен ли fallback
            "performance"
                "memory"
                    "linear_allocator_size_mb": int         # DEFAULT_SIZE
                    "allocator_strategy":   "auto" | "linear" | "general" | "vendor"
                    "vendor_strategy":
                        "nvidia":           "linear" | "general"
                        "amd":              "linear" | "general"
                        "intel":            "linear" | "general"
                        "img":              "linear" | "general"
            "queues": 
                "count": int
                
```

