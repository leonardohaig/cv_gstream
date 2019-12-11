//
// Created by liheng on 19-12-11.
//

#include "opencv2/opencv.hpp"
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <iostream>
#include <stdio.h>

using namespace cv;
using namespace std;

String face_cascade_name, eyes_cascade_name;
CascadeClassifier face_cascade;
CascadeClassifier eyes_cascade;
String window_name = "Capture - Face detection";
VideoCapture cap;

//gst-launch-1.0 rtspsrc latency=20 location="rtsp://192.168.0.36:8554/test" ! rtph264depay ! h264parse ! avdec_h264 ! autovideosink

typedef struct
{
    GstClockTime timestamp;
} MyContext;

void detectAndDisplay( Mat frame )
{
    std::vector<Rect> faces;
    Mat frame_gray;

    cvtColor( frame, frame_gray, COLOR_YUV2GRAY_I420 );
    equalizeHist( frame_gray, frame_gray );

    //-- Detect faces
    face_cascade.detectMultiScale( frame_gray, faces, 1.1, 2, 0|CASCADE_SCALE_IMAGE, Size(30, 30) );
    for ( size_t i = 0; i < faces.size(); i++ )
    {
        Point center( faces[i].x + faces[i].width/2, faces[i].y + faces[i].height/2 );
        ellipse( frame, center, Size( faces[i].width/2, faces[i].height/2 ), 0, 0, 360, Scalar( 255, 0, 255 ), 4, 8, 0 );
        Mat faceROI = frame_gray( faces[i] );
        std::vector<Rect> eyes;
        //-- In each face, detect eyes
        eyes_cascade.detectMultiScale( faceROI, eyes, 1.1, 2, 0 |CASCADE_SCALE_IMAGE, Size(30, 30) );
        for ( size_t j = 0; j < eyes.size(); j++ )
        {
            Point eye_center( faces[i].x + eyes[j].x + eyes[j].width/2, faces[i].y + eyes[j].y + eyes[j].height/2 );
            int radius = cvRound( (eyes[j].width + eyes[j].height)*0.25 );
            circle( frame, eye_center, radius, Scalar( 255, 0, 0 ), 4, 8, 0 );
        }
    }
    //-- Show what you got
//    imshow( window_name, frame );
}

/* called when we need to give data to appsrc */
static void need_data (GstElement * appsrc, guint unused, MyContext * ctx)
{
    GstBuffer *buffer;
    guint size;
    GstFlowReturn ret;
    Mat frame;

    static int i=0;
    /* read frame */
    cap.read(frame);

    if( frame.empty() ) {
        g_print ("No captured frame\n");
        return;
    }
    //cv::imshow("frame",frame);
    cv::cvtColor(frame,frame,cv::COLOR_BGR2YUV_I420);
    std::cout<<"Frame:"<<i++<<std::endl;

    /* image processing */
    //detectAndDisplay( frame );

    /* allocate buffer */
    size = frame.total() * frame.elemSize();
    buffer = gst_buffer_new_allocate(NULL, size, NULL);
    gst_buffer_fill(buffer, 0, (gpointer)frame.data, size);

    /* increment the timestamp every 1/30 second */
    GST_BUFFER_PTS(buffer) = ctx->timestamp;
    GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale_int(1, GST_SECOND, 30);
    ctx->timestamp += GST_BUFFER_DURATION(buffer);

    g_signal_emit_by_name(appsrc, "push-buffer", buffer, &ret);
}

/* called when a new media pipeline is constructed. We can query the
 * pipeline and configure our appsrc */
static void media_configure (GstRTSPMediaFactory * factory, GstRTSPMedia * media,
                             gpointer user_data)
{
    GstElement *element, *appsrc;
    MyContext *ctx;

    g_print("media_configure\n");

    /* get the element used for providing the streams of the media */
    element = gst_rtsp_media_get_element(media);

    /* get our appsrc, we named it 'mysrc' with the name property */
    appsrc = gst_bin_get_by_name_recurse_up(GST_BIN (element), "mysrc");

    /* this instructs appsrc that we will be dealing with timed buffer */
    gst_util_set_object_arg(G_OBJECT (appsrc), "format", "time");

    /* configure the caps of the video */
    g_object_set(G_OBJECT (appsrc), "caps",
                 gst_caps_new_simple("video/x-raw",
//                                     "format", G_TYPE_STRING, "I420",
                                     "format", G_TYPE_STRING, "I420",
                                     "width", G_TYPE_INT, 1280,
                                     "height", G_TYPE_INT, 720,
                                     "framerate", GST_TYPE_FRACTION, 30, 1, NULL), NULL);

    ctx = g_new0(MyContext, 1);
    ctx->timestamp = 0;
    /* make sure ther datais freed when the media is gone */
    g_object_set_data_full(G_OBJECT (media), "my-extra-data", ctx, (GDestroyNotify) g_free);

    /* install the callback that will be called when a buffer is needed */
    g_signal_connect(appsrc, "need-data", (GCallback) need_data, ctx);
}

int main (int argc, char *argv[])
{
    GMainLoop *loop;
    GstRTSPServer *server;
    GstRTSPMountPoints *mounts;
    GstRTSPMediaFactory *factory;
    String face_cascade_name, eyes_cascade_name;
    String inPipe;
    String outPipe;

    cv::String keys =
            "{i | v4l2src ! video/x-raw,format=YUY2,framerate=30/1,width=640,height=480 ! videoconvert ! video/x-raw,format=I420,framerate=30/1,width=640,height=480 ! appsink | input pipeline}"
            //"{o | ( appsrc name=mysrc is-live=true ! x264enc speed-preset=ultrafast tune=zerolatency ! rtph264pay name=pay0 pt=96 ) | output pipeline}"//SUCCESS
            "{o | ( appsrc name=mysrc ! decodebin ! videoconvert ! x264enc speed-preset=ultrafast tune=zerolatency ! rtph264pay name=pay0 pt=96 ) | output pipeline}"
            "{face_cascade | /usr/share/opencv_testing/haarcascade_frontalface_alt.xml | }"
            "{eyes_cascade | /usr/share/opencv_testing/haarcascade_eye_tree_eyeglasses.xml | }"
            "{help | | show help message}";


    cv::CommandLineParser parser(argc, argv, keys);
    if (parser.has("help")) {
        parser.printMessage();
        return 0;
    }

    inPipe = parser.get<String>("i");
    outPipe = parser.get<String>("o");
    face_cascade_name = parser.get<string>("face_cascade");
    eyes_cascade_name = parser.get<string>("eyes_cascade");

    /* Load the cascades */
    if( !face_cascade.load( face_cascade_name ) ) {
        g_print("--(!)Error loading face cascade\n");
        //return -1;
    };
    if( !eyes_cascade.load( eyes_cascade_name ) ) {
        g_print("--(!)Error loading eyes cascade\n");
        //return -1;
    };

    /* init video capture */
    //cap.open(inPipe.c_str());
    cap.open("/home/liheng/ADAS_Video/1120/ADAS_Video-20191120-151849.mp4");

    /* check if we succeeded */
    if(!cap.isOpened()) {
        g_print("VideoCapture not opened\n");
        return -1;
    }

    gst_init (&argc, &argv);

    loop = g_main_loop_new (NULL, FALSE);

    /* create a server instance */
    server = gst_rtsp_server_new();
//    g_object_set (server, "service", "8554", NULL);
//    g_object_set (server, "address", "192.168.0.68", NULL);

    /* get the mount points for this server, every server has a default object
     * that be used to map uri mount points to media factories */
    mounts = gst_rtsp_server_get_mount_points(server);

    /* make a media factory for a test stream. The default media factory can use
     * gst-launch syntax to create pipelines.
     * any launch line works as long as it contains elements named pay%d. Each
     * element with pay%d names will be a stream */
    factory = gst_rtsp_media_factory_new();
    gst_rtsp_media_factory_set_launch(factory, outPipe.c_str());

    /* notify when our media is ready, This is called whenever someone asks for
     * the media and a new pipeline with our appsrc is created */
    g_signal_connect(factory, "media-configure", (GCallback) media_configure, NULL);

    /* attach the test factory to the /test url */
    gst_rtsp_mount_points_add_factory(mounts, "/test", factory);

    /* don't need the ref to the mounts anymore */
    g_object_unref(mounts);

    /* attach the server to the default maincontext */
    gst_rtsp_server_attach(server, NULL);

    /* start serving */
    g_print("stream ready at rtsp://127.0.0.1:8554/test\n");
    g_main_loop_run(loop);

    return 0;
}