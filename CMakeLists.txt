cmake_minimum_required(VERSION 3.10)
project(NuPogodiIR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OpenCV_DIR "/usr/lib/x86_64-linux-gnu/cmake/opencv4")
find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

# Добавляем все наши cpp файлы в сборку
add_executable(ir_game main.cpp wolf.cpp rabbit.cpp map_manager.cpp)
target_link_libraries(ir_game ${OpenCV_LIBS})
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/data" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")