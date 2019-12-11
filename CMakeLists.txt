cmake_minimum_required(VERSION 3.12)
project(cv_gstream)

set(CMAKE_CXX_STANDARD 11)

#1.判断编译类型release or debug，没有指定时，默认为release模式
if(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Release)
endif()


find_package(PkgConfig)
pkg_search_module(GSTREAMER REQUIRED gstreamer-1.0)
pkg_search_module(GSTREAMER_RTSP_SERVER REQUIRED gstreamer-rtsp-server-1.0)
pkg_check_modules(FFMPEG REQUIRED libavdevice libavformat libavfilter libavcodec libswscale libavutil )
include_directories( ${GST_INCLUDE_DIRS} )
include_directories(${GSTREAMER_RTSP_SERVER_INCLUDE_DIRS})


#set(OpenCV_DIR /usr/local/opencv-2.4.13.6/share/OpenCV)
set(OpenCV_DIR /usr/local/opencv3.4.1/share/OpenCV)
find_package( OpenCV REQUIRED )

add_executable(cv_gstream main.cpp)


add_executable(sender sender.cpp)
target_link_libraries(sender ${OpenCV_LIBS})

add_executable(reciver reciver.cpp)
target_link_libraries(reciver ${OpenCV_LIBS})


add_executable(opencv_testing opencv_testing.cpp)
target_link_libraries(opencv_testing ${OpenCV_LIBS} ${GST_LIBRARIES} ${GSTRTSP_LIBRARIES} ${FFMPEG_LIBRARIES})
#${GSTREAMER_LDFLAGS}
#${GSTREAMER_RTSP_SERVER_LDFLAGS}