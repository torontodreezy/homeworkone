cmake_minimum_required(VERSION 3.20)
project(homeworkone_3)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(
        ENABLE_SANITIZERS
        "AddressSanitizer and LeakSanitizer for tests"
        ON
)

add_executable(homeworkone_3
        main.cpp
        Realization/unq_ptr.h
        Realization/shrd_ptr.h
        Realization/shrd_control.h
        Tests/TestRunner.h
        Tests/TestObjects.h
        Tests/TestUnqPtr.h
        Tests/TestShrdPtr.h
        Realization/weak_ptr.h
        Tests/TestWeakPtr.h
        Tests/TestStandardComparison.h
)

target_compile_options(homeworkone_3 PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -g
)

if(ENABLE_SANITIZERS)
    target_compile_options(homeworkone_3 PRIVATE
            -fsanitize=address,leak
            -fno-omit-frame-pointer
    )

    target_link_options(homeworkone_3 PRIVATE
            -fsanitize=address,leak
    )
endif()

enable_testing()

add_test(
        NAME pointer_tests
        COMMAND homeworkone_3
)

set_tests_properties(pointer_tests PROPERTIES
        ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1"
)

add_executable(pointer_benchmark
        benchmark.cpp
        Tests/TestPerformance.h
        Tests/TestMemory.h
        Tests/TestWeakPerformance.h
        Tests/TestArrayPerformance.h
        Tests/TestMovePerformance.h
)

target_compile_options(pointer_benchmark PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -O2
)
