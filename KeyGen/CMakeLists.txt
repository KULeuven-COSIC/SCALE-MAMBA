set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "../")

add_library(keygen OBJECT
        KeyGenM.cpp
        MASCOTTriples.cpp
        )

add_executable(PlayerKeyGen.x PlayerKeyGen.cpp)
target_link_libraries(PlayerKeyGen.x PRIVATE scale keygen)
