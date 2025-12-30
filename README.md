# mini-ecs

모던 C++ 학습을 위한 미니 ECS(Entity Component System) 프레임워크

## 목적

- 모던 C++ 기능을 **실제 문제 해결**에 적용하며 학습
- ECS 아키텍처를 직접 구현하며 **데이터 지향 설계(DOD)** 이해
- "동작하는 코드"가 아닌 **"이해하는 코드"** 작성

## 데모

```
+--------------------------------------------------------------------------------+
|                    *     *                                                     |
|                  *   *     *   *                                               |
|                *       *         *                                             |
|              *           *         *    ← 클릭하면 파티클 폭발!                 |
|                *       *         *                                             |
|                  *   *     *   *                                               |
|                    *     *                                                     |
+--------------------------------------------------------------------------------+
  Click to spawn particles! (Ctrl+C to exit)
```

## 구현 완료

### Core ECS
- ✅ **EntityManager**: Generation 기반 ID 재활용
- ✅ **SparseSet**: O(1) 컴포넌트 조회/추가/삭제
- ✅ **World**: 다중 컴포넌트 타입 관리 (variadic templates)
- ✅ **Query**: 복합 컴포넌트 쿼리 (fold expressions)
- ✅ **Concepts**: 컴포넌트 타입 제약

### Particle Example
- ✅ **System 아키텍처**: ISystem 인터페이스
- ✅ **InputSystem**: 마우스 클릭 감지 (Windows Console API)
- ✅ **SpawnSystem**: SpawnRequest 처리 → 파티클 생성
- ✅ **MovementSystem**: Position += Velocity * dt
- ✅ **LifetimeSystem**: 수명 관리 및 엔티티 삭제
- ✅ **RenderSystem**: 깜빡임 없는 콘솔 렌더링

## 사용된 모던 C++ 기능

| 기능 | 사용처 |
|------|--------|
| variadic templates | `World<Ts...>`, `queryEntities<Ts...>` |
| fold expressions | `(hasComponent<Ts>() && ...)` |
| concepts | `Component` concept 정의 |
| `std::tuple` | 다중 타입 컴포넌트 저장소 |
| `static_assert` | 컴파일 타임 타입 검증 |
| `constexpr` | 컴파일 타임 상수 |
| CTAD | `SparseSet` 템플릿 인자 추론 |

## 빌드 및 실행

```bash
mkdir build && cd build
cmake ..
cmake --build . --config Debug

# 파티클 데모 실행
./Debug/particle_example.exe
```

## 프로젝트 구조

```
mini-ecs/
├── include/mini-ecs/
│   ├── entity.hpp      # EntityManager
│   ├── sparse_set.hpp  # SparseSet<T>
│   ├── world.hpp       # World<Components...>
│   └── concepts.hpp    # Component concept
├── examples/particle/
│   ├── main.cpp
│   ├── components.hpp  # Position, Velocity, Lifetime, Particle, SpawnRequest
│   ├── types.hpp       # GameWorld typedef
│   └── systems/
│       ├── system.hpp    # ISystem 인터페이스
│       ├── input.hpp     # 마우스 입력
│       ├── spawn.hpp     # 파티클 생성
│       ├── movement.hpp  # 이동
│       ├── lifetime.hpp  # 수명
│       └── render.hpp    # 렌더링
└── docs/               # 학습 문서
```

## 참고 자료

- [EnTT](https://github.com/skypjack/entt) - 모던 C++ ECS 라이브러리
- [flecs](https://github.com/SanderMertens/flecs) - 고성능 ECS
