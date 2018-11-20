find_package(Threads REQUIRED)
find_package(SDL2)
find_package(GLEW)
find_package(OpenGL)
find_package(OpenAL)
find_package(PkgConfig)
pkg_check_modules(VORBIS vorbisfile)
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${GLEW_INCLUDE_DIRS})
include_directories(${OPENGL_INCLUDE_DIRS})
include_directories(${OPENAL_INCLUDE_DIR})
include_directories(${VORBIS_INCLUDE_DIRS})
#include_directories(${PROJECT_SOURCE_DIR}/external/stb)
#include_directories(${PROJECT_SOURCE_DIR}/external/nuklear)
include_directories(${PROJECT_SOURCE_DIR}/external)
include_directories(${PROJECT_SOURCE_DIR}/external/entt/src)
include_directories(${PROJECT_SOURCE_DIR}/external/glm)

add_subdirectory(rs4)

add_executable(rs4game
    main
    planet_graphics planet_sound
    menu_graphics)

target_link_libraries(rs4game rs4engine
    ${SDL2_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${OPENAL_LIBRARY}
    ${VORBIS_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT})

