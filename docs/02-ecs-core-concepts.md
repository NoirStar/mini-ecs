# 02. ECS 핵심 개념

## 이 문서의 목표

ECS의 세 가지 구성 요소를 **구체적인 예시**와 함께 이해합니다.

- Entity: 왜 "그냥 ID"인가?
- Component: 왜 "순수 데이터"만 가지는가?
- System: 왜 "로직만 분리"하는가?

---

## Entity: 정체성만 있는 존재

### 정의

> Entity = 고유 ID. 그 이상도 이하도 아님.

```cpp
// 가장 단순한 형태
using Entity = uint32_t;

Entity player = 1;
Entity enemy = 2;
Entity bullet = 3;
```

### 왜 이렇게 하는가?

**OOP 사고방식:**
```cpp
class Player {
    int hp;
    Position pos;
    Velocity vel;
    // Player"라서" 가지는 속성들
};
```

**ECS 사고방식:**
```cpp
Entity player = createEntity();
addComponent<Health>(player, {.hp = 100});
addComponent<Position>(player, {.x = 0, .y = 0});
addComponent<Velocity>(player, {.dx = 0, .dy = 0});
// player"에게" 부여하는 속성들
```

**차이점:**
- OOP: "Player가 무엇인가"를 클래스로 정의
- ECS: "이 Entity가 무엇을 가지는가"를 런타임에 결정

### 보안 도메인 예시

```cpp
Entity proc1 = createEntity();
addComponent<PID>(proc1, {.value = 1234});
addComponent<ProcessName>(proc1, {.name = "nginx"});
addComponent<CPUUsage>(proc1, {.percent = 45.2f});

// 나중에 이 프로세스가 의심스러워지면?
addComponent<ThreatScore>(proc1, {.score = 85});
// 클래스 수정 없이 속성 추가!
```

---

## Component: 순수 데이터 덩어리

### 정의

> Component = 데이터만 있는 구조체. 메서드 없음.

```cpp
struct Position {
    float x;
    float y;
};

struct Velocity {
    float dx;
    float dy;
};

struct Health {
    int current;
    int max;
};
```

### 왜 메서드가 없는가?

**OOP에서의 캡슐화:**
```cpp
class Health {
    int current_;
    int max_;
public:
    void takeDamage(int amount) {
        current_ = std::max(0, current_ - amount);
    }
    bool isDead() const { return current_ <= 0; }
};
```

**ECS에서는 이렇게 하지 않습니다.**

이유:
1. **"데미지를 받는다"는 로직이 Health에만 관련된 게 아닐 수 있음**
   - 방어력(Armor) Component가 있으면?
   - 무적(Invincible) Component가 있으면?
   - 로직이 여러 Component에 걸쳐 있음

2. **같은 데이터, 다른 처리**
   - 플레이어의 Health와 적의 Health는 데이터 구조는 같지만
   - 처리 로직(UI 표시, 사망 처리 등)은 다를 수 있음

3. **로직을 한 곳에서 관리**
   - 모든 "데미지 처리"는 DamageSystem에서
   - 모든 "죽음 처리"는 DeathSystem에서
   - 흩어져 있지 않아서 추적이 쉬움

### 보안 도메인 예시

```cpp
// 순수 데이터 Component들
struct PID {
    int value;
};

struct ProcessName {
    std::string name;
};

struct CPUUsage {
    float percent;
};

struct ThreatScore {
    int score;
    std::string reason;
};

struct Quarantined {
    std::chrono::system_clock::time_point since;
};

// ❌ 이렇게 하지 않음
struct ThreatScore {
    int score;
    void escalate() { score += 10; }  // 로직이 여기 있으면 안 됨
};
```

---

## System: 로직의 집

### 정의

> System = 특정 Component 조합을 가진 Entity들을 처리하는 함수/클래스

```cpp
// "Position과 Velocity를 가진 모든 Entity"를 처리
void MovementSystem(World& world) {
    for (auto [entity, pos, vel] : world.query<Position, Velocity>()) {
        pos.x += vel.dx * deltaTime;
        pos.y += vel.dy * deltaTime;
    }
}
```

### 핵심 특징

1. **System은 "어떤 Entity"인지 모름**
   - Player인지 Enemy인지 Bullet인지 관심 없음
   - 그냥 "Position과 Velocity가 있는 모든 것"을 처리

2. **관심사 분리**
   ```cpp
   MovementSystem:  Position + Velocity → 위치 업데이트
   RenderSystem:    Position + Sprite → 화면에 그리기
   CollisionSystem: Position + Collider → 충돌 검사
   ```

3. **순서 제어**
   ```cpp
   // 매 프레임 이 순서로 실행
   InputSystem();
   MovementSystem();
   CollisionSystem();
   DamageSystem();
   DeathSystem();
   RenderSystem();
   ```

### 보안 도메인 예시

```cpp
// CPU 사용량이 높은 프로세스를 감지하는 System
void HighCPUDetectionSystem(World& world) {
    for (auto [entity, pid, cpu] : world.query<PID, CPUUsage>()) {
        if (cpu.percent > 90.0f) {
            // 아직 ThreatScore가 없으면 추가
            if (!world.hasComponent<ThreatScore>(entity)) {
                world.addComponent<ThreatScore>(entity, {
                    .score = 30,
                    .reason = "High CPU usage"
                });
            }
        }
    }
}

// 위협 점수가 높은 Entity를 격리하는 System
void QuarantineSystem(World& world) {
    for (auto [entity, threat] : world.query<ThreatScore>()) {
        if (threat.score >= 80 && !world.hasComponent<Quarantined>(entity)) {
            world.addComponent<Quarantined>(entity, {
                .since = std::chrono::system_clock::now()
            });
            // 실제 격리 로직...
        }
    }
}
```

---

## 전체 그림

```
┌─────────────────────────────────────────────────────────────┐
│                          World                               │
│  ┌─────────────────────────────────────────────────────┐    │
│  │                    Entities                          │    │
│  │    [1]  [2]  [3]  [4]  [5]  ...                     │    │
│  └─────────────────────────────────────────────────────┘    │
│                                                              │
│  ┌─────────────────────────────────────────────────────┐    │
│  │                   Components                         │    │
│  │  Position:    [1: x,y] [3: x,y] [5: x,y]            │    │
│  │  Velocity:    [1: dx,dy] [3: dx,dy]                 │    │
│  │  Health:      [1: 100] [2: 50]                      │    │
│  │  ThreatScore: [2: 85] [4: 30]                       │    │
│  └─────────────────────────────────────────────────────┘    │
│                                                              │
│  ┌─────────────────────────────────────────────────────┐    │
│  │                    Systems                           │    │
│  │  MovementSystem:     query<Position, Velocity>      │    │
│  │  HealthSystem:       query<Health>                  │    │
│  │  ThreatSystem:       query<ThreatScore>             │    │
│  └─────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
```

---

## 생각해볼 질문

### Q1. Component 저장 방식

위 그림에서 Component들이 **Entity별**이 아니라 **Component 타입별**로 그룹화되어 있습니다.

```
Position: [Entity1의 Position, Entity3의 Position, Entity5의 Position]
```

이렇게 저장하면 어떤 이점이 있을까요?
- 힌트: CPU 캐시는 "연속된 메모리"를 좋아합니다.

### Q2. Query의 의미

`query<Position, Velocity>()`는 "Position **AND** Velocity를 가진 Entity"를 반환합니다.

이걸 효율적으로 구현하려면 어떻게 해야 할까요?
- 모든 Entity를 순회하며 "이 Entity가 Position 있나? Velocity 있나?" 체크?
- 더 나은 방법이 있을까요?

### Q3. System의 독립성

MovementSystem은 Position과 Velocity만 알면 됩니다.
Health가 있는지, ThreatScore가 있는지 모릅니다.

이게 왜 좋은 설계일까요?
- 힌트: 새로운 Component/System을 추가할 때 뭐가 달라지나요?

---

## 다음 단계

이 개념들이 이해되셨다면:
→ [03-data-oriented-design.md](./03-data-oriented-design.md) - 왜 이 구조가 성능에 좋은가?

---

## 체크리스트

- [ ] Entity가 "그냥 ID"인 이유를 설명할 수 있는가?
- [ ] Component에 메서드가 없는 이유를 설명할 수 있는가?
- [ ] System이 "특정 Component 조합"을 처리한다는 게 무슨 뜻인지 이해했는가?
- [ ] OOP 방식과 ECS 방식의 차이를 자신의 말로 설명할 수 있는가?
