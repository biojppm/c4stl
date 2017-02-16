cmake_minimum_required(VERSION 3.6)
project(c4stl)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
include(CreateSourceGroup)

add_subdirectory(src)
