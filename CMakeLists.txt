cmake_minimum_required(VERSION 3.23)
project (swal
    LANGUAGES CXX
    VERSION 0.1
    DESCRIPTION "Simple WinAPI Abstraction Layer"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Target  swal
add_library(swal INTERFACE)
target_sources(swal PUBLIC
FILE_SET HEADERS
BASE_DIRS include
FILES
    include/swal/com.h
    include/swal/enum_bitwise.h
    include/swal/error.h
    include/swal/gdi.h
    include/swal/handle.h
    include/swal/menu.h
    include/swal/reg.h
    include/swal/strconv.h
    include/swal/win_headers.h
    include/swal/window.h
    include/swal/zero_or_resource.h
)

add_library(swal::swal ALIAS swal)
install(TARGETS swal EXPORT swal FILE_SET HEADERS)
install(EXPORT swal NAMESPACE swal:: DESTINATION cmake FILE swal-config.cmake)
