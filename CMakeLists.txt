cmake_minimum_required(VERSION 3.20)
project(mini-ecs LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 헤더 온리 라이브러리
add_library(mini-ecs INTERFACE)
target_include_directories(mini-ecs INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 테스트용 실행 파일
add_executable(test_ecs test_main.cpp)
target_link_libraries(test_ecs PRIVATE mini-ecs)

# 파티클 예제
add_executable(particle_example examples/particle/main.cpp)
target_link_libraries(particle_example PRIVATE mini-ecs)
