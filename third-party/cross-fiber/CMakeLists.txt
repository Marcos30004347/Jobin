cmake_minimum_required(VERSION 3.10)

project(cross-fiber)

add_library(cross-fiber INTERFACE)
target_include_directories(cross-fiber INTERFACE .)
