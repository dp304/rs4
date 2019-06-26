find_package(Threads REQUIRED)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_subdirectory(rs4)

add_executable(rs4game
    main
    planet_graphics planet_sound
    menu_graphics
    )

target_link_libraries(rs4game
    PRIVATE rs4engine
    )
