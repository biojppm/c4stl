include(ExternalProjectUtils)

ExternalProject_Import(gbenchmark
    PREFIX ${C4STL_EXTERN_DIR}
    INCLUDES benchmark/benchmark.h
    LIBRARIES benchmark)
find_package(Threads REQUIRED)

set(c4stl_bm_dir ${CMAKE_CURRENT_BINARY_DIR})

function(c4stl_add_bm name_wo_pfx)
    set(_abm_name c4stl-bm-${name_wo_pfx})
    string(REPLACE "-" "/" _abm_folder ${name_wo_pfx})

    # target to build the benchmark executable
    add_executable(${_abm_name} ${ARGN})
    target_include_directories(${_abm_name}
        PUBLIC ${gbenchmark_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/../test)
    target_link_libraries(${_abm_name}
        PUBLIC c4stl ${gbenchmark_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
    get_filename_component(_abm_output_dir ${_abm_folder} DIRECTORY)
    get_filename_component(_abm_output_exe ${_abm_folder} NAME)
    set_target_properties(${_abm_name} PROPERTIES
        OUTPUT_NAME ${_abm_output_exe}
        RUNTIME_OUTPUT_DIRECTORY ${c4stl_bm_dir}/${_abm_output_dir}
        FOLDER "c4stl-bm/${_abm_folder}")

    # output file for the benchmark
    if(NOT EXISTS "${c4stl_bm_dir}/results/${_abm_output_dir}")
        file(MAKE_DIRECTORY "${c4stl_bm_dir}/results/${_abm_output_dir}")
    endif()
    set(_abm_output "${c4stl_bm_dir}/results/${_abm_output_dir}/${_abm_output_exe}.csv")

    # target to run the benchmark
    add_custom_command(OUTPUT ${_abm_output}
        COMMAND $<TARGET_FILE:${_abm_name}> --benchmark_out_format=csv --benchmark_out=${_abm_output}
        DEPENDS ${_abm_name}
        COMMENT "${_abm_name} - writing to ${_abm_output}")
    add_custom_target(${_abm_name}-run
        DEPENDS ${_abm_output})
    #add_custom_target(${_abm_name}-run
    #    COMMAND $<TARGET_FILE:${_abm_name}> --benchmark_out_format=csv --benchmark_out=${_abm_output}
    #    BYPRODUCTS ${_abm_output}
    #    DEPENDS ${_abm_name}
    #    COMMENT "${_abm_name} - writing to ${_abm_output}")
    set_target_properties(${_abm_name}-run PROPERTIES
        FOLDER "c4stl-bm/${_abm_folder}")

    # add convenience targets to allow building or running several benchmarks at once
    if(NOT TARGET c4stl-bm)
        add_custom_target(c4stl-bm)
        set_target_properties(c4stl-bm PROPERTIES FOLDER c4stl-bm)
    endif()
    add_dependencies(c4stl-bm ${_abm_name})
    if(NOT TARGET c4stl-bm-run)
        add_custom_target(c4stl-bm-run)
        set_target_properties(c4stl-bm-run PROPERTIES FOLDER c4stl-bm)
        add_dependencies(c4stl-bm-run c4stl-bm)
    endif()
    add_dependencies(c4stl-bm-run ${_abm_name}-run)
    string(REPLACE "/" ";" _abm_split ${_abm_folder})   # example: list/push_back_with_reserve/split_fwd_list__paged__NumBytes_4096___int32_t ---> list;push_back_with_reserve;split_fwd_list__paged__NumBytes_4096___int32_t
    string(REPLACE "__" ";" _abm_split "${_abm_split}") # example: list;push_back_with_reserve;split_fwd_list__paged__NumBytes_4096___int32_t ---> list;push_back_with_reserve;split_fwd_list;paged;NumBytes_4096;_int32_t
    list(REMOVE_AT _abm_split -1) # pop the last list element
    foreach(_abm_elm ${_abm_split})
        if(NOT _abm_curr_tgt)
            set(_abm_curr_tgt "c4stl-bm-${_abm_elm}")
        else()
            set(_abm_curr_tgt "${_abm_curr_tgt}-${_abm_elm}")
        endif()
        if(NOT TARGET ${_abm_curr_tgt})
            add_custom_target(${_abm_curr_tgt})
        endif()
        add_dependencies(${_abm_curr_tgt} ${_abm_name})
        if(NOT TARGET ${_abm_curr_tgt}-run)
            add_custom_target(${_abm_curr_tgt}-run)
            add_dependencies(${_abm_curr_tgt}-run ${_abm_curr_tgt})
        endif()
        add_dependencies(${_abm_curr_tgt}-run ${_abm_name}-run)
    endforeach()
endfunction()

add_subdirectory(list)
