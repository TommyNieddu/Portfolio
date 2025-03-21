cmake_minimum_required(VERSION 3.27)
project(so_prog_threads C)

set(CMAKE_C_STANDARD 99)

add_executable(so_prog_threads main.c
        npc_rana.h
        npc_rana.c
        padre.h
        padre.c
        prato_tane.h
        prato_tane.c
        regole_gioco.h
        regole_gioco.c
        buffer.h
        buffer.c
)
