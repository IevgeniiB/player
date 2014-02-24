cmake_minimum_required(VERSION 2.8)
add_library(player_lib SHARED player.c player_priv.c keyboard_thread.c)
