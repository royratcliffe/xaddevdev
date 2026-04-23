cmake_minimum_required(VERSION 3.10)
project(xaddevdev)

add_executable(xaddevdev
    xaddevdev.c
    ev.c
    devinput.c
    epoll.c
    inotify.c
    cons.c
    cons_str.c
)
target_include_directories(xaddevdev PRIVATE .)
