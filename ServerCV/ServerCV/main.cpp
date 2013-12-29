//
//  main.cpp
//  ServerCV
//
//  Created by shouian on 13/4/6.
//  Copyright (c) 2013年 Sail. All rights reserved.
//
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <string>
#include <pthread.h>

#include </usr/include/unistd.h>

#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

VideoCapture capture;
Mat     img, img1;
int     is_data_ready = 0;
int     listenSock, connectSock;
int     clientSock;
int 	listenPort; // Server port

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

void* streamServer(void* arg);
void  quit(string msg, int retval);

int main(int argc, char** argv)
{
    pthread_t thread_s;
    int width, height, key;
	width = 640;
	height = 480;

 	listenPort = 9899; // Set up the port
    
    capture.open(0);
    if (!capture.isOpened()) {
        quit("\n--> cvCapture failed", 1);
    }
    
    capture >> img;
    img1 = Mat::zeros(img.rows, img.cols ,CV_8UC1);
    
    /* run the streaming server as a separate thread */
    if (pthread_create(&thread_s, NULL, streamServer, NULL)) {
        quit("pthread_create failed.", 1);
    }
    
    cout << "\n-->Press 'q' to quit." << endl;
    namedWindow("stream_server", CV_WINDOW_AUTOSIZE);
    flip(img, img, 1);
    cvtColor(img, img1, CV_BGR2GRAY); // Transfer img to img1 by grayscale image
    
    cout << "-->Waiting for TCP connection on port ";
    
    while(key != 'q') {
        // Get a frame from camera
        capture >> img;
        if (img.empty()) {
            break;
        }
        pthread_mutex_lock(&mutex1);
        flip(img, img, 1);
        cvtColor(img, img1, CV_BGR2GRAY);
        is_data_ready = 1;
        pthread_mutex_unlock(&mutex1);
        imshow("stream_server", img);
        key = waitKey(30);
    }
    
    if (pthread_cancel(thread_s)) {
        quit("pthread_cancel failed.", 1);
    }
    
    destroyWindow("stream_server");
    quit("NULL", 0);
    return 0;
}
/**
 * This is the streaming server, run as separate thread = > Now we have to POST the image data to Client
 */
void* streamServer(void* arg)
{
    struct  sockaddr_in   serverAddr,  clientAddr;
    socklen_t             clientAddrLen = sizeof(clientAddr);
    
    /* make this thread cancellable using pthread_cancel() */
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    
    if ((listenSock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        quit("socket() failed.", 1);
    }
    
    // Set up server
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(listenPort);
    
    if (bind(listenSock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        quit("bind() failed", 1);
    }
    
    if (listen(listenSock, 5) == -1) {
        quit("listen() failed.", 1);
    }

    int  imgSize = img1.total() * img1.elemSize();
    int  bytes=0;
    /* start send images */
    while(1)
    {
        cout << "-->Waiting for TCP connection on port " << listenPort << " ...\n\n";
		/* accept a request from a client */
        // Note that : the code will not continue if accept() is return value
        if ((connectSock = accept(listenSock, (sockaddr*)&clientAddr, &clientAddrLen)) == -1) {
            quit("accept() failed", 1);
        }else{
            cout << "-->Receiving image from " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << "..." << endl;
        }
		// Send Images....
        while(1){
            /* send the grayscaled frame, thread safe */
            if (is_data_ready) {
                
                pthread_mutex_lock(&mutex1);
                // Compress img1 frame to jpeg...
                vector<uchar>buff;
                vector<int> param = vector<int>(2);
                param[0] = CV_IMWRITE_JPEG_QUALITY;
                param[1] = 10; // default(95) 0 - 100
                imencode(".jpg", img1, buff, param); // encode img1 to jpeg format
                imgSize = (int)buff.size();
                // Get the frame spec = > note that buff: 255 216 (FFD8) and the end is 255 217 (FFD9) is the correct format for a JPEG file
                // Note that: we the imgsize is too large to send out, that is we should seperate the stream to send out
                
                // Send Header
                if ((bytes = send(connectSock, (const void *)&imgSize, sizeof(uint32_t), 0)) < 0){
                    cerr << "\n--> bytes = " << bytes << endl;
                    quit("\n--> send() failed", 1);
                }
                
                // Send Data
                if ((bytes = send(connectSock, buff.data(), imgSize, 0)) < 0){
                    cerr << "\n--> bytes = " << bytes << endl;
                    quit("\n--> send() failed", 1);
                }

                // Succeed to send image bytes
                is_data_ready = 0;
                pthread_mutex_unlock(&mutex1);
                memset(&clientAddr, 0x0, sizeof(clientAddr));
            }
            /* if something went wrong, restart the connection */
            if (bytes != imgSize) {
                cerr << "\n-->  Connection closed (bytes != imgSize)" << endl;
                close(connectSock);
                
                if (connect(connectSock, (sockaddr*) &serverAddr, sizeof(serverAddr)) == -1) {
                    quit("\n--> connect() failed", 1);
                }
            }
            
            /* have we terminated yet? */
            pthread_testcancel();
            
            /* no, take a rest for a while */
            usleep(1000);   //1000 Micro Sec
            
        }
    }

}
/////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * This function provides a way to exit nicely from the system
 */
void quit(string msg, int retval)
{
    if (retval == 0) {
        cout << (msg == "NULL" ? "" : msg) << "\n" <<endl;
    } else {
        cerr << (msg == "NULL" ? "" : msg) << "\n" <<endl;
    }
    
    if (listenSock){
        close(listenSock);
    }
    
    if (connectSock){
        close(connectSock);
    }
    
    if (!img.empty()){
        (img.release());
    }
    
    if (!img1.empty()) {
        img1.release();
    }
    
    pthread_mutex_destroy(&mutex1);
    exit(retval);
}

