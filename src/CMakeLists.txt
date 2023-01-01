set(SOURCE_FILES
	fixed.c		fixed.h
	main.cpp)

if(WIN32)
	add_executable(eternal-horror WIN32 ${SOURCE_FILES})
else()
	add_executable(eternal-horror ${SOURCE_FILES})
endif()

target_link_libraries(eternal-horror ${SDL2_LIBRARIES} ${SDL2_MIXER_LIBRARIES})