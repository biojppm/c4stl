enable_testing()

add_custom_target(unit_tests ${CMAKE_CTEST_COMMAND} -V)

ExternalProject_Import(gtest
    PREFIX ${C4STL_EXTERN_DIR}
    INCLUDES gtest/gtest.h
    LIBRARIES gtest_main gtest)
find_package(Threads REQUIRED)

set(testsrc
    c4/archetypes.cpp
    c4/archetypes.hpp
    c4/test.hpp
    )
c4stl_add_target(c4test LIBRARY SANITIZE
    SOURCES ${testsrc}
    LIBS c4stl ${gtest_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}
    INC_DIRS ${gtest_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/../test
    )

#------------------------------------------------------------------------------

function(c4stl_add_test name)
    c4stl_add_target(${name} EXECUTABLE SANITIZE
        SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../test/main.cpp ${ARGN}
        LIBS c4test ${gtest_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}
        INC_DIRS ${gtest_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/../test
        )
    add_dependencies(unit_tests ${name})
    add_test(NAME ${name}-run COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${name})
    foreach(s asan msan tsan ubsan)
        set(t ${name}-${s})
        if(TARGET ${t})
            add_dependencies(unit_tests ${t})
            sanitize_get_target_command(${CMAKE_CURRENT_BINARY_DIR}/${t} C4STL ${s} cmd)
            add_test(NAME ${t}-run COMMAND ${cmd})
        endif()
    endforeach()
    if(UNIX)
        # consider doing this for valgrind:
        # http://stackoverflow.com/questions/40325957/how-do-i-add-valgrind-tests-to-my-cmake-test-target
        # for now we explicitly run it:
        add_test(NAME ${name}-valgrind
            COMMAND valgrind ${CMAKE_CURRENT_BINARY_DIR}/${name})
    endif()
endfunction(c4stl_add_test)

#------------------------------------------------------------------------------

set(s ../src)

c4stl_add_test(c4stl-test-core
    ${s}/c4/char_traits.test.cpp
    ${s}/c4/error.test.cpp
    ${s}/c4/log.test.cpp
    ${s}/c4/memory_resource.test.cpp
    ${s}/c4/memory_util.test.cpp
    ${s}/c4/preprocessor.test.cpp
    ${s}/c4/span.test.cpp
    ${s}/c4/storage/raw.test.cpp
    ${s}/c4/storage/raw.test.hpp
    ${s}/c4/storage/growth.test.cpp
    ${s}/c4/types.test.cpp
    )

c4stl_add_test(c4stl-test-sstream ${s}/c4/sstream.test.cpp)
c4stl_add_test(c4stl-test-string  ${s}/c4/string.test.cpp ${s}/c4/string.test.hpp)
if(WIN32)
    if(MSVC)
        set_property(SOURCE ${s}/c4/sstream.test.cpp APPEND_STRING PROPERTY COMPILE_FLAGS " /bigobj ")
        set_property(SOURCE ${s}/c4/string.test.cpp  APPEND_STRING PROPERTY COMPILE_FLAGS " /bigobj ")
    endif()
endif()