//====================================================================//
// Created by liheng on 19-5-23.
//====================================================================//

#include <opencv2/opencv.hpp>
using namespace cv;

#include <iostream>
using namespace std;

void sender()
{
    // VideoCapture: Getting frames using 'v4l2src' plugin, format is 'BGR' because
    // the VideoWriter class expects a 3 channel image since we are sending colored images.
    // Both 'YUY2' and 'I420' are single channel images.
    //VideoCapture cap("v4l2src ! video/x-raw,format=BGR,width=640,height=480,framerate=30/1 ! appsink",CAP_GSTREAMER);
    VideoCapture cap("/home/liheng/Video-6mm-20190521-071528-640X360.avi");
    if(!cap.isOpened())
    {
        cout<<"VideoCapture not opened"<<endl;
        exit(-1);
    }

    // VideoWriter: 'videoconvert' converts the 'BGR' images into 'YUY2' raw frames to be fed to
    // 'jpegenc' encoder since 'jpegenc' does not accept 'BGR' images. The 'videoconvert' is not
    // in the original pipeline, because in there we are reading frames in 'YUY2' format from 'v4l2src'
    //cv::String str("appsrc ! videoconvert ! video/x-raw,format=YUY2,width=640,height=360,framerate=30/1 ! jpegenc ! rtpjpegpay ! udpsink host=127.0.0.1 port=5000");
    cv::String str("appsrc ! videoconvert ! video/x-raw,format=YUY2,width=640,height=360,"
                   "framerate=30/1 ! jpegenc ! rtpjpegpay ! udpsink "
                   "host=192.168.0.18 port=5000");
    VideoWriter out(str,
            CAP_GSTREAMER,0,30.0,Size(640,360),true);

    if(!out.isOpened())
    {
        cout<<"VideoWriter not opened"<<endl;
        exit(-1);
    }

    Mat frame;

    while(true) {

        cap.read(frame);

        if(frame.empty())
            break;

        out.write(frame);

        imshow("Sender", frame);
        if(waitKey(1) == 's')
            break;
    }
    destroyWindow("Sender");
}

int main()
{
    sender();

    return 0;
}
