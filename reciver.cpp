//====================================================================//
// Created by liheng on 19-5-23.
//====================================================================//


#include <opencv2/opencv.hpp>
using namespace cv;

#include <iostream>
using namespace std;

void receiver()
{
    // The sink caps for the 'rtpjpegdepay' need to match the src caps of the 'rtpjpegpay' of the sender pipeline
    // Added 'videoconvert' at the end to convert the images into proper format for appsink, without
    // 'videoconvert' the receiver will not read the frames, even though 'videoconvert' is not present
    // in the original working pipeline
    VideoCapture cap("udpsrc port=5000 ! application/x-rtp,"
                     "media=video,payload=26,clock-rate=90000,"
                     "encoding-name=JPEG,framerate=30/1 "
                     "! rtpjpegdepay ! jpegdec ! videoconvert ! appsink "
                     "fps-update-interval=1000 sync=false",//后来添加
                     CAP_GSTREAMER);

//    VideoCapture cap("udpsrc port=5000 ! application/x-rtp, media=video, clock-rate=90000, encoding-name=H264, payload=96 ! rtph264depay ! h264parse ! queue ! avdec_h264 ! videoconvert ! appsink sync=false async=false", CAP_GSTREAMER);
//VideoCapture cap("rtsp://127.0.0.1:8554/test");
//        VideoCapture cap("rtspsrc location=rtsp://127.0.0.1:8554/test ! rtph264depay ! h264parse ! avdec_h264  ! videoconvert "
//                         "! appsink fps-update-interval=1000 sync=false async=false",
//                     CAP_GSTREAMER);

    if(!cap.isOpened())
    {
        cout<<"VideoCapture not opened"<<endl;
        exit(-1);
    }

    Mat frame;

    int nWaitTime = 1;
    while(true) {

        cap.read(frame);

        if(frame.empty())
            break;

        imshow("Receiver", frame);
        char chKey = cv::waitKey(nWaitTime);
        //ESC
        if (27 == chKey) {
            break;
        }
        else if (' ' == chKey) nWaitTime = !nWaitTime;
    }
    destroyWindow("Receiver");
}

int main()
{
    receiver();
    return 0;
}

