cmake_minimum_required(VERSION 3.10)

project(myvim)

add_executable(myvim
    src/main.c
    src/text_buffer/buffer.c
    src/render/screen.c
    src/config/terminal.c
    src/control/normal_mode.c
    src/control/insert_mode.c
    src/control/command_mode.c
    src/control/commands.c
    src/files/file_manager.c
    src/windows/windows.c
    src/render/render_config.c
    src/text_buffer/macros.c
)

target_include_directories(myvim PRIVATE includes)

