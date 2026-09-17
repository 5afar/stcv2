add_executable(users_client
    main.cpp
)

target_include_directories(users_client PRIVATE ${CMAKE_SOURCE_DIR})

target_link_libraries(users_client PRIVATE
    protocol
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Widgets
)