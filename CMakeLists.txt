cmake_minimum_required(VERSION 2.8)

SET(CMAKE_C_COMPILER   gcc)
SET(CMAKE_CXX_COMPILER g++)

SET(CMAKE_BUILD_TYPE "Debug")

set(CMAKE_EXE_LINKER_FLAGS "-static")

project(mediac)

include_directories(./ ./MQTTPacket ./JSON)

# Add block directories
add_subdirectory(MQTTPacket)
add_subdirectory(JSON)
aux_source_directory(. DIR_MEDIAC_SRCS)

add_executable(p2ptun ${DIR_MEDIAC_SRCS})#上述方法有问题，这种方法才有效
target_link_libraries(p2ptun JSON)
target_link_libraries(p2ptun MQTTPacket)#将多目录下的hello库和world库链接上可执行文件helloworld.o
target_link_libraries(p2ptun -lpthread)
target_link_libraries(p2ptun -lm)
