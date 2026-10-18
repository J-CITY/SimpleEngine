# Пространственные структуры и collision pipeline

## Разделение

`utilsModule/spatial` содержит самостоятельные CPU-структуры без ECS, renderer, физических миров и сторонних библиотек:

- `SpatialGrid<N, Payload>` — ячейки и консервативные пространственные запросы, включая fallback для очень больших bounds.
- `Bvh<Volume, Payload, Traits>` и `AabbBvh<N, Payload>` — сбалансированная иерархия с владением узлами по значению. Payload может быть числом, handle или другим сравнимым значением, включая ноль.
- `ISpatialIndex<N>` и реализации `GridIndex`, `BvhIndex`, `BruteForceIndex` — подменяемые индексы proxy ID/bounds.
- `INeighborSearchFactory<N>` — создание отдельных индексов для разных particle jobs.

Общие `Aabb<N, Scalar>` находятся в `mathModule/geometry/aabb.h`. Координаты проверяются на конечность, bounds — на правильный порядок. Отрицательные координаты Grid используют `floor`, а не усечение к нулю. Grid отклоняет координаты за диапазоном целочисленных ячеек.

`collisionModule` содержит геометрию, shape views, manifold и narrow phase отдельно от физических тел. `COLLISION::Vec2` и `PHYSICS2D::Vec2` — aliases общего `MATH::Vector2f`, собственного векторного типа в collision нет. Его определение выделено из `mathModule/math.h` в лёгкий `mathModule/vector2.h`; поэтому 2D сохраняет зависимость только от стандартной библиотеки. Старый include `math.h` по-прежнему предоставляет Vector2 и его serde-сериализацию. 3D также использует `math.h` и его serde/json-зависимости. ECS, renderer и contact solver collision-модулю не нужны.

`physicsCommon/collisionPipeline.h` соединяет интерфейсы в конфигурацию мира. Физика хранит тела, применяет фильтры, материалы и реакцию solver. Renderer подключает BVH через `renderModule/spatial/frustumBvh.h`, где и остаётся знание о frustum и bounding spheres.

## Настройка мира

Конструкторы без параметров сохраняют Grid и текущие аналитические/SAT проверки. Новые конструкторы обоих миров принимают `Settings`, затем перемещаемый `CollisionPipeline`. Есть также конструктор только от pipeline. Прежний 3D-конструктор `PhysicWorld(maxContacts, iterations)` сохранён.

```cpp
#include "physics2dModule/physics2d.h"
using namespace IKIGAI;

PHYSICS2D::CollisionPipeline pipeline;
pipeline.broadPhase = std::make_unique<PHYSICS2D::BvhBroadPhase2D>();
pipeline.neighbors = std::make_unique<PHYSICS2D::BvhNeighborSearchFactory2D>();
pipeline.narrowPhase = std::make_unique<PHYSICS2D::DefaultNarrowPhase2D>();
PHYSICS2D::World world(PHYSICS2D::Settings{}, std::move(pipeline));
```

Для 3D используются `PHYSICS::CollisionPipeline`, `BvhBroadPhase3D`, `BvhNeighborSearchFactory3D`, `DefaultNarrowPhase3D` и `World`. Размер ячейки задаётся `world.settings.cellSize` в обоих движках.

Broad phase и поиск соседей можно выбирать независимо: например, BVH для rigid bodies и Grid для воды. Полный перебор rigid bodies доступен как `BruteForceBroadPhase2D/3D`. Для соседей полный перебор создаётся через `UTILS::SPATIAL::NeighborSearchFactory<N, UTILS::SPATIAL::BruteForceIndex<N>>`.

Каждый pipeline владеет стратегиями через `unique_ptr`; каждый particle job получает собственный индекс через фабрику. Null-стратегии и null-результат фабрики отклоняются. Выбор фиксируется при создании мира, замена во время симуляции не предусмотрена. Публичный `collisionPipeline()` возвращает const-конфигурацию для диагностики.

## Пользовательский narrow phase

```cpp
struct TracedNarrowPhase : IKIGAI::COLLISION::INarrowPhase2D {
    mutable std::size_t calls = 0;
    IKIGAI::COLLISION::DefaultNarrowPhase2D fallback;

    IKIGAI::COLLISION::Status collide(
        const IKIGAI::COLLISION::ShapeView2D& a,
        const IKIGAI::COLLISION::ShapeView2D& b,
        IKIGAI::COLLISION::Manifold2D& out) const override {
        ++calls;
        return fallback.collide(a, b, out); // здесь можно вызвать свой алгоритм
    }
};

IKIGAI::PHYSICS2D::CollisionPipeline pipeline;
pipeline.narrowPhase = std::make_unique<TracedNarrowPhase>();
IKIGAI::PHYSICS2D::World world(std::move(pipeline));
```

В 3D аналогичный интерфейс — `INarrowPhase3D`, `ShapeView3D`, `Manifold3D`. Views содержат геометрию и положение; 3D view получает ортонормированные world-space оси. Алгоритм не изменяет тела.

Результат — `Status::Separated`, `Status::Contact` или `Status::Unsupported`. Последний означает ошибку выбранной конфигурации, а не отсутствие контакта. Мир проверяет результаты перед записью в solver:

- общая нормаль единичная, направлена A→B;
- точки контакта находятся в мировых координатах;
- глубина проникновения конечна и неотрицательна;
- 2D manifold содержит 1–2 точки и общую глубину, 3D — 1–4 точки с глубиной каждой точки.

Для legacy 3D solver адаптер меняет направление нормали на B→A и добавляет ссылки на тела/материалы. Переполнение заданного 3D contact buffer вызывает явную ошибку `3D contact buffer exhausted` вместо молчаливой потери контактов.

Оба `DefaultNarrowPhase` поддерживают `setHandler(kindA, kindB, handler)`: можно переопределить одну упорядоченную пару форм. Обратная пара регистрируется отдельно. Остальные пары продолжают использовать аналитические проверки/SAT. Поддерживаются существующие формы: circle/convex polygon в 2D, sphere/box в 3D. Добавление нового вида формы требует расширения геометрической схемы; интерфейс сам по себе этого не делает. GJK/EPA и CCD не добавлены.

Подмена применяется к первичному поиску rigid-контактов, повторной проверке при 2D position solve и particle-rigid контакту обоих миров. Self-collision ткани использует отдельные particle/face/edge геометрические операции; rigid narrow phase их не заменяет.

## Контракт spatial backend

`IBroadPhase2D/3D` — aliases соответствующего `ISpatialIndex<N>`. Реализация поддерживает `reset(cellSize)`, `upsert(id,bounds)`, `remove(id)`, `query(bounds,out)` и `pairs(out)`. Reset удаляет все proxy; cell size — подсказка Grid, которую другие backends могут игнорировать. Выходные буферы дополняются, их очистка — обязанность вызывающего кода.

Кандидаты могут быть избыточными, но не должны пропускать пересекающиеся bounds. Мир канонизирует пары, убирает self-pairs/дубли, сортирует их и проверяет существование proxy. Затем он независимо применяет layer/mask, static-static и joint exclusions. Сторонняя стратегия не отменяет фильтры физики.

Миры пересобирают rigid index на каждом substep. Particle-индексы строятся на границах соответствующих solver iterations/jobs; изменения положений, создающие новые self-контакты в середине итерации, учитываются следующей итерацией. Для particle-rigid coupling движущиеся rigid proxy обновляются сразу после коррекции, чтобы следующие частицы видели новые bounds.

BVH сейчас использует пакетную перестройку: `upsert/remove` инвалидируют дерево, следующий query строит сбалансированное дерево по медиане. Это корректный backend с обновлением bounds, но не реализация dynamic tree с локальными rotations/refit. Индексы не синхронизируют конкурентные mutations и queries; каждый мир/job владеет своим экземпляром.

## Фабрики сцен

```cpp
IKIGAI::SCENE_SYSTEM::PhysicsFactories factories;
factories.physics2d = [] {
    IKIGAI::PHYSICS2D::CollisionPipeline pipeline;
    pipeline.broadPhase = std::make_unique<IKIGAI::PHYSICS2D::BvhBroadPhase2D>();
    return std::make_unique<IKIGAI::PHYSICS2D::World>(std::move(pipeline));
};
IKIGAI::SCENE_SYSTEM::Scene scene(nullptr, factories);
// Scene создаёт свой мир при первом physics2d()/stepPhysics2D().
```

`physics3d` настраивается аналогично. Поля фабрик доступны при включённом соответствующем `USE_PHYSICS_*`. Фабрика передаёт исключительное владение новым миром сцене; ECS переводит его в shared ownership для runtime leases. Пустая фабрика использует прежний default.

`Scene::setPhysicsFactories` можно вызвать до создания её физических миров. После этого изменение отклоняется, чтобы не оставить компоненты с lease старого мира. `SceneManager(root, factories)` применяет настройки к создаваемым сценам, включая async loading. `setPhysicsFactoriesForNewScenes` меняет только будущие сцены; текущую сцену он не перенастраивает. Настройки и runtime-указатели не сериализуются.

## Сборка и переход

```cmake
include(path/to/IkigaiEngine/cmake/Collision.cmake)
ikigai_add_spatial_library()
target_link_libraries(my_app PRIVATE ikigai_spatial)
# Геометрия без физических миров:
ikigai_add_collision_library(2) # либо 3
target_link_libraries(my_app PRIVATE ikigai_collision2d)
```

Флаги физики не нужны для использования spatial/collision targets. Выключенные физические миры не подключают свои ядра к движку. Условное сохранение физического `broadPhase.cpp` при `USE_PHYSICS_3D=OFF` убрано.

Старые `physics2dModule/spatialGrid.h` и `physicsModule/spatialGrid.h` сохраняют vector API как адаптеры. Старый renderer-зависимый `PHYSICS::BVHTree` с `GetNodeToDraw` удалён; его пользователь в Scene переведён на `RENDER::FrustumBvh`, нейтральный `Bvh::query(predicate,visitor)` и `frustumPredicate`. `physicsModule/broadPhase.h` оставлен forwarding include на новые утилиты.

## Проверки и измерения

```powershell
cmake -S utils/tests/physics -B utils/.pack/physics-tests
cmake --build utils/.pack/physics-tests --config Debug --parallel 4
ctest --test-dir utils/.pack/physics-tests -C Debug --output-on-failure
utils/.pack/physics-tests/Debug/ikigai_collision_benchmark.exe
```

`spatial_utilities` линкуется только с header-only spatial target и компилируется с обоими физическими флагами 0. Проверяет полноту кандидатов всех трёх индексов относительно полного перебора, запросы, большие/невалидные bounds, ID 0, удаление, обновление, copy/move и pruning predicate.

`collision_pipeline` проверяет вызов внедрённых narrow phase и neighbor factories, повторный 2D position solve, particle-rigid coupling, Unsupported/null, канонизацию дублированных пар, floor/cloth/fluid на всех backends и сравнение новой 3D-геометрии с прежним детектором на 1200 случайных парах. Остальные физические регрессии сохранены. ECS smoke создают миры сцен через пользовательские фабрики с BVH и проверяют lifecycle/изоляцию.

Benchmark отдельно от CTest, без временных assertions. Выводит CSV: median/p95 полного шага и среднее число rigid-кандидатов/контактов. Для каждого случая 3 шага прогрева и 30 измеряемых шагов; rigid/mixed — 32 тела и пол, cloth — 8×8 частиц, water — 64 частицы. Это малые CPU-сцены, не предел производительности движка. Результаты Debug с одновременно выполняющейся сборкой не заменяют Release profiling; peak memory и allocations этим harness не измеряются.

Логи проверки этого рефакторинга находятся в `utils/.pack/collision-*`; первичный CSV — `collision-benchmark-debug.csv`. Выбор default оставлен прежним. Измерения малых сцен показывают, что полный перебор иногда дешевле дерева, а Grid с текущим cell size может выдавать значительно больше кандидатов. Траектории между разными backend не обязаны совпадать побитово.

Проверка 18 октября 2026, Windows / MSVC Debug / OpenGL: независимые CTest 11/11 прошли, включая некорректный пользовательский manifold и переполнение contact buffer; полные сборки движка с обоими модулями ON и OFF прошли. Оба ECS smoke прошли с фабриками сцен/BVH, `ikigai_physics2d_disabled_check` скомпилирован. Render smoke без обоих модулей прошёл. Финальная рабочая конфигурация оставлена `USE_PHYSICS_2D=ON`, `USE_PHYSICS_3D=ON`.

Демонстрации cloth и soft cube также проверены по 5 кадров с явно включённым `GeometryBVHCulling`. GPU depth readback подтвердил непустую геометрию: cloth min=0.119152, variance=0.00160635; soft min=0.119119, variance=0.000363557. Color/depth captures просмотрены, ошибок OpenGL не обнаружено. Логи: `collision-bvh-cloth-render.log`, `collision-bvh-soft-render.log`, `collision-bvh-cloth-depth.log`, `collision-bvh-soft-depth.log`. Генерируемые проверочные сцены удалены после запуска, исходные demo JSON не изменялись.
