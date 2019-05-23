cmake_minimum_required(VERSION 3.12)
project(cv_gstream)

set(CMAKE_CXX_STANDARD 11)

#1.判断编译类型release or debug，没有指定时，默认为release模式
if(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Release)
endif()


#set(OpenCV_DIR /usr/local/opencv-2.4.13.6/share/OpenCV)
set(OpenCV_DIR /usr/local/opencv3.4.1/share/OpenCV)
find_package( OpenCV REQUIRED )

add_executable(cv_gstream main.cpp)


add_executable(sender sender.cpp)
target_link_libraries(sender ${OpenCV_LIBS})

add_executable(reciver reciver.cpp)
target_link_libraries(reciver ${OpenCV_LIBS})