# Сравнение библиотек для создания механизма конфигурирования
Вся информация взята из репозиториев соответствующих проектов. Дата обращения: 21.04.26.

## Критерии сравнения

| Критерий | Метрика | Единицы измерения |
|:---|:---|:---|
| **Зависимости** | наличие | +/- |
| **Источники** | CLI (argv) / env / файлы | +/-, форматы |
| **Стандарт C++** | минимальная версия | C++хх |
| **Поддерживает стандарт иерархии** | Linux/Windows/MacOS | +/- |
| **Пакетные менеджеры** | vcpkg / Conan / Conda / apt / brew / pacman | +/- |
| **Способ распространения** | header-only / статическая / динамическая | — |
| **ОС** | Linux / Windows / macOS | +/-, компиляторы |
| **Активность** | среднее число коммитов | коммитов/мес (за последний мес) |
| **Issues** | open, closed/open | n, коэффициент |
| **Сообщество** | GitHub Stars | n |
| **Проходит ли CI** | CI | +/- |



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
| **Пакетные менеджеры** | | |
| &emsp;vcpkg | + | + | 
| &emsp;Conan | + | + | 
| &emsp;Conda | + | ? | 
| &emsp;apt | ? | ? | 
| &emsp;brew | ? | + | 
| &emsp;pacman | ? | ? | 
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
| **Проходит ли CI** | + | + | + |


## Парсеры файлов

| Критерий | nlohmann/json  | yaml-cpp | toml++ | toml11 |
|:---|:---:|:---:|:---:|:---:|
| Зависимости | - | - | - | -
| **Источники** | | |
| &emsp;CLI (argv) | - | - | - | -
| &emsp;env | - | - | - | -
| &emsp;Файлы | JSON | YAML | TOML | TOML
| Стандарт C++ | &ge; C++11 | &ge; C++11 | &ge; C++17 | &ge; C++11
| **Поддерживает стандарт иерархии** | | |
| &emsp;Linux | + | + | + | + |
| &emsp;Windows | + | + | + | + |
| &emsp;MacOS | + | + | + | + | 
| **Пакетные менеджеры** | | |
| &emsp;vcpkg | + | + | + | +
| &emsp;Conan | + | + | + | ?
| &emsp;Conda | + | + | ? | ?
| &emsp;apt | + | + | ? | ?
| &emsp;brew | + | + | ? | ?
| &emsp;pacman | + | + | ? | ? |
| Способ распространения | header-only | статическая/динамическая¹ | header-only/статическая/динамическая¹ | header-only/статическая/динамическая¹
| **ОС** | | |
| &emsp;Linux | + (GCC 4.8–14.2, Clang 3.4–21.0) | + (GCC, Clang) | + (Clang 8+, GCC 8+) | + (GCC, Clang)
| &emsp;Windows | + (MSVC 2015–2022) | + (MSVC) | + (MSVC VS2019+) | + (MSVC, MinGW)
| &emsp;macOS | + (AppleClang 9.1–16.0) | + (Xcode, AppleClang) | + (AppleClang) | + (AppleClang)
| **Сообщество** | | | 
| &emsp;Активность (коммитов/мес) | 2 | 9 | 2 | 4
| &emsp;Issues всего | 3271 | 905 | 193 | 194
| &emsp;Issues closed/open | 68 | 3 | 8 | 5
| &emsp;Stars | 49.4k | 15k | 2k | 1.3k 
| **Проходит ли CI** | + | + | + | - | + |

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
| **Пакетные менеджеры** | | |
| &emsp;vcpkg | + |  
| &emsp;Conan | + |  
| &emsp;Conda | + | 
| &emsp;apt | + | 
| &emsp;brew | + | 
| &emsp;pacman | + | 
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
| **Проходит ли CI** | + | + |

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