
add_library(rs4engine STATIC
    rs4 rs4_shaders rs4_textures rs4_pcm
    )

target_compile_features(rs4engine PUBLIC cxx_std_17)

target_link_libraries(rs4engine
    PRIVATE ${CONAN_LIBS}
    PRIVATE Threads::Threads
    )

#TODO use Conan package and CMake target for nuklear
target_include_directories(rs4engine PUBLIC ${PROJECT_SOURCE_DIR}/external)
