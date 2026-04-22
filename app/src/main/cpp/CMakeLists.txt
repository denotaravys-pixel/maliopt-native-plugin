cmake_minimum_required(VERSION 3.22.1)
project("maliopt")

add_library(maliopt SHARED
    mali_opt_extensions.c
    mali_opt_egl.c
)

find_library(log-lib log)
find_library(GLESv3-lib GLESv3)
find_library(EGL-lib EGL)

target_link_libraries(maliopt
    ${log-lib}
    ${GLESv3-lib}
    ${EGL-lib}
)

target_compile_options(maliopt PRIVATE
    -O2
    -fvisibility=hidden
    -Wall
)
