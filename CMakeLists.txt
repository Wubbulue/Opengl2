cmake_minimum_required(VERSION 3.10)
set (CMAKE_CXX_STANDARD 17)

#find_package(OpenGL REQUIRED)

 

set (ThirdParty ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty)

add_library(glad ${ThirdParty}/lib/glad/glad.c)

message(${ThirdParty})

PROJECT (GL_Practice)


add_executable( GL_Practice main.cpp shader.h stb_image.h camera.h camera.cpp 
minecraft.h minecraft.cpp PerlinNoise.h EBO.cpp EBO.h VBO.cpp VBO.h VAO.cpp VAO.h
line.cpp line.h geometery.h geometery.cpp Logging.h Logging.cpp Player.h Player.cpp 
font.h font.cpp Colors.h save.h save.cpp Timer.h Timer.cpp ThreadPool.h ThreadPool.cpp)

include_directories(${ThirdParty}/include ${CMAKE_CURRENT_SOURCE_DIR}/glm)


TARGET_LINK_LIBRARIES (GL_Practice PUBLIC ${ThirdParty}/lib/glfw/glfw3.lib ${ThirdParty}/lib/freetype/freetype.lib glad)         


