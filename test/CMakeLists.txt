# add a configuration type with the define C4_QUICKTEST
# to make a quick test covering less combinations

include(GetFlags)
include(ConfigurationTypes)

get_define(dquicktest C4_QUICKTEST)
add_configuration_type(quicktest
    DEFAULT_FROM DEBUG
    C_FLAGS ${dquicktest}
    CXX_FLAGS ${dquicktest}
    )

#------------------------------------------------------------------------------

add_custom_target(c4stl-test ${CMAKE_CTEST_COMMAND} -V)
set_target_properties(c4stl-test PROPERTIES FOLDER c4stl-test)

add_custom_target(c4stl-test-build)
add_dependencies(c4stl-test c4stl-test-build)
set_target_properties(c4stl-test-build PROPERTIES FOLDER c4stl-test)

#------------------------------------------------------------------------------
# Add the coverage command(s)

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "(Apple)?[Cc]lang")
    if("${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS 3)
	message(STATUS "Coverage: clang version must be 3.0.0 or greater. No coverage available.")
        return()
    endif()
elseif(NOT CMAKE_COMPILER_IS_GNUCXX)
    message(STATUS "Coverage: compiler is not GNUCXX. No coverage available.")
    return()
endif()

set(covflags "-g -O0 --coverage -fprofile-arcs -ftest-coverage")
add_configuration_type(Coverage
    DEFAULT_FROM DEBUG
    C_FLAGS ${covflags}
    CXX_FLAGS ${covflags}
    )

if(CMAKE_BUILD_TYPE)
    string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_LOWER)
endif()
if(${CMAKE_BUILD_TYPE_LOWER} MATCHES "coverage")
    find_program(GCOV gcov)
    find_program(LCOV lcov)
    find_program(GENHTML genhtml)
    find_program(CTEST ctest)
    if (GCOV AND LCOV AND GENHTML AND CTEST)
        add_custom_command(
            OUTPUT ${CMAKE_BINARY_DIR}/lcov/index.html
            COMMAND ${LCOV} -q -z -d .
            COMMAND ${LCOV} -q --no-external -c -b "${CMAKE_SOURCE_DIR}" -d . -o before.lcov -i
            COMMAND ${CTEST} --force-new-ctest-process
            COMMAND ${LCOV} -q --no-external -c -b "${CMAKE_SOURCE_DIR}" -d . -o after.lcov
            COMMAND ${LCOV} -q -a before.lcov -a after.lcov --output-file final.lcov
            COMMAND ${LCOV} -q -r final.lcov "'${CMAKE_SOURCE_DIR}/test/*'" -o final.lcov
            COMMAND ${GENHTML} final.lcov -o lcov --demangle-cpp --sort -p "${CMAKE_BINARY_DIR}" -t benchmark
            DEPENDS c4stl-test
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Running LCOV"
            )
        add_custom_target(c4stl-coverage
            DEPENDS ${CMAKE_BINARY_DIR}/lcov/index.html
            COMMENT "LCOV report at ${CMAKE_BINARY_DIR}/lcov/index.html"
            )
        message(STATUS "Coverage command added")
    else()
        if (HAVE_CXX_FLAG_COVERAGE)
            set(CXX_FLAG_COVERAGE_MESSAGE supported)
        else()
            set(CXX_FLAG_COVERAGE_MESSAGE unavailable)
        endif()
        message(WARNING
            "Coverage not available:\n"
            "  gcov: ${GCOV}\n"
            "  lcov: ${LCOV}\n"
            "  genhtml: ${GENHTML}\n"
            "  ctest: ${CTEST}\n"
            "  --coverage flag: ${CXX_FLAG_COVERAGE_MESSAGE}")
    endif()
endif()

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
    create_source_group("" "../src" "${ARGN}")
    c4stl_add_target(${name} EXECUTABLE SANITIZE
        SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../test/main.cpp ${ARGN}
        LIBS c4stl-libtest ${gtest_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}
        INC_DIRS ${gtest_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/../test
        )
    list(APPEND c4stl_test_list ${name})
    add_custom_target(${name}-build)
    add_dependencies(${name}-build ${name})
    add_dependencies(c4stl-test-build ${name}-build)
    add_test(NAME ${name}-run COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${name})
    set_target_properties(${name} PROPERTIES FOLDER c4stl-test/${name})
    set_target_properties(${name}-build PROPERTIES FOLDER c4stl-test/${name})
    foreach(s asan msan tsan ubsan)
        set(t ${name}-${s})
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
    endif()
endfunction(c4stl_add_test)

#------------------------------------------------------------------------------

set(s ../src)

c4stl_add_test(c4stl-test-preprocessor ${s}/c4/preprocessor.test.cpp)
c4stl_add_test(c4stl-test-typename     ${s}/c4/type_name.test.cpp)
c4stl_add_test(c4stl-test-types        ${s}/c4/types.test.cpp)
c4stl_add_test(c4stl-test-log          ${s}/c4/log.test.cpp)
c4stl_add_test(c4stl-test-error        ${s}/c4/error.test.cpp)
c4stl_add_test(c4stl-test-memutil      ${s}/c4/memory_util.test.cpp)
c4stl_add_test(c4stl-test-memresource  ${s}/c4/memory_resource.test.cpp)
c4stl_add_test(c4stl-test-allocator    ${s}/c4/allocator.test.cpp)
c4stl_add_test(c4stl-test-ctordtor     ${s}/c4/ctor_dtor.test.cpp)

c4stl_add_test(c4stl-test-span         ${s}/c4/span.test.cpp)

c4stl_add_test(c4stl-test-chartraits   ${s}/c4/char_traits.test.cpp)
c4stl_add_test(c4stl-test-sstream      ${s}/c4/sstream.test.cpp)

# the string tests are separated to save compile time
c4stl_add_test(c4stl-test-string-substring   ${s}/c4/string.test.substring.cpp ${s}/c4/string.test.hpp)
c4stl_add_test(c4stl-test-string-substringrs ${s}/c4/string.test.substringrs.cpp ${s}/c4/string.test.hpp)
c4stl_add_test(c4stl-test-string-string      ${s}/c4/string.test.string.cpp ${s}/c4/string.test.hpp)
c4stl_add_test(c4stl-test-string-text        ${s}/c4/string.test.text.cpp ${s}/c4/string.test.hpp)
c4stl_add_test(c4stl-test-string-interop     ${s}/c4/string.test.interop.cpp ${s}/c4/string.test.hpp)

c4stl_add_test(c4stl-test-growth       ${s}/c4/storage/growth.test.cpp)

c4stl_add_test(c4stl-test-raw          ${s}/c4/storage/raw.test.cpp ${s}/c4/storage/raw.test.hpp)
c4stl_add_test(c4stl-test-contiguous   ${s}/c4/storage/contiguous.test.cpp)

# the list tests are separated to save compile time
c4stl_add_test(c4stl-test-list-flat      ${s}/c4/list.flat.test.cpp      ${s}/c4/list.test.hpp)
c4stl_add_test(c4stl-test-list-split     ${s}/c4/list.split.test.cpp     ${s}/c4/list.test.hpp)
c4stl_add_test(c4stl-test-list-flat_fwd  ${s}/c4/list.flat_fwd.test.cpp  ${s}/c4/list.test.hpp)
c4stl_add_test(c4stl-test-list-split_fwd ${s}/c4/list.split_fwd.test.cpp ${s}/c4/list.test.hpp)

if(WIN32)
    if(MSVC AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC") # it may be clang as well
        set(bigfiles
            ${s}/c4/list.flat.test.cpp
            ${s}/c4/list.split.test.cpp
            ${s}/c4/list.flat_fwd.test.cpp
            ${s}/c4/list.split_fwd.test.cpp
            ${s}/c4/sstream.test.cpp
            ${s}/c4/string.test.string.cpp
            ${s}/c4/string.test.substring.cpp
            ${s}/c4/string.test.substringrs.cpp
            ${s}/c4/string.test.text.cpp
            )
        foreach(_bf ${bigfiles})
            set_property(SOURCE ${_bf} APPEND_STRING PROPERTY COMPILE_FLAGS " /bigobj ")
        endforeach()
    endif()
endif()
