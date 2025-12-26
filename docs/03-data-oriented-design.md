# 03. 데이터 지향 설계 (Data-Oriented Design)

## 이 문서의 목표

ECS가 왜 **성능**에서 유리한지 이해합니다.
핵심 개념: **CPU 캐시를 어떻게 활용하느냐**

---

## 먼저: 왜 성능을 신경 써야 하나?

"요즘 컴퓨터 빠른데, 그냥 짜면 안 되나?"

### 현실

```
CPU 속도:        1 사이클 = 0.3ns
L1 캐시 접근:    ~1ns      (3 사이클)
L2 캐시 접근:    ~4ns      (12 사이클)
L3 캐시 접근:    ~12ns     (40 사이클)
메인 메모리:     ~100ns    (300 사이클) ← CPU가 300번 놀고 있음
```

**메모리 접근 한 번에 CPU가 300사이클을 기다립니다.**

코드를 아무리 최적화해도, 메모리 접근 패턴이 나쁘면 느립니다.

---

## OOP vs DOD: 메모리 배치

### 시나리오

1000개의 게임 오브젝트가 있고, 매 프레임 위치를 업데이트합니다.

### OOP 방식

```cpp
class GameObject {
    uint64_t id;           // 8 bytes
    std::string name;      // 32 bytes (SSO)
    Position pos;          // 8 bytes
    Velocity vel;          // 8 bytes
    Health health;         // 8 bytes
    Sprite sprite;         // 24 bytes
    // ... 총 ~128 bytes
};

std::vector<GameObject> objects;  // 1000개
```

**메모리 배치:**
```
[GameObject 0: id|name|pos|vel|health|sprite|...]  128B
[GameObject 1: id|name|pos|vel|health|sprite|...]  128B
[GameObject 2: id|name|pos|vel|health|sprite|...]  128B
...
```

**위치 업데이트 시:**
```cpp
for (auto& obj : objects) {
    obj.pos.x += obj.vel.dx;  // pos, vel만 필요
    obj.pos.y += obj.vel.dy;
}
```

문제:
- 128바이트 중 16바이트(pos + vel)만 사용
- 캐시 라인(64B)에 GameObject 0.5개만 들어감
- **캐시 효율: 12.5%**

### DOD/ECS 방식

```cpp
// Component별로 분리 저장
std::vector<Position> positions;   // 1000개, 연속
std::vector<Velocity> velocities;  // 1000개, 연속
std::vector<Health> healths;       // 1000개, 연속
```

**메모리 배치:**
```
positions:  [pos0|pos1|pos2|pos3|pos4|pos5|pos6|pos7|...]
             8B   8B   8B   8B   8B   8B   8B   8B
            |<------------ 64B 캐시 라인 ------------>|

velocities: [vel0|vel1|vel2|vel3|vel4|vel5|vel6|vel7|...]
```

**위치 업데이트 시:**
```cpp
for (size_t i = 0; i < positions.size(); i++) {
    positions[i].x += velocities[i].dx;
    positions[i].y += velocities[i].dy;
}
```

- 캐시 라인 하나에 Position 8개
- 필요한 데이터만 읽음
- **캐시 효율: 100%**

---

## 실제 성능 차이

### 벤치마크 예시

```
Entity 수: 100,000개
작업: Position += Velocity (매 프레임)

OOP 방식:     15.2ms
ECS 방식:      1.8ms

→ 약 8배 차이
```

Entity가 많아질수록 격차가 커집니다.

### 왜 이런 차이가 나는가?

```
OOP: 캐시 미스 ~100,000번 (거의 매번)
ECS: 캐시 미스 ~12,500번 (8개당 1번)

캐시 미스 1번 = ~300 사이클 낭비
절약된 사이클 = 87,500 × 300 = 26,250,000 사이클
```

---

## Archetype: 더 똑똑한 저장 방식

### 문제

모든 Entity가 같은 Component를 가지진 않습니다.

```
Entity 1: Position, Velocity, Health
Entity 2: Position, Velocity          (Health 없음)
Entity 3: Position, Health            (Velocity 없음)
```

### 해결: Archetype

**같은 Component 조합**을 가진 Entity들을 묶습니다.

```
Archetype A: [Position, Velocity, Health]
    Entity 1의 데이터들

Archetype B: [Position, Velocity]
    Entity 2의 데이터들
    Entity 5의 데이터들
    Entity 8의 데이터들
    ...
    
Archetype C: [Position, Health]
    Entity 3의 데이터들
```

**각 Archetype 내부는 연속 메모리:**

```
Archetype B 내부:
  positions:  [pos2, pos5, pos8, ...]  연속!
  velocities: [vel2, vel5, vel8, ...]  연속!
```

### Query 효율

`query<Position, Velocity>()` 호출 시:

```cpp
// Archetype A 순회 (Position, Velocity 있음 ✓)
// Archetype B 순회 (Position, Velocity 있음 ✓)
// Archetype C는 건너뜀 (Velocity 없음)
```

- if문으로 Entity마다 검사? ❌
- Archetype 단위로 한 번만 확인 ✓

---

## 모던 C++과의 연결

이 구조를 구현하려면 다음이 필요합니다:

| 요구사항 | 모던 C++ 기능 |
|----------|---------------|
| "Position, Velocity, Health"를 타입으로 표현 | `std::tuple`, variadic templates |
| Archetype이 어떤 Component를 가지는지 컴파일 타임에 확인 | `if constexpr`, type traits |
| Query가 올바른 타입만 반환하도록 강제 | concepts |
| 런타임 오버헤드 없이 타입 정보 처리 | `constexpr`, 템플릿 메타프로그래밍 |

---

## 생각해볼 질문

### Q1. Trade-off

ECS/DOD가 항상 좋은 건 아닙니다.

```cpp
// 이 경우는 OOP가 나을 수 있음
player.takeDamage(10);
player.checkDeath();
player.playSound();
player.updateUI();
```

한 Entity의 여러 Component를 **동시에** 접근해야 할 때,
ECS는 여러 저장소를 왔다갔다해야 합니다.

언제 OOP가 낫고, 언제 ECS가 나을까요?

### Q2. Component 추가/제거

Entity에 Component를 추가하면 **Archetype이 바뀝니다.**

```
Entity 2: [Position, Velocity]  → Archetype B

// Health 추가
addComponent<Health>(entity2, {...});

Entity 2: [Position, Velocity, Health]  → Archetype A로 이동
```

이 "이동"은 비용이 있습니다. 어떤 비용일까요?

### Q3. 보안 도메인 적용

당신의 보안 모니터링 시스템에서:

- 매 초 수천 개의 프로세스 CPU 사용량 체크
- 가끔 특정 프로세스만 상세 분석

어떤 작업이 DOD의 이점을 많이 받을까요?

---

## 다음 단계

→ [04-modern-cpp-connection.md](./04-modern-cpp-connection.md) - ECS 구현에 필요한 모던 C++ 기능들

---

## 체크리스트

- [ ] 캐시 라인이 뭔지, 왜 연속 메모리가 빠른지 설명할 수 있는가?
- [ ] OOP와 DOD의 메모리 배치 차이를 그림으로 그릴 수 있는가?
- [ ] Archetype이 왜 필요한지 설명할 수 있는가?
- [ ] DOD가 항상 좋은 게 아니라는 것을 이해했는가?
