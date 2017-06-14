include(ExternalProjectUtils)

ExternalProject_Import(gbenchmark
    PREFIX ${C4STL_EXTERN_DIR}
    INCLUDES benchmark/benchmark.h
    LIBRARIES benchmark)
find_package(Threads REQUIRED)

add_custom_target(c4stl-bm-build)
set_target_properties(c4stl-bm-build PROPERTIES FOLDER c4stl-bm)

add_custom_target(c4stl-bm-run)
set_target_properties(c4stl-bm-run PROPERTIES FOLDER c4stl-bm)
add_dependencies(c4stl-bm-run c4stl-bm-build)

function(c4stl_add_bm name_wo_pfx)
    set(_abm_name c4stl-bm-${name_wo_pfx})
    string(REPLACE "-" "/" _abm_folder ${name_wo_pfx})

    # target to build the benchmark executable
    add_executable(${_abm_name} ${ARGN})
    target_include_directories(${_abm_name}
        PUBLIC ${gbenchmark_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/../test)
    target_link_libraries(${_abm_name}
        PUBLIC c4stl ${gbenchmark_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
    set_target_properties(${_abm_name} PROPERTIES FOLDER "c4stl-bm/${_abm_folder}")
    add_dependencies(c4stl-bm-build ${_abm_name})

    # target to run the benchmark
    add_custom_target(${_abm_name}-run
        COMMAND $<TARGET_FILE:${_abm_name}>
        DEPENDS ${_abm_name})
    set_target_properties(${_abm_name}-run PROPERTIES FOLDER "c4stl-bm/${_abm_folder}")
    add_dependencies(c4stl-bm-run ${_abm_name})
endfunction()

c4stl_add_bm(list-push_back list/push_back.cpp)
