# add a configuration type with the define C4_QUICKTEST
# to make a quick test covering less combinations

include(GetFlags)

get_define(dquicktest C4_QUICKTEST)
add_configuration_type(Quicktest
    DEFAULT_FROM DEBUG
    C_FLAGS ${dquicktest}
    CXX_FLAGS ${dquicktest}
    )

option(C4STL_PEDANTIC "Compile in pedantic mode" OFF)
if(C4STL_PEDANTIC)
    if(MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Wall")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -pedantic -Wfloat-equal -fstrict-aliasing")
    endif()
endif()

option(C4STL_WERROR "Compile with compiler warnings as errors" OFF)
if(C4STL_WERROR)
    if(MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
        #set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} /WX")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -pedantic-errors")
    endif()
endif()

#------------------------------------------------------------------------------
# Add the coverage command(s)

set(c4stl_coverage_available ON)
if("${CMAKE_CXX_COMPILER_ID}" MATCHES "(Apple)?[Cc]lang")
    if("${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS 3)
	message(STATUS "Coverage: clang version must be 3.0.0 or greater. No coverage available.")
        set(c4stl_coverage_available OFF)
    endif()
elseif(NOT CMAKE_COMPILER_IS_GNUCXX)
    message(STATUS "Coverage: compiler is not GNUCXX. No coverage available.")
    set(c4stl_coverage_available OFF)
endif()

if(c4stl_coverage_available)
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
        set(COVERAGE_BUILD ON)
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
endif()

#------------------------------------------------------------------------------

add_custom_target(c4stl-test-build)
set_target_properties(c4stl-test-build PROPERTIES FOLDER c4stl-test)

add_custom_target(c4stl-test
    ${CMAKE_COMMAND} -E echo CWD=${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E echo CMD=${CMAKE_CTEST_COMMAND} -C $<CONFIG>
    COMMAND ${CMAKE_COMMAND} -E echo ----------------------------------
    COMMAND ${CMAKE_COMMAND} -E env CTEST_OUTPUT_ON_FAILURE=1 ${CMAKE_CTEST_COMMAND} ${C4STL_CTEST_OPTIONS} -C $<CONFIG>
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS c4stl-test-build
    )
set_target_properties(c4stl-test PROPERTIES FOLDER c4stl-test)

#------------------------------------------------------------------------------

ExternalProject_Import(gtest
    PREFIX ${C4STL_EXTERN_DIR}
    INCLUDES gtest/gtest.h
    LIBRARIES gtest_main gtest)
find_package(Threads REQUIRED)

#------------------------------------------------------------------------------
set(testsrc
    c4/test.hpp
    c4/libtest/test.cpp
    c4/libtest/archetypes.cpp
    c4/libtest/archetypes.hpp
    c4/libtest/supprwarn_push.hpp
    c4/libtest/supprwarn_pop.hpp
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
    create_source_group("" "c4/test" "${ARGN}")
    set(sanitize SANITIZE)
    if(COVERAGE_BUILD)
        set(sanitize "")
    endif()
    c4stl_add_target(${name} EXECUTABLE ${sanitize}
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
    if(UNIX AND NOT COVERAGE_BUILD)
        # @todo: consider doing this for valgrind:
        # http://stackoverflow.com/questions/40325957/how-do-i-add-valgrind-tests-to-my-cmake-test-target
        # for now we explicitly run it:
        add_test(NAME ${name}-valgrind
            COMMAND valgrind ${CMAKE_CURRENT_BINARY_DIR}/${name})
    endif()
endfunction(c4stl_add_test)

#------------------------------------------------------------------------------

set(t c4/test)

c4stl_add_test(c4stl-test-preprocessor ${t}/preprocessor.cpp)
c4stl_add_test(c4stl-test-typename     ${t}/type_name.cpp)
c4stl_add_test(c4stl-test-types        ${t}/types.cpp)
c4stl_add_test(c4stl-test-szconv       ${t}/szconv.cpp)
c4stl_add_test(c4stl-test-log          ${t}/log.cpp)
c4stl_add_test(c4stl-test-error        ${t}/error.cpp)
c4stl_add_test(c4stl-test-memutil      ${t}/memory_util.cpp)
c4stl_add_test(c4stl-test-memresource  ${t}/memory_resource.cpp)
c4stl_add_test(c4stl-test-allocator    ${t}/allocator.cpp)
c4stl_add_test(c4stl-test-ctordtor     ${t}/ctor_dtor.cpp)

c4stl_add_test(c4stl-test-span         ${t}/span.cpp)

c4stl_add_test(c4stl-test-chartraits   ${t}/char_traits.cpp)
c4stl_add_test(c4stl-test-sstream      ${t}/sstream.cpp)

# the string tests are separated to save compile time
c4stl_add_test(c4stl-test-string-substring   ${t}/string.substring.cpp ${t}/string.hpp)
c4stl_add_test(c4stl-test-string-substringrs ${t}/string.substringrs.cpp ${t}/string.hpp)
c4stl_add_test(c4stl-test-string-string      ${t}/string.string.cpp ${t}/string.hpp)
c4stl_add_test(c4stl-test-string-text        ${t}/string.text.cpp ${t}/string.hpp)
c4stl_add_test(c4stl-test-string-interop     ${t}/string.interop.cpp ${t}/string.hpp)

c4stl_add_test(c4stl-test-growth       ${t}/storage/growth.cpp)

c4stl_add_test(c4stl-test-raw          ${t}/storage/raw.cpp ${t}/storage/raw.hpp)
c4stl_add_test(c4stl-test-contiguous   ${t}/storage/contiguous.cpp)

# the list tests are separated to save compile time
c4stl_add_test(c4stl-test-list-flat-paged    ${t}/list.flat.paged.cpp    ${t}/list.hpp)
c4stl_add_test(c4stl-test-list-flat-paged_rt ${t}/list.flat.paged_rt.cpp ${t}/list.hpp)
c4stl_add_test(c4stl-test-list-flat-raw      ${t}/list.flat.raw.cpp      ${t}/list.hpp)
c4stl_add_test(c4stl-test-list-flat-fixed    ${t}/list.flat.fixed.cpp    ${t}/list.hpp)
c4stl_add_test(c4stl-test-list-flat-small    ${t}/list.flat.small.cpp    ${t}/list.hpp)
c4stl_add_test(c4stl-test-list-split-paged    ${t}/list.split.paged.cpp    ${t}/list.hpp)
c4stl_add_test(c4stl-test-list-split-paged_rt ${t}/list.split.paged_rt.cpp ${t}/list.hpp)
c4stl_add_test(c4stl-test-list-split-raw      ${t}/list.split.raw.cpp      ${t}/list.hpp)
c4stl_add_test(c4stl-test-list-split-fixed    ${t}/list.split.fixed.cpp    ${t}/list.hpp)
c4stl_add_test(c4stl-test-list-split-small    ${t}/list.split.small.cpp    ${t}/list.hpp)
c4stl_add_test(c4stl-test-list-flat_fwd-paged    ${t}/list.flat_fwd.paged.cpp    ${t}/list.hpp)
c4stl_add_test(c4stl-test-list-flat_fwd-paged_rt ${t}/list.flat_fwd.paged_rt.cpp ${t}/list.hpp)
c4stl_add_test(c4stl-test-list-flat_fwd-raw      ${t}/list.flat_fwd.raw.cpp      ${t}/list.hpp)
c4stl_add_test(c4stl-test-list-flat_fwd-fixed    ${t}/list.flat_fwd.fixed.cpp    ${t}/list.hpp)
c4stl_add_test(c4stl-test-list-flat_fwd-small    ${t}/list.flat_fwd.small.cpp    ${t}/list.hpp)
c4stl_add_test(c4stl-test-list-split_fwd-paged    ${t}/list.split_fwd.paged.cpp    ${t}/list.hpp)
c4stl_add_test(c4stl-test-list-split_fwd-paged_rt ${t}/list.split_fwd.paged_rt.cpp ${t}/list.hpp)
c4stl_add_test(c4stl-test-list-split_fwd-raw      ${t}/list.split_fwd.raw.cpp      ${t}/list.hpp)
c4stl_add_test(c4stl-test-list-split_fwd-fixed    ${t}/list.split_fwd.fixed.cpp    ${t}/list.hpp)
c4stl_add_test(c4stl-test-list-split_fwd-small    ${t}/list.split_fwd.small.cpp    ${t}/list.hpp)

if(WIN32)
    if(MSVC AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC") # it may be clang as well
        set(bigfiles
            ${t}/list.flat.paged.cpp
            ${t}/list.flat.paged_rt.cpp
            ${t}/list.flat.raw.cpp
            ${t}/list.flat.fixed.cpp
            ${t}/list.flat.small.cpp
            ${t}/list.split.paged.cpp
            ${t}/list.split.paged_rt.cpp
            ${t}/list.split.raw.cpp
            ${t}/list.split.fixed.cpp
            ${t}/list.split.small.cpp
            ${t}/list.flat_fwd.paged.cpp
            ${t}/list.flat_fwd.paged_rt.cpp
            ${t}/list.flat_fwd.raw.cpp
            ${t}/list.flat_fwd.fixed.cpp
            ${t}/list.flat_fwd.small.cpp
            ${t}/list.split_fwd.paged.cpp
            ${t}/list.split_fwd.paged_rt.cpp
            ${t}/list.split_fwd.raw.cpp
            ${t}/list.split_fwd.fixed.cpp
            ${t}/list.split_fwd.small.cpp
            ${t}/sstream.cpp
            ${t}/string.string.cpp
            ${t}/string.substring.cpp
            ${t}/string.substringrs.cpp
            ${t}/string.text.cpp
            )
        foreach(_bf ${bigfiles})
            set_property(SOURCE ${_bf} APPEND_STRING PROPERTY COMPILE_FLAGS " /bigobj ")
        endforeach()
    endif()
endif()
