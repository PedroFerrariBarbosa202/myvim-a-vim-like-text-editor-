cmake_minimum_required(VERSION 3.10)

project(myvim)

add_executable(myvim
    src/main.c
    src/buffer.c
    src/screen.c
    src/terminal.c
    src/modes.c
    src/commands.c
    src/file_manager.c
)

target_include_directories(myvim PRIVATE includes)

