cmake_minimum_required(VERSION 3.16)

project(CJAM LANGUAGES CXX)

set(CMAKE_CXX_STRANDERED 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

add_executable(CJAM
        src/main.cpp
)

# target_link_libraries(RUN
#        sfml-graphics
#        sfml-window
#        sfml-system
# )