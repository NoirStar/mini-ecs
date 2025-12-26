# 04. 모던 C++과 ECS의 연결

## 이 문서의 목표

ECS를 구현할 때 **왜** 특정 모던 C++ 기능이 필요한지 이해합니다.
"이런 기능이 있다"가 아니라 "이 문제를 풀려면 이게 필요하다"를 체감합니다.

---

## 문제 → 해법 매핑

| ECS 구현 문제 | 모던 C++ 해법 |
|---------------|---------------|
| 임의 개수의 Component 타입 처리 | variadic templates |
| 여러 Component를 하나로 묶기 | `std::tuple` |
| 컴파일 타임에 타입별 분기 | `if constexpr` |
| 타입의 특성 검사 | type traits |
| Query에 잘못된 타입 사용 방지 | concepts |
| 런타임 오버헤드 없는 타입 정보 | `constexpr` |
| 여러 타입에 동시에 연산 적용 | fold expressions |

---

## 1. Variadic Templates: 임의 개수의 타입

### 문제

```cpp
// 이렇게 쓰고 싶음
world.addComponents<Position, Velocity, Health>(entity, pos, vel, hp);
world.query<Position, Velocity>();  // 2개
world.query<Position, Health, ThreatScore>();  // 3개
```

Component 개수가 정해져 있지 않습니다. 어떻게 받을까요?

### 해법: Variadic Templates

```cpp
// ... 는 "0개 이상의 타입"을 받음
template<typename... Components>
void addComponents(Entity entity, Components&&... components) {
    // 구현
}

// 사용
addComponents<Position, Velocity>(entity, pos, vel);  // 2개
addComponents<Position>(entity, pos);                  // 1개
```

**핵심:**
- `typename... Components` → 타입 파라미터 팩
- `Components&&... components` → 함수 파라미터 팩

---

## 2. std::tuple: 여러 타입을 하나로

### 문제

Archetype은 "Position, Velocity, Health"처럼 **타입 조합**을 표현해야 합니다.

### 해법: std::tuple

```cpp
// 타입 조합을 타입으로 표현
using ArchetypeA = std::tuple<Position, Velocity, Health>;
using ArchetypeB = std::tuple<Position, Velocity>;

// 실제 데이터 저장
std::tuple<Position, Velocity, Health> data = {pos, vel, hp};

// 특정 타입 접근
auto& position = std::get<Position>(data);
auto& health = std::get<Health>(data);
```

---

## 3. if constexpr: 컴파일 타임 분기

### 문제

"이 Archetype이 Health를 포함하는가?"를 **컴파일 타임**에 확인하고 싶습니다.

### 왜 일반 if가 안 되는가?

```cpp
template<typename T>
void process(T& component) {
    if (std::is_same_v<T, Health>) {
        component.hp -= 10;  // ❌ T가 Position이면 컴파일 에러!
    }
}
```

`if`는 런타임 분기라서, **양쪽 브랜치 모두 컴파일**됩니다.
Position에는 `.hp`가 없으니 컴파일 실패.

### 해법: if constexpr

```cpp
template<typename T>
void process(T& component) {
    if constexpr (std::is_same_v<T, Health>) {
        component.hp -= 10;  // ✅ T가 Health일 때만 컴파일됨
    } else if constexpr (std::is_same_v<T, Position>) {
        component.x = 0;     // ✅ T가 Position일 때만 컴파일됨
    }
}
```

**핵심:** `if constexpr`은 조건이 false인 브랜치를 **아예 버림**

---

## 4. Type Traits: 타입의 특성 검사

### 문제

"이 타입이 Component로 쓰기 적합한가?" 검사가 필요합니다.

- 복사 가능해야 함
- 기본 생성자가 있어야 함
- POD(Plain Old Data)인지 확인

### 해법: Type Traits

```cpp
#include <type_traits>

template<typename T>
void addComponent(Entity e, T component) {
    // 컴파일 타임에 검사
    static_assert(std::is_copy_constructible_v<T>, 
                  "Component must be copyable");
    static_assert(std::is_default_constructible_v<T>, 
                  "Component must have default constructor");
    
    // 구현...
}
```

**자주 쓰는 type traits:**
```cpp
std::is_same_v<T, U>           // T와 U가 같은 타입?
std::is_base_of_v<Base, T>     // T가 Base를 상속?
std::is_trivially_copyable_v<T> // memcpy로 복사 가능?
std::is_empty_v<T>             // 빈 구조체? (태그 Component용)
```

---

## 5. Concepts: 타입 제약을 명시적으로

### 문제

Type traits + static_assert는 **에러 메시지가 끔찍합니다.**

```
error: static assertion failed: Component must be copyable
  in instantiation of 'void addComponent<BadType>(Entity, BadType)'
  ... 50줄의 템플릿 스택 ...
```

### 해법: Concepts (C++20)

```cpp
// Component가 되려면 만족해야 할 조건
template<typename T>
concept Component = std::is_copy_constructible_v<T> 
                 && std::is_default_constructible_v<T>
                 && !std::is_pointer_v<T>;

// 사용
template<Component T>  // T는 Component concept을 만족해야 함
void addComponent(Entity e, T component) {
    // ...
}

// 에러 메시지가 명확해짐
// error: 'BadType' does not satisfy 'Component'
```

### 더 복잡한 concept 예시

```cpp
// System이 되려면: World를 받아서 실행 가능해야 함
template<typename T>
concept System = requires(T sys, World& world) {
    { sys.update(world) } -> std::same_as<void>;
};

// Query 가능한 타입
template<typename... Ts>
concept Queryable = (Component<Ts> && ...);  // 모든 Ts가 Component여야 함

template<Queryable... Components>
auto query() {
    // ...
}
```

---

## 6. Fold Expressions: 파라미터 팩에 연산 적용

### 문제

"모든 Component 타입에 대해 뭔가를 수행"해야 합니다.

```cpp
template<typename... Components>
bool hasAllComponents(Entity e) {
    // Position 있나? && Velocity 있나? && Health 있나? ...
}
```

### 해법: Fold Expressions (C++17)

```cpp
template<typename... Components>
bool hasAllComponents(Entity e) {
    // (... && expr) → expr1 && expr2 && expr3 ...
    return (hasComponent<Components>(e) && ...);
}

template<typename... Components>
void removeAllComponents(Entity e) {
    // (expr, ...) → expr1, expr2, expr3 (순차 실행)
    (removeComponent<Components>(e), ...);
}
```

**Fold 패턴:**
```cpp
(... && pack)   // pack1 && pack2 && pack3
(... || pack)   // pack1 || pack2 || pack3
(... + pack)    // pack1 + pack2 + pack3
(func(pack), ...) // func(pack1), func(pack2), func(pack3)
```

---

## 7. constexpr: 컴파일 타임 계산

### 문제

타입 ID를 런타임에 계산하면 오버헤드가 있습니다.

```cpp
// 런타임 - 느림
size_t getTypeId(const std::type_info& info) {
    return std::hash<std::string>{}(info.name());
}
```

### 해법: constexpr

```cpp
// 컴파일 타임에 타입 ID 생성
template<typename T>
struct TypeId {
    static constexpr size_t value = /* 컴파일 타임 계산 */;
};

// 사용
constexpr size_t positionId = TypeId<Position>::value;  // 컴파일 타임에 결정
```

**실제 구현 예시 (단순화):**
```cpp
inline size_t nextTypeId = 0;

template<typename T>
size_t getTypeId() {
    static size_t id = nextTypeId++;  // 타입당 한 번만 실행
    return id;
}
```

---

## 실제 코드 예시: 조합해서 사용

```cpp
// Concept 정의
template<typename T>
concept Component = std::is_copy_constructible_v<T>;

// 여러 기능 조합
template<Component... Cs>
class Archetype {
    std::tuple<std::vector<Cs>...> storage;  // 각 Component별 vector
    
public:
    template<Component C>
    auto& getStorage() {
        return std::get<std::vector<C>>(storage);
    }
    
    template<Component C>
    bool contains() const {
        return (std::is_same_v<C, Cs> || ...);  // fold expression
    }
    
    void addEntity(Entity e, Cs&&... components) {
        // 각 storage에 component 추가
        (getStorage<Cs>().push_back(std::forward<Cs>(components)), ...);
    }
};
```

---

## 생각해볼 질문

### Q1. 왜 컴파일 타임인가?

많은 기능이 "컴파일 타임"에 처리됩니다. 왜 중요할까요?

힌트: 게임은 매 프레임 16ms 안에 모든 처리를 해야 합니다.

### Q2. Concepts vs SFINAE

C++20 이전에는 concepts 대신 SFINAE를 썼습니다.
Concepts가 왜 더 나은가요?

### Q3. 어떤 기능부터?

이 프로젝트에서 어떤 기능부터 구현해볼까요?

- (A) 기본: variadic templates + tuple + fold expressions
- (B) 타입 안전성: concepts + type traits
- (C) 성능: constexpr + if constexpr

---

## 다음 단계

→ [05-implementation-roadmap.md](./05-implementation-roadmap.md) - 단계별 구현 계획

---

## 체크리스트

- [ ] Variadic templates로 임의 개수 타입을 받는 방법을 이해했는가?
- [ ] `if constexpr`과 일반 `if`의 차이를 설명할 수 있는가?
- [ ] Concepts가 왜 필요한지 이해했는가?
- [ ] Fold expressions로 파라미터 팩을 처리하는 방법을 알았는가?
