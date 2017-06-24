cmake_minimum_required(VERSION 3.2)
project(c4stl)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(c4stlAddTarget)

option(C4STL_DEV "enable development targets: tests, benchmarks, sanitize, static analysis, coverage" OFF)
option(C4STL_BUILD_TESTS "build unit tests" ${C4STL_DEV})
option(C4STL_BUILD_BENCHMARKS "build benchmarks" ${C4STL_DEV})

setup_sanitize(C4STL ${C4STL_DEV})
setup_static_analysis(C4STL ${C4STL_DEV})

# -----------------------------------------------------------
if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

option(C4STL_PEDANTIC "Compile in pedantic mode" ${C4STL_DEV})
if(C4STL_PEDANTIC)
    if(MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -pedantic -Wfloat-equal -fstrict-aliasing")
    endif()
endif()

option(C4STL_WERROR "Compile with warnings as errors" ${C4STL_DEV})
if(C4STL_WERROR)
    if(MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
        #set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} /WX")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -pedantic-errors")
    endif()
endif()

# -----------------------------------------------------------
set(C4STL_EXTERN_DIR ${CMAKE_INSTALL_PREFIX} CACHE PATH "")

add_subdirectory(src)

if(C4STL_BUILD_TESTS)
    enable_testing() # this must be done here (and not inside the test dir)
                     # so that the test targets are available at the top level
    add_subdirectory(test)
endif()

if(C4STL_BUILD_BENCHMARKS)
    add_subdirectory(bm)
endif()
