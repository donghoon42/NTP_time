cmake_minimum_required(VERSION 3.10.1)
project(ntp_time_test)

SET(ROOT_PATH ${CMAKE_CURRENT_SOURE_DIR})



add_executable(ntp_time_test ${ROOT_PATH}get_time.cpp)




