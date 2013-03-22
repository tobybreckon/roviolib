// *****************************************************************************

// Rovio API C++ Class Library : Test Harness

// Copyright (c) 2011 Toby Breckon, School of Engineering, Cranfield University
// License : GPL - http://www.gnu.org/copyleft/gpl.htmlime

// *****************************************************************************

#include "rovio_cc_lib.h"

// *****************************************************************************

// #define TESTHARNESS_ROVIO_HOST "host.domain.tld" // or IP address
// #define TESTHARNESS_ROVIO_USER "username"
// #define TESTHARNESS_ROVIO_PASS "password"

#define TESTHARNESS_ROVIO_HOST "192.168.10.18" // or IP address
#define TESTHARNESS_ROVIO_USER ""
#define TESTHARNESS_ROVIO_PASS ""

// *****************************************************************************

#include <iostream>
using namespace std;

#include <cv.h>
#include <highgui.h>
using namespace cv;

// *****************************************************************************

// Test switches to test different API functionality

#define TESTHARNESS_TEST_MDRIVE 0
#define TESTHARNESS_TEST_MDRIVE_ANGLE 0
#define TESTHARNESS_TEST_CAMERA_IMG 0
#define TESTHARNESS_TEST_CAMERA_VIDEO 0
#define TESTHARNESS_TEST_CAMERA_VIDEO_AND_MOTION 0
#define TESTHARNESS_TEST_CAMERA_SETTINGS 1
#define TESTHARNESS_TEST_WHEEL_ENCODERS_KINEMATICS 0
#define TESTHARNESS_TEST_STAT_INFO 0
#define TESTHARNESS_TEST_IR 0
#define TESTHARNESS_TEST_LIGHTS 0
#define TESTHARNESS_TEST_PATHS 0       // DOES NOT WORK WELL (even from browser!)
#define TESTHARNESS_TEST_HOME_RESET 0
#define TESTHARNESS_TEST_USER 0
#define TESTHARNESS_TEST_REBOOT 0	  // causes MS Windows to disconnect if Ad-Hoc
#define TESTHARNESS_TEST_WLAN_SCAN 0
#define TESTHARNESS_TEST_BASE_STATION_INFO 0
#define TESTHARNESS_TEST_INFO_STRINGS 0

// *****************************************************************************

int main(void)
{
	string windowName = "Rovio Test Image";
    Mat img;
    namedWindow(windowName, CV_WINDOW_AUTOSIZE);

	// check versions

    printf("\n*************** TESTS : LIBRARY VERSIONS\n\n");


	printf ("OpenCV version %s (%d.%d.%d)\n",
	    CV_VERSION,
	    CV_MAJOR_VERSION, CV_MINOR_VERSION, CV_SUBMINOR_VERSION);
	printf ("libCURL version %s\n", curl_version());

    printf("\n*************** TESTS : CHECK ROBOT CONNECTIVITY\n\n");

	// connect to robot

	Rovio* robot = new Rovio((char *) TESTHARNESS_ROVIO_HOST,
                              (char *) TESTHARNESS_ROVIO_USER,
							  (char *) TESTHARNESS_ROVIO_PASS);

	cout << "TESTHARNESS: set verbose  = " << robot->setAPIVerbose(true);
	cout << " get verbose = " << robot->getAPIVerbose() << endl;

	bool connectionOK = robot->isConnected();

	cout << "TESTHARNESS: connected = " << connectionOK << endl;
	if (!connectionOK){
	        printf("\nERROR: not connected to robot %s with user/pass = %s/%s\n\n",
                (char *) TESTHARNESS_ROVIO_HOST, (char *) TESTHARNESS_ROVIO_USER,
                (char *) TESTHARNESS_ROVIO_PASS);
		exit(1);
	}

#if TESTHARNESS_TEST_CAMERA_IMG

    // test getting an image

    printf("\n*************** TESTS : TESTHARNESS_TEST_CAMERA_IMG\n\n");

	img = robot->getImage(CV_LOAD_IMAGE_UNCHANGED);
	if (!img.empty())
	{
		imshow(windowName, img);
		waitKey(100);
	}
#endif

#if TESTHARNESS_TEST_CAMERA_VIDEO

	// get video - WORKS LINUX ONLY

    printf("\n*************** TESTS : TESTHARNESS_TEST_CAMERA_VIDEO\n\n");

    robot->startVideoStreaming();
    printf("TESTHARNESS: press 's' to skip video stream loop\n");

    for (int i = 0; i < 30000; i++)
    {
        img = robot->getVideoStreamFrame();
        if (!img.empty()){
            imshow(windowName, img);
        }

        // In order to make it display the video, you have to put the
        // waitKey() function, with a delay of 2 ms. Otherwise, it doesn't work
        // well. This must be some kind of buffering issue ???

        char c = waitKey(2);
        if (c == 's')
        {
            i = 300000;
        }
    }

    robot->stopVideoStreaming();

#endif

#if TESTHARNESS_TEST_CAMERA_VIDEO_AND_MOTION

	// get video - WORKS LINUX ONLY

    printf("\n*************** TESTS : TESTHARNESS_TEST_CAMERA_VIDEO\n\n");

    robot->startVideoStreaming();
    printf("TESTHARNESS: press 's' to skip video stream loop\n");

    for (int i = 0; i < 30; i++)
    {
        img = robot->getVideoStreamFrame();
        if (!img.empty()){
            imshow(windowName, img);
        }

        // In order to make it display the video, you have to put the
        // waitKey() function, with a delay of 2 ms. Otherwise, it doesn't work
        // well. This must be some kind of buffering issue ???

        char c = waitKey(2);

        robot->rotateRight(3, 45); // rotate 45

        if (c == 's')
        {
            i = 30;
        }
    }

    robot->stopVideoStreaming();

#endif


#if TESTHARNESS_TEST_MDRIVE

    // drive forward into an open space

    for(int i = 0; i < 60; i++)
    {
        robot->manualDrive(ROVIO_FORWARD, 2);
    }

	// test manual drive (as a cycle)

    printf("\n*************** TESTS : TESTHARNESS_TEST_MDRIVE\n\n");

		for(int command = 0; command < 13; command++)
		{
			for (int speed = 10; speed > 0; speed--)
			{
				robot->manualDrive(command, speed);
				sleep(1); // sleep 1 second
			}
		}


	robot->manualDrive(ROVIO_STOP, 1);
	sleep(1);
	robot->manualDrive(ROVIO_HEADDOWN, 1);
	sleep(1);
	robot->rotateRight(10, 45);
	sleep(1); // sleep 1 second
#endif

#if TESTHARNESS_TEST_MDRIVE_ANGLE

	for (int i = 1; i <= 8; i++)
	{
	    robot->manualDrive(ROVIO_STOP, 1);
		robot->rotateRight(3, 45); // rotate 45
		robot->waitUntilComplete(-1);
		sleep(3); // sleep 3 seconds
	}

    for (int i = 1; i <= 10; i++)
	{
	    robot->manualDrive(ROVIO_STOP, 1);
		robot->rotateLeft(3, 36); // rotate 45
		robot->waitUntilComplete(-1);
		sleep(3); // sleep 3 seconds
	}

#endif

#if TESTHARNESS_TEST_CAMERA_SETTINGS

    printf("\n*************** TESTS : TESTHARNESS_TEST_CAMERA_SETTINGS\n\n");

	// test frame rate

	robot->setFrameRate(2);
	robot->setFrameRate(32);
	robot->setFrameRate(25);

    // test contrast

	for (int i = 0; i < 255; i+=50)
	{
		robot->setContrast(i);
		img = robot->getImage(CV_LOAD_IMAGE_UNCHANGED);
		imshow(windowName, img);
		waitKey(40);
	}
    robot->setContrast(ROVIO_IMAGE_DEFAULT_CONTRAST_LEVEL);

	// test brightness

	for (int i = 1; i < 7; i++)
	{
		robot->setBrightness(i);
		img = robot->getImage(CV_LOAD_IMAGE_UNCHANGED);
		imshow(windowName, img);
		waitKey(40);
	}

	// test resolution

	for (int i = 0; i < 4; i++)
	{
		robot->setResolution(i);
		img = robot->getImage(CV_LOAD_IMAGE_UNCHANGED);
		if (!(img.empty()))
        {
            printf("TESTHARNES: returned image is (%d x %d)", img.cols, img.rows);
            imshow(windowName, img);
            waitKey(40);
        }
	}

	// test compression

	for (int i = 0; i < 3; i++)
	{
		robot->setCompressionQuality(i);
		img = robot->getImage(CV_LOAD_IMAGE_UNCHANGED);
		imshow(windowName, img);
		waitKey(40);
	}

	// test frequency set/get

	robot->setSensorFrequencyCompensation(50);
	robot->getSensorFrequencyCompensation();
	img = robot->getImage(CV_LOAD_IMAGE_UNCHANGED);
	namedWindow(windowName, CV_WINDOW_AUTOSIZE);
	imshow(windowName, img);
	waitKey(40);

	robot->setSensorFrequencyCompensation(60);
	robot->getSensorFrequencyCompensation();
	img = robot->getImage(CV_LOAD_IMAGE_UNCHANGED);
	namedWindow(windowName, CV_WINDOW_AUTOSIZE);
	imshow(windowName, img);
	waitKey(40);

	robot->setSensorFrequencyCompensation(0);
	robot->getSensorFrequencyCompensation();
	img = robot->getImage(CV_LOAD_IMAGE_UNCHANGED);
	namedWindow(windowName, CV_WINDOW_AUTOSIZE);
	imshow(windowName, img);
	waitKey(40);

	// test AGC set

	robot->setAGC(true);
	img = robot->getImage(CV_LOAD_IMAGE_UNCHANGED);
	namedWindow(windowName, CV_WINDOW_AUTOSIZE);
	imshow(windowName, img);
	waitKey(40);
	robot->setAGC(false);
	img = robot->getImage(CV_LOAD_IMAGE_UNCHANGED);
	namedWindow(windowName, CV_WINDOW_AUTOSIZE);
	imshow(windowName, img);
	waitKey(40);

	// test AWB set

	robot->setAWB(true);
	img = robot->getImage(CV_LOAD_IMAGE_UNCHANGED);
	namedWindow(windowName, CV_WINDOW_AUTOSIZE);
	imshow(windowName, img);
	waitKey(40);
	robot->setAWB(false);
	img = robot->getImage(CV_LOAD_IMAGE_UNCHANGED);
	namedWindow(windowName, CV_WINDOW_AUTOSIZE);
	imshow(windowName, img);
	waitKey(40);

	// test AWB set

	robot->setAEC(true);
	img = robot->getImage(CV_LOAD_IMAGE_UNCHANGED);
	namedWindow(windowName, CV_WINDOW_AUTOSIZE);
	imshow(windowName, img);
	waitKey(40);
	robot->setAEC(false);
	img = robot->getImage(CV_LOAD_IMAGE_UNCHANGED);
	namedWindow(windowName, CV_WINDOW_AUTOSIZE);
	imshow(windowName, img);
	waitKey(40);

	// test night mode

	robot->setNightMode(false);
    img = robot->getImage(CV_LOAD_IMAGE_UNCHANGED);
	namedWindow(windowName, CV_WINDOW_AUTOSIZE);
	imshow(windowName, img);
	waitKey(40);
	for (int i = 2; i<=8; i = i+i)
	{
	    robot->setNightMode(true, i);
        img = robot->getImage(CV_LOAD_IMAGE_UNCHANGED);
        namedWindow(windowName, CV_WINDOW_AUTOSIZE);
        imshow(windowName, img);
        waitKey(40);
	}
	robot->setNightMode(false);
    img = robot->getImage(CV_LOAD_IMAGE_UNCHANGED);
	namedWindow(windowName, CV_WINDOW_AUTOSIZE);
	imshow(windowName, img);
	waitKey(40);

#endif

#if TESTHARNESS_TEST_WHEEL_ENCODERS_KINEMATICS

    printf("\n*************** TESTS : TESTHARNESS_TEST_WHEEL_ENCODERS\n\n");

	// test wheel encoder access (-789 is just a flag value - if it
	// changes post-test we know all is OK)

	bool rightDir = false;
	int rightTicks = -789;
	bool leftDir = false;
	int leftTicks = -789;
	bool rearDir = false;
	int rearTicks = -789;

	robot->getWheelEncoders(rightDir, rightTicks,
							leftDir, leftTicks,
							rearDir, rearTicks);

    for(int i = 0; i < 30; i++)
    {
        robot->manualDrive(ROVIO_FORWARD, 2);
    }

    robot->getWheelEncoders(rightDir, rightTicks,
							leftDir, leftTicks,
                            rearDir, rearTicks);

	for(int i = 0; i < 30; i++)
    {
        robot->manualDrive(ROVIO_FORWARD, 2);
    }

    robot->getWheelEncoder(rightDir, rightTicks,ROVIO_WHEEL_RIGHT, true);
    robot->getWheelEncoder(leftDir, leftTicks,ROVIO_WHEEL_LEFT, true);
    robot->getWheelEncoder(rearDir, rearTicks,ROVIO_WHEEL_REAR, true);

    printf("TESTHARNESS: accumulated wheel encoders returned : ldir %d ltick %d "
               "rdir %d rtick %d "
               "reardir %d reartick %d \n",
               leftDir, leftTicks, rightDir, rightTicks,
               rearDir, rearTicks);

    robot->getWheelEncoder(rightDir, rightTicks,ROVIO_WHEEL_RIGHT, false);
    robot->getWheelEncoder(leftDir, leftTicks,ROVIO_WHEEL_LEFT, false);
    robot->getWheelEncoder(rearDir, rearTicks,ROVIO_WHEEL_REAR, false);

    printf("TESTHARNESS: last wheel encoders returned : ldir %d ltick %d "
               "rdir %d rtick %d "
               "reardir %d reartick %d \n",
               leftDir, leftTicks, rightDir, rightTicks,
               rearDir, rearTicks);

    double x_k,y_k,omega_k;

    robot->getForwardKinematics(x_k,y_k,omega_k, true);
    printf("TESTHARNESS: accumulated kinematics : ( %d , %d ) @ %d radians",
           x_k,y_k,omega_k);
    robot->getForwardKinematics(x_k,y_k,omega_k, false);
    printf("TESTHARNESS: last move kinematics : ( %d , %d ) @ %d radians",
           x_k,y_k,omega_k);

#endif

#if TESTHARNESS_TEST_STAT_INFO

    printf("\n*************** TESTS : TESTHARNESS_TEST_STAT_INFO\n\n");

	// test head position access

	robot->getHeadPosition();

	// test battery status

	robot->getBatteryLevel();

	// test charging status

	robot->getChargingStatus();

	// test wifi signal strength

	robot->getWifiSS();

	// test the get status stuff

	robot->getNavStatus();

#endif

#if TESTHARNESS_TEST_IR

	// set/test IR power status

	robot->setIRPowerState(true);
	robot->getIRPowerStatus();
	robot->setIRPowerState(false);
	robot->getIRPowerStatus();
	robot->setIRPowerState(true);
	robot->getIRPowerStatus();

	// test IR obstacle status

	robot->getIRObstacle();
#endif

#if TESTHARNESS_TEST_LIGHTS

    printf("\n*************** TESTS : TESTHARNESS_TEST_LIGHTS\n\n");

	// test set light status

	robot->setLightState(false);
	robot->setLightState(true);
	robot->setLightState(false);
	robot->setLightState(true);

	// test headlight control

	robot->getHeadLightState();
	robot->setHeadLightState(false);
	robot->getHeadLightState();
	robot->setHeadLightState(true);
	robot->getHeadLightState();
	robot->setHeadLightState(false);
	robot->getHeadLightState();
	robot->setHeadLightState(true);
	robot->getHeadLightState();
	robot->setHeadLightState(false);

	// test LED light patterns

	robot->setLightPattern(true, true, true, false, false, false);
	sleep(1);
	robot->setLightPattern(false, false, false, true, true, true);
	sleep(1);
	for(int i = 0; i < 10; i++)
	{
		robot->setLightPattern(true, false, true, false, true, false);
		sleep(1);
		robot->setLightPattern(false, true, false, true, false, true);
		sleep(1);
	}
	robot->setLightState(true);

#endif

#if TESTHARNESS_TEST_PATHS

    printf("\n*************** TESTS : TESTHARNESS_TEST_PATHS\n\n");

	// test navigation path recording

	 robot->startPathRecording();
		robot->manualDrive(ROVIO_FORWARD, 1);
		sleep(1);
		robot->manualDrive(ROVIO_FORWARD, 1);
		sleep(1);
		robot->manualDrive(ROVIO_LEFT, 1);
		sleep(1);
		robot->manualDrive(ROVIO_LEFT, 1);
		sleep(1);
		robot->manualDrive(ROVIO_BACKWARD, 1);
		sleep(1);
		robot->manualDrive(ROVIO_BACKWARD, 1);
		sleep(1);
		robot->manualDrive(ROVIO_RIGHT, 1);
		sleep(1);
		robot->manualDrive(ROVIO_RIGHT, 1);

	robot->stopAbortPathRecording();

	robot->startPathRecording();
		robot->manualDrive(ROVIO_FORWARD, 1);
		sleep(1);
		robot->manualDrive(ROVIO_FORWARD, 1);
		sleep(1);
		robot->manualDrive(ROVIO_FORWARD, 1);
		sleep(1);
		robot->manualDrive(ROVIO_FORWARD, 1);
		sleep(1);
		robot->manualDrive(ROVIO_FORWARD, 1);
		sleep(1);
		robot->manualDrive(ROVIO_FORWARD, 1);
		sleep(1);
		robot->manualDrive(ROVIO_FORWARD, 1);
		sleep(1);
		robot->manualDrive(ROVIO_FORWARD, 1);
		sleep(1);
		robot->manualDrive(ROVIO_FORWARD, 1);
		sleep(1);
		robot->manualDrive(ROVIO_FORWARD, 1);
		sleep(1);

	robot->stopStorePathRecording((char *) "Test Path 1");

	 robot->startPathRecording();
		robot->manualDrive(ROVIO_BACKWARD, 1);
		sleep(1);
		robot->manualDrive(ROVIO_BACKWARD, 1);
		sleep(1);
		robot->manualDrive(ROVIO_RIGHT, 1);
		sleep(1);
		robot->manualDrive(ROVIO_RIGHT, 1);
		sleep(1);
		robot->manualDrive(ROVIO_FORWARD, 1);
		sleep(1);
		robot->manualDrive(ROVIO_FORWARD, 1);
		sleep(1);
		robot->manualDrive(ROVIO_LEFT, 1);
		sleep(1);
		robot->manualDrive(ROVIO_LEFT, 1);
		sleep(1);

	robot->stopStorePathRecording((char *) "Test Path 2");

	robot->runPath((char *)"Test Path 1", false);
	robot->waitUntilComplete(-1);
	robot->runPath((char *)"Test Path 2", true);
	robot->waitUntilComplete(-1);

	robot->deletePath((char *) "Test Path 2");

	robot->renamePath((char *)"Test Path 1", (char *)"Test Path 2");

	robot->stopPath();
	robot->runPath((char *)"Test Path 1", false);
	robot->pausePath();

	robot->deleteAllPaths();

#endif
#if TESTHARNESS_TEST_HOME_RESET

    printf("\n*************** TESTS : TESTHARNESS_TEST_HOME_RESET\n\n");

	// test home location stuff

	robot->returnHome();
	robot->waitUntilComplete(-1);
	robot->returnHomeDock();
	robot->waitUntilComplete(-1);
	robot->setAsHome();
	robot->clearHome();
	robot->navReset();

#endif
#if TESTHARNESS_TEST_USER

    printf("\n*************** TESTS : TESTHARNESS_TEST_USER\n\n");

	// test user management stuff

	robot->setUser((char *) "pingu", (char *) "EsK1M0", true);

	robot->delUser((char *) "pingu");

#endif

#if TESTHARNESS_TEST_REBOOT

    printf("\n*************** TESTS : TESTHARNESS_TEST_REBOOT\n\n");

	// reboot

	robot->reboot();

	cout << "TESTHARNESS waiting 30 seconds for robot reboot .... " << flush;
	sleep(20);
    cout << "done" << endl;

#endif

#if TESTHARNESS_TEST_INFO_STRINGS

    printf("\n*************** TESTS : TESTHARNESS_TEST_INFO_STRINGS\n\n");

	// test info strings

	char *info = NULL;

	info = robot->getLibVersion();
	if (info)
		cout << "TESTHARNESS allocated string returned: " << endl << info << endl;
	free(info);

    info = robot->getFirmwareVersion();
	if (info)
		cout << "TESTHARNESS allocated string returned: " << endl << info << endl;
	free(info);

	info = robot->getAPIVersion();
	if (info)
		cout << "TESTHARNESS allocated string returned: " << endl << info << endl;
	free(info);

#endif

#if TESTHARNESS_TEST_WLAN_SCAN

    printf("\n*************** TESTS : TESTHARNESS_TEST_WLAN_SCAN\n\n");

    vector<ROVIOWifiAccessPoint> results;

    robot->getWlanScan(results);

    for (unsigned int i = 0; i < results.size(); i++)
    {
       cout << "WLAN SCAN: " << results[i].essid << " - " << results[i].mac << " @ quality " << results[i].quality << endl;
    }

#endif // TESTHARNESS_TEST_WLAN_SCAN

#if TESTHARNESS_TEST_BASE_STATION_INFO

    printf("\n*************** TESTS : TESTHARNESS_TEST_BASE_STATION_INFO\n\n");

int x,y, room, ss;
float theta;

    robot->getBaseStationSignalInfo(x, y, theta, room, ss);

    cout << "Base Station info: x " << x << " y " << y << " theta " << theta << " @ room ID " << room
         << " with signal strength " << ss << endl;

#endif

    printf("\n*************** TESTS : CHECK ROBOT CLOSE-DOWN\n\n");

	// destroy object

	robot->~Rovio();

	return 0;

}

// *****************************************************************************
