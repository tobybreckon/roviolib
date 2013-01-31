// *****************************************************************************

// Example: controlling the rovio robot using the RovioLib C++ API

// Dependancies : OpenCV 2.1 (or later), libCurl 7.18 (or later),
//                RovioLib C++ API 1.3 or later

// Author : Toby Breckon, toby.breckon@cranfield.ac.uk

// Copyright (c) 2011 Toby Breckon, School of Engineering, Cranfield University
// License : GPL - http://www.gnu.org/copyleft/gpl.html

// *****************************************************************************

// standard includes

#include <iostream>
#include <ctime>
using namespace std;

// OpenCV includes

#include <cv.h>
#include <highgui.h>
using namespace cv;

// RovioLib API includes

#include "rovio_cc_lib.h"

// *****************************************************************************

// define robot network access parameters

// #define TESTHARNESS_ROVIO_HOST "host.domain.tld" // or IP address
// #define TESTHARNESS_ROVIO_USER "username"
// #define TESTHARNESS_ROVIO_PASS "password"

#define TESTHARNESS_ROVIO_HOST "192.168.10.18" // or IP address
#define TESTHARNESS_ROVIO_USER ""
#define TESTHARNESS_ROVIO_PASS ""

// *****************************************************************************

// do basic display of robot position

void disp_kinematics(Rovio *robot)
{
    static string windowName = (string) "Robot Kinematics View :"
                                + (string) robot->name; // window name
    static bool firstCall = false;
    static Point lastPos = Point(25, 25);
    static Mat img = Mat::zeros(500, 500, CV_8UC3);
    static double omega_cumulative = 0; // accumulated rotation

    double x, y, omega; // kinematics values
    Point next; // next positio of robot on map

    // first time called - create window

    if (!firstCall)
    {
        namedWindow(windowName, CV_WINDOW_AUTOSIZE);
        firstCall = true;
    }

    // check we are still on map (!)

    if ((lastPos.x < 0) || (lastPos.y < 0) || (lastPos.x > 500 ) || (lastPos.y > 500))
    {
        // if not reset us to the origin (and redraw map)
        lastPos = Point(25, 25);
        img = Mat::zeros(500, 500, CV_8UC3);
    }

    // get kinematic from robot

    robot->getForwardKinematics(x,y,omega);
    omega_cumulative += omega;

    cout << "1: x: " << x << " y: " << y << " omega: " << omega
         << " lastpos: " << lastPos.x << " " << lastPos.y << endl;

    // rotate offset by omega
    // reference:
    // http://stackoverflow.com/questions/7953316/rotate-a-point-around-a-point-with-opencv

    next.x = cvCeil((x*cos(omega_cumulative) - y*sin(omega_cumulative)) + lastPos.x);
    next.y = cvCeil((x*sin(omega_cumulative) + y*cos(omega_cumulative)) + lastPos.y);

    cout << "2: x: " << x << " y: " << y << " omega cumulative: " << omega_cumulative
         << " lastpos: " << lastPos.x << " " << lastPos.y << endl << endl;

    // draw robot movement

    line(img, lastPos, next, Scalar(255,0,0), 2);
    imshow(windowName, img);

    lastPos = next; // where we are now is where we start next time

}

// *****************************************************************************

// do basic display image

void disp_cam_image(Rovio *robot)
{
    static string windowName = (string) "Camera View :"
                                + (string) robot->name; // window name
    static bool first = false;
    string dir[] = {(string) "backward", (string) "forward"}; // forward == 1

    bool rightDir, leftDir, rearDir;
    int rightTicks, leftTicks, rearTicks, x, y, room, sigst;
    float theta;

    if (!first)
    {
        namedWindow(windowName, CV_WINDOW_AUTOSIZE);
        first = true;
    }

    Mat img = robot->getImage(CV_LOAD_IMAGE_UNCHANGED);
    if (!img.empty())
    {
        // report wheel encoders

        robot->getWheelEncoders(leftDir, leftTicks,
                            rightDir, rightTicks,
                            rearDir, rearTicks);

        stringstream ss;
        ss << "right: " << dir[rightDir] << " @ " << rightTicks << " left: " << dir[leftDir]
           << " @ " << leftTicks << " rear: " << dir[rearDir] << " @ " << rearTicks;

        putText(img, ss.str(),
            Point(10, 20), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0,255,0), 2, 8, false );

        ss.str(std::string());

        // report base station info

        robot->getBaseStationSignalInfo(x,y,theta, room, sigst);

        ss << "x: " << x << " y: " << y << " theta: " << theta << " room: " << room << " signal: " << sigst;
        putText(img, ss.str(),
        Point(10, 40), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0,255,0), 2, 8, false );

        imshow(windowName, img);
    }

}

// *****************************************************************************

// very basic obstacle avoidance

void do_obstcale_avoid(Rovio* robot, RNG &random)
{
    // get random choice and angle from system time

    int choice = random.uniform(ROVIO_LEFT, ROVIO_RIGHT + 1);
    int angle = random.uniform(5, 60);
    int repeat = random.uniform(1, 4);

    for (int i = 0; i < repeat; i++){
        disp_cam_image(robot);
        robot->manualDrive(choice, 3);
        robot->waitUntilComplete(-1);
        disp_kinematics(robot);
    }
    disp_cam_image(robot);
    robot->rotateRight(3, (int) angle);
    robot->waitUntilComplete(-1);
    disp_kinematics(robot);

    // if still blocked recursively solve problem
    // (as we are probably not blocked on all angles)

     if (robot->getIRObstacle()) {
            do_obstcale_avoid(robot, random);
    }

}
// *****************************************************************************

// do basic plot wifi signal strength

void plot_wifi(Mat &img, int strength)
{
    static int count = 0; // static variable preserved across calls

    if (img.empty() || (count == 200))
    {
        img = Mat(255, 200, CV_8U);
        img.setTo(Scalar(255));
        count = 0;
    }

    line(img, Point(count, img.rows), Point(count, 255 - strength),
         Scalar::all(0), 1, 8, 0 );
    count++;
}

// *****************************************************************************

int main(int argc, char** argv ) {

    Mat img, wifi; // image objects
    string windowName2 = (string) "Wifi Strength"; // window name
    char key;       // user input
    bool keepExploring = true;  // control state of robot
    RNG random ((uint64) time(NULL));
    int choice;
    int backOff = 0;

    // create named window

    namedWindow(windowName2, CV_WINDOW_AUTOSIZE);

    // if specified then take from command line in order

    char *host, *user, *pass;

    if (argc > 1)
    {
        host = argv[1];
        user = argv[2];
        pass = argv[3];
    } else {
        host = (char *) TESTHARNESS_ROVIO_HOST;
        user = (char *) TESTHARNESS_ROVIO_USER;
        pass = (char *) TESTHARNESS_ROVIO_PASS;
    }

    // connect to robot and check connection

    Rovio* robot = new Rovio((char *) host,
                              (char *) user,
                              (char *) pass);

	if (!robot->isConnected()){
	        cout << "ERROR: not connected to robot " << TESTHARNESS_ROVIO_HOST
                 << " with user/pass = " << TESTHARNESS_ROVIO_USER "/"
                 << TESTHARNESS_ROVIO_PASS << endl << endl;
            return 1;
    }

    // turn off blue lights

    robot->setLightState(false);

    // uncomment following line to give verbose output
    // from API of robot <-> PC communications

    // robot->setAPIVerbose(true);

    // setup camera

    robot->setBrightness(3);
    robot->setAGC(true);
    robot->setContrast(60);
    robot->setFrameRate(25);
    robot->setResolution(3);
    robot->setSensorFrequencyCompensation(50);

    // turn of IR sensor

    if (!(robot->setIRPowerState(true)))
    {
        cout << "ERROR: robot IR sensor failed to activate" << endl;
        return 1;
    }

    // explore environment

    while (keepExploring)
    {

        // check for obstacles and avoid

        if (robot->getIRObstacle())
        {
            do_obstcale_avoid(robot, random);
        }

        // check battery level (go to dock if low)

        if (robot->getBatteryLevel() < 106)
        {
            cout << "WARNING: robot battery low returning to dock" << endl;
            robot->returnHome();
            robot->waitUntilComplete(-1);
            robot->returnHomeDock();

            // until we get home keep relaying images

            while (!robot->getChargingStatus())
            {
                disp_cam_image(robot);
            }
            return 0;
        }

        // make exploration moves

        robot->manualDrive(ROVIO_FORWARD, 5);
        robot->waitUntilComplete(-1);
        backOff++;

        disp_cam_image(robot);
        disp_kinematics(robot);

        // periodically do back to avoid getting stuck

        if (backOff == 25) {
            choice = random.uniform(ROVIO_BACKWARDLEFT, ROVIO_BACKWARDRIGHT + 1);
            for (int i = 0; i < 5; i++)
            {
                robot->manualDrive(choice, 2);
                robot->waitUntilComplete(-1);
                disp_cam_image(robot);
                disp_kinematics(robot);
            }
            backOff = 0;
        }

        // plot wifi

        plot_wifi(wifi, robot->getWifiSS());
        imshow(windowName2, wifi);
        key = waitKey(10);
        if (key == 'x')
        {
            keepExploring = false;
        }

    }

    return 0;
}
// *****************************************************************************
