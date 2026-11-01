cmake_minimum_required(VERSION 3.16)
project(A2_University_Registration_System)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(a2
    src/main.cpp
    src/Student.cpp
    src/Course.cpp
    src/DataRepository.cpp
    src/RegistrationManager.cpp
)

target_include_directories(a2 PRIVATE src)
