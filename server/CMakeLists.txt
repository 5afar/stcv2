add_executable(users_server
    main.cpp
    database.h
    database.cpp
    server.h server.cpp
    clientconnection.h clientconnection.cpp
    requesttask.h requesttask.cpp
)

target_include_directories(users_server PRIVATE ${CMAKE_SOURCE_DIR})

target_link_libraries(users_server PRIVATE
    protocol
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Sql
)