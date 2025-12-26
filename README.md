# mini-ecs

모던 C++ 학습을 위한 미니 ECS(Entity Component System) 프레임워크

## 목적

- 모던 C++ 기능을 **실제 문제 해결**에 적용하며 학습
- ECS 아키텍처를 직접 구현하며 **데이터 지향 설계(DOD)** 이해
- "동작하는 코드"가 아닌 **"이해하는 코드"** 작성

## 학습 문서

| 문서 | 내용 |
|------|------|
| [01-why-ecs](docs/01-why-ecs.md) | OOP의 한계와 ECS의 필요성 |
| [02-ecs-core-concepts](docs/02-ecs-core-concepts.md) | Entity, Component, System 개념 |
| [03-data-oriented-design](docs/03-data-oriented-design.md) | 캐시 효율과 메모리 배치 |
| [04-modern-cpp-connection](docs/04-modern-cpp-connection.md) | ECS 구현에 필요한 모던 C++ 기능 |
| [05-implementation-roadmap](docs/05-implementation-roadmap.md) | 단계별 구현 계획 |

## 구현 진행 상황

- [x] **Step 1.1**: EntityManager (generation 기반 ID 재활용)
- [ ] Step 1.2: ComponentStorage (단일 타입)
- [ ] Step 1.3: ComponentStorage (다중 타입) - variadic templates
- [ ] Step 1.4: Query - fold expressions
- [ ] Step 2.x: Concepts를 활용한 타입 안전성
- [ ] Step 3.x: Archetype 기반 최적화

## 사용된 모던 C++ 기능

### Phase 1 (현재)
- 기본 템플릿
- `std::vector`, `std::queue`

### Phase 2 (예정)
- variadic templates
- `std::tuple`
- fold expressions
- concepts
- `if constexpr`
- `constexpr`

## 빌드

```bash
# 추후 CMake 설정 추가 예정
```

## 참고 자료

- [EnTT](https://github.com/skypjack/entt) - 모던 C++ ECS 라이브러리
- [flecs](https://github.com/SanderMertens/flecs) - 고성능 ECS
