# Контейнеры, строки и ID

Реализация C++20, 18 октября 2026. Публичные header-only утилиты находятся в
`src/engine/utilsModule`. Они используют стандартные алгоритмы и подключаются
без renderer/editor. `std::vector` остаётся основным контейнером для непрерывных
массивов; новые классы решают конкретные задачи времени жизни и хранения.

## Выбор контейнера

| Задача | Тип |
| --- | --- |
| Непрерывные данные, быстрый обход, GPU upload | `std::vector`, желательно с reserve |
| Короткие списки с возможностью роста | `UTILS::SmallVector<T, N, Allocator>` |
| Рост с сохранением адресов объектов | `UTILS::ChunkedVector<T, ChunkSize, Allocator>` |
| Небольшой упорядоченный словарь, редкие изменения | `UTILS::FlatMap<Key, Value, Compare, Allocator>` |
| Повторяющиеся неизменяемые имена | `UTILS::StringPool` + `UTILS::Identifier` |
| Небольшой словарь по именам с готовыми symbol handles | `UTILS::InternedNameMap<Value>` |
| Большая часто меняющаяся хеш-таблица | Пока `std::unordered_map`; отдельный FlatHashMap не добавлен |

## SmallVector

```cpp
#include "utilsModule/containers/smallVector.h"

IKIGAI::UTILS::SmallVector<std::string, 4> names;
names.emplace_back("position"); // строка размещается во встроенном буфере
names.push_back("normal");
names.insert(names.begin() + 1, "tangent");
std::span<const std::string> view(names); // непрерывное хранение
names.reserve(32);             // перенос элементов в кучу
names.resize(2);
names.shrink_to_fit();          // возврат во встроенный буфер при size <= 4
```

`N` — число встроенных слотов, не ограничение размера. До N элементов контейнер
не запрашивает память для элементов у аллокатора, включая вставку в середину.
Неиспользуемые слоты не конструируют T. Внутренняя память самих T (например,
длинной std::string) и первоначальное создание общего allocator domain в это
обещание не входят. При переполнении все элементы переносятся в один растущий
буфер. Размер самого SmallVector включает N * sizeof(T), выравнивание и служебные
поля. Поддерживаются N = 0, over-aligned T и move-only T. Тип T должен быть
перемещаемым или копируемым; для неподвижных объектов используйте ChunkedVector.

API включает конструкторы count/fill/range/initializer_list, copy/move с явным
allocator, assign, data, прямые и обратные contiguous-итераторы, front/back/at,
reserve, resize, emplace_back/push_back/pop_back, одиночные insert/emplace,
erase позиции/диапазона, clear, shrink_to_fit и swap. Проверить режим можно через
`is_inline()`, встроенную вместимость — через `inline_capacity`. Для вставки в
середину и erase требуется move assignment T. Арифметика итераторов принимает
позиции только своего контейнера. Потоковую синхронизацию обеспечивает вызывающий
код. Специализации для bool нет: это обычные bool-элементы с адресами.

Рост за capacity, reserve с увеличением capacity и фактический shrink_to_fit
инвалидируют все ссылки, указатели и итераторы. Вставка/удаление без роста
инвалидируют их от позиции изменения; добавление в конец без роста сохраняет
адреса прежних элементов. clear вызывает деструкторы, сохраняя буфер. Уменьшение
size само не возвращает контейнер из кучи; для этого есть shrink_to_fit.
Перемещение inline-контейнера переносит каждый элемент, а совместимого heap-
контейнера передаёт буфер. После успешного move источник пуст и пригоден для
повторного использования. Для единообразия считайте итераторы недействительными
после move/swap; swap с inline-хранением перемещает элементы.

По умолчанию используется MEMORY::StlAllocator<T>. Heap-буфер выделяется и
освобождается в одном домене, copy assignment сохраняет домен назначения,
move assignment и swap распространяют его согласно traits аллокатора. При разных
нераспространяемых allocator move переносит элементы в домен назначения, а swap
отвергается исключением invalid_argument. Конструктор move копирует allocator,
оставляя исходному контейнеру пригодный домен. Для обычных типов с noexcept move
и StlAllocator move-конструктор/присваивание имеют noexcept.

При переносе используется move_if_noexcept. reserve/shrink_to_fit, рост при
emplace/resize и конструкторы освобождают частично созданные объекты при исключении.
Новые элементы при росте конструируются до переноса старых: push_back(v[0]) и
resize(count, v[0]) корректны при перераспределении памяти. При копируемом T и
обычных allocator hooks неуспешный перенос сохраняет прежнее содержимое; явно
переданный std::move(v[i]) может изменить аргумент ещё до ошибки. Для move-only T
с бросающим move прежние значения могут быть изменены, но объекты и буферы остаются
учтены. Вставка/erase внутри буфера, swap и присваивание дают базовую гарантию:
при исключении контейнер остаётся пригоден для использования, значения могут
измениться. Copy assignment сначала готовит копию; ошибка её создания сохраняет
назначение, ошибка последующего переноса во встроенный буфер может оставить
назначение пустым или частично заполненным.

Проверки SmallVector находятся в `utils/tests/containers/smallVector.cpp` и
запускаются вместе с containers_tracking_0/1. Они покрывают границы N/N+1, N=0,
отсутствие allocations до N, alignment, aliasing, 8000 случайных операций против
std::vector, все комбинации inline/heap swap, allocator propagation, move-only T,
ошибки выделения памяти и конструирования/присваивания с подсчётом живых объектов.
Benchmark `small_vector_8`/`short_std_vector` сравнивает 10000 коротких списков
размера 0/1/4/8/9/32 в одном явном домене; создание домена исключено из замера.
Число allocations относится к буферам контейнера. Размер N выбирайте по частым
размерам реальных списков: слишком большое значение увеличивает каждый объект.

## ChunkedVector

```cpp
#include "utilsModule/containers/chunkedVector.h"

IKIGAI::UTILS::ChunkedVector<MyObject, 128> objects;
auto* first = &objects.emplace_back(constructorArgument);
objects.reserve(10000); // first остаётся валидным; MyObject не перемещается
for (auto& object : objects) update(object);
for (std::size_t i = 0; i < objects.chunk_count(); ++i)
    process(objects.chunk(i)); // span<MyObject>, последний чанк может быть неполным
```

Живые `T` конструируются отдельно в allocator-aware блоках. Default-конструктор,
копирование и перемещение `T` для `emplace_back` не требуются. Каталог блоков также
использует переданный allocator через rebind. Размер чанка задаётся числом элементов.

Рост/reserve сохраняют адреса и итераторы существующих элементов; старый `end()`
после добавления не используется. Итераторы random-access, const-обход не позволяет
менять элементы. Непрерывного общего `data()` нет. Операции с разными контейнерами
через арифметику итераторов не определены, как и для стандартных контейнеров.

`clear()` вызывает деструкторы и оставляет блоки для повторного использования;
`release()` требует пустой контейнер и освобождает блоки. `pop_back/back/operator[]`
имеют стандартные preconditions, `at()` проверяет полный индекс. Копирование
включено для копируемых T. Move с совместимым/распространяемым allocator переносит
хранение; с разными нераспространяемыми allocator перемещает/копирует элементы.
Для неперемещаемого T такой перенос между доменами запрещён ошибкой.

Старый `ChunkList<T, CONT, ChunkSize>` сохранён как обёртка с категорией Component.
Оба ComponentArray используют новое хранение через неё. Удалены небезопасный
middle-insert и связанный список узлов; `moveFrom/moveTo/isEmpty` пока сохранены.
Удаление в ECS всё ещё перемещает последний компонент и обновляет weak control
block. Контейнер не обещает неподвижность компонента при таком удалении.

## FlatMap

```cpp
#include "utilsModule/containers/flatMap.h"
IKIGAI::UTILS::FlatMap<std::string, int> bindings;
bindings.try_emplace("position", 0);
bindings.insert_or_assign("normal", 1);
int binding = bindings.at(std::string_view("position"));
for (auto [name, value] : bindings) { /* name const&, value int& */ }
```

Это один отсортированный vector пар, не flat hash map. Поиск O(log N),
вставка/удаление O(N). Ключи защищены от изменения через iterator proxy;
значения доступны для изменения. Прокси поддерживает `it->first/second`,
structured bindings через `auto`/`auto&&`; он не является обычным `pair&`.
Итератор предоставляет input-iterator API, а не contiguous/random-access API.

Поиск поддерживает совместимые с Compare типы, включая `string_view` при
`std::less<>`. Ключ вставки сначала приводится к Key. Аргументы Value при повторном
`try_emplace` не потребляются. `assign(first, last)` сортирует новый набор и
отклоняет дубликаты, оставляя прежнюю таблицу; доступны move-итераторы.

Модификации могут инвалидировать ссылки/итераторы; reserve не предотвращает сдвиги
при вставке. Для noexcept-relocatable пар используются сдвиги на месте; иначе
операции строят новый буфер копированием и делают commit. Некопируемые пары с
бросающим перемещением отвергаются при компиляции. Comparator должен иметь
небросающий swap. Сам контейнер не синхронизирован.

## Строки и Identifier

```cpp
#include "utilsModule/stringPool.h"
IKIGAI::UTILS::StringPool names;
auto id = names.intern("engine_UBO");
auto same = names.intern(std::string("engine_UBO")); // id == same
std::string_view text = names.view(id);
auto missing = names.find("unknown"); // invalid(), без вставки
```

Уникальные байты копируются в неподвижные страницы. Hash collisions разрешаются
сравнением строк. Равенство case-sensitive, по байтам; UTF-8 и embedded NUL
сохраняются. Пустая строка валидна и отличается от invalid Identifier.
Байты дополнены нулём, но длина определяет содержимое.

`Identifier` занимает 8 байт: уникальный token пула + `Id<StringEntryTag>`.
Сравнения учитывают оба поля. `view()` отвергает чужие/невалидные ID. Номера не
переиспользуются, переполнение проверяется. Identifier не владеет пулом.
Выданные view действительны до уничтожения пула, включая рост страниц и rehash.
Удаления отдельных строк, reset, копирования/перемещения пула нет. При отказе
вставки уже выделенные байты могут остаться slack до уничтожения пула, но неполный
ID не публикуется.

В файлы/сеть записывайте исходную строку, не числовой ID. Порядок числовых ID
зависит от интернирования; для воспроизводимого порядка сортируйте строки.
Не интернируйте неограниченный пользовательский текст в долгоживущий domain.

`StringPool` не синхронизирован. `SynchronizedStringPool` защищает intern/find/view
и каталог записей; чтение уже полученного view блокировки не требует. Уничтожение
пула при продолжающейся работе потребителей запрещено.
Для небольших scoped-таблиц синхронизированный фасад начинает со страниц 4 КиБ;
размер можно передать конструктору. Базовый StringPool по умолчанию использует 64 КиБ.

`InternedNameMap<Value>` объединяет пул и `FlatMap<Identifier, Value>`, поддерживает
`operator[](string_view)`, `at/contains(string_view)` и `at/contains(Identifier)`.
`identifier(name)` находит ID без вставки. Копии разделяют пул, но имеют собственные
таблицы; владеющий shared_ptr удерживает байты. Саму таблицу защищает вызывающий код.

`ShaderReflection::mNameToUniforms` переведён на этот тип. Строковые обращения
сохранены; потребитель может один раз получить identifier и выполнять числовой
поиск. После создания новой reflection/recompile identifier нужно получить заново.
Пул принадлежит reflection и её копиям, поэтому старые имена не копятся глобально
при hot reload. Это уточнение первоначального плана с engine-global пулом.

## Id и выдача значений

`Id<Tag, Rep = uint32_t>` — типизированное число, по умолчанию invalid (max Rep).
Есть `value/valid/invalid`, constexpr-сравнения и noexcept hash. Целочисленный
конструктор и явные преобразования проверяют диапазон; отрицательные/усечённые
значения отвергаются. Ноль валиден как значение Id.

`IdSequence<Tag, Rep, First, Last>` предоставляет `next()` и `observeLoaded(id)`.
Обе операции используют atomic CAS; исчерпание даёт `overflow_error`, без wraparound.
`observeLoaded` только двигает верхнюю границу — загрузчик проверяет дубликаты.
Это не генерационные handles и не сохраняемые UUID.

`IdGenerator<Tag>` остаётся legacy-фасадом. Entity выдаются от 1 до INT_MAX;
component-type ID остаются от 0. Загруженные корректные ID не меняются; ноль
сохраняется для отсутствующего родителя. Копирование Entity сохраняет идентичность.
Текущий int-формат сцен и prefab-ссылок сохранён.

## Аллокаторы

```cpp
#include "utilsModule/memory/monotonicAllocator.h"
auto domain = std::make_shared<IKIGAI::MEMORY::HeapAllocator>();
using A = IKIGAI::MEMORY::StlAllocator<int>;
std::vector<int, A> values{A(domain)};
```

`StlAllocator<T, Category>` теперь принимает владеющий `shared_ptr<IAllocator>`;
null отвергается. Без аргумента остаётся прежний общий category domain.
Copy/rebind сохраняют домен; copy assignment контейнера сохраняет домен назначения,
move assignment и swap распространяют allocator. Moved-from адаптер остаётся
валидным. Внутренние allocations самого T (например, символы std::string)
автоматически на этот allocator не переводятся.

`MonotonicAllocator(pageBytes, upstream, budget, category)` растёт страницами,
поддерживает over-alignment и лимит backing-страниц. Metadata не входит в budget.
Индивидуальный deallocate не освобождает байты; `reset()` переиспользует страницы,
`release()` освобождает их. Reset/release не вызывают деструкторы размещённых T:
потребители должны быть уничтожены заранее. `stats()` показывает reserved, used,
peak и количество запросов после reset; peak сохраняется.

При неотслеживаемом upstream арена регистрирует логические allocations и снимает
их при reset/release. При отслеживаемом upstream профайлер показывает удерживаемые
backing-страницы, без двойного учёта suballocations; used берётся из arena.stats().
Для этого IAllocator имеет неизменяемый `tracksAllocations()`; профилирующие и
синхронизирующие обёртки распространяют этот признак. Оборачивать саму уже
инструментированную арену ещё одним ProfiledAllocator не требуется.

`ScratchMemory` — отдельный owner на задачу/поток/кадр. `allocator<T>()` выдаёт
владеющий STL-адаптер; `reset()` отвергается, пока такой адаптер/контейнер живёт.
Передавать сырые указатели за срок жизни/reset domain нельзя. GPU/frame lifetime
контролирует вызывающая система. StackAllocator не подходит произвольным STL
контейнерам с освобождением не в LIFO-порядке.

## Сборка и проверки

```sh
cmake -S utils/tests/containers -B build-containers
cmake --build build-containers --config Debug
ctest --test-dir build-containers -C Debug --output-on-failure
cmake --build build-containers --config Release
ctest --test-dir build-containers -C Release --output-on-failure
```

В составе движка: `-DIKIGAI_CONTAINER_TESTS=ON`. Тесты включают tracking ON/OFF,
границы чанков/const-итераторы, lifetime и allocation failures, независимые домены,
случайные операции FlatMap относительно std::map, коллизии/rehash и отказ вставки
строк, конкурентную выдачу ID/работу синхронизированного пула, реальные ECS weak
handles и существующий `scene_lifetime.cpp`. Существующие memory-тесты запускаются
отдельно из `utils/tests/memory`.

`ikigai_container_benchmarks` строится по умолчанию; отключение:
`-DIKIGAI_CONTAINER_BENCHMARKS=OFF`. Запускайте Release без memory tracking.
CSV содержит hit/miss/build/churn для таблиц 8/32/128/1024 элементов, рост/обход
последовательностей, unique/repeated interning и cached-ID/string lookup.
Все сравниваемые контейнеры используют один тип counting heap adapter.
Число allocations/peak относится к storage этого адаптера, без внутренних
std::string allocations и системных headers. В hit/miss строках счётчики накоплены
со времени build; время относится только к указанной операции.

Бенчмарк является микротестом, а не доказательством ускорения кадра или всех
платформ. Старый ошибочный ChunkList не включён как отдельный исполняемый baseline;
сравниваются новое хранение, deque и vector с reserve/без reserve.
PMR bridge, FlatHashMap и SlotMap остаются следующими независимыми
задачами после анализа реальных нагрузок.

## Выполненная верификация на Windows

18 октября 2026: MSVC 19.38 Debug/Release — по 3/3 теста, Clang-cl 21.1.8 Release
с MSVC STL — 3/3, MSVC AddressSanitizer — 3/3, существующие memory/DX12 memory
регрессии — 4/4. Tracking ON/OFF проверяются отдельными исполняемыми файлами.

Добавленный затем SmallVector включён в общий suite: MSVC Debug/Release,
Clang-cl Release и MSVC AddressSanitizer также прошли по 3/3 теста. Его benchmark
сохранён в `utils/.pack/small-vector-benchmarks.csv`: для 10000 списков size_t
размера 1/4/8 при N=8 контейнер сделал 0 buffer allocations, размера 9 — 10000.
Сравниваемый std::vector без reserve сделал соответственно 10000/40000/60000/60000
allocations. Это проверка конкретного сценария коротких списков, без обещаний
ускорения реального кадра. Миграция существующих потребителей на SmallVector пока
не выполнялась.

Движок собран в Debug с tracking ON для OpenGL и Vulkan. Прямой скрытый запуск
OpenGL завершился с кодом 0 и маркерами `SCENE_LOADING_SMOKE_OK` и `PREFAB_SMOKE_OK`:
проверены cache/isolation, hierarchy, duplicate/preflight validation, cancellation,
replacement, export/roundtrip/unpack и ссылки prefab. Vulkan завершился с кодом 0,
`RENDER_SMOKE_OK frames=4`, `MEMORY_SYSTEM_SMOKE`, без оставшихся GPU records/VMA
allocations при shutdown. Логи: `utils/.pack/utilities-loading.stdout.log`,
`utils/.pack/utilities-vk.stdout.log` и соответствующие `.stderr.log`.

Release microbenchmark выполнен; CSV сохранён в `utils/.pack/container-benchmarks.csv`.
Например, на 20 000 size_t новый ChunkedVector выполнил 212 storage allocations,
vector с reserve — одну. В этом прогоне непрерывный vector также обошёл данные
быстрее. ChunkedVector выбирается ради стабильности адресов и chunk API; результат
не является основанием массово заменять vector. Строковые lookup сохраняют
расход на hashing/блокировку пула, поэтому готовый Identifier нужно кешировать
у потребителя; измерение полного кадра до/после миграции не проводилось.

Clang AddressSanitizer с установленным MSVC runtime не прошёл сценарии исключений:
сбой в обработчике C++ exception воспроизведён отдельным минимальным примером
`std::vector<pair<int, ThrowingCopy>>` со стандартным allocator, без утилит движка.
Эта конфигурация не считается прошедшей проверку; санитарная проверка выполнена
через MSVC ASan. Минимальный пример сохранён локально в
`utils/.pack/asan-vector-repro.cpp`. Для запуска MSVC ASan DLL его compiler runtime
должна находиться в PATH, отдельно от runtime других версий Clang.

Linux/GCC, macOS/Metal, Android/Quest и Web в этой сессии не проверялись.
