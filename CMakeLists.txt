cmake_minimum_required(VERSION 3.2)
project(c4stl)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
include(ConfigurationTypes)
include(CreateSourceGroup)
include(SanitizeTarget)

set(C4STL_EXTERN_DIR ${CMAKE_INSTALL_PREFIX} CACHE PATH "")

setup_sanitize(C4STL)
option(C4STL_SANITIZE_ONLY "don't compile unsanitized targets" OFF)

option(C4STL_PVS_STUDIO_ENABLED "use the PVS-Studio static analyzer https://www.viva64.com/en/b/0457/" OFF)
if(C4STL_PVS_STUDIO_ENABLED)
    set(C4STL_PVS_STUDIO_FORMAT "errorfile" CACHE STRING "PVS-Studio output format. Choices: xml,csv,errorfile(like gcc/clang),tasklist(qtcreator)")
    #set(C4STL_PVS_STUDIO_EXCLUDES "" CACHE STRING "PVS-Studio exclude list: directories to be skipped")
    #set(C4STL_PVS_STUDIO_SUPPRESS "" CACHE STRING "PVS-Studio diagnostic suppressions. Eg V595,V730")
    #set(C4STL_PVS_STUDIO_FILTER_RULES "" CACHE STRING "PVS-Studio diagnostic suppressions")
endif()

if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

option(C4STL_BUILD_TESTS "build unit tests" ON)
option(C4STL_BUILD_BENCHMARKS "build benchmarks" ON)
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
        FOLDER
    )
    set(optionsnarg
        SOURCES
        INC_DIRS
        LIBS
        MORE_ARGS
    )
    cmake_parse_arguments(_c4al "${options0arg}" "${options1arg}" "${optionsnarg}" ${ARGN})
    if(${_c4al_LIBRARY})
        set(_what LIBRARY)
    elseif(${_c4al_EXECUTABLE})
        set(_what EXECUTABLE)
    endif()

    create_source_group("" "${CMAKE_CURRENT_SOURCE_DIR}" "${_c4al_SOURCES}")
    if(NOT C4STL_SANITIZE_ONLY)
        if(${_c4al_LIBRARY})
            add_library(${name} ${_c4al_SOURCES} ${_c4al_MORE_ARGS})
        elseif(${_c4al_EXECUTABLE})
            add_executable(${name} ${_c4al_SOURCES} ${_c4al_MORE_ARGS})
        endif()
        if(_c4al_INC_DIRS)
            target_include_directories(${name} PUBLIC ${_c4al_INC_DIRS})
        endif()
        if(_c4al_LIBS)
            target_link_libraries(${name} PUBLIC ${_c4al_LIBS})
        endif()
        if(_c4al_FOLDER)
            set_target_properties(${name} PROPERTIES FOLDER ${_c4al_FOLDER})
        endif()
        if(C4STL_PVS_STUDIO_ENABLED)
            include(PVS-Studio)
            include(GetFlags)
            include(GetTargetPropertyRecursive)
            get_target_property_recursive(_c4al_pvs_incs ${name} INCLUDE_DIRECTORIES)
            get_include_flags(_c4al_pvs_incs ${_c4al_pvs_incs})
            separate_arguments(_c4al_cxx_flags_sep UNIX_COMMAND "${CMAKE_CXX_FLAGS} ${_c4al_pvs_incs}")
            separate_arguments(_c4al_c_flags_sep UNIX_COMMAND "${CMAKE_C_FLAGS} ${_c4al_pvs_incs}")
            pvs_studio_add_target(TARGET ${name}-pvs
                ALL # indicates that the analysis starts when you build the project
                #PREPROCESSOR ${_c4al_preproc}
                FORMAT tasklist
                LOG "${CMAKE_CURRENT_BINARY_DIR}/${name}.pvs-analysis.tasks"
                ANALYZE ${name} #main_target subtarget:path/to/subtarget
                CXX_FLAGS ${_c4al_cxx_flags_sep}
                C_FLAGS ${_c4al_c_flags_sep}
                #CONFIG "/path/to/PVS-Studio.cfg"
                )
            if(NOT TARGET c4stl-pvs-all)
                add_custom_target(c4stl-pvs-all)
            endif()
            add_dependencies(c4stl-pvs-all ${name}-pvs)
            #pvs_studio_add_target(TARGET ${name}-pvs
            #    OUTPUT FORMAT ${C4STL_PVS_STUDIO_FORMAT}
            #    ANALYZE ${name}
            #    LOG "${CMAKE_CURRENT_BINARY_DIR}/${name}.pvs-report.plog"
            #    #LICENSE "/path/to/PVS-Studio.lic"
            #    )
        endif(C4STL_PVS_STUDIO_ENABLED)
    endif()
    if(_c4al_SANITIZE AND C4STL_SANITIZE)
        sanitize_target(${name} c4stl
            ${_what}
            SOURCES ${_c4al_SOURCES}
            INC_DIRS ${_c4al_INC_DIRS}
            LIBS ${_c4al_LIBS}
            OUTPUT_TARGET_NAMES targets
            FOLDER "${_c4al_FOLDER}"
            )
    endif()
    if(NOT C4STL_SANITIZE_ONLY)
        list(INSERT targets 0 ${name})
    endif()
    if(_c4al_OUTPUT_TARGET_NAMES)
        set(${_c4al_OUTPUT_TARGET_NAMES} ${targets} PARENT_SCOPE)
    endif()
endfunction() # c4stl_add_target


add_subdirectory(src)

if(C4STL_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

if(C4STL_BUILD_BENCHMARKS)
    add_subdirectory(bm)
endif()
