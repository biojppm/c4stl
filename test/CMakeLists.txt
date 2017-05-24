add_custom_target(c4stl-test ${CMAKE_CTEST_COMMAND} -V)
set_target_properties(c4stl-test PROPERTIES FOLDER c4stl-test)

add_custom_target(c4stl-test-build)
add_dependencies(c4stl-test c4stl-test-build)
set_target_properties(c4stl-test-build PROPERTIES FOLDER c4stl-test)

#------------------------------------------------------------------------------

ExternalProject_Import(gtest
    PREFIX ${C4STL_EXTERN_DIR}
    INCLUDES gtest/gtest.h
    LIBRARIES gtest_main gtest)
find_package(Threads REQUIRED)

#------------------------------------------------------------------------------

set(testsrc
    c4/archetypes.cpp
    c4/archetypes.hpp
    c4/test.hpp
    )
c4stl_add_target(c4stl-libtest LIBRARY SANITIZE
    SOURCES ${testsrc}
    LIBS c4stl ${gtest_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}
    INC_DIRS ${gtest_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/../test
    )
set_target_properties(c4stl-libtest PROPERTIES FOLDER c4stl-test)

#------------------------------------------------------------------------------

macro(c4stl_get_exe bin_dir exe_name)
    if(MSVC)
        set(${bin_dir} "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${exe_name}.exe")
    else()
        set(${bin_dir} "${CMAKE_CURRENT_BINARY_DIR}/${exe_name}")
    endif()
endmacro()

function(c4stl_add_test name)
    c4stl_add_target(${name} EXECUTABLE SANITIZE
        SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../test/main.cpp ${ARGN}
        LIBS c4stl-libtest ${gtest_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}
        INC_DIRS ${gtest_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/../test
        )
    add_custom_target(${name}-build)
    add_dependencies(${name}-build ${name})
    add_dependencies(c4stl-test-build ${name}-build)
    add_test(NAME ${name}-run COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${name})
    set_target_properties(${name} PROPERTIES FOLDER c4stl-test/${name})
    set_target_properties(${name}-build PROPERTIES FOLDER c4stl-test/${name})
    foreach(s asan msan tsan ubsan)
        set(t c4stl-${name}-${s})
        if(TARGET ${t})
            add_dependencies(${name}-build ${t})
            sanitize_get_target_command(${CMAKE_CURRENT_BINARY_DIR}/${t} C4STL ${s} cmd)
            add_test(NAME ${t}-run COMMAND ${cmd})
            set_target_properties(${t} PROPERTIES FOLDER c4stl-test/${name})
        endif()
    endforeach()
    if(UNIX)
        # consider doing this for valgrind:
        # http://stackoverflow.com/questions/40325957/how-do-i-add-valgrind-tests-to-my-cmake-test-target
        # for now we explicitly run it:
        add_test(NAME ${name}-valgrind
            COMMAND valgrind ${CMAKE_CURRENT_BINARY_DIR}/${name})
        set_target_properties(${name}-valgrind PROPERTIES FOLDER c4stl-test/${name})
    endif()
endfunction(c4stl_add_test)

#------------------------------------------------------------------------------

set(s ../src)

c4stl_add_test(c4stl-test-core
    ${s}/c4/allocator.test.cpp
    ${s}/c4/char_traits.test.cpp
    ${s}/c4/ctor_dtor.test.cpp
    ${s}/c4/error.test.cpp
    ${s}/c4/log.test.cpp
    ${s}/c4/memory_resource.test.cpp
    ${s}/c4/memory_util.test.cpp
    ${s}/c4/preprocessor.test.cpp
    ${s}/c4/storage/growth.test.cpp
    ${s}/c4/type_name.test.cpp
    ${s}/c4/types.test.cpp
    )

c4stl_add_test(c4stl-test-span       ${s}/c4/span.test.cpp)
c4stl_add_test(c4stl-test-raw        ${s}/c4/storage/raw.test.cpp ${s}/c4/storage/raw.test.hpp)
c4stl_add_test(c4stl-test-contiguous ${s}/c4/storage/contiguous.test.cpp)
c4stl_add_test(c4stl-test-sstream    ${s}/c4/sstream.test.cpp)
c4stl_add_test(c4stl-test-string     ${s}/c4/string.test.cpp ${s}/c4/string.test.hpp)

c4stl_add_test(c4stl-test-list-flat      ${s}/c4/list.flat.test.cpp      ${s}/c4/list.test.hpp)
c4stl_add_test(c4stl-test-list-split     ${s}/c4/list.split.test.cpp     ${s}/c4/list.test.hpp)
c4stl_add_test(c4stl-test-list-flat_fwd  ${s}/c4/list.flat_fwd.test.cpp  ${s}/c4/list.test.hpp)
c4stl_add_test(c4stl-test-list-split_fwd ${s}/c4/list.split_fwd.test.cpp ${s}/c4/list.test.hpp)

if(WIN32)
    if(MSVC)
        set_property(SOURCE ${s}/c4/list.flat.test.cpp      APPEND_STRING PROPERTY COMPILE_FLAGS " /bigobj ")
        set_property(SOURCE ${s}/c4/list.split.test.cpp     APPEND_STRING PROPERTY COMPILE_FLAGS " /bigobj ")
        set_property(SOURCE ${s}/c4/list.flat_fwd.test.cpp  APPEND_STRING PROPERTY COMPILE_FLAGS " /bigobj ")
        set_property(SOURCE ${s}/c4/list.split_fwd.test.cpp APPEND_STRING PROPERTY COMPILE_FLAGS " /bigobj ")
        set_property(SOURCE ${s}/c4/sstream.test.cpp    APPEND_STRING PROPERTY COMPILE_FLAGS " /bigobj ")
        set_property(SOURCE ${s}/c4/string.test.cpp     APPEND_STRING PROPERTY COMPILE_FLAGS " /bigobj ")
    endif()
endif()
