cmake_minimum_required(VERSION 2.8)

SET(CMAKE_C_COMPILER   gcc)
SET(CMAKE_CXX_COMPILER g++)

SET(CMAKE_BUILD_TYPE "Debug")

set(CMAKE_EXE_LINKER_FLAGS "-static")

project(p2ptun)

include_directories(./ ./JSON ./KCP)

# Add block directories
add_subdirectory(JSON)
add_subdirectory(KCP)
aux_source_directory(. DIR_P2PTUN_SRCS)

add_executable(p2ptun ${DIR_P2PTUN_SRCS})#上述方法有问题，这种方法才有效
target_link_libraries(p2ptun JSON)
target_link_libraries(p2ptun KCP)
target_link_libraries(p2ptun -lpthread)
target_link_libraries(p2ptun -lm)
