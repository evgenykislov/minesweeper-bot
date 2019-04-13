cmake_minimum_required(VERSION 2.8)

project(models_checker)

set(SRC_LIST "main.cpp")
set(SRC_LIST ${SRC_LIST} "classifier.h" "classifier.cpp")
set(SRC_LIST ${SRC_LIST} "tetragonal_neural.h" "tetragonal_neural.cpp")

add_executable(${PROJECT_NAME} ${SRC_LIST})
