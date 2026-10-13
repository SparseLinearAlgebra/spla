# User Guide

Руководство пользователя библиотеки spla. Содержит сценарии использования механизма конфигурирования: от быстрого запуска без настройки до запуска тестов в CI. Полный справочник всех параметров, флагов и переменных окружения приведён в [User Manual](UserManual.md).

## Содержание

1. [Проверка окружения](#проверка-окружения)
2. [Сценарий 1. Быстрый запуск без настройки](#сценарий-1-быстрый-запуск-без-настройки)
3. [Сценарий 2. Выбор между CPU и GPU без указания индексов](#сценарий-2-выбор-между-cpu-и-gpu-без-указания-индексов)
4. [Сценарий 3. Несколько устройств и режимов в одном файле](#сценарий-3-несколько-устройств-и-режимов-в-одном-файле)
5. [Сценарий 4. Запуск в CI с файлом из репозитория](#сценарий-4-запуск-в-ci-с-файлом-из-репозитория)
6. [Приоритет источников конфигурации](#приоритет-источников-конфигурации)
7. [Где лежат файлы конфигурации](#где-лежат-файлы-конфигурации)

## Проверка окружения

После установки выполните команду, которая подтверждает, что библиотека готова к использованию:

```bash
spla --softcheck
```

Команда:

1. Читает конфигурационный файл, поставляемый библиотекой.
2. Инициализирует OpenCL, выбирает платформу и устройство.
3. Запускает простое тестовое задание `2 + 3`.
4. Сообщает статус.

Пример успешного вывода:

```bash
$ spla --softcheck
[spla:check] Starting sanity check...
[spla:check] Initializing OpenCL...
[spla:check] Selected accelerator: OpenCL NVIDIA CUDA | device: NVIDIA GeForce RTX 3070 Laptop GPU | vendor: nvidia | mcu: 40 | wave: 32 | mwgs: 1024
[spla:check] Running test task: 2 + 3 = ...
[spla:check] Result: 5 (expected 5)
[spla:check] SUCCESS - spla works correctly
```

## Сценарий 1. Быстрый запуск без настройки

Вы хотите получить работоспособное приложение без настройки - ничего настраивать не нужно. Библиотека использует параметры по умолчанию из конфигурационного файла, поставляемого вместе с библиотекой.

```bash
./program
```

Сценарий работает одинаково на Linux, macOS и Windows. Если GPU недоступен, библиотека по умолчанию автоматически переключится на CPU (параметр `if_gpu_unavailable: "use_cpu"`), сообщив об этом.

Перед первым запуском рекомендуется выполнить [`spla --softcheck`](#проверка-окружения), чтобы убедиться, что всё необходимое для работы spla установлено.

## Сценарий 2. Выбор между CPU и GPU без указания индексов

Вы хотите управлять типом устройства, но не указывать конкретные индексы OpenCL-платформ и устройств.

Тип устройства задаётся параметром `backend` в конфигурационном файле:

```json
{
    "gpu_only": { "backend": "gpu" },
    "cpu_only": { "backend": "cpu" },
    "any":      { "backend": "any" }
}
```

- `"gpu"` - использовать первое доступное GPU,
- `"cpu"` - использовать CPU,
- `"any"` - использовать первое доступное устройство любого типа.

Индексы `platform_index` и `device_index` при этих значениях игнорируются.

### Поведение при отсутствии GPU

Если выбран `backend: "gpu"`, но GPU недоступен, поведение определяется параметром `if_gpu_unavailable`:

```json
{
    "gpu_or_cpu": {
        "backend": "gpu",
        "if_gpu_unavailable": "use_cpu"
    },
    "gpu_only": {
        "backend": "gpu",
        "if_gpu_unavailable": "abort"
    }
}
```

- `"use_cpu"` - библиотека переключается на CPU и продолжает работу,
- `"abort"` -
 библиотека возвращает ошибку и не продолжает работу.

Запуск нужной конфигурации:

```bash
./program --spla-config=gpu_or_cpu
./program --spla-config=gpu_only
```

`if_gpu_unavailable` применяется при `backend: "gpu"`, `"any"` и `"by_index"`. При `backend: "cpu"` параметр игнорируется.

## Сценарий 3. Несколько устройств и режимов в одном файле

Вы регулярно запускаете одну и ту же программу на разных GPU и в разных режимах - например, на GPU 0 и 1, в режиме отладки и в режиме измерения производительности. Вместо нескольких файлов конфигурации вы описываете всё в одном.

```json
{
    "default": { "backend": "gpu", "verbosity": 2 },

    "gpu0": {
        "extends": ["default"],
        "backend": "by_index",
        "platform_index": 0,
        "device_index": 0
    },
    "gpu1": {
        "extends": ["default"],
        "backend": "by_index",
        "platform_index": 0,
        "device_index": 1
    },

    "debug": {
        "extends": ["default"],
        "verbosity": 3,
        "profiling": true
    },
    "bench": {
        "extends": ["default"],
        "verbosity": 0
    },

    "debug_gpu1": { "extends": ["debug", "gpu1"] },
    "bench_gpu0": { "extends": ["bench", "gpu0"] }
}
```

Запуск:

```bash
./program --spla-config=gpu1
./program --spla-config=debug
./program --spla-config=debug_gpu1
./program --spla-config=bench_gpu0
```

Конфигурации, наследуемые через `extends`, применяются по порядку: каждый следующий родитель переопределяет предыдущего, а параметры самой конфигурации переопределяют всех родителей. Циклическое наследование обнаруживается и приводит к ошибке.

## Сценарий 4. Запуск в CI с файлом из репозитория

Вам нужно запустить тесты в CI с отдельным файлом конфигурации.

В среде CI неудобно и нежелательно трогать системный и пользовательский файлы конфигурации, а конфигурации тестов должны лежать в репозитории. Поэтому конфигурации описывают в отдельном файле внутри репозитория и указывают путь к нему при запуске вместе с именем конфигурации - это аргументы командной строки `--spla-config-file` и `--spla-config`.

```
my_project/
├── .github/
│   └── workflows/
│       ├── ci.yml
│       └── ci_spla_conf.json
├── src/
└── tests/
```

### ci_spla_conf.json

```json
{
    "test_cpu": {
        "backend": "cpu",
        "queues_count": 1,
        "profiling": false,
        "allocator_type": "general",
        "verbosity": 1
    },
    "test_gpu0": {
        "backend": "by_index",
        "platform_index": 0,
        "device_index": 0,
        "if_gpu_unavailable": "abort",
        "queues_count": 2,
        "profiling": false,
        "allocator_type": "general",
        "verbosity": 0
    },
    "test_gpu1": {
        "backend": "by_index",
        "platform_index": 0,
        "device_index": 1,
        "if_gpu_unavailable": "abort",
        "queues_count": 2,
        "profiling": false,
        "allocator_type": "linear",
        "linear_alloc_size": 8,
        "verbosity": 0
    }
}
```

### ci.yml

```yml
name: CI

on: [push, pull_request]

jobs:
  test:
    name: Test (${{ matrix.config }})
    runs-on: [self-hosted, gpu]

    strategy:
      fail-fast: false
      matrix:
        config: [test_cpu, test_gpu0, test_gpu1]

    steps:
      - name: Checkout repository
        uses: actions/checkout@v4

      - name: Set up Python
        uses: actions/setup-python@v5
        with:
          python-version: "3.11"

      - name: Install dependencies
        run: |
          python -m pip install --upgrade pip
          pip install pyspla pytest

      - name: Run tests
        run: python src/my_app.py --spla-config-file=.github/workflows/ci_spla_conf.json --spla-config=${{ matrix.config }}
```

Путь к файлу и имя конфигурации передаются при запуске как аргументы команды, а не через переменные окружения - в CI это делает команду запуска самодостаточной и воспроизводимой.

## Приоритет источников конфигурации

Источники применяются в порядке убывания приоритета: параметры из более приоритетного источника переопределяют менее приоритетные. Это важно для сценариев: любой флаг или переменная окружения перебивает значения из файлов.

| # | Источник |
|---:|:---|
| 1 | Аргументы командной строки (`--spla-*`) |
| 2 | Переменные окружения (`SPLA_*`) |
| 3 | Файл с явно указанным путём (`--spla-config-file`) |
| 4 | Пользовательский конфигурационный файл |
| 5 | Системный конфигурационный файл |
| 6 | Конфигурационный файл, поставляемый библиотекой |

Если имя конфигурации не указано, используется конфигурация `"default"` из файла, поставляемого библиотекой. Подробно о порядке сбора и слиянии источников - в [User Manual](UserManual.md#обзор-механизма-конфигурирования).

## Где лежат файлы конфигурации

- **Поставляемый библиотекой** - рядом с файлом библиотеки (в пакете pyspla - внутри установленного пакета).
- **Системный** - Linux: `/etc/spla/spla_conf.json`.
- **Пользовательский** - Linux: `~/.config/spla/spla_conf.json`.
- **Явный** - любой путь через `--spla-config-file`.

Пути для macOS и Windows и правила «файла нет / файл не открывается» - в [User Manual](UserManual.md#конфигурационные-файлы).