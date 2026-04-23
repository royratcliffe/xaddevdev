cmake_minimum_required(VERSION 3.10)
project(xaddevdev)

add_executable(xaddevdev
    xaddevdev.c
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
