cmake_minimum_required(VERSION 2.8)
aux_source_directory(. DIR_SRCS)
add_library(JSON ${DIR_SRCS})
