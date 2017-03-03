cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 11)

set(src
    c4/allocator.hpp
    c4/char_traits.cpp
    c4/char_traits.hpp
    c4/common.hpp
    c4/compiler.hpp
    c4/config.hpp
    c4/cpu.hpp
    c4/ctor_dtor.hpp
    c4/error.cpp
    c4/error.hpp
    c4/hash.hpp
    c4/language.hpp
    c4/language.cpp
    c4/log.cpp
    c4/log.hpp
    c4/memory_resource.cpp
    c4/memory_resource.hpp
    c4/memory_util.cpp
    c4/memory_util.hpp
    c4/platform.hpp
    c4/preprocessor.hpp
    c4/span.hpp
    c4/sstream.cpp
    c4/sstream.hpp
    c4/sstream.def.hpp
    c4/storage/raw.cpp
    c4/storage/raw.hpp
    c4/storage/growth.cpp
    c4/storage/growth.hpp
    c4/string.cpp
    c4/string.hpp
    c4/string_fwd.hpp
    c4/szconv.hpp
    c4/time.hpp
    c4/time.cpp
    c4/types.hpp
    c4/windows.hpp

    c4/util.hpp
)

create_source_group("" "${CMAKE_CURRENT_SOURCE_DIR}" "${src}")

if(MSVC)
    list(APPEND src c4stl.natvis)
endif()

add_library(c4stl ${src})
target_include_directories(c4stl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../extern)

#------------------------------------------------------------------------------
set(C4STL_EXTERN_DIR ${CMAKE_INSTALL_PREFIX} CACHE PATH "")

option(C4STL_BUILD_TESTS "" ON)
option(C4STL_BUILD_BENCHMARKS "" ON)
if(C4STL_BUILD_TESTS OR C4STL_BUILD_BENCHMARKS)
    include(ExternalProjectUtils)
endif()

#------------------------------------------------------------------------------

if(C4STL_BUILD_TESTS)

    enable_testing()
    add_custom_target(unit_tests ${CMAKE_CTEST_COMMAND} -V)

    ExternalProject_Import(gtest
        PREFIX ${C4STL_EXTERN_DIR}
        INCLUDES gtest/gtest.h
        LIBRARIES gtest_main gtest)
    find_package(Threads)

    add_library(c4testcommon
        ../test/c4/archetypes.cpp
        ../test/c4/archetypes.hpp
        ../test/c4/test.hpp
    )
    target_link_libraries(c4testcommon PUBLIC
        c4stl
        ${gtest_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT})
    target_include_directories(c4testcommon PUBLIC
        "${gtest_INCLUDE_DIRS}"
        ${CMAKE_CURRENT_SOURCE_DIR}/../test)

    macro(c4stl_add_test name)
        add_executable(c4stl-test-${name}
            ${CMAKE_CURRENT_SOURCE_DIR}/../test/main.cpp
            ${ARGN})
        add_dependencies(unit_tests c4stl-test-${name})
        target_link_libraries(c4stl-test-${name} c4testcommon)
        add_test(NAME c4stl-test-${name}-run
            COMMAND ${CMAKE_CURRENT_BINARY_DIR}/c4stl-test-${name})
        if(UNIX)
            add_test(NAME c4stl-test-${name}-valgrind
                COMMAND valgrind ${CMAKE_CURRENT_BINARY_DIR}/c4stl-test-${name})
        endif()
    endmacro(c4stl_add_test)

    c4stl_add_test(core
        c4/char_traits.test.cpp
        c4/error.test.cpp
        c4/log.test.cpp
        c4/memory_resource.test.cpp
        c4/memory_util.test.cpp
        c4/preprocessor.test.cpp
        c4/span.test.cpp
        c4/storage/raw.test.cpp
        c4/storage/raw.test.hpp
        c4/storage/growth.test.cpp
        c4/types.test.cpp
    )

    c4stl_add_test(sstream c4/sstream.test.cpp)
    c4stl_add_test(string  c4/string.test.cpp c4/string.test.hpp)
    if(WIN32)
        if(MSVC)
            set_property(SOURCE c4/sstream.test.cpp APPEND_STRING PROPERTY COMPILE_FLAGS " /bigobj ")
            set_property(SOURCE c4/string.test.cpp  APPEND_STRING PROPERTY COMPILE_FLAGS " /bigobj ")
        endif()
    endif()

endif(C4STL_BUILD_TESTS)

#------------------------------------------------------------------------------

if(C4STL_BUILD_BENCHMARKS)
    ExternalProject_Import(gbenchmark
        PREFIX ${C4STL_EXTERN_DIR}
        INCLUDES benchmark/benchmark.h
        LIBRARIES benchmark)
endif(C4STL_BUILD_BENCHMARKS)
