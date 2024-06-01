cmake_minimum_required(VERSION 3.5)
project(CSALab2)

set(CMAKE_CXX_STANDARD 11)

include_directories(hv)

# Исполняемый файл для CSALab2
add_executable(CSALab2
    src/CSALab2.cpp
    src/MyServer.cpp
)

target_link_libraries(CSALab2 ${CMAKE_SOURCE_DIR}/lib/libhv.a)

set_target_properties(CSALab2
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY build
)

install(TARGETS CSALab2
    DESTINATION package/usr/bin
)

# Исполняемый файл для Test
add_executable(Test
    src/Test.cpp
    src/MyServer.cpp
)

target_link_libraries(Test ${CMAKE_SOURCE_DIR}/lib/libCatch2.a ${CMAKE_SOURCE_DIR}/lib/libhv.a)

set_target_properties(Test
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY test
)
