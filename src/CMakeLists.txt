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

c4stl_add_target(c4stl LIBRARY SANITIZE
    SOURCES ${src}
    INC_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../extern
)

#------------------------------------------------------------------------------

if(C4STL_BUILD_TESTS)

    add_custom_target(unit_tests ${CMAKE_CTEST_COMMAND} -V)

    ExternalProject_Import(gtest
        PREFIX ${C4STL_EXTERN_DIR}
        INCLUDES gtest/gtest.h
        LIBRARIES gtest_main gtest)
    find_package(Threads)

    set(testsrc
        ../test/c4/archetypes.cpp
        ../test/c4/archetypes.hpp
        ../test/c4/test.hpp
    )
    c4stl_add_target(c4test LIBRARY SANITIZE
        SOURCES ${testsrc}
        LIBS c4stl ${gtest_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}
        INC_DIRS ${gtest_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/../test
    )

    function(c4stl_add_test name)
        c4stl_add_target(${name} EXECUTABLE SANITIZE
            SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../test/main.cpp ${ARGN}
            LIBS c4test ${gtest_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}
            INC_DIRS ${gtest_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/../test
            OUTPUT_TARGET_NAMES _test_targets
        )
        foreach(t ${_test_targets})
            add_dependencies(unit_tests ${t})
            add_test(NAME ${t}-run
                COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${t})
        endforeach()
        if(UNIX)
            # consider doing this for valgrind:
            # http://stackoverflow.com/questions/40325957/how-do-i-add-valgrind-tests-to-my-cmake-test-target
            # for now we explicitly run it:
            add_test(NAME ${name}-valgrind
                COMMAND valgrind ${CMAKE_CURRENT_BINARY_DIR}/${name})
        endif()
    endfunction(c4stl_add_test)

    c4stl_add_test(c4stl-test-core
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

    c4stl_add_test(c4stl-test-sstream c4/sstream.test.cpp)
    c4stl_add_test(c4stl-test-string  c4/string.test.cpp c4/string.test.hpp)
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
