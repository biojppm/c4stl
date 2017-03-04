cmake_minimum_required(VERSION 3.5)
project(c4stl)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
include(CreateSourceGroup)

set(C4STL_EXTERN_DIR ${CMAKE_INSTALL_PREFIX} CACHE PATH "")

option(C4STL_BUILD_TESTS "" ON)
option(C4STL_BUILD_BENCHMARKS "" ON)
if(C4STL_BUILD_TESTS OR C4STL_BUILD_BENCHMARKS)
    include(ExternalProjectUtils)
endif()

#-----------------------------------------------------------------------
# clang sanitize
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    option(C4STL_ASAN  "" ON)
    option(C4STL_TSAN  "" ON)
    option(C4STL_MSAN  "" ON)
    option(C4STL_UBSAN "" ON)
    set(C4STL_ASAN_CFLAGS "-O1 -g -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls" CACHE STRING "compile flags for clang address sanitizer: https://clang.llvm.org/docs/AddressSanitizer.html")
    set(C4STL_TSAN_CFLAGS "-O1 -g -fsanitize=thread -fno-omit-frame-pointer" CACHE STRING "compile flags for clang address sanitizer: https://clang.llvm.org/docs/ThreadSanitizer.html")
    set(C4STL_MSAN_CFLAGS "-O1 -g -fsanitize=memory -fsanitize-memory-track-origins -fno-omit-frame-pointer -fno-optimize-sibling-calls" CACHE STRING "compile flags for clang address sanitizer: https://clang.llvm.org/docs/MemorySanitizer.html")
    set(C4STL_UBSAN_CFLAGS "-g -fsanitize=undefined" CACHE STRING "compile flags for clang address sanitizer: https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html")
    set(C4STL_ASAN_LFLAGS "-g -fsanitize=address" CACHE STRING "linker flags for clang address sanitizer: https://clang.llvm.org/docs/AddressSanitizer.html")
    set(C4STL_TSAN_LFLAGS "-g -fsanitize=thread" CACHE STRING "linker flags for clang address sanitizer: https://clang.llvm.org/docs/ThreadSanitizer.html")
    set(C4STL_MSAN_LFLAGS "-g -fsanitize=memory" CACHE STRING "linker flags for clang address sanitizer: https://clang.llvm.org/docs/MemorySanitizer.html")
    set(C4STL_UBSAN_LFLAGS "-g -fsanitize=undefined" CACHE STRING "linker flags for clang address sanitizer: https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html")

    # the flags are strings; we need to separate them into a list
    # to prevent cmake from quoting them when passing to the targets
    separate_arguments(C4STL_ASAN_CFLAGS_SEP  UNIX_COMMAND ${C4STL_ASAN_CFLAGS})
    separate_arguments(C4STL_TSAN_CFLAGS_SEP  UNIX_COMMAND ${C4STL_TSAN_CFLAGS})
    separate_arguments(C4STL_MSAN_CFLAGS_SEP  UNIX_COMMAND ${C4STL_MSAN_CFLAGS})
    separate_arguments(C4STL_UBSAN_CFLAGS_SEP UNIX_COMMAND ${C4STL_UBSAN_CFLAGS})
    separate_arguments(C4STL_ASAN_LFLAGS_SEP  UNIX_COMMAND ${C4STL_ASAN_LFLAGS})
    separate_arguments(C4STL_TSAN_LFLAGS_SEP  UNIX_COMMAND ${C4STL_TSAN_LFLAGS})
    separate_arguments(C4STL_MSAN_LFLAGS_SEP  UNIX_COMMAND ${C4STL_MSAN_LFLAGS})
    separate_arguments(C4STL_UBSAN_LFLAGS_SEP UNIX_COMMAND ${C4STL_UBSAN_LFLAGS})
endif()

function(c4_sanitize_target name)
    set(options0arg
        LIBRARY
        EXECUTABLE
    )
    set(options1arg
        OUTPUT_TARGET_NAMES
    )
    set(optionsnarg
        SOURCES
        INC_DIRS
        LIBS
        LIB_DIRS
    )
    cmake_parse_arguments(_c4st "${options0arg}" "${options1arg}" "${optionsnarg}" ${ARGN})

    if(TARGET sanitize)
    else()
        add_custom_target(sanitize)
    endif()
    if(TARGET asan)
    else()
        add_custom_target(asan)
        add_dependencies(sanitize asan)
    endif()
    if(TARGET msan)
    else()
        add_custom_target(msan)
        add_dependencies(sanitize msan)
    endif()
    if(TARGET tsan)
    else()
        add_custom_target(tsan)
        add_dependencies(sanitize tsan)
    endif()
    if(TARGET ubsan)
    else()
        add_custom_target(ubsan)
        add_dependencies(sanitize ubsan)
    endif()

    set(targets)

    # https://clang.llvm.org/docs/AddressSanitizer.html
    if(C4STL_ASAN)
        if(${_c4st_LIBRARY})
            add_library(${name}-asan EXCLUDE_FROM_ALL ${_c4st_SOURCES})
        elseif(${_c4st_EXECUTABLE})
            add_executable(${name}-asan EXCLUDE_FROM_ALL ${_c4st_SOURCES})
        endif()
        list(APPEND targets ${name}-asan)
        target_include_directories(${name}-asan PUBLIC ${_c4st_INC_DIRS})
        set(_real_libs)
        foreach(_l ${_c4st_LIBS})
            if(TARGET ${_l}-asan)
                list(APPEND _real_libs ${_l}-asan)
            else()
                list(APPEND _real_libs ${_l})
            endif()
        endforeach()
        target_link_libraries(${name}-asan PUBLIC ${_real_libs})
        target_compile_options(${name}-asan PUBLIC ${C4STL_ASAN_CFLAGS_SEP})
        # http://stackoverflow.com/questions/25043458/does-cmake-have-something-like-target-link-options
        target_link_libraries(${name}-asan PUBLIC ${C4STL_ASAN_LFLAGS_SEP})
        add_dependencies(asan ${name}-asan)
    endif()

    # https://clang.llvm.org/docs/ThreadSanitizer.html
    if(C4STL_TSAN)
        if(${_c4st_LIBRARY})
            add_library(${name}-tsan EXCLUDE_FROM_ALL ${_c4st_SOURCES})
        elseif(${_c4st_EXECUTABLE})
            add_executable(${name}-tsan EXCLUDE_FROM_ALL ${_c4st_SOURCES})
        endif()
        list(APPEND targets ${name}-tsan)
        target_include_directories(c4stl-tsan PUBLIC ${_c4st_INC_DIRS})
        set(_real_libs)
        foreach(_l ${_c4st_LIBS})
            if(TARGET ${_l}-tsan)
                list(APPEND _real_libs ${_l}-tsan)
            else()
                list(APPEND _real_libs ${_l})
            endif()
        endforeach()
        target_link_libraries(${name}-tsan PUBLIC ${_real_libs})
        target_compile_options(${name}-tsan PUBLIC ${C4STL_TSAN_CFLAGS_SEP})
        # http://stackoverflow.com/questions/25043458/does-cmake-have-something-like-target-link-options
        target_link_libraries(${name}-tsan PUBLIC ${C4STL_TSAN_LFLAGS_SEP})
        add_dependencies(tsan ${name}-tsan)
    endif()

    # https://clang.llvm.org/docs/MemorySanitizer.html
    if(C4STL_MSAN)
        if(${_c4st_LIBRARY})
            add_library(${name}-msan EXCLUDE_FROM_ALL ${_c4st_SOURCES})
        elseif(${_c4st_EXECUTABLE})
            add_executable(${name}-msan EXCLUDE_FROM_ALL ${_c4st_SOURCES})
        endif()
        list(APPEND targets ${name}-msan)
        target_include_directories(c4stl-msan PUBLIC ${_c4st_INC_DIRS})
        set(_real_libs)
        foreach(_l ${_c4st_LIBS})
            if(TARGET ${_l}-msan)
                list(APPEND _real_libs ${_l}-msan)
            else()
                list(APPEND _real_libs ${_l})
            endif()
        endforeach()
        target_link_libraries(${name}-msan PUBLIC ${_real_libs})
        target_compile_options(${name}-msan PUBLIC ${C4STL_MSAN_CFLAGS_SEP})
        # http://stackoverflow.com/questions/25043458/does-cmake-have-something-like-target-link-options
        target_link_libraries(${name}-msan PUBLIC ${C4STL_MSAN_LFLAGS_SEP})
        add_dependencies(msan ${name}-msan)
    endif()

    # https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html
    if(C4STL_UBSAN)
        if(${_c4st_LIBRARY})
            add_library(${name}-ubsan EXCLUDE_FROM_ALL ${_c4st_SOURCES})
        elseif(${_c4st_EXECUTABLE})
            add_executable(${name}-ubsan EXCLUDE_FROM_ALL ${_c4st_SOURCES})
        endif()
        list(APPEND targets ${name}-ubsan)
        target_include_directories(c4stl-ubsan PUBLIC ${_c4st_INC_DIRS})
        set(_real_libs)
        foreach(_l ${_c4st_LIBS})
            if(TARGET ${_l}-ubsan)
                list(APPEND _real_libs ${_l}-ubsan)
            else()
                list(APPEND _real_libs ${_l})
            endif()
        endforeach()
        target_link_libraries(${name}-ubsan PUBLIC ${_real_libs})
        target_compile_options(${name}-ubsan PUBLIC ${C4STL_UBSAN_CFLAGS_SEP})
        # http://stackoverflow.com/questions/25043458/does-cmake-have-something-like-target-link-options
        target_link_libraries(${name}-ubsan PUBLIC ${C4STL_UBSAN_LFLAGS_SEP})
        add_dependencies(ubsan ${name}-ubsan)
    endif()

    if(${_c4st_OUTPUT_TARGET_NAMES})
        set(${_c4st_OUTPUT_TARGET_NAMES} ${targets} PARENT_SCOPE)
    endif()
endfunction()


function(c4stl_add_target name)
    set(options0arg
        LIBRARY
        EXECUTABLE
        SANITIZE
    )
    set(options1arg
        OUTPUT_TARGET_NAMES
    )
    set(optionsnarg
        SOURCES
        INC_DIRS
        LIBS
        MORE_ARGS
    )
    cmake_parse_arguments(_c4al "${options0arg}" "${options1arg}" "${optionsnarg}" ${ARGN})
    if(${_c4al_LIBRARY})
        add_library(${name} ${_c4al_SOURCES} ${_c4al_MORE_ARGS})
        set(_what LIBRARY)
    elseif(${_c4al_EXECUTABLE})
        add_executable(${name} ${_c4al_SOURCES} ${_c4al_MORE_ARGS})
        set(_what EXECUTABLE)
    endif()
    if(_c4al_INC_DIRS)
        target_include_directories(${name} PUBLIC ${_c4al_INC_DIRS})
    endif()
    if(_c4al_LIBS)
        target_link_libraries(${name} PUBLIC ${_c4al_LIBS})
    endif()
    if(NOT ${_c4al_SANITIZE})
        set(targets ${name})
    else()
        c4_sanitize_target(${name}
            ${_what}
            SOURCES ${_c4al_SOURCES}
            INC_DIRS ${_c4al_INC_DIRS}
            LIBS ${_c4al_LIBS}
            OUTPUT_TARGET_NAMES targets
        )
        list(INSERT targets 0 ${name})
    endif()
    if(${_c4al_OUTPUT_TARGET_NAMES})
        set(${_c4al_OUTPUT_TARGET_NAMES} ${name} ${targets} PARENT_SCOPE)
    endif()
endfunction()


add_subdirectory(src)
