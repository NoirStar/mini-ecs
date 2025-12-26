# Variadic Templates 학습 노트

## 핵심 개념

**Variadic Templates = 임의 개수의 타입을 받는 템플릿**

```cpp
template<typename... Ts>  // Ts = 0개 이상의 타입
```

---

## 기본 문법

### 1. 선언 (패킹)

```cpp
template<typename T>       // 타입 1개
template<typename... Ts>   // 타입 0개 이상

template<typename First, typename... Rest>  // 최소 1개 + 나머지
```

### 2. 사용 (언패킹)

```cpp
template<typename... Ts>
class Foo {
    std::tuple<Ts...> data;  // Ts...가 펼쳐짐
};

Foo<int, float, string> f;
// → std::tuple<int, float, string> data;
```

---

## 패턴: 타입 변환하며 펼치기

```cpp
template<typename... Components>
class World {
    std::tuple<ComponentStorage<Components>...> storages_;
};

World<Position, Health, Velocity> world;
```

**컴파일러가 하는 일:**

```
Components = Position, Health, Velocity

ComponentStorage<Components>...
= ComponentStorage<Position>, ComponentStorage<Health>, ComponentStorage<Velocity>

std::tuple<ComponentStorage<Components>...>
= std::tuple<
    ComponentStorage<Position>,
    ComponentStorage<Health>,
    ComponentStorage<Velocity>
  >
```

---

## 패턴: 펼치기 위치

`...`는 **왼쪽 표현식 전체**에 적용됩니다.

```cpp
// 패턴 1: 타입 그대로
std::tuple<Ts...>
// int, float → std::tuple<int, float>

// 패턴 2: 변환 적용
std::tuple<std::vector<Ts>...>
// int, float → std::tuple<std::vector<int>, std::vector<float>>

// 패턴 3: 템플릿 적용
std::tuple<ComponentStorage<Ts>...>
// Position, Health → std::tuple<ComponentStorage<Position>, ComponentStorage<Health>>
```

---

## 왜 강력한가?

### 런타임 다형성 (전통적 방식)

```cpp
class IStorage { virtual void add(...) = 0; };
class PositionStorage : public IStorage { ... };
class HealthStorage : public IStorage { ... };

std::vector<IStorage*> storages;  // 가상 함수 호출 비용
```

### 컴파일 타임 다형성 (variadic templates)

```cpp
std::tuple<
    ComponentStorage<Position>,
    ComponentStorage<Health>
> storages;  // 가상 함수 없음, 인라인 가능
```

| | 런타임 다형성 | 컴파일 타임 다형성 |
|---|--------------|-------------------|
| 유연성 | 런타임에 타입 추가 가능 | 컴파일 타임에 고정 |
| 성능 | 가상 함수 오버헤드 | 오버헤드 없음 |
| 타입 안전성 | 런타임 에러 가능 | 컴파일 타임에 체크 |

---

## ECS에서의 활용

```cpp
template<typename... Components>
class World {
    EntityManager entityManager_;
    std::tuple<ComponentStorage<Components>...> storages_;
    
public:
    // 특정 타입의 Storage 꺼내기
    template<typename C>
    ComponentStorage<C>& getStorage() {
        return std::get<ComponentStorage<C>>(storages_);
    }
    
    // 특정 타입의 Component 추가
    template<typename C>
    void addComponent(Entity e, C component) {
        getStorage<C>().add(e, std::move(component));
    }
};
```

---

## 기억할 것

1. `typename... Ts` = 타입 여러 개 받기 (파라미터 팩)
2. `Ts...` = 받은 타입들 펼치기
3. `Something<Ts>...` = 각 타입에 Something 적용 후 펼치기
4. `std::tuple`과 함께 쓰면 여러 타입을 하나로 관리 가능
5. 런타임 비용 없음 (컴파일 타임에 결정)

---

## 관련 기능 (나중에 배울 것)

- **Fold expressions**: 파라미터 팩에 연산 적용
- **if constexpr**: 컴파일 타임 분기
- **concepts**: 타입 제약 조건
