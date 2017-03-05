cmake_minimum_required(VERSION 3.5)
project(c4stl)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
include(CreateSourceGroup)
include(SanitizeTarget)
setup_sanitize(C4STL)

set(C4STL_EXTERN_DIR ${CMAKE_INSTALL_PREFIX} CACHE PATH "")

if(NOT MSVC)
    list(APPEND CMAKE_CXX_FLAGS "-std=c++11")
endif()

option(C4STL_BUILD_TESTS "" ON)
option(C4STL_BUILD_BENCHMARKS "" ON)
if(C4STL_BUILD_TESTS OR C4STL_BUILD_BENCHMARKS)
    include(ExternalProjectUtils)
endif()

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
        sanitize_target(${name} C4STL
            ${_what}
            SOURCES ${_c4al_SOURCES}
            INC_DIRS ${_c4al_INC_DIRS}
            LIBS ${_c4al_LIBS}
            OUTPUT_TARGET_NAMES targets
        )
        list(INSERT targets 0 ${name})
    endif()
    if(_c4al_OUTPUT_TARGET_NAMES)
        set(${_c4al_OUTPUT_TARGET_NAMES} ${targets} PARENT_SCOPE)
    endif()
endfunction()


add_subdirectory(src)
if(C4STL_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
if(C4STL_BUILD_BENCHMARKS)
    ExternalProject_Import(gbenchmark
        PREFIX ${C4STL_EXTERN_DIR}
        INCLUDES benchmark/benchmark.h
        LIBRARIES benchmark)
endif(C4STL_BUILD_BENCHMARKS)
