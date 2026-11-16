# GraphResolver

GraphResolver — это C++20 библиотека для сборки и выполнения графа зависимостей между модулями. Она позволяет регистрировать модули с зависимостями от других модулей, автоматически проверять циклы и порядок инициализации, а затем безопасно создавать и разрушать их в корректной последовательности.

Основная идея: каждый модуль описывает, от каких модулей он зависит, и библиотека строит порядок инициализации на основе этого графа. Это удобно для загрузки компонентов, плагинов, подсистем и сервисов без ручной ручной сортировки.

## Возможности

- автоматическая топологическая сортировка модулей
- проверка отсутствующих зависимостей
- обнаружение циклов в графе
- защита от дублирования идентификаторов модулей
- отдельный этап `PostConstruct` после инициализации графа
- корректная очистка через `DestructInOrder` и `DestroyGraph`
- C API для интеграции с плагинами и внешними системами
- поддержка CMake и примеров использования

## Требования

- CMake 3.28+
- компилятор с поддержкой C++20
- ninja или любой другой генератор CMake

## Быстрый старт

### Сборка

```bash
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

### Минимальный пример

```cpp
#include "graph_resolver/internal/graph_resolver.h"

struct Logger
{
    CREATE_MODULE(Logger)

    Logger() = default;
};

struct Renderer
{
    CREATE_MODULE(Renderer)

    Renderer(Logger* logger) {}
};

int main()
{
    gr_GraphResolver graph;

    graph.AddModule<Logger>(
        [](const gr_GraphResolverApi*, gr_GraphResolver*)
        {
            return static_cast<void*>(new Logger());
        });

    graph.AddModule<Renderer, gr_TypeList<Logger>>(
        [](const gr_GraphResolverApi*, gr_GraphResolver*)
        {
            auto* logger = graph.GetModule<Logger>();
            return static_cast<void*>(new Renderer(logger));
        });

    if (!graph.CompileGraph())
    {
        return 1;
    }

    if (!graph.ExecuteGraph())
    {
        return 2;
    }

    auto* renderer = graph.GetModule<Renderer>();
    (void)renderer;
    graph.DestroyGraph();
    return 0;
}
```

## Основные концепции

### 1. Модуль

Модуль — это сущность с типом, явным ID через `CREATE_MODULE(name)`, фабрикой создания, необязательным post-construct callback и деструктором. Имя в макросе определяет ID независимо от имени класса. Фабрика возвращает указатель на объект, который затем будет храниться в контейнере библиотеки.

### 2. Зависимости

`gr_TypeList<T1, T2, ...>` задаёт список зависимостей типа `T`. Во время компиляции графа библиотека:

- проверяет, что все зависимости существуют
- считает степень входа каждого узла
- строит порядок выполнения
- сообщает об ошибках, если обнаружен цикл или потерян модуль

### 3. `PostConstruct`

После того, как все модули созданы и граф скомпилирован, `ExecuteGraph()` вызывает:

1. `Construct` для каждого модуля по топологическому порядку
2. `PostConstruct` для каждого модуля по порядку зависимостей post-deps

Это полезно, когда один модуль должен получить готовые зависимости и только после этого активировать доп. логику.

### 4. Жизненный цикл

Каждый модуль проходит через следующие шаги:

- регистрация через `AddModule`
- компиляция через `CompileGraph`
- создание через `ExecuteGraph`
- уничтожение через `DestructInOrder` или `DestroyGraph`

## API

### Основной класс

```cpp
gr_GraphResolver graph;
```

### Добавление модуля

```cpp
graph.AddModule<MyModule>(construct_fn, post_construct_fn, destruct_fn);
```

или с зависимостями:

```cpp
graph.AddModule<MyModule, gr_TypeList<Logger, Settings>>(
    construct_fn,
    post_construct_fn,
    destruct_fn);
```

### Компиляция и выполнение

```cpp
auto compile_result = graph.CompileGraph();
if (!compile_result)
    return 1;

auto execute_result = graph.ExecuteGraph();
if (!execute_result)
    return 2;
```

### Получение модуля

```cpp
auto* logger = graph.GetModule<Logger>();
```

### Проверка наличия

```cpp
if (graph.HasModule<Logger>())
{
    // ...
}
```

### Очистка

```cpp
graph.DestructInOrder();
// или
graph.DestroyGraph();
```

## C API

Библиотека также предоставляет `gr_GetGraphResolverApi()`, который позволяет подключать функциональность в плагинных архитектурах.

Основные методы API:

- `execute_graph`
- `compile_graph`
- `destruct_in_order`
- `destroy_graph`
- `has_module`
- `get_module_int`
- `add_module_from_descriptor`

## Структура проекта

```text
.
├── CMakeLists.txt
├── README.md
├── include/
│   └── graph_resolver/
│       ├── api/
│       ├── internal/
│       └── utils/
├── src/
│   ├── core.cpp
│   ├── graph_resolver.cpp
│   ├── module_slot.cpp
│   └── module_slot_desc.cpp
├── tests/
│   ├── CMakeLists.txt
│   ├── test_graph_resolver.cpp
│   └── test_graph_resolver_api.cpp
├── examples/
│   ├── first/
│   ├── second/
│   └── third/
└── build/
```

## Примеры

В каталоге `examples` находятся демо-приложения:

- `examples/first` — базовый пример работы с зависимостями и пост-конструкцией
- `examples/second` — сложный сценарий создания системы модулей
- `examples/third` — пример интеграции с плагинной архитектурой

## Тестирование

Проект использует GoogleTest. Тесты автоматически подключаются через CMake и запускаются командой:

```bash
ctest --test-dir build --output-on-failure
```

Базовые сценарии покрывают:

- создание и выполнение графа
- дублирующиеся идентификаторы модулей
- отсутствие зависимостей
- циклы
- порядок конструирования и пост-конструирования
- разрушение в обратном порядке
- повторную перезагрузку графа после `DestroyGraph`

## Ограничения и замечания

- `gr_modid_t` — это идентификатор модуля, который должен быть уникальным в пределах графа.
- Максимальный размер списка зависимостей ограничен внутренним буфером `gr_ModIdView::capacity`.
- Для корректной работы модули должны быть безопасно описаны и возвращать валидный указатель в конструкторе.

## Лицензия

Проект распространяется под лицензией MIT. Подробности см. в файле `LICENSE`.
