cmake_minimum_required(VERSION 3.7)

project(eternal-horror)

if(MSVC)
	add_definitions("/D_CRT_SECURE_NO_WARNINGS")
endif()

find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIRS})

add_subdirectory(src)