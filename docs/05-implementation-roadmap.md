# 05. 구현 로드맵

## 이 문서의 목표

ECS를 **단계별로** 구현하면서 모던 C++을 익힙니다.
각 단계에서 **왜 이 기능이 필요한지** 체감하고, **직접 부딪혀서** 배웁니다.

---

## 전체 로드맵

```
Phase 1: 동작하게 만들기 (Week 1-2)
├── Step 1.1: Entity 관리자
├── Step 1.2: Component 저장소 (단일 타입)
├── Step 1.3: Component 저장소 (다중 타입) ← variadic templates
└── Step 1.4: 기본 Query ← fold expressions

Phase 2: 안전하게 만들기 (Week 3)
├── Step 2.1: Component concept 정의
├── Step 2.2: Query 타입 제약
└── Step 2.3: 컴파일 타임 에러 개선

Phase 3: 빠르게 만들기 (Week 4+)
├── Step 3.1: Archetype 도입
├── Step 3.2: constexpr 타입 ID
├── Step 3.3: 캐시 친화적 저장소
└── Step 3.4: 벤치마킹
```

---

## Phase 1: 동작하게 만들기

### Step 1.1: Entity 관리자

**목표:** Entity 생성/삭제

**배우는 것:** 없음 (워밍업)

**구현할 것:**
```cpp
class EntityManager {
public:
    Entity create();           // 새 Entity 생성, ID 반환
    void destroy(Entity e);    // Entity 삭제
    bool isAlive(Entity e);    // 유효한 Entity인가?
};
```

**생각할 질문:**
- Entity ID를 재사용할 것인가?
- 재사용한다면, 삭제된 Entity와 어떻게 구분할 것인가?
- (힌트: "generation" 개념)

---

### Step 1.2: Component 저장소 (단일 타입)

**목표:** 한 종류의 Component를 저장/조회

**배우는 것:** 기본 구조 설계

**구현할 것:**
```cpp
template<typename T>
class ComponentStorage {
public:
    void add(Entity e, T component);
    void remove(Entity e);
    T* get(Entity e);           // 없으면 nullptr
    bool has(Entity e);
};
```

**생각할 질문:**
- 내부 저장소로 `std::vector<T>` vs `std::unordered_map<Entity, T>`?
- 각각의 trade-off는?

---

### Step 1.3: Component 저장소 (다중 타입)

**목표:** 여러 종류의 Component를 한 World에서 관리

**배우는 것:** `std::tuple`, variadic templates

**문제 상황:**
```cpp
// 이렇게 하고 싶은데...
world.addComponent<Position>(entity, pos);
world.addComponent<Health>(entity, hp);
world.addComponent<Velocity>(entity, vel);

// World가 Position, Health, Velocity 저장소를 각각 가져야 함
// 하지만 Component 종류가 컴파일 타임에 정해져야 함
```

**구현할 것:**
```cpp
template<typename... Components>
class World {
    std::tuple<ComponentStorage<Components>...> storages;
    
public:
    template<typename C>
    void addComponent(Entity e, C component);
    
    template<typename C>
    C* getComponent(Entity e);
};
```

**핵심 과제:**
- `std::tuple`에서 특정 타입의 저장소를 어떻게 꺼내는가?
- `std::get<ComponentStorage<Position>>(storages)` 사용법

---

### Step 1.4: 기본 Query

**목표:** 특정 Component 조합을 가진 Entity들 조회

**배우는 것:** fold expressions

**구현할 것:**
```cpp
// 사용 예시
for (auto [entity, pos, vel] : world.query<Position, Velocity>()) {
    pos.x += vel.dx;
    pos.y += vel.dy;
}
```

**단계별 접근:**

1. 먼저 단순하게:
```cpp
template<typename... Cs>
std::vector<Entity> queryEntities() {
    std::vector<Entity> result;
    for (Entity e : allEntities) {
        if (hasComponent<Cs>(e) && ...) {  // fold expression!
            result.push_back(e);
        }
    }
    return result;
}
```

2. 그 다음 Component 참조까지:
```cpp
template<typename... Cs>
auto query() {
    // Entity와 함께 Component 참조도 반환
    // → std::tuple<Entity, Cs&...> 또는 structured binding 지원
}
```

---

## Phase 2: 안전하게 만들기

### Step 2.1: Component Concept

**목표:** "Component로 쓸 수 있는 타입"을 정의

**배우는 것:** concepts, type_traits

**문제 상황:**
```cpp
// 이런 실수를 컴파일 타임에 잡고 싶음
world.addComponent<int*>(entity, ptr);      // 포인터는 안 됨
world.addComponent<std::mutex>(entity, m);  // 복사 불가능한 건 안 됨
```

**구현할 것:**
```cpp
template<typename T>
concept Component = 
    std::is_copy_constructible_v<T> &&
    std::is_default_constructible_v<T> &&
    !std::is_pointer_v<T> &&
    !std::is_reference_v<T>;
```

---

### Step 2.2: Query 타입 제약

**목표:** Query에 잘못된 타입 조합 방지

**배우는 것:** requires 절, concept 조합

**구현할 것:**
```cpp
// World에 등록되지 않은 Component를 query하면 컴파일 에러
template<Component... Cs>
    requires (contains_type<Cs, Components> && ...)  // 모든 Cs가 World의 Components에 있어야 함
auto query();
```

---

### Step 2.3: 컴파일 타임 에러 개선

**목표:** 에러 메시지를 사람이 읽을 수 있게

**배우는 것:** static_assert 메시지, concept 진단

**Before:**
```
error: no matching function for call to 'World<Position, Velocity>::addComponent<std::mutex>'
  in instantiation of ... (100줄)
```

**After:**
```
error: 'std::mutex' does not satisfy 'Component'
note: because 'std::is_copy_constructible_v<std::mutex>' is false
```

---

## Phase 3: 빠르게 만들기

### Step 3.1: Archetype 도입

**목표:** 같은 Component 조합끼리 묶어서 저장

**배우는 것:** 메모리 레이아웃 설계, 타입 ID

**현재 문제:**
```cpp
// Entity마다 hasComponent 체크 → O(n) * O(component 수)
for (Entity e : allEntities) {
    if (hasComponent<Position>(e) && hasComponent<Velocity>(e)) {
        // ...
    }
}
```

**목표 구조:**
```cpp
// Archetype 단위로 순회 → if 없음!
for (auto& archetype : archetypesWithComponents<Position, Velocity>()) {
    for (size_t i = 0; i < archetype.size(); i++) {
        auto& pos = archetype.get<Position>(i);
        auto& vel = archetype.get<Velocity>(i);
        // ...
    }
}
```

---

### Step 3.2: constexpr 타입 ID

**목표:** 타입을 숫자 ID로 변환 (런타임 오버헤드 없이)

**배우는 것:** constexpr, template 특수화

**구현할 것:**
```cpp
template<typename T>
constexpr size_t typeId();

static_assert(typeId<Position>() != typeId<Velocity>());
```

---

### Step 3.3: 캐시 친화적 저장소

**목표:** Component 데이터가 연속 메모리에 저장되도록

**배우는 것:** 메모리 정렬, SoA(Structure of Arrays)

**측정할 것:**
- 캐시 미스 횟수
- 순회 시간

---

### Step 3.4: 벤치마킹

**목표:** 실제로 빨라졌는지 확인

**배우는 것:** `<chrono>`, 성능 측정 방법론

**측정 시나리오:**
```cpp
// 10만 Entity, Position + Velocity 업데이트
// Phase 1 구현 vs Phase 3 구현 비교
```

---

## 시작하기

### 프로젝트 구조

```
mini-ecs/
├── docs/                    # 지금 이 문서들
├── include/
│   └── mini-ecs/
│       ├── entity.hpp       # Step 1.1
│       ├── component.hpp    # Step 1.2, 1.3
│       ├── query.hpp        # Step 1.4
│       ├── concepts.hpp     # Step 2.1, 2.2
│       ├── archetype.hpp    # Step 3.1
│       └── world.hpp        # 전체 통합
├── tests/                   # 각 Step별 테스트
├── benchmarks/              # Phase 3용
└── CMakeLists.txt
```

---

## 진행 방식

각 Step마다:

1. **문제 정의**: 무엇을 해결하려는가?
2. **설계 논의**: 어떤 방식이 있는가? trade-off는?
3. **직접 구현**: 코드 작성
4. **리뷰**: 이게 모던한가? 더 나은 방법은?
5. **테스트**: 동작 확인
6. **다음 단계**: 현재 구현의 한계 → 다음 Step 동기

---

## 첫 번째 과제: Step 1.1

Entity 관리자를 설계해보세요.

**요구사항:**
- `Entity create()` - 새 Entity 생성
- `void destroy(Entity e)` - Entity 삭제  
- `bool isAlive(Entity e)` - 유효성 확인

**생각할 것:**
1. `Entity`를 어떤 타입으로 정의할 것인가?
2. 삭제된 Entity의 ID를 재사용할 것인가?
3. 재사용한다면, "예전 Entity 42"와 "새 Entity 42"를 어떻게 구분할 것인가?

**제출할 것:**
- 설계 아이디어 (코드 없이 말로)
- 왜 그렇게 설계했는지 이유

코드는 설계가 확정된 후에 작성합니다.
