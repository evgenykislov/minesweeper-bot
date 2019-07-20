cmake_minimum_required(VERSION 2.8)

project(models_checker)

set(SRC_LIST "main.cpp")
set(SRC_LIST ${SRC_LIST} "classifier.h" "classifier.cpp")
set(SRC_LIST ${SRC_LIST} "tetragonal_neural.h" "tetragonal_neural.cpp")
set(SRC_LIST ${SRC_LIST} "bruteforce.h" "bruteforce.cpp")
set(SRC_LIST ${SRC_LIST} "getopt.h" "getopt.c")

add_executable(${PROJECT_NAME} ${SRC_LIST})
