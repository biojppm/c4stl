
set(src
    c4/allocator.hpp
    c4/char_traits.cpp
    c4/char_traits.hpp
    c4/common.hpp
    c4/compiler.hpp
    c4/config.hpp
    c4/cpu.hpp
    c4/ctor_dtor.hpp
    c4/error.cpp
    c4/error.hpp
    c4/hash.hpp
    c4/language.hpp
    c4/language.cpp
    c4/list.hpp
    c4/log.cpp
    c4/log.hpp
    c4/memory_resource.cpp
    c4/memory_resource.hpp
    c4/memory_util.cpp
    c4/memory_util.hpp
    c4/platform.hpp
    c4/preprocessor.hpp
    c4/span.hpp
    c4/sstream.cpp
    c4/sstream.hpp
    c4/sstream.def.hpp
    c4/storage/contiguous.hpp
    c4/storage/growth.cpp
    c4/storage/growth.hpp
    c4/storage/raw.cpp
    c4/storage/raw.hpp
    c4/string.cpp
    c4/string.hpp
    c4/string_fwd.hpp
    c4/szconv.hpp
    c4/time.hpp
    c4/time.cpp
    c4/type_name.hpp
    c4/types.hpp
    c4/windows.hpp
    c4/c4stl.natvis
)

c4stl_add_target(c4stl LIBRARY SANITIZE
    SOURCES ${src}
    INC_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../extern
)
