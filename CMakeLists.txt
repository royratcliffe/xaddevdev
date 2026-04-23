cmake_minimum_required(VERSION 3.10)
project(xaddevdev)

add_executable(xaddevdev
    xaddevdev.c
    xadd.c
    ev.c
    devinput.c
    input_devices.c
    evioc.c
    inotify.c
    epoll.c
    cons.c
    cons_str.c
)
target_include_directories(xaddevdev PRIVATE .)

# libhiredis-dev is required for Redis support, which is used for caching input
# device information and events. The library provides a C client for Redis,
# allowing the application to interact with a Redis server for efficient data
# storage and retrieval. The find_library and find_path commands are used to
# locate the hiredis library and its header files, ensuring that the application
# can link against the library and include the necessary headers for Redis
# functionality.
find_package(PkgConfig REQUIRED)
pkg_check_modules(HIREDIS hiredis)
if(HIREDIS_FOUND)
    message(STATUS "Found hiredis library: ${HIREDIS_LIBRARIES}")

    target_include_directories(xaddevdev PRIVATE ${HIREDIS_INCLUDE_DIRS})
    target_link_libraries(xaddevdev PRIVATE ${HIREDIS_LIBRARIES})
else()
    message(WARNING "hiredis library not found. Redis support will be disabled.")
endif()
