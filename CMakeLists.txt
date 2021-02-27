cmake_minimum_required(VERSION 3.13.4)

project("main")

set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME}
	ncurses
)
