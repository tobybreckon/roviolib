// ****************************************************************************

// Rovio API C++ Class Library : C++ Interface to Rovio Robot API

// Dependancies : OpenCV 2.1 (or later), libCurl 7.18 (or later)

// Author : Toby Breckon, toby.breckon@cranfield.ac.uk

// Copyright (c) 2011 Toby Breckon, School of Engineering, Cranfield University
// License : GPL - http://www.gnu.org/copyleft/gpl.html

// Platform Support : Linux (GCC 4.x) / Windows (32-bit, XP/Vista/7)

// Acknowledgements: Rovio API documentation v 1.3
//				     Rovio .NET API library
//
// *****************************************************************************

#ifndef ROVIO_LIB_H
#define ROVIO_LIB_H

// *****************************************************************************
// library dependancy includes

#include <curl/curl.h>
#include <cv.h>
#include <highgui.h>

// *****************************************************************************

// ROVIO Robot Setting Defaults (restored on close)

#define ROVIO_IMAGE_DEFAULT_RESOLUTION 3
#define ROVIO_IMAGE_DEFAULT_BRIGHTNESS 3
#define ROVIO_IMAGE_DEFAULT_SATURATION -1
#define ROVIO_IMAGE_DEFAULT_HUE -1
#define ROVIO_IMAGE_DEFAULT_FRAMERATE 25
#define ROVIO_IMAGE_DEFAULT_COMPRESSION 1
#define ROVIO_IMAGE_DEFAULT_FREQ_COMPENSATION 0
#define ROVIO_IMAGE_DEFAULT_AGC_STATE 0
#define ROVIO_IMAGE_DEFAULT_NIGHT_MODE_STATE 0
#define ROVIO_IMAGE_DEFAULT_CONTRAST_LEVEL 48
#define ROVIO_IR_DEFAULT_POWER_STATE 1
#define ROVIO_LIGHT_DEFAULT_STATE_ALL 1
#define ROVIO_HEAD_DEFAULT_POSITION 0
#define ROVIO_HEADLIGHT_DEFAULT_STATE false
#define ROVIO_RESET_HOME_IF_DOCKED_ON_CLOSEDOWN 0

// *****************************************************************************

// ROVIO Robot Manual Drive Commands

#define ROVIO_STOP				0 // (Stop)
#define ROVIO_FORWARD			1 // (Forward)
#define ROVIO_BACKWARD			2 // (Backward)
#define ROVIO_LEFT				3 // (Straight left)
#define ROVIO_RIGHT				4 // (Straight right)
#define ROVIO_TURNLEFT			5 // (Rotate left by speed)
#define ROVIO_TURNRIGHT			6 // (Rotate right by speed)
#define ROVIO_FORWARDLEFT		7 // (Diagonal forward left)
#define ROVIO_FORWARDRIGHT		8 // (Diagonal forward right)
#define ROVIO_BACKWARDLEFT		9 // (Diagonal backward left)
#define ROVIO_BACKWARDRIGHT		10 // (Diagonal backward right)
#define ROVIO_HEADUP			11 // (Head up)
#define ROVIO_HEADDOWN			12 // (Head down)
#define ROVIO_HEADMIDDLE		13 // (Head middle)
#define ROVIO_ROTATELEFT20		17 // (Rotate left by 20 degree increments)
#define ROVIO_ROTATERIGHT20		18 // (Rotate right by 20 degree increments)


// ROVIO Robot Navigation State Return Values

#define ROVIO_IDLE 0            // robot doing nothing
#define ROVIO_DRIVING_HOME 1    // robot driving back to home position
#define ROVIO_DOCKING 2         // robot is docking in charging dock
#define ROVIO_EXEC_PATH 3       // robot is executing a specified path
#define ROVIO_COMM_ERROR -1     // communication error to robot

// ROVIO wheel labels (for getting wheel encoder values)

#define ROVIO_WHEEL_RIGHT 0
#define ROVIO_WHEEL_LEFT 1
#define ROVIO_WHEEL_REAR 2

// *****************************************************************************

// Platform Specific Options

#ifdef linux // this is set for all Linux build environments
#include <unistd.h>
#include <pthread.h>

// to avoid compiler warnings we use the MS VS 2008/10 secure versions of the
// following functions. Here we define MACROS to avoid problems with other
// compilers

#define strcpy_s(D, L, S) (strcpy(D, S))
#define sprintf_s snprintf
#define sscanf_s sscanf
#endif

#ifdef _WIN32 // this is set for 32-bit and 64-bit platforms
#include <windows.h>

// in MS Windows sleep(N) for N seconds is undefined but Sleep(N) for
// N milliseconds is defined. This fixes this.

#define sleep(X) Sleep(X * 1000)

#endif

// *****************************************************************************

// system includes

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vector>

// *****************************************************************************

// RETURN CODES, PATHS and MAGIC NUMBERS

#define ROVIO_HTTP_RETURN_CODE_ALL_OK 200
#define ROVIO_CGI_RETURN_CODE_ALL_OK 0
#define ROVIO_CAMERA_IMAGE_DEFAULT_URL_STEM "Jpeg/CamImg0000.jpg"
#define ROVIO_CAMERA_VIDEO_URL_STEM "GetData.cgi?Status=false"
#define ROVIO_CAMERA_VIDEO_INTERLEAVE "--WINBONDBOUDARY\r\nContent-Type: image/jpeg\r\n\r\n"
#define ROVIO_CAMERA_VIDEO_INTERLEAVE_SIZE 46   //Size in bytes of the
#define ROVIO_CAMERA_VIDEO_STREAMING_STOP_WAIT_TIME 1 // wait time in seconds
#define ROVIO_CAMERA_VIDEO_STREAMING_SERVICE_WAIT_TIME 3333 // wait time in u-seconds
#define ROVIO_CAMERA_VIDEO_STREAMING_BUFFER_SIZE_TOO_BIG_HACK 51200 // assume 50k max.
#define ROVIO_CAMERA_VIDEO_STREAMING_BUFFER_SIZE_TOO_SMALL_HACK 3500 // assume 10k min.
#define ROVIO_WIFI_SCAN_RESULT_STEP_ALLOC 10

#define ROVIO_IS_CHARGING_MAGIC_NUMBER 80

#define ROVIO_JPEG_MIN_SIZE 134         // min. valid size of JPEG in bytes
#define ROVIO_JPEG_SOI_FIRST_BYTE 0xFF  // Start Of Image (SOI) header byte 1
#define ROVIO_JPEG_SOI_SECOND_BYTE 0xD8 // Start Of Image (SOI) header byte 2

// *****************************************************************************

// Navigation Constants for Wheel Encoder Conversion
// (acknowledgement: http://code.google.com/p/cs1567-rovio-project/source/browse/)

#define ROVIO_CM_PER_TICK 0.6897
#define ROVIO_DEGREE_PER_TICK 2.1160
#define ROVIO_RADIAN_PER_TICK 0.0369
#define ROVIO_PI 3.14159265
#define ROVIO_BASE_DIAMETER 29 // centimetres

// *****************************************************************************

// the timeout for communication with the robot in seconds

#define ROVIO_COMMUNICATION_TIMEOUT 180 // N.B. units = seconds

// max specified memory reserved for HTTP URLs

#define ROVIO_COMMUNICATION_URL_MEMORY_SIZE 255

// default RovioLib behaviour is to update the wheel encoder accumulators after
// each drive command issued to the robot

#define ROVIO_WILL_UPDATE_WHEEL_ENCODERS_AFTER_DRIVE_COMMAND true

// *****************************************************************************

// API VERSION NUMBERING

#define ROVIO_CGI_API_VERSION_MAJOR 1
#define ROVIO_CGI_API_VERSION_MINOR 3

#define ROVIO_CPP_API_VERSION_MAJOR 0
#define ROVIO_CPP_API_VERSION_MINOR 2
#define ROVIO_CPP_API_VERSION_MINOR_STEP 0

// *****************************************************************************

// structure containing the information about an wifi access point

// mac : the mac adress on the AP as an array of char XX:XX:XX:XX:XX:XX
// essid : the ESSID of the network
// quality : array of integer containing the signal strengths

typedef struct
{
	char mac[19];
	char essid[33];
	int quality;
} ROVIOWifiAccessPoint;


// *****************************************************************************

// internal memory buffer definition(s)

struct ROVIOCURLFrameStruct
{
    char *memory;
    size_t size;
#ifdef linux
        pthread_mutex_t completed;   // is the image decoding / access completed
#endif
#ifdef _WIN32
        HANDLE completed;   // is the image decoding / access completed (mutex)
#endif
    cv::Mat image;
    char *inputSaveBuffer;		// for the belt and brace skip action
	size_t inputSaveBufferSize;	// for the belt and brace skip action
};

struct ROVIOCURLMemoryStruct
{
    char *memory;
    size_t size;
};

// *****************************************************************************

class Rovio
{
public:

    // robot object initialisation
    // - hostname = hostname of the rovio (e.g. myrovio.ip.net) or IP (e.g. 192.168.10.18)
    // - username = http web access username ("" if unset)
    // - password = http web access username ("" if unset)

    Rovio(char* hostname, char* username, char* password);

    // test connection to robot
    // return value: OK (true), failure (false)

    bool isConnected();

    // robot camera image access

    // openCV compatible C++
    // format parameter function as per OpenCV loadImage()
    // returns newly allocated image on success / empty image on failure

    cv::Mat getImage(int format);

    // robot camera video access

    // openCV compatible C++
    // return frames from the MJPEG or MPEG4 stream

    // start stop video streaming
    // return value: OK (true), failure (false)

    bool startVideoStreaming();
    bool stopVideoStreaming();

    // returns newly allocated image on success / empty image on failure

    cv::Mat getVideoStreamFrame();

    // robot drive motors

    // manually drive
    // - command : direction of drive (see ROVIOFORWARD, ROVIOBACKWARD ... )
    // - speed : range 0 (fast) to 10 (slow)

    bool manualDrive(int command, int speed);

    // rotate robot
    // - angle : angle to rotate in degrees
    // - speed : range 0 (slow) to 10 (fast)

    bool rotateRight(int speed, int angle);
    bool rotateLeft(int speed, int angle);

    // wait until current driving command or sequence of commands (path) is
    // complete by polling the robot at 1 second intervals for a maximum number
    // of seconds
    // seconds - maximum time to wait (specify -1 to wait until task complete)

    void waitUntilComplete(int seconds);

    // robot camera settings

    // set camera frame rate in fps
    // - rate = 2-32 fps
    // return value: success/failure

    bool setFrameRate(int rate);

    //return the frame rate
    //return values: 1-30 (frames per second)

    int getFrameRate();

    // set camera brightness level
    // - level = range 0 (dim) - 6 (bright)
    // return value: success/failure

    bool setBrightness(int level);

    // set camera contrast level
    // - level = range 0 (dim) - 255 (bright)
    // return value: success/failure

    bool setContrast(int level);

    // set camera resolution
    // - res = one of 0 = {176, 144}, 1 = {352, 288}, 2 = {320, 240},
    //                3 = {640, 480}
    // return value: success/failure

    bool setResolution(int res);

    //return the image resolution. Possible values:
    //0 = [176x144]
    //1 = [320x240]
    //2 = [352x240]
    //3 = [640x480]

    int getResolution();

    // set video compression quality (for streaming MPEG4 video only)
    // - level = one of 0 = low, 1 = medium, 2 = high
    // return value: success/failure

    bool setCompressionQuality(int level);

    // set sensor indoor lighting frequency compensation
    // - level = 50 – 50Hz (UK), 60 – 60Hz (US), 0 – Auto detect
    // return value: success/failure

    bool setSensorFrequencyCompensation(int level);

    // get sensor indoor lighting frequency compensation
    // return value: 50 – 50Hz, 60 – 60Hz, 0 – Auto detect, -1 - failure

    int getSensorFrequencyCompensation();

    // set Automatic Gain Control (AGC)
    // state = on (true) or off (false)
    // return value: success/failure

    bool setAGC(bool state);

    // set night mode on camera
    // state = on (true) or off (false)
    // framerate = 2, 4 or 8 (only)
    // "Minimum frame rate during night mode.
    // When the light is low, the night mode will automatically reduce
    // the frame rate by multi-sampling. The more the frame rate is reduced,
    // the more brightness it will get." Framerate value only used when
    // state parameter is true
    // return value: success/failure

    bool setNightMode(bool state, int framerate=2);

    // robot sensors / encoders

    // return 6 values of the {right,left,rear} wheel encoders
    // *Dir values - {true, false } = {forward, backward}
    // *Tick values - accumulated ticks since last reset
    // useAccumulated - if true use the accumulated wheel encoder values (since the last reset)
    // return value: success/failure

    bool getWheelEncoders(bool& rightDir, int& rightTicks,
                          bool& leftDir, int& leftTicks,
                          bool& rearDir, int& rearTicks,
                          bool useAccumulated=true);

    // return 2 values of specified wheel encoder
    // dir - {true, false } = {forward, backward}
    // tick - accumulated ticks since last reset
    // wheel - one of {ROVIO_WHEEL_RIGHT, ROVIO_WHEEL_LEFT, ROVIO_WHEEL_REAR}
    // useAccumulated - if true use the accumulated wheel encoder values (since the last reset)
    // return value: success/failure

    bool getWheelEncoder(bool &dir, int& ticks, int wheel, bool useAccumulated=true);

    // reset wheel encoders to zero (default direction: forwards)

    bool resetWheelEncoders();

    // return the forward kinematics of the last (or accumulated) robot movement in terms of the
    // vector position x, y and angle omega
    // x - movement in x axis (cm)
    // y - movement in y axis (cm)
    // omega - angular rotation (radians)
    // useAccumulated - if true use the accumulated wheel encoder values (since the last reset)

    void getForwardKinematics(double& x, double& y, double& omega, bool useAccumulated=false);

    // return value of head position
    // returns : ~204 = low, 135-140 = mid, 65 = high, -1 on failure

    int getHeadPosition();

    // return battery level
    // returns : <100 = self turn off, 100-106 = needs to charge,
    //                  106-127 = normal, -1 on failure

    int getBatteryLevel();

    // get IR radar power state
    // return value: on (true) or off (false)

    bool getIRPowerStatus();

    // get IR radar obstacle detection status
    // return value: path blocked (true) or clear (false)

    bool getIRObstacle();

    // set IR radar power state on or off
    // state = on (true) or off (false)
    // return value: success/failure

    bool setIRPowerState(bool state);

    // get charging status
    // return value: is charging (true) or not charging (false)

    bool getChargingStatus();

    // robot lights

    // set light state (blue LEDs)
    // state = all on (true) or all off (false)
    // return value: success/failure

    bool setLightState(bool state);

    // set light state (blue LEDs)
    // a-f = on (true) or off (false) addressed clockwise from front of robot
    // return value: success/failure

    bool setLightPattern(bool a, bool b, bool c, bool d, bool e, bool f);

    // set headlight state
    // state = on (true) or off (false)
    // return value: success/failure

    bool setHeadLightState(bool state);

    // return light state (blue LEDs)
    // return value: on (true) or off (false)

    bool getHeadLightState();

    // robot status

    // Returns wifi signal strength
    // return value: 0-254

    int getWifiSS();

    // return navigation status
    // return value: ROVIO_IDLE, ROVIO_DRIVING_HOME, ROVIO_DOCKED,
    // ROVIO_EXEC_PATH, ROVIO_COMM_ERROR

    int getNavStatus();

    // robot path record/play operations

    // start recording a path
    // return value: success/failure

    bool startPathRecording();

    // stop recording and discard path
    // return value: success/failure

    bool stopAbortPathRecording();

    // stop recording and store path
    // - pathName = string name of stored path
    // return value: success/failure

    bool stopStorePathRecording(char * pathName);

    // delete stored path
    // - pathName = string name of stored path
    // return value: success/failure

    bool deletePath(char * pathName);

    // delete all stored paths
    // return value: success/failure

    bool deleteAllPaths();

    // rename a stored path
    // - pathNameC = Current string name of stored path
    // - pathNameN = New string name of stored path
    // return value: success/failure

    bool renamePath(char * pathNameC, char * pathNameN);

    // run stored path
    // - pathName = string name of stored path
    // - direction = true for forwards, false for backwards
    // return value: success/failure

    bool runPath(char * pathName, bool playDirection);

    // stop running current path
    // return value: success/failure

    bool stopPath();

    // pause running current path
    // return value: success/failure

    bool pausePath();

    // robot beacon based navigation (works inside only)

    // return to home location
    // return value: success/failure

    bool returnHome();

    // return to home location and dock
    // return value: success/failure

    bool returnHomeDock();

    // set current position as home (in charging station)
    // return value: success/failure

    bool setAsHome();

    // clear current home position (in charging station)
    // return value: success/failure

    bool clearHome();

    // reset navigation state machine (stopping current navigation)
    // return value: success/failure

    bool navReset();

    // robot admin (admin username/password required)

    // Add username/password or reset existing user password
    // - username = http web access username
    // - password = http web access username
    // - admin = boolean value (true = admin rights, false = guest rights)
    // return value: success/failure

    bool setUser(char * username, char * password, bool admin);

    // Delete username
    // - username = http web access username
    // return value: success/failure

    bool delUser(char * username);

    // reboot the robot

    bool reboot();

    // robot information

    // return string of onboard library version

    char* getLibVersion();

    // return string of firmware version

    char* getFirmwareVersion();

    // return string of API version

    char * getAPIVersion();

    // API verbosity / debugging help modes

    // set verbose mode for API
    // - status : true (on), false (off)
    // return value: status

    bool setAPIVerbose(bool status);

    // get verbose mode setting for API
    // return value: status

    bool getAPIVerbose();

    // set verbose mode for HTTP (libCURL API)
    // - status : true (on), false (off)
    // return value: status

    bool setHTTPVerbose(bool status);

    // get verbose mode for HTTP (libCURL API)
    // return value: status

    bool getHTTPVerbose();

    // hostname of the robot

    char name[ROVIO_COMMUNICATION_URL_MEMORY_SIZE];

    ~Rovio();

    // return the base-station signal information
    // - x : distance x-coord.
    // - y : distance y-coord.
    // - theta : angle
    // - room : room id
    // - ss : signal strength

    void getBaseStationSignalInfo(int &x, int &y, float &theta, int &room, int &ss);

	// Return a list of wireless networks
	// - result : a vector of ROVIOWifiAccessPoint structures
    // (e.g. essid : 000D0BB25984, mac : 00:0D:0B:B2:59:85, quality : -58)

	bool getWlanScan(std::vector<ROVIOWifiAccessPoint> &result);

protected:

    // service the video stream thread

#ifdef linux
    friend void* ROVIOthreadPasser(void* args);
#endif
#ifdef _WIN32
	friend DWORD ROVIOthreadPasser(LPVOID lpParameter);
#endif

    void videoStreamServiceThread();

private:

    // send a URL stem string (e.g. /rev.cgi?Cmd=nav&action=12) to the robot
    // return value : server response code

    int sendToRobot(char* urlStem); // use default curl object for commands
    int sendToRobot(CURL *curlObject, char* urlStem); // use specified curl object

    bool pathRecordingState;	// status of path recording
    bool verboseMode;			// verbose mode on/off
    bool httpVerboseMode;		// http verbose mode on/off

    bool wheelDirR;             // right wheel encoder direction (true = forward/false = back)
    int  wheelLastR;            // last right wheel encoder value reading
    int  wheelAccumulatorR;     // accumulated right wheel encoder value
    bool wheelDirL;             // left wheel encoder direction (true = forward/false = back)
    int  wheelLastL;            // last left wheel encoder value reading
    int  wheelAccumulatorL;     // accumulated left wheel encoder value
    bool wheelDirB;             // back (rear) wheel encoder direction (true = forward/false = back)
    int  wheelLastB;            // last back wheel encoder value reading
    int  wheelAccumulatorB;     // accumulated back wheel encoder value

    CURL *curlCom;				// lib CURL rovio COMmand object
    CURL *curlVid;				// lib CURL rovio streaming VIDeo object

    char urlBase [ROVIO_COMMUNICATION_URL_MEMORY_SIZE];
    char urlToSend[ROVIO_COMMUNICATION_URL_MEMORY_SIZE];

    ROVIOCURLMemoryStruct buffer;
    ROVIOCURLFrameStruct frame;

    // manually drive worker method

    bool manualDrive(int command, int speed, int angle);

    // update wheel encoder worker method

    bool updateWheelEncoders();

    // compute robot forward kinematics
    // follows the Rovio forward kinematics model/code of John Rogers, jgrogers@cc.gatech.edu
    // Copyright (C) 2009 Georgia Institute of Technology)

    void computeForwardKinematics(double V_l, double V_r, double V_c,
                                  double& Vx, double& Vy, double& omega);

    // return bit n of byte as byte set to either 1 or 0

    unsigned char getBitN(unsigned char byte, int n);

    // return most recent CGI response

    int getCGIResponse();

    // video thread object and flag value

    #ifdef linux
    pthread_t videoStreamThread;
    #endif
    #ifdef _WIN32
    HANDLE videoStreamThread;
    #endif

    // streaming status

    bool streaming;

    // clear the buffer

    void clearBuffer();

    // clear the frame (memory only)

    void clearFrame();

};
// ****************************************************************************
// libCURL write functions

size_t NULLWriteFunction( void *ptr, size_t size, size_t nmemb,
                          void *stream);

size_t ROVIO_CURL_WriteMemoryCallback (void *ptr, size_t size,
                                       size_t nmemb, void *data);

size_t ROVIO_CURL_WriteMemoryVideoFrameCallback (void *ptr, size_t size,
        size_t nmemb, void *data);

// *****************************************************************************

#endif // ROVIO_LIB_H

// *****************************************************************************
