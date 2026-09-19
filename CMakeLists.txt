cmake_minimum_required(VERSION 3.16)

project(stcv2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network Widgets Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network Widgets Sql)
message(STATUS "Using Qt ${QT_VERSION_MAJOR}")

# кодирование/декодирование кадров
add_library(protocol STATIC
    common/protocol.cpp
    common/protocol.h
    common/validation.h
    common/validation.cpp
)

target_include_directories(protocol PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(protocol PUBLIC Qt${QT_VERSION_MAJOR}::Core)

add_subdirectory(server)
add_subdirectory(client)
