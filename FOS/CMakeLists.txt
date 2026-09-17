cmake_minimum_required(VERSION 4.0)

project(FOS
    VERSION 0.5.0
    LANGUAGES C
)

add_library(FOS STATIC
    src/FOS_Bignum.c
    src/FOS_HashMap.c
    src/FOS_Heap.c
    src/FOS_List.c
    src/FOS_Memory.c
    src/FOS_Queue.c
    src/FOS_Rng.c
    src/FOS_Set.c
    src/FOS_Stack.c
    src/FOS_String.c
    src/FOS_Vec.c
)

target_include_directories(FOS
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(FOS
    PUBLIC
        c_std_11
)

if(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(FOS PRIVATE
        /W4
    )
elseif(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(FOS PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
elseif(CMAKE_C_COMPILER_ID STREQUAL "PellesC")
    target_compile_options(FOS PRIVATE
        -MT
    )
endif()
