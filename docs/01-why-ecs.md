# 01. 왜 ECS인가? - OOP의 한계에서 시작하기

## 이 문서의 목표

ECS를 배우기 전에, **왜 ECS가 필요한지**를 먼저 이해합니다.
"이런 구조가 있다"가 아니라 "이런 문제가 있어서 이런 구조가 나왔다"를 체감해야 합니다.

---

## 시나리오: 보안 모니터링 시스템

당신은 리눅스 시스템에서 다양한 객체를 모니터링하는 도구를 만들고 있습니다.

### 모니터링 대상

| 대상 | 속성 |
|------|------|
| 프로세스 | PID, 이름, CPU 사용량, 메모리 사용량 |
| 네트워크 연결 | 로컬 IP/포트, 원격 IP/포트, 상태 |
| 파일 | 경로, 해시, 수정 시간, 권한 |

### 추가 요구사항

- 일부 프로세스는 네트워크 연결을 가짐
- 일부 프로세스는 감시 대상 파일을 열고 있음
- 모든 대상에 대해 "마지막 검사 시간"을 추적해야 함
- 나중에 "위협 점수", "격리 상태" 등 새로운 속성이 추가될 예정

---

## OOP로 접근해보기

### 시도 1: 단순한 클래스 계층

```cpp
class MonitoredObject {
    Timestamp lastChecked;
};

class Process : public MonitoredObject {
    int pid;
    std::string name;
    float cpuUsage;
    float memUsage;
};

class NetworkConnection : public MonitoredObject {
    std::string localIp;
    int localPort;
    // ...
};

class File : public MonitoredObject {
    std::filesystem::path path;
    std::string hash;
    // ...
};
```

**문제 1: 프로세스가 네트워크 연결을 "가지는" 경우**

```cpp
class Process : public MonitoredObject {
    // ...
    std::vector<NetworkConnection> connections;  // 소유? 참조?
};
```

- `NetworkConnection`도 독립적으로 모니터링 대상인데, Process 안에 넣으면?
- 같은 연결이 여러 프로세스와 관련되면?

**문제 2: 새로운 속성 추가**

"위협 점수"를 추가하려면?

```cpp
class MonitoredObject {
    Timestamp lastChecked;
    int threatScore;        // 모든 클래스에 영향
};
```

- 파일에는 위협 점수가 필요 없을 수도 있음
- 기존 코드 전부 재컴파일

---

### 시도 2: 다중 상속

```cpp
class Threatable {
    int threatScore;
};

class NetworkCapable {
    std::vector<NetworkConnection> connections;
};

class Process : public MonitoredObject, 
                public Threatable, 
                public NetworkCapable {
    // ...
};
```

**문제:**
- 다이아몬드 상속 문제
- 어떤 Process는 NetworkCapable이고 어떤 건 아님 → 런타임에 결정 불가
- 조합이 늘어날수록 클래스 폭발

---

### 시도 3: 조건문으로 처리

```cpp
class Process : public MonitoredObject {
    std::optional<ThreatInfo> threat;
    std::optional<NetworkInfo> network;
    std::optional<FileAccessInfo> fileAccess;
    // ...
};
```

**문제:**
- 새 속성 추가할 때마다 클래스 수정
- "위협 점수가 있는 모든 객체"를 조회하려면?

```cpp
for (auto& proc : processes) {
    if (proc.threat.has_value()) { /* ... */ }
}
for (auto& file : files) {
    if (file.threat.has_value()) { /* ... */ }  // File에도 추가해야 함
}
// 타입마다 반복...
```

---

## 핵심 문제 정리

| 문제 | 설명 |
|------|------|
| **조합 폭발** | 속성 N개의 조합 = 2^N 가지 가능한 클래스 |
| **수정 전파** | 새 속성 추가 → 여러 클래스 수정 필요 |
| **횡단 관심사** | "특정 속성을 가진 모든 객체" 조회가 어려움 |
| **정적 구조** | 런타임에 속성 추가/제거 불가 |

---

## 생각해볼 질문

다음 단계로 넘어가기 전에, 스스로 생각해보세요:

### Q1. 상속의 근본 문제

> 상속은 **"is-a"** 관계를 표현합니다.
> "Process is a MonitoredObject"는 맞지만,
> "Process is a NetworkCapable"은... 항상 맞나요?

- 어떤 프로세스는 네트워크를 쓰고, 어떤 건 안 씁니다.
- "is-a"로 표현하기엔 너무 유동적입니다.
- 그렇다면 어떤 관계로 표현해야 할까요?

### Q2. 데이터와 행동의 결합

> OOP에서 클래스는 **데이터 + 메서드**를 함께 가집니다.
> 이게 항상 좋은 걸까요?

- "CPU 사용량 계산" 로직이 Process 클래스 안에 있으면?
- 나중에 "CPU 사용량"이 다른 대상에도 필요하면?
- 같은 로직을 복사? 상속? 유틸리티 함수?

### Q3. 조회의 효율성

> "위협 점수가 80 이상인 모든 객체"를 찾으려면?

- OOP: 모든 타입을 순회하며 조건 검사
- 더 나은 방법이 있을까요?

---

## 다음 단계

이 문제들에 대해 생각해보셨다면, 다음 문서에서 ECS가 이 문제들을 어떻게 해결하는지 살펴봅니다.

→ [02-ecs-core-concepts.md](./02-ecs-core-concepts.md)

---

## 체크리스트

다음 단계로 넘어가기 전에 확인:

- [ ] OOP 상속이 "조합"을 표현하기 어려운 이유를 설명할 수 있는가?
- [ ] "횡단 관심사(cross-cutting concern)"가 무엇인지 이해했는가?
- [ ] 왜 "데이터와 로직의 분리"가 필요할 수 있는지 생각해봤는가?
