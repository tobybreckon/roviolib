// *****************************************************************************

// Rovio API C++ Class Library : C++ Interface to Rovio Robot API

// Copyright (c) 2011 Toby Breckon, School of Engineering, Cranfield University
// License : GPL - http://www.gnu.org/copyleft/gpl.html

// *****************************************************************************


// ********************* SEE DOCUMENTATION IN HEADER FILE **********************

#include "rovio_cc_lib.h"

// *****************************************************************************

Rovio::Rovio (char* hostname, char* username, char* password)
{

    // create the base URL string

    if ((strlen(username) > 0) && (strlen(password) > 0))
    {
        sprintf_s(urlBase, ROVIO_COMMUNICATION_URL_MEMORY_SIZE, "http://%s:%s@%s/", username, password, hostname);
    }
    else
    {
        sprintf_s(urlBase, ROVIO_COMMUNICATION_URL_MEMORY_SIZE, "http://%s/", hostname);
    }

    //setup global hostname

    strcpy(name, hostname);

    // setup CURL object(s)

    curlCom = curl_easy_init();

    curl_easy_setopt(curlCom, CURLOPT_VERBOSE, 0);
    curl_easy_setopt(curlCom, CURLOPT_NOPROGRESS, 1);
    curl_easy_setopt(curlCom, CURLOPT_TIMEOUT, ROVIO_COMMUNICATION_TIMEOUT);

    curlVid = curl_easy_init();

    curl_easy_setopt(curlVid, CURLOPT_VERBOSE, 0);
    curl_easy_setopt(curlVid, CURLOPT_NOPROGRESS, 1);
    curl_easy_setopt(curlVid, CURLOPT_TIMEOUT, ROVIO_COMMUNICATION_TIMEOUT);

    // set modes within object

    verboseMode = false;
    pathRecordingState = false;
    httpVerboseMode = false;

    // set pointers to NULL for memory buffer

    buffer.memory = NULL;
    buffer.size = 0;

    // set pointers to NULL for frame buffer

    frame.memory = NULL;
    frame.size = 0;
	frame.inputSaveBuffer = NULL;
	frame.inputSaveBufferSize = 0;

    // init. the video streaming stuff
    #ifdef linux
    pthread_mutex_init (&(frame.completed), NULL);
    #endif
	#ifdef _WIN32
    frame.completed = CreateMutex(NULL, FALSE, NULL);

    if (frame.completed == NULL)
    {
        printf("CreateMutex error: %d\n", GetLastError());
    }
    #endif

    streaming = false;

    // reset the wheel encoders

    resetWheelEncoders();

}
// *****************************************************************************

bool Rovio::isConnected()
{

    // set up a NULL write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, NULLWriteFunction);

    // if return code is zero, then all OK

    if (sendToRobot((char *) "") == ROVIO_HTTP_RETURN_CODE_ALL_OK)
    {
        return true;
    }

    return false;
}
// *****************************************************************************

cv::Mat Rovio::getImage(int format)
{
    long serverReturnCode; // server return code
    cv::Mat imgTmp; 		// image object

    // set up the write to memory buffer

    curl_easy_setopt(curlCom,CURLOPT_WRITEFUNCTION, ROVIO_CURL_WriteMemoryCallback);
    curl_easy_setopt(curlCom,CURLOPT_WRITEDATA, (void *) &buffer);

    // send command (with image data going to memory)

    serverReturnCode = sendToRobot((char *) ROVIO_CAMERA_IMAGE_DEFAULT_URL_STEM);

    if (verboseMode)
    {
        printf("\nROVIO: image request : %s response is OK ? : %i (buffer size = %d bytes)\n",
               ROVIO_CAMERA_IMAGE_DEFAULT_URL_STEM,
               (serverReturnCode ==  ROVIO_HTTP_RETURN_CODE_ALL_OK), (int) buffer.size);
    }

    // check the buffer looks like a JPEG image

    if ((buffer.size >= ROVIO_JPEG_MIN_SIZE) &&
            (buffer.memory[0] & ROVIO_JPEG_SOI_FIRST_BYTE) &&
            (buffer.memory[1] & ROVIO_JPEG_SOI_SECOND_BYTE))
    {
        // decode image (in memory) using OpenCV

        imgTmp = cv::imdecode(cv::Mat(1, buffer.size, CV_8UC1, buffer.memory), format);
    }

    // clean up the buffer and return image

    clearBuffer();

    return imgTmp;
}

// *****************************************************************************

#ifdef linux
void* ROVIOthreadPasser(void* args)
{
    Rovio* pass = (Rovio*) args;
    while(pass->streaming)
    {
        pass->videoStreamServiceThread();
        usleep(ROVIO_CAMERA_VIDEO_STREAMING_SERVICE_WAIT_TIME); // assume ~30 fps max.
    }
    sleep(ROVIO_CAMERA_VIDEO_STREAMING_STOP_WAIT_TIME);
    return 0;
}
#endif
#ifdef _WIN32
DWORD ROVIOthreadPasser(LPVOID lpParameter) {
	Rovio* pass = (Rovio*) lpParameter;
    while(pass->streaming)
    {
        pass->videoStreamServiceThread();
        Sleep(ROVIO_CAMERA_VIDEO_STREAMING_SERVICE_WAIT_TIME); // assume ~30 fps max.
    }
    sleep(ROVIO_CAMERA_VIDEO_STREAMING_STOP_WAIT_TIME);
    return 0;
}
#endif

bool Rovio::startVideoStreaming()
{
    // first check if already streaming

    if (streaming)
    {
        return false;

    }

    streaming = true; // set streaming here (or we get a race condition)
#ifdef linux
    if (pthread_create(&(this->videoStreamThread), NULL, ROVIOthreadPasser, (void *) this))
#endif
#ifdef _WIN32
	this->videoStreamThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ROVIOthreadPasser, (LPVOID) this, 0, NULL);
	if (this->videoStreamThread == NULL)
#endif
    {
        if (verboseMode)
        {
            printf("ROVIO: video thread could not be created.\n");
        }
        streaming = false;
        return false;
    }
    else
    {

        if (verboseMode)
        {
            printf("ROVIO: video thread created.\n");
        }
        return true;
    }
}

// *****************************************************************************


bool Rovio::stopVideoStreaming()
{
    // first check if it has stopped streaming

    if (!streaming)
    {
        return true; // streaming is already stopped so return false

    }

    streaming = false;
#ifdef linux
    if (pthread_cancel(videoStreamThread))
#endif
#ifdef _WIN32
    if (TerminateThread(videoStreamThread, 0))
#endif
    {
        if (verboseMode)
        {
            printf("ROVIO: video thread could not be stopped.\n");
        }
        return false;
    }
    else
    {

        if (verboseMode)
        {
            printf("ROVIO: video thread stopped.\n");
        }

        // clear any frame memory

        sleep(ROVIO_CAMERA_VIDEO_STREAMING_STOP_WAIT_TIME);

        clearFrame();

        #ifdef _WIN32
        CloseHandle(this->videoStreamThread);
        #endif

        return true;
    }
}

// *****************************************************************************

void Rovio::videoStreamServiceThread()
{
    long serverReturnCode; // server return code

    // set up the write to memory buffer
    curl_easy_setopt(curlVid,CURLOPT_WRITEFUNCTION, ROVIO_CURL_WriteMemoryVideoFrameCallback);
    curl_easy_setopt(curlVid,CURLOPT_WRITEDATA, (void *) &frame);

    serverReturnCode = sendToRobot(curlVid, (char*)ROVIO_CAMERA_VIDEO_URL_STEM);

}

// *****************************************************************************

cv::Mat Rovio::getVideoStreamFrame()
{
    cv::Mat returnImageFrame;

    // if we are not streaming we return an empty frame

    if (!streaming)
    {
        return returnImageFrame;
    }

    // return a clone so that when this image is used externally the
    // effects of it being updated internally are not felt outside the Rovio object

#ifdef linux
    pthread_mutex_lock( &(frame.completed));
    returnImageFrame = frame.image.clone();
    pthread_mutex_unlock( &(frame.completed));
#endif
#ifdef _WIN32
    WaitForSingleObject(frame.completed, INFINITE);
    returnImageFrame = frame.image.clone();
    ReleaseMutex(frame.completed);
#endif
    return returnImageFrame;
}

// *****************************************************************************

bool Rovio::rotateRight(int speed, int angle)
{
    return manualDrive(ROVIO_ROTATERIGHT20, speed,
                       (int) floor(((double) angle / 360.0) * 30));
}

bool Rovio::rotateLeft(int speed, int angle)
{
    return manualDrive(ROVIO_ROTATELEFT20, speed,
                       (int) floor(((double) angle / 360.0) * 30));
}

// *****************************************************************************

bool Rovio::manualDrive(int command, int speed)
{
    return manualDrive(command, speed, 0);
}

// *****************************************************************************

bool Rovio::manualDrive(int command, int speed, int angle)
{

    char commandStringStem[ROVIO_COMMUNICATION_URL_MEMORY_SIZE];
    int cgicode;

    // check inputs
    if (((command < 0) || (command > 18)) || ((speed < 0) || (speed > 10))
            || ((angle < 0) || (angle > 30)))
        return false;

    // construct command

    sprintf_s(commandStringStem, ROVIO_COMMUNICATION_URL_MEMORY_SIZE,
              "rev.cgi?Cmd=nav&action=18&drive=%d&speed=%d&angle=%d", command, speed, angle);

    if (verboseMode)
    {
        printf("\nROVIO: drive command code: %d @ speed : %d w/ angle (approx.) %d\n",
               command, speed, (int) (((double) angle / 30.0) * 360));
    }

    // set up the write to memory buffer

    curl_easy_setopt(curlCom,CURLOPT_WRITEFUNCTION, ROVIO_CURL_WriteMemoryCallback);
    curl_easy_setopt(curlCom,CURLOPT_WRITEDATA, (void *) &buffer);

    // if return code is zero, then all OK

    if (sendToRobot(commandStringStem) == ROVIO_HTTP_RETURN_CODE_ALL_OK)
    {
        cgicode = getCGIResponse();
        clearBuffer();

        // update encoders as drive command ws successful
        #if ROVIO_WILL_UPDATE_WHEEL_ENCODERS_AFTER_DRIVE_COMMAND
            updateWheelEncoders();
        #endif

        return (cgicode == ROVIO_CGI_RETURN_CODE_ALL_OK);
    }

    // update encoders anyway (!)

#if ROVIO_WILL_UPDATE_WHEEL_ENCODERS_AFTER_DRIVE_COMMAND
    updateWheelEncoders();
#endif

    clearBuffer();
    return false;
}
// *****************************************************************************

void Rovio::waitUntilComplete(int seconds)
{

    int swait;

    if (seconds < 0)
    {
        swait = (int) HUGE;
    }
    else
    {
        swait = seconds;
    }

    for(int i = 0; i < swait; i++)
    {

        if (this->getNavStatus() != ROVIO_IDLE)
        {
            if (verboseMode)
            {
                printf("\nROVIO: busy (path/docking/home) - waiting 1 sec.\n");
            }
            // in MS Windows this parameter is in milliseconds (but our #def sleep(X)
            // deals with this case) - #def sleep(X) Sleep(X * 1000)
            // in Linux this parameter is in seconds

            sleep(1);

        }
        else
        {
            return;
        }
    }

    return;

}

// *****************************************************************************

bool Rovio::setFrameRate(int rate)
{

    char commandStringStem[ROVIO_COMMUNICATION_URL_MEMORY_SIZE];

    // check input

    if ((rate < 2) || (rate > 32))
        return false;

    // construct command

    sprintf_s(commandStringStem, ROVIO_COMMUNICATION_URL_MEMORY_SIZE,
              "ChangeFramerate.cgi?Framerate=%d", rate);


    if (verboseMode)
    {
        printf("\nROVIO: frame rate command @ rate : %d\n", rate);
    }

    // set up a NULL write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, NULLWriteFunction);

    // if return code is zero, then all OK

    if (sendToRobot(commandStringStem) == ROVIO_HTTP_RETURN_CODE_ALL_OK)
    {
        return true;
    }

    return false;
}

// *****************************************************************************

int Rovio::getFrameRate()
{

    int returnValue = -1;

    // set up a memory write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, ROVIO_CURL_WriteMemoryCallback);
    curl_easy_setopt(curlCom,CURLOPT_WRITEDATA, (void *) &buffer);

    // send the command

    sendToRobot((char *) "rev.cgi?Cmd=nav&action=1");

    // extract information from the buffer

    if (buffer.memory)
    {
        char* ptr = strstr(buffer.memory, "frame_rate=");
        if (ptr)
        {
            ptr += strlen("frame_rate=");
            if (ptr < (buffer.memory + buffer.size))
            {
                sscanf_s(ptr, "%d|", &returnValue);
            }
        }
    }

    if (verboseMode)
    {
        printf("ROVIO: Frame rate returned : %d\n", returnValue);
    }

    // reset buffer

    clearBuffer();

    return returnValue;

}

// *****************************************************************************

bool Rovio::setBrightness(int level)
{

    char commandStringStem[ROVIO_COMMUNICATION_URL_MEMORY_SIZE];

    // check input

    if ((level < 0) || (level > 6))
        return false;

    // construct command

    sprintf_s(commandStringStem, ROVIO_COMMUNICATION_URL_MEMORY_SIZE,
              "ChangeBrightness.cgi?Brightness=%d", level);


    if (verboseMode)
    {
        printf("\nROVIO: brightness command @ level : %d\n", level);
    }

    // set up a NULL write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, NULLWriteFunction);

    // if return code is zero, then all OK

    if (sendToRobot(commandStringStem) == ROVIO_HTTP_RETURN_CODE_ALL_OK)
    {
        return true;
    }

    return false;
}

// *****************************************************************************

bool Rovio::setContrast(int level)
{

    char commandStringStem[ROVIO_COMMUNICATION_URL_MEMORY_SIZE];

    // check input

    if ((level < 0) || (level > 255))
        return false;

    // construct command

    sprintf_s(commandStringStem, ROVIO_COMMUNICATION_URL_MEMORY_SIZE,
              "debug.cgi?action=write_i2c&address=0x56&value=0x%02x", level);


    if (verboseMode)
    {
        printf("\nROVIO: contrast command @ level : %d\n", level);
    }

    // set up a NULL write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, NULLWriteFunction);

    // if return code is zero, then all OK

    if (sendToRobot(commandStringStem) == ROVIO_HTTP_RETURN_CODE_ALL_OK)
    {
        return true;
    }

    return false;
}

// *****************************************************************************

bool Rovio::setResolution(int res)
{

    char commandStringStem[ROVIO_COMMUNICATION_URL_MEMORY_SIZE];

    // check input

    if ((res < 0) || (res > 3))
        return false;

    // construct command

    sprintf_s(commandStringStem, ROVIO_COMMUNICATION_URL_MEMORY_SIZE,
              "ChangeResolution.cgi?ResType=%d", res);


    if (verboseMode)
    {
        printf("\nROVIO: resolution command @ setting : %d\n", res);
    }

    // set up a NULL write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, NULLWriteFunction);

    // if return code is zero, then all OK

    if (sendToRobot(commandStringStem) == ROVIO_HTTP_RETURN_CODE_ALL_OK)
    {
        return true;
    }

    return false;
}

// *****************************************************************************

int Rovio::getResolution()
{

    int returnValue = -1;

    // set up a memory write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, ROVIO_CURL_WriteMemoryCallback);
    curl_easy_setopt(curlCom,CURLOPT_WRITEDATA, (void *) &buffer);

    // send the command

    sendToRobot((char *) "rev.cgi?Cmd=nav&action=1");

    // extract information from the buffer

    if (buffer.memory)
    {
        char* ptr = strstr(buffer.memory, "resolution=");
        if (ptr)
        {
            ptr += strlen("resolution=");
            if (ptr < (buffer.memory + buffer.size))
            {
                sscanf_s(ptr, "%d|", &returnValue);
            }
        }
    }

    if (verboseMode)
    {
        printf("ROVIO: Camera resolution returned : %d\n", returnValue);
    }

    // reset buffer

    clearBuffer();

    return returnValue;

}

// *****************************************************************************

bool Rovio::setCompressionQuality(int level)
{

    char commandStringStem[ROVIO_COMMUNICATION_URL_MEMORY_SIZE];

    // check input

    if ((level < 0) || (level > 2))
        return false;

    // construct command

    sprintf_s(commandStringStem, ROVIO_COMMUNICATION_URL_MEMORY_SIZE,
              "ChangeCompressRatio.cgi?Ratio=%d", level);


    if (verboseMode)
    {
        printf("\nROVIO: compression command @ setting : %d\n", level);
    }

    // set up a NULL write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, NULLWriteFunction);

    // if return code is zero, then all OK

    if (sendToRobot(commandStringStem) == ROVIO_HTTP_RETURN_CODE_ALL_OK)
    {
        return true;
    }

    return false;
}

// *****************************************************************************

bool Rovio::setSensorFrequencyCompensation(int level)
{

    char commandStringStem[ROVIO_COMMUNICATION_URL_MEMORY_SIZE];

    // check input

    if ((level != 0) && (level != 50) && (level != 60))
        return false;

    // construct command

    sprintf_s(commandStringStem, ROVIO_COMMUNICATION_URL_MEMORY_SIZE,
              "SetCamera.cgi?Frequency=%d", level);


    if (verboseMode)
    {
        printf("ROVIO: frequency command @ setting : %d\n", level);
    }

    // set up a NULL write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, NULLWriteFunction);

    // if return code is zero, then all OK

    if (sendToRobot(commandStringStem) == ROVIO_HTTP_RETURN_CODE_ALL_OK)
    {
        return true;
    }

    return false;
}

// *****************************************************************************

int Rovio::getSensorFrequencyCompensation()
{

    int setting = - 1;

    // set up a memory write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, ROVIO_CURL_WriteMemoryCallback);
    curl_easy_setopt(curlCom,CURLOPT_WRITEDATA, (void *) &buffer);

    // send the command

    sendToRobot((char *) "GetCamera.cgi");

    // extract frequency information from the buffer

    if (buffer.memory)
    {
        char* ptr = strstr(buffer.memory, "Frequency = ");
        if (ptr)
        {
            ptr += strlen("Frequency = ");
            if (ptr < (buffer.memory + buffer.size))
            {
                sscanf_s(ptr, "%d\n", &setting);
            }
        }
    }

    if (verboseMode)
    {
        printf("ROVIO: frequency get command returned : %d\n", setting);
    }

    // reset buffer

    clearBuffer();

    return setting;

}

// *****************************************************************************

bool Rovio::setAGC(bool state)
{

    char commandStringStem[ROVIO_COMMUNICATION_URL_MEMORY_SIZE];

    // if AGC on / off consturct command

    // sets AGC ceiling using http://.../debug.cgi?action=write_i2c&address=0x14&value=0xn8
    // where last n = 0,1,2,....,6 (default appears to be when n=1

    if (state)
    {
        sprintf_s(commandStringStem, ROVIO_COMMUNICATION_URL_MEMORY_SIZE, "/debug.cgi?action=write_i2c&address=0x14&value=0x68");
    }
    else
    {
        sprintf_s(commandStringStem, ROVIO_COMMUNICATION_URL_MEMORY_SIZE, "/debug.cgi?action=write_i2c&address=0x14&value=0x18");
    }

    if (verboseMode)
    {
        printf("ROVIO: AGC command @ setting : %d\n", state);
    }

    // set up a NULL write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, NULLWriteFunction);

    // if return code is zero, then all OK

    if (sendToRobot(commandStringStem) == ROVIO_HTTP_RETURN_CODE_ALL_OK)
    {
        return true;
    }

    return false;
}

// *****************************************************************************

bool Rovio::setNightMode(bool state, int framerate)
{

    char commandStringStem[ROVIO_COMMUNICATION_URL_MEMORY_SIZE];
    char commandStringStem2[ROVIO_COMMUNICATION_URL_MEMORY_SIZE];

    // first we need to check the level is appropriate

    switch (framerate)
    {
    case 2:
        sprintf_s(commandStringStem2, ROVIO_COMMUNICATION_URL_MEMORY_SIZE,
                  "debug.cgi?action=write_i2c&address=0x3b&value=0xa2");
        break;
    case 4:
        sprintf_s(commandStringStem2, ROVIO_COMMUNICATION_URL_MEMORY_SIZE,
                  "debug.cgi?action=write_i2c&address=0x3b&value=0xc2");
        break;
    case 8:
        sprintf_s(commandStringStem2, ROVIO_COMMUNICATION_URL_MEMORY_SIZE,
                  "debug.cgi?action=write_i2c&address=0x3b&value=0xe2");
        break;

    default:

        // if we are turning this mode on and level is invalid return false
        if (state)
        {
            return false;
        }
        break;
    }

    // if the state is false the 2nd command is the off (or no reduced framerate) command
    // N.B. this is the rovio default

    if (!state)
    {
        sprintf_s(commandStringStem2, ROVIO_COMMUNICATION_URL_MEMORY_SIZE,
                  "debug.cgi?action=write_i2c&address=0x3b&value=0x82");
    }

    if (verboseMode)
    {
        printf("ROVIO: night mode command @ setting : %d (framerate: %d)\n", state, framerate);
    }

    // set up a NULL write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, NULLWriteFunction);

    // in either state we need to first turn off night mode (as it must be "toggled"
    // to be activated

    sprintf_s(commandStringStem, ROVIO_COMMUNICATION_URL_MEMORY_SIZE, "debug.cgi?action=write_i2c&address=0x3b&value=0x02");

    if ((sendToRobot(commandStringStem) == ROVIO_HTTP_RETURN_CODE_ALL_OK) &&
            (sendToRobot(commandStringStem2) == ROVIO_HTTP_RETURN_CODE_ALL_OK))
    {
        return true;
    }

    return false;
}

// *****************************************************************************

bool Rovio::getWheelEncoders(bool& rightDir, int& rightTicks,
                             bool& leftDir, int& leftTicks,
                             bool& rearDir, int& rearTicks, bool useAccumulated)
{
    rightDir = wheelDirR; // right wheel encoder direction (true = forward/false = back)
    leftDir = wheelDirL; // left wheel encoder direction (true = forward/false = back)
    rearDir = wheelDirB; // back (rear) wheel encoder direction (true = forward/false = back)

    if (useAccumulated)
    {
        rightTicks = wheelAccumulatorR; // accumulated right wheel encoder value
        leftTicks =  wheelAccumulatorL; // accumulated left wheel encoder value
        rearTicks =  wheelAccumulatorB; // accumulated back wheel encoder value
    } else {
        rightTicks = wheelLastR; // last read right wheel encoder value
        leftTicks =  wheelLastL; // last read left wheel encoder value
        rearTicks =  wheelLastB; // last read back wheel encoder value
    }

    return true; // for backwards compatability with RovioLib 0.1x

}

// *****************************************************************************

bool Rovio::getWheelEncoder(bool &dir, int& ticks, int wheel, bool useAccumulated)
{

    switch (wheel)
    {
    case ROVIO_WHEEL_RIGHT:

        dir = wheelDirR; // right wheel encoder direction (true = forward/false = back)
        ticks = ((useAccumulated)? wheelAccumulatorR:wheelLastR); // accumulated right wheel encoder value

        break;
    case ROVIO_WHEEL_LEFT:

        dir = wheelDirL; // left wheel encoder direction (true = forward/false = back)
        ticks =  ((useAccumulated)? wheelAccumulatorL:wheelLastL); // accumulated left wheel encoder value

        break;
    case ROVIO_WHEEL_REAR:

        dir = wheelDirB; // back (rear) wheel encoder direction (true = forward/false = back)
        ticks =  ((useAccumulated)? wheelAccumulatorB:wheelLastB); // accumulated back wheel encoder value

        break;

    default:

        return false;
        break;
    }

    return true;

}

// *****************************************************************************

// wheel coders are updated after every drive command (only time they change)
// and return via the above routine

bool Rovio::updateWheelEncoders()
{

    bool returnValue = false;
    char hex_short[5];

    // set up a memory write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, ROVIO_CURL_WriteMemoryCallback);
    curl_easy_setopt(curlCom,CURLOPT_WRITEDATA, (void *) &buffer);

    // send the command

    sendToRobot((char *) "rev.cgi?Cmd=nav&action=20");

    // extract wheel encoder information from the buffer

    // N.B. *Dir wheel directons 0 - forwards rotation, 1 - backwards rotation

    if (buffer.memory)
    {
        char* ptr = strstr(buffer.memory, "responses = ");
        if (ptr)
        {
            ptr += strlen("responses = ");
        }

        if ((buffer.size - (ptr - buffer.memory)) >= 30)
        {

            // convert buffer from hexidecimal

            // extract byte 3 (from 5th hex nibble, indexed from zero => start @ 4)

            hex_short[0] = ptr[4];
            hex_short[1] = ptr[5];
            hex_short[2] = '\0';
            // leftDir = (int) (strtol(hex_short, NULL, 16)); // needs to be bit 2 of this value
            // bit set if reversing (we want true if going forwards)
            wheelDirL = !(getBitN((uchar) (strtol(hex_short, NULL, 16)), 1) > 0);

            // extract byte 4 and 5 (from 7th hex nibble)

            hex_short[0] = ptr[6];
            hex_short[1] = ptr[7];
            hex_short[2] = ptr[8];
            hex_short[3] = ptr[9];
            hex_short[4] = '\0';
            wheelLastL = (int) (strtol(hex_short, NULL, 16));
            wheelAccumulatorL += wheelLastL;

            // extract byte 6 (from 11th hex nibble)

            hex_short[0] = ptr[10];
            hex_short[1] = ptr[11];
            hex_short[2] = '\0';
            // rightDir = (int) (strtol(hex_short, NULL, 16));  // needs to be bit 2 of this value
            // bit set if reversing (we want true if going forwards)
            wheelDirR = !(getBitN((uchar) (strtol(hex_short, NULL, 16)), 1) > 0);

            // extract byte 7 and 8 (from 13th hex nibble)

            hex_short[0] = ptr[12];
            hex_short[1] = ptr[13];
            hex_short[2] = ptr[14];
            hex_short[3] = ptr[15];
            hex_short[4] = '\0';
            wheelLastR = (int) (strtol(hex_short, NULL, 16));
            wheelAccumulatorR += wheelLastL;

            // extract byte 9 (from 17th hex nibble)

            hex_short[0] = ptr[16];
            hex_short[1] = ptr[17];
            hex_short[2] = '\0';
            // rearDir = (int) (strtol(hex_short, NULL, 16));  // needs to be bit 2 of this value
            // bit set if reversing (we want true if going forwards)
            wheelDirB = !(getBitN((uchar) (strtol(hex_short, NULL, 16)), 1) > 0);

            // extract byte 10 and 11 (from 19th hex nibble)

            hex_short[0] = ptr[18];
            hex_short[1] = ptr[19];
            hex_short[2] = ptr[20];
            hex_short[3] = ptr[21];
            hex_short[4] = '\0';
            wheelLastB = (int) (strtol(hex_short, NULL, 16));
            wheelAccumulatorB += wheelLastB;

            returnValue = true;
        }
    }

    if (verboseMode)
    {
        printf("ROVIO: wheel encoders returned : ldir %d ltick %d "
               "rdir %d rtick %d "
               "reardir %d reartick %d \n",
               wheelDirL, wheelAccumulatorL, wheelDirR, wheelAccumulatorR,
               wheelAccumulatorB, wheelAccumulatorB);
    }

    // reset buffer

    clearBuffer();

    return returnValue;

}
// *****************************************************************************

// return the forward kinematics of the last (or accumulated) robot movement in terms of the
// vector position x, y and angle omega

void Rovio::getForwardKinematics(double& x, double& y, double& omega, bool useAccumulated)
{
    // compute forward dynamics taking into account wheel
    // last rotation direction

    if (useAccumulated) {
        computeForwardKinematics((double) ((wheelDirL)? wheelAccumulatorL:-1.0*wheelAccumulatorL),
                                 (double) ((wheelDirR)? wheelAccumulatorR:-1.0*wheelAccumulatorR),
                                 (double) ((wheelDirB)? wheelAccumulatorB:-1.0*wheelAccumulatorB),
                                  x, y, omega);
    }else {
        computeForwardKinematics((double) ((wheelDirL)? wheelLastL:-1.0*wheelLastL),
                                 (double) ((wheelDirR)? wheelLastR:-1.0*wheelLastR),
                                 (double) ((wheelDirB)? wheelLastB:-1.0*wheelLastB),
                                 x, y, omega);
    }
}

// *****************************************************************************

// follows the Rovio forward kinematics model/code
// from University of Pittsburgh CS 1567 note
// "Many views of the world Coordinate systems and sensor data
// Mapping to a common reference" with encoder constants from
// John Rogers, Georgia Institute of Technology

// Some code from John Rogers, jgrogers@cc.gatech.edu
// Copyright (C) 2009 Georgia Institute of Technology)

// V_l, V_r, V_c = difference in Left (l), Right (r) and back/centre (c) wheels

void Rovio::computeForwardKinematics(double V_l, double V_r, double V_c,
                                     double& Vx, double& Vy, double& omega) {

  // **** John Rogers kinematics model

  //static const double phi = 0.34907; // 20 degrees
  //static const double Lf = 0.74;//0.5;//0.1875;
  //static const double Lb = 1.0 - Lf;

  //static const double Wx = 375; //375 encoder ticks per meter forwards
  //static const double Wy = 140; //encoder ticks per meter sideways?
  //static const double Wt = 205; //205 encoder ticks per radian of rotation?

  //Vx = (-V_l + -V_r)/2*cos(phi)/Wx;
  //double front_vel = (V_l - V_r)/2.0 * sin(phi)*Lf;

  //double rear_vel = (V_c * Lb);
  //Vy = (front_vel - rear_vel)/Wy;
  //omega = (-V_r + V_l + V_c) / Wt;

  // **** University of Pittsburgh CS 1567 kinematics model

  // all meaures in "ticks" of encdoers (some of this dould be pre-computed - yes)

  Vx =  ((V_l * cos((30.0/180) * ROVIO_PI)) + (V_r * cos((150.0/180) * ROVIO_PI))) / 2.0;
  Vy =  ((V_l * sin((30.0/180) * ROVIO_PI)) + (V_r * sin((150.0/180) * ROVIO_PI)) + (V_c * sin((90.0/180) * ROVIO_PI))) / 3.0;
  omega = V_c / (ROVIO_PI * ROVIO_BASE_DIAMETER);

  // convert to centimetres and rads.

  Vx = Vx * ROVIO_CM_PER_TICK;
  Vy = Vy * ROVIO_CM_PER_TICK;
  omega = omega * ROVIO_RADIAN_PER_TICK;

}

// *****************************************************************************

bool Rovio::resetWheelEncoders()
{
    wheelDirL = true; // right wheel encoder direction (true = forward/false = back)
    wheelAccumulatorL = 0; // accumulated right wheel encoder value
    wheelDirR = true; // left wheel encoder direction (true = forward/false = back)
    wheelAccumulatorR =  0; // accumulated left wheel encoder value
    wheelDirB = true; // back (rear) wheel encoder direction (true = forward/false = back)
    wheelAccumulatorB =  0; // accumulated back wheel encoder value

    return true; // for backwards compatability with RovioLib 0.1x
}

// *****************************************************************************

int Rovio::getHeadPosition()
{

    int returnValue = -1;

    // set up a memory write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, ROVIO_CURL_WriteMemoryCallback);
    curl_easy_setopt(curlCom,CURLOPT_WRITEDATA, (void *) &buffer);

    // send the command

    sendToRobot((char *) "rev.cgi?Cmd=nav&action=1");

    // extract information from the buffer

    if (buffer.memory)
    {
        char* ptr = strstr(buffer.memory, "head_position=");
        if (ptr)
        {
            ptr += strlen("head_position=");
            if (ptr < (buffer.memory + buffer.size))
            {
                sscanf_s(ptr, "%d|", &returnValue);
            }
        }
    }

    if (verboseMode)
    {
        printf("ROVIO: head position encoders returned : %d\n", returnValue);
    }

    // reset buffer

    clearBuffer();

    return returnValue;

}

// *****************************************************************************

int Rovio::getBatteryLevel()
{

    int returnValue = -1;

    // set up a memory write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, ROVIO_CURL_WriteMemoryCallback);
    curl_easy_setopt(curlCom,CURLOPT_WRITEDATA, (void *) &buffer);

    // send the command

    sendToRobot((char *) "rev.cgi?Cmd=nav&action=1");

    // extract information from the buffer

    if (buffer.memory)
    {
        char* ptr = strstr(buffer.memory, "battery=");
        if (ptr)
        {
            ptr += strlen("battery=");
            if (ptr < (buffer.memory + buffer.size))
            {
                sscanf_s(ptr, "%d|", &returnValue);
            }
        }
    }

    if (verboseMode)
    {
        printf("ROVIO: battery level returned : %d\n", returnValue);
    }

    // reset buffer

    clearBuffer();

    return returnValue;

}

// *****************************************************************************

bool Rovio::getIRPowerStatus()
{

    bool returnValue = false;
    unsigned char lastByte = 0;

    // set up a memory write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, ROVIO_CURL_WriteMemoryCallback);
    curl_easy_setopt(curlCom,CURLOPT_WRITEDATA, (void *) &buffer);

    // send the command

    sendToRobot((char *) "rev.cgi?Cmd=nav&action=20");

    // extract information from the buffer

    if (buffer.memory)
    {
        char* ptr = strstr(buffer.memory, "responses = ");
        if (ptr)
        {
            ptr += strlen("responses = ");
            if ((ptr + 28) < (buffer.memory + buffer.size))
            {
                ptr += 28; // offset is 14 bytes but in HEX nibbles
                lastByte = (unsigned char) strtol(ptr, NULL, 16);
            }
        }
    }

    if (getBitN(lastByte, 1))
    {
        returnValue = true;
    }

    if (verboseMode)
    {
        printf("ROVIO: IR power status returned : %d\n", returnValue);
    }

    // reset buffer

    clearBuffer();

    return returnValue;

}

// *****************************************************************************

bool Rovio::getIRObstacle()
{

    bool returnValue = false;
    unsigned char lastByte = 0;

    // set up a memory write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, ROVIO_CURL_WriteMemoryCallback);
    curl_easy_setopt(curlCom,CURLOPT_WRITEDATA, (void *) &buffer);

    // send the command

    sendToRobot((char *) "rev.cgi?Cmd=nav&action=20");

    // extract information from the buffer

    if (buffer.memory)
    {
        char* ptr = strstr(buffer.memory, "responses = ");
        if (ptr)
        {
            ptr += strlen("responses = ");
            if ((ptr + 28) < (buffer.memory + buffer.size))
            {
                ptr += 28; // offset is 14 bytes but in HEX nibbles
                lastByte = (unsigned char) strtol(ptr, NULL, 16);
            }
        }
    }

    if (getBitN(lastByte, 2))
    {
        returnValue = true;
    }

    if (verboseMode)
    {
        printf("ROVIO: IR obstacle status returned : %d\n", returnValue);
    }

    // reset buffer

    clearBuffer();

    return returnValue;

}
// *****************************************************************************

bool Rovio::setIRPowerState(bool state)
{

    char commandStringStem[ROVIO_COMMUNICATION_URL_MEMORY_SIZE];

    // construct command

    sprintf_s(commandStringStem, ROVIO_COMMUNICATION_URL_MEMORY_SIZE,
              "rev.cgi?Cmd=nav&action=19&IR=%d", state);

    if (verboseMode)
    {
        printf("ROVIO: IR power command @ setting : %d\n", state);
    }

    // set up a NULL write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, NULLWriteFunction);

    // if return code is zero, then all OK

    if (sendToRobot(commandStringStem) == ROVIO_HTTP_RETURN_CODE_ALL_OK)
    {
        return true;
    }

    return false;
}

// *****************************************************************************

bool Rovio::getChargingStatus()
{

    bool returnValue = false;	// default error code
    int setting = -1;

    // set up a memory write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, ROVIO_CURL_WriteMemoryCallback);
    curl_easy_setopt(curlCom,CURLOPT_WRITEDATA, (void *) &buffer);

    // send the command

    sendToRobot((char *) "rev.cgi?Cmd=nav&action=1");

    // extract information from the buffer

    if (buffer.memory)
    {
        char* ptr = strstr(buffer.memory, "charging=");
        if (ptr)
        {
            ptr += strlen("charging=");
            if (ptr < (buffer.memory + buffer.size))
            {
                sscanf_s(ptr, "%d|", &setting);
            }
        }
    }

    if (setting == ROVIO_IS_CHARGING_MAGIC_NUMBER)
    {
        returnValue = true;
    }

    if (verboseMode)
    {
        printf("ROVIO: charging status returned : %d\n", returnValue);
    }

    // reset buffer

    clearBuffer();

    return returnValue;

}

// *****************************************************************************

bool Rovio::setLightState(bool state)
{

    char commandStringStem[ROVIO_COMMUNICATION_URL_MEMORY_SIZE];

    // check input and construct command

    if (state)
    {
        // all lights on

        sprintf_s(commandStringStem, ROVIO_COMMUNICATION_URL_MEMORY_SIZE,
                  "mcu?parameters=114D4D00010053485254000100011AFF0000");
    }
    else
    {

        // all lights off

        sprintf_s(commandStringStem, ROVIO_COMMUNICATION_URL_MEMORY_SIZE,
                  "mcu?parameters=114D4D00010053485254000100011A000000");
    }

    if (verboseMode)
    {
        printf("ROVIO: lights command @ setting : %d\n", state);
    }

    // set up a NULL write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, NULLWriteFunction);

    // if return code is zero, then all OK

    if (sendToRobot(commandStringStem) == ROVIO_HTTP_RETURN_CODE_ALL_OK)
    {
        return true;
    }

    return false;
}

// *****************************************************************************

bool Rovio::setLightPattern(bool a, bool b, bool c, bool d, bool e, bool f)
{

    char commandStringStem[ROVIO_COMMUNICATION_URL_MEMORY_SIZE];
    char firstNibble = 0;
    char secondNibble = 0;

    // check input and construct command

    if (a) firstNibble+=2;
    if (b) firstNibble+=1;
    if (c) secondNibble+=8;
    if (f) secondNibble+=1;
    if (e) secondNibble+=2;
    if (d) secondNibble+=4;

    sprintf_s(commandStringStem, ROVIO_COMMUNICATION_URL_MEMORY_SIZE,
              "mcu?parameters=114D4D00010053485254000100011A%x%x0000",
              firstNibble, secondNibble);

    if (verboseMode)
    {
        printf("ROVIO: light pattern command @ setting (a-f) : %d%d%d%d%d%d\n",
               a,b,c,d,e,f);
    }

    // set up a NULL write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, NULLWriteFunction);

    // if return code is zero, then all OK

    if (sendToRobot(commandStringStem) == ROVIO_HTTP_RETURN_CODE_ALL_OK)
    {
        return true;
    }

    return false;
}

// *****************************************************************************

bool Rovio::setHeadLightState(bool state)
{

    char commandStringStem[ROVIO_COMMUNICATION_URL_MEMORY_SIZE];

    // construct command

    sprintf_s(commandStringStem, ROVIO_COMMUNICATION_URL_MEMORY_SIZE,
              "rev.cgi?Cmd=nav&action=19&LIGHT=%d", state);

    if (verboseMode)
    {
        printf("ROVIO: headlight command @ setting : %d\n", state);
    }

    // set up a NULL write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, NULLWriteFunction);

    // if return code is zero, then all OK

    if (sendToRobot(commandStringStem) == ROVIO_HTTP_RETURN_CODE_ALL_OK)
    {
        return true;
    }

    return false;
}

// *****************************************************************************

bool Rovio::getHeadLightState()
{

    bool returnValue = false;
    char hex_short[3];

    // set up a memory write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, ROVIO_CURL_WriteMemoryCallback);
    curl_easy_setopt(curlCom,CURLOPT_WRITEDATA, (void *) &buffer);

    // send the command

    if (sendToRobot((char *) "rev.cgi?Cmd=nav&action=20") !=  ROVIO_HTTP_RETURN_CODE_ALL_OK)
    {
        return false;
    }

    // extract headlight state bit information from the buffer

    if (buffer.memory)
    {
        char* ptr = strstr(buffer.memory, "responses = ");
        if (ptr)
        {
            ptr += strlen("responses = ");
            if ((ptr + 29) <= (buffer.memory + buffer.size))
            {

                // extract byte 14 (and the 0th bit)

                hex_short[0] = ptr[28];
                hex_short[1] = ptr[29];
                hex_short[2] = '\0';

                // convert it to a boolean explicitly to stop MS VS 2008+ complaining

                returnValue = (getBitN((uchar) (strtol(hex_short, NULL, 16)), 0) > 0);
            }
        }
    }

    if (verboseMode)
    {
        printf("ROVIO: head light status returned : %d\n", returnValue);
    }

    // reset buffer

    clearBuffer();

    return returnValue;

}

// *****************************************************************************

int Rovio::getWifiSS()
{

    int returnValue = -1;

    // set up a memory write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, ROVIO_CURL_WriteMemoryCallback);
    curl_easy_setopt(curlCom,CURLOPT_WRITEDATA, (void *) &buffer);

    // send the command

    sendToRobot((char *) "rev.cgi?Cmd=nav&action=1");

    // extract information from the buffer

    if (buffer.memory)
    {
        char* ptr = strstr(buffer.memory, "wifi_ss=");
        if (ptr)
        {
            ptr += strlen("wifi_ss=");
            if (ptr < (buffer.memory + buffer.size))
            {
                sscanf_s(ptr, "%d|", &returnValue);
            }
        }
    }

    if (verboseMode)
    {
        printf("ROVIO: wifi signal strength returned : %d\n", returnValue);
    }

    // reset buffer

    clearBuffer();

    return returnValue;

}

// *****************************************************************************

int Rovio::getNavStatus()
{

    int returnValue = -1;

    // set up a memory write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, ROVIO_CURL_WriteMemoryCallback);
    curl_easy_setopt(curlCom,CURLOPT_WRITEDATA, (void *) &buffer);

    // send the command

    sendToRobot((char *) "rev.cgi?Cmd=nav&action=1");

    // extract information from the buffer

    if (buffer.memory)
    {
        char* ptr = strstr(buffer.memory, "state=");
        if (ptr)
        {
            ptr += strlen("state=");
            if (ptr < (buffer.memory + buffer.size))
            {
                sscanf_s(ptr, "%d|", &returnValue);
            }
        }
    }

    if (verboseMode)
    {
        printf("ROVIO: navigation state returned : %d\n", returnValue);
    }

    // reset buffer

    clearBuffer();

    return returnValue;

}
// *****************************************************************************

bool Rovio::startPathRecording()
{

    bool returnValue = false;

    // set up a NULL write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, NULLWriteFunction);

    // if return code is zero, then all OK

    if ((!pathRecordingState) &&
            (sendToRobot((char *) "rev.cgi?Cmd=nav&action=2") == ROVIO_HTTP_RETURN_CODE_ALL_OK))
    {
        pathRecordingState = true;
        returnValue = true;
    }

    if (verboseMode)
    {
        printf("ROVIO: start path recording - OK = %d\n", returnValue);
    }

    return returnValue;
}

// *****************************************************************************

bool Rovio::stopAbortPathRecording()
{

    bool returnValue = false;

    // set up a NULL write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, NULLWriteFunction);

    // if return code is zero, then all OK

    if (pathRecordingState &&
            (sendToRobot((char *) "rev.cgi?Cmd=nav&action=3") == ROVIO_HTTP_RETURN_CODE_ALL_OK))
    {
        pathRecordingState = false;
        returnValue = true;
    }

    if (verboseMode)
    {
        printf("ROVIO: abort path recording - OK = %d\n", returnValue);
    }

    return returnValue;
}

// *****************************************************************************

bool Rovio::stopStorePathRecording(char *pathName)
{

    char commandStringStem[ROVIO_COMMUNICATION_URL_MEMORY_SIZE];
    char* urlEncoded = NULL;
    bool returnValue = false;

    // check input

    if(!pathName)
        return false;

    // set up command

    urlEncoded = curl_easy_escape(curlCom,  pathName, 0);
    sprintf_s(commandStringStem, ROVIO_COMMUNICATION_URL_MEMORY_SIZE,
              "rev.cgi?Cmd=nav&action=4&name=%s", urlEncoded);
    curl_free(urlEncoded);

    // set up a NULL write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, NULLWriteFunction);

    // if return code is zero, then all OK

    if (pathRecordingState &&
            (sendToRobot(commandStringStem) == ROVIO_HTTP_RETURN_CODE_ALL_OK))
    {
        pathRecordingState = false;
        returnValue = true;
    }

    if (verboseMode)
    {
        printf("ROVIO: stop and store path %s - OK = %d\n", pathName,
               returnValue);
    }
    return returnValue;
}

// *****************************************************************************

bool Rovio::deletePath(char * pathName)
{

    char commandStringStem[ROVIO_COMMUNICATION_URL_MEMORY_SIZE];
    char* urlEncoded = NULL;
    bool returnValue = false;

    // check input

    if(!pathName)
        return false;

    // set up command

    urlEncoded = curl_easy_escape(curlCom, pathName, 0);
    sprintf_s(commandStringStem, ROVIO_COMMUNICATION_URL_MEMORY_SIZE,
              "rev.cgi?Cmd=nav&action=5&name=%s", urlEncoded);
    curl_free(urlEncoded);

    // set up a NULL write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, NULLWriteFunction);

    // if return code is zero, then all OK

    if (sendToRobot(commandStringStem) == ROVIO_HTTP_RETURN_CODE_ALL_OK)
    {
        returnValue = true;
    }

    if (verboseMode)
    {
        printf("ROVIO: delete path %s - OK = %d\n", pathName, returnValue);
    }
    return returnValue;
}

// *****************************************************************************

bool Rovio::deleteAllPaths()
{

    bool returnValue = false;

    // set up a NULL write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, NULLWriteFunction);

    // if return code is zero, then all OK

    if (sendToRobot((char *) "rev.cgi?Cmd=nav&action=21") == ROVIO_HTTP_RETURN_CODE_ALL_OK)
    {
        returnValue = true;
    }

    if (verboseMode)
    {
        printf("ROVIO: clear all paths - OK = %d\n", returnValue);
    }

    return returnValue;
}

// *****************************************************************************

bool Rovio::runPath(char * pathName, bool direction)
{

    char commandStringStem[ROVIO_COMMUNICATION_URL_MEMORY_SIZE];
    char* urlEncoded = NULL;
    bool returnValue = false;

    // check input

    if(!pathName)
        return false;

    // set up command

    urlEncoded = curl_easy_escape(curlCom, pathName, 0);
    if (direction)
    {
        sprintf_s(commandStringStem, ROVIO_COMMUNICATION_URL_MEMORY_SIZE,
                  "rev.cgi?Cmd=nav&action=7&name=%s", urlEncoded);
    }
    else
    {
        sprintf_s(commandStringStem, ROVIO_COMMUNICATION_URL_MEMORY_SIZE,
                  "rev.cgi?Cmd=nav&action=8&name=%s", urlEncoded);
    }
    curl_free(urlEncoded);

    // set up a NULL write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, NULLWriteFunction);

    // if return code is zero, then all OK

    if (sendToRobot(commandStringStem) == ROVIO_HTTP_RETURN_CODE_ALL_OK)
    {
        returnValue = true;
    }

    if (verboseMode)
    {
        printf("ROVIO: run path %s forward = %d - OK = %d\n", pathName,
               direction, returnValue);
    }
    return returnValue;
}

// *****************************************************************************

bool Rovio::renamePath(char * pathNameC, char * pathNameN)
{

    char commandStringStem[ROVIO_COMMUNICATION_URL_MEMORY_SIZE];
    char* urlEncodedC = NULL;
    char* urlEncodedN = NULL;
    bool returnValue = false;

    // check input

    if ((!pathNameC) || (!pathNameN))
        return false;

    // set up command

    urlEncodedC = curl_easy_escape(curlCom, pathNameC, 0);
    urlEncodedN = curl_easy_escape(curlCom, pathNameN, 0);
    sprintf_s(commandStringStem, ROVIO_COMMUNICATION_URL_MEMORY_SIZE,
              "rev.cgi?Cmd=nav&action=11&name=%s&newname=%s", urlEncodedC,
              urlEncodedN);
    curl_free(urlEncodedC);
    curl_free(urlEncodedN);

    // set up a NULL write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, NULLWriteFunction);

    // if return code is zero, then all OK

    if (sendToRobot(commandStringStem) == ROVIO_HTTP_RETURN_CODE_ALL_OK)
    {
        returnValue = true;
    }

    if (verboseMode)
    {
        printf("ROVIO: rename path %s -> %s OK = %d\n", pathNameC,
               pathNameN, returnValue);
    }
    return returnValue;
}

// *****************************************************************************

bool Rovio::stopPath()
{

    bool returnValue = false;

    // set up a NULL write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, NULLWriteFunction);

    // if return code is zero, then all OK

    if (sendToRobot((char *) "rev.cgi?Cmd=nav&action=9") == ROVIO_HTTP_RETURN_CODE_ALL_OK)
    {
        returnValue = true;
    }

    if (verboseMode)
    {
        printf("ROVIO: stop playing path - OK = %d\n", returnValue);
    }

    return returnValue;
}

// *****************************************************************************

bool Rovio::pausePath()
{

    bool returnValue = false;

    // set up a NULL write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, NULLWriteFunction);

    // if return code is zero, then all OK

    if (sendToRobot((char *) "rev.cgi?Cmd=nav&action=10") == ROVIO_HTTP_RETURN_CODE_ALL_OK)
    {
        returnValue = true;
    }

    if (verboseMode)
    {
        printf("ROVIO: pause playing path - OK = %d\n", returnValue);
    }

    return returnValue;
}

// *****************************************************************************

bool Rovio::returnHome()
{

    bool returnValue = false;

    // set up a NULL write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, NULLWriteFunction);

    // if return code is zero, then all OK

    if (sendToRobot((char *) "rev.cgi?Cmd=nav&action=12") == ROVIO_HTTP_RETURN_CODE_ALL_OK)
    {
        returnValue = true;
    }

    if (verboseMode)
    {
        printf("ROVIO: return home - OK = %d\n", returnValue);
    }

    return returnValue;
}

// *****************************************************************************

bool Rovio::returnHomeDock()
{

    bool returnValue = false;

    // set up a NULL write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, NULLWriteFunction);

    // if return code is zero, then all OK

    if (sendToRobot((char *) "rev.cgi?Cmd=nav&action=13") == ROVIO_HTTP_RETURN_CODE_ALL_OK)
    {
        returnValue = true;
    }

    if (verboseMode)
    {
        printf("ROVIO: return home + dock - OK = %d\n", returnValue);
    }

    return returnValue;
}

// *****************************************************************************

bool Rovio::setAsHome()
{

    bool returnValue = false;

    // set up a NULL write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, NULLWriteFunction);

    // if return code is zero, then all OK

    if (sendToRobot((char *) "rev.cgi?Cmd=nav&action=14") == ROVIO_HTTP_RETURN_CODE_ALL_OK)
    {
        returnValue = true;
    }

    if (verboseMode)
    {
        printf("ROVIO: set current position as home - OK = %d\n", returnValue);
    }

    return returnValue;
}

// *****************************************************************************

bool Rovio::clearHome()
{

    bool returnValue = false;

    // set up a NULL write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, NULLWriteFunction);

    // if return code is zero, then all OK

    if (sendToRobot((char *) "rev.cgi?Cmd=nav&action=27") == ROVIO_HTTP_RETURN_CODE_ALL_OK)
    {
        returnValue = true;
    }

    if (verboseMode)
    {
        printf("ROVIO: clear current home position - OK = %d\n", returnValue);
    }

    return returnValue;
}

// *****************************************************************************

bool Rovio::navReset()
{

    bool returnValue = false;

    // set up a NULL write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, NULLWriteFunction);

    // if return code is zero, then all OK

    if (sendToRobot((char *) "rev.cgi?Cmd=nav&action=17") == ROVIO_HTTP_RETURN_CODE_ALL_OK)
    {
        returnValue = true;
    }

    if (verboseMode)
    {
        printf("ROVIO: reset navigation state machine - OK = %d\n", returnValue);
    }

    return returnValue;
}

// *****************************************************************************

bool Rovio::setUser(char * username, char * password, bool admin)
{

    char commandStringStem[ROVIO_COMMUNICATION_URL_MEMORY_SIZE];
    char* urlEncodedUsername = NULL;
    char* urlEncodedPassword = NULL;
    bool returnValue = false;

    // check input

    if ((!username) || (!password))
        return false;

    // set up command

    urlEncodedUsername = curl_easy_escape(curlCom,  username, 0);
    urlEncodedPassword = curl_easy_escape(curlCom,  password, 0);

    sprintf_s(commandStringStem, ROVIO_COMMUNICATION_URL_MEMORY_SIZE,
              "SetUser.cgi?User=%s&Pass=%s&Privilege=%i", urlEncodedUsername,
              urlEncodedPassword, admin);
    curl_free(urlEncodedUsername);
    curl_free(urlEncodedPassword);

    // set up a NULL write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, NULLWriteFunction);

    // if return code is zero, then all OK

    if (sendToRobot(commandStringStem) == ROVIO_HTTP_RETURN_CODE_ALL_OK)
    {
        returnValue = true;
    }

    if (verboseMode)
    {
        printf("ROVIO: add user %s - OK = %d\n", username,
               returnValue);
    }
    return returnValue;
}

// *****************************************************************************

bool Rovio::delUser(char * username)
{

    char commandStringStem[ROVIO_COMMUNICATION_URL_MEMORY_SIZE];
    char* urlEncoded = NULL;
    bool returnValue = false;

    // check input

    if(!username)
        return false;

    // set up command

    urlEncoded = curl_easy_escape(curlCom, username, 0);
    sprintf_s(commandStringStem, ROVIO_COMMUNICATION_URL_MEMORY_SIZE,
              "DelUser.cgi?User=%s", urlEncoded);
    curl_free(urlEncoded);

    // set up a NULL write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, NULLWriteFunction);

    // if return code is zero, then all OK

    if (sendToRobot(commandStringStem) ==  ROVIO_HTTP_RETURN_CODE_ALL_OK)
    {
        returnValue = true;
    }

    if (verboseMode)
    {
        printf("ROVIO: delete user %s - OK = %d\n", username, returnValue);
    }
    return returnValue;
}

// *****************************************************************************

bool Rovio::reboot()
{

    bool returnValue = false;

    // set up a NULL write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, NULLWriteFunction);

    // if return code is zero, then all OK

    if (sendToRobot((char *) "Reboot.cgi") == ROVIO_HTTP_RETURN_CODE_ALL_OK)
    {
        returnValue = true;
    }

    if (verboseMode)
    {
        printf("ROVIO: robot reboot - OK = %d\n", returnValue);
    }

    return returnValue;
}

// *****************************************************************************

char* Rovio::getLibVersion()
{

    char * returnValue = NULL;	// default error return

    // set up a memory write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, ROVIO_CURL_WriteMemoryCallback);
    curl_easy_setopt(curlCom,CURLOPT_WRITEDATA, (void *) &buffer);

    // send the command

    if (sendToRobot((char *) "rev.cgi?Cmd=nav&action=25") == ROVIO_HTTP_RETURN_CODE_ALL_OK)
    {

        // extract information from buffer

        if (buffer.memory)
        {
            char* ptr = strstr(buffer.memory, "version = ");
            if (ptr)
            {
                ptr += strlen("version = ");
                if (ptr < (buffer.memory + buffer.size))
                {
                    returnValue = (char*) malloc(strlen(ptr) + 1);
                    strcpy_s(returnValue, strlen(ptr) + 1, ptr);
                }
            }
        }

        // reset buffer

        clearBuffer();

        if (verboseMode)
        {
            printf("ROVIO: robot lib version : %s\n", returnValue);
        }
        return returnValue;
    }

    if (verboseMode)
    {
        printf("ROVIO: get lib version : FAILURE\n");
    }
    return returnValue;

}

// *****************************************************************************

char* Rovio::getFirmwareVersion()
{

    char * returnValue = NULL;	// default error return

    // set up a memory write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, ROVIO_CURL_WriteMemoryCallback);
    curl_easy_setopt(curlCom,CURLOPT_WRITEDATA, (void *) &buffer);

    // send the command

    if (sendToRobot((char *) "GetVer.cgi") == ROVIO_HTTP_RETURN_CODE_ALL_OK)
    {

        // return pointer to buffer

        returnValue = buffer.memory;

        // reset buffer for re-allocation on next call
        // (previous memory now belongs outside of call)

        buffer.memory = NULL;
        buffer.size = 0;

        if (verboseMode)
        {
            printf("ROVIO: robot firmware version :\n%s\n", returnValue);

        }

        return returnValue;
    }

    if (verboseMode)
    {
        printf("ROVIO: get firmware version : FAILURE\n");
    }

    return returnValue;

}
// *****************************************************************************

char* Rovio::getAPIVersion()
{

    int length = (strlen("CGI API: %d.%d C++ API %d.%d") + 1);
    char *returnValue = (char *) malloc(sizeof(char) * length);

    sprintf_s(returnValue, length, "CGI API: %d.%d C++ API %d.%d%d",
              ROVIO_CGI_API_VERSION_MAJOR, ROVIO_CGI_API_VERSION_MINOR,
              ROVIO_CPP_API_VERSION_MAJOR, ROVIO_CPP_API_VERSION_MINOR,
              ROVIO_CPP_API_VERSION_MINOR_STEP);

    return returnValue;

}

// *****************************************************************************

bool Rovio::setAPIVerbose(bool status)
{
    verboseMode = status;
    return true;
}

bool Rovio::getAPIVerbose()
{
    return verboseMode;
}

bool Rovio::setHTTPVerbose(bool status)
{
    curl_easy_setopt(curlCom, CURLOPT_VERBOSE, (int) status);
    httpVerboseMode = status;
    return true;
}

bool Rovio::getHTTPVerbose()
{
    return httpVerboseMode;
}

// *****************************************************************************

Rovio::~Rovio()
{
    // if we have a connection (still) then reset the defaults

    if (isConnected())
    {
        if (verboseMode)
        {
            printf("ROVIO: close down : resetting default settings\n");
        }

        setResolution(ROVIO_IMAGE_DEFAULT_RESOLUTION);
        setBrightness(ROVIO_IMAGE_DEFAULT_BRIGHTNESS);
        setContrast(ROVIO_IMAGE_DEFAULT_CONTRAST_LEVEL);
        setFrameRate(ROVIO_IMAGE_DEFAULT_FRAMERATE);
        setCompressionQuality(ROVIO_IMAGE_DEFAULT_COMPRESSION);
        setSensorFrequencyCompensation(ROVIO_IMAGE_DEFAULT_FREQ_COMPENSATION);
        setIRPowerState(ROVIO_IR_DEFAULT_POWER_STATE);
        setLightState(ROVIO_LIGHT_DEFAULT_STATE_ALL);
        setHeadLightState(ROVIO_HEADLIGHT_DEFAULT_STATE);
        setAGC(ROVIO_IMAGE_DEFAULT_AGC_STATE);
        setNightMode(ROVIO_IMAGE_DEFAULT_NIGHT_MODE_STATE);
        manualDrive(ROVIO_HEADDOWN, ROVIO_HEAD_DEFAULT_POSITION);

        if ((getChargingStatus()) && (ROVIO_RESET_HOME_IF_DOCKED_ON_CLOSEDOWN))
        {
            setAsHome();
            if (verboseMode)
            {
                printf("ROVIO: close down : home reset as current docked position\n");
            }
        }
    }

    // clean up CURL object and buffers

    curl_easy_cleanup(curlCom);
    curl_easy_cleanup(curlVid);

    clearBuffer();
    clearFrame();

    // all done

    if (verboseMode)
    {
        printf("ROVIO: close-down : complete\n");
    }

}

// *****************************************************************************

int Rovio::sendToRobot(char* urlStem)
{
    return sendToRobot(curlCom, urlStem);
}

int Rovio::sendToRobot(CURL *curlObject, char* urlStem)
{

    long serverReturnCode; // server return code
    CURLcode res;

    // construct full URL to send

    int l = strlen(urlBase);
    urlToSend[0] = '\0';
    strcpy_s(urlToSend, ROVIO_COMMUNICATION_URL_MEMORY_SIZE, urlBase);
    strcpy_s(urlToSend + l, (ROVIO_COMMUNICATION_URL_MEMORY_SIZE - l), urlStem);

    // send URL to robot

    curl_easy_setopt(curlObject, CURLOPT_URL, urlToSend);
    res = curl_easy_perform(curlObject);

    // get server response code

    res = curl_easy_getinfo(curlObject, CURLINFO_RESPONSE_CODE, &serverReturnCode);

    if (verboseMode)
    {
        printf("ROVIO: command sent : %s http response code : %ld\n",
               urlToSend, serverReturnCode);
    }

    // return server response code

    return (int) serverReturnCode;

}

// *****************************************************************************

// return (0 or 1) value of bit n from byte (bits 0->7)

unsigned char Rovio::getBitN(unsigned char byte, int n)
{
    unsigned char b;
    b = 1 & (byte >> n);
    return b;
};

// *****************************************************************************

// extract the CGI response from the buffer

int Rovio::getCGIResponse()
{
    int code = -1;
    if ((buffer.size > 0) && (buffer.memory != NULL))
    {
        // find the "responses =" string and return number
        // after it

        char* ptr = strstr(buffer.memory, "responses = ");
        sscanf_s(ptr, "responses = %d", &code);
    }

    if (verboseMode)
    {
        printf("ROVIO: CGI response code from robot : %i\n", code);
    }

    return code;
}

// *****************************************************************************

// clear buffer used for URL responses

void Rovio::clearBuffer()
{
    if ((buffer.size > 0) && (buffer.memory != NULL))
    {
        buffer.size = 0;
        free(buffer.memory);
        buffer.memory = NULL;
    }
}

// *****************************************************************************

// clear frame used for video streaming responses

void Rovio::clearFrame()
{
    if ((frame.size > 0) && (frame.memory != NULL))
    {
        frame.size = 0;
        free(frame.memory);
        frame.memory = NULL;
    }

    if ((frame.inputSaveBufferSize > 0) && (frame.inputSaveBuffer != NULL))
    {
        frame.inputSaveBufferSize = 0;
        free(frame.inputSaveBuffer);
        frame.inputSaveBuffer = NULL;
    }
}

// *****************************************************************************

// call-back funtions for libCURL to deliver us no buffer
//  (cannot be inside class as libCURL then seems to crash)

size_t NULLWriteFunction( void *ptr, size_t size, size_t nmemb,
                          void *stream)
{
    // do nothing

    return nmemb*size;
};

// *****************************************************************************

// call-back funtions for libCURL to deliver us the file from the URL as a memory
// buffer

size_t ROVIO_CURL_BinaryWriteFunction
( void *ptr, size_t size, size_t nmemb, void *stream)
{
    fwrite(ptr, size, nmemb, (FILE *) stream);
    return nmemb*size;
};

static void* ROVIO_CURL_realloc(void *ptr, size_t size)
{
    /* There might be a realloc() out there that doesn't like reallocing
       NULL pointers, so we take care of it here */
    if(ptr)
        return realloc(ptr, size);
    else
        return malloc(size);
}

size_t ROVIO_CURL_WriteMemoryCallback
(void *ptr, size_t size, size_t nmemb, void *data)
{
    size_t realsize = size * nmemb;
    struct ROVIOCURLMemoryStruct *mem = (struct ROVIOCURLMemoryStruct *)data;

    mem->memory = (char *)
                  ROVIO_CURL_realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory)
    {
        memcpy(&(mem->memory[mem->size]), ptr, realsize);
        mem->size += realsize;
        mem->memory[mem->size] = 0;
    }
    return realsize;
}

// *****************************************************************************

#ifdef _WIN32
// FROM http://pastebin.com/J2ibQjAj
///////////////////////////////////////////////////////////////////////////////
//
// memmem()
//
// Purpose:     Find a byte sequence within a memory buffer
//
// Parameters:  buf               - pointer to buffer
//              buf_len           - size of buffer in bytes
//              byte_sequence     - byte sequence to look for
//              byte_sequence_len - size of byte sequence in bytes
//
// Returns:     void * - if successful, returns a pointer to the first
//                       occurrence of byte_sequence in buf;  otherwise,
//                       returns NULL
//
// Notes;       Characters in byte_sequence and characters in buf will be
//              compared "as is", with no case conversion.
//
void* memmem(const void *buf,
                         size_t buf_len,
                         const void *byte_sequence,
                         size_t byte_sequence_len)
{
        BYTE *bf = (BYTE *)buf;
        BYTE *bs = (BYTE *)byte_sequence;
        BYTE *p  = bf;

        while (byte_sequence_len <= (buf_len - (p - bf)))
        {
                UINT b = *bs & 0xFF;
                if ((p = (BYTE *) memchr(p, b, buf_len - (p - bf))) != NULL)
                {
                        if ((memcmp(p, byte_sequence, byte_sequence_len)) == 0)
                                return p;
                        else
                                p++;
                }
                else
                {
                        break;
                }
        }
        return NULL;
}
#endif

// *****************************************************************************

// call-back funtions for libCURL to deliver us the file from the URL as a OpenCV
// image

size_t ROVIO_CURL_WriteMemoryVideoFrameCallback (void *ptr, size_t size, size_t nmemb, void *data)
{
    size_t realsize = size * nmemb;
    struct ROVIOCURLFrameStruct *frame = (struct ROVIOCURLFrameStruct*)data;

    void *inputbuf; // buffer on which the whole function will be working
	size_t inputbufsize; // its size

	// check the input

	if(!ptr) {
		#ifdef ROVIO_CURL_WriteMemoryVideoFrameCallbackDEBUG
        	printf("bad pointer skip\n");
		#endif
        return 0;
    }

    // to keep an imput buffer with a size of at least ROVIO_CAMERA_VIDEO_INTERLEAVE_SIZE

    if(realsize < ROVIO_CAMERA_VIDEO_INTERLEAVE_SIZE) {

		// save the current input

		frame->inputSaveBuffer = (char*) realloc(frame->inputSaveBuffer, frame->inputSaveBufferSize + realsize);
		memcpy(&(frame->inputSaveBuffer[frame->inputSaveBufferSize]), ptr, realsize);
		frame->inputSaveBufferSize += realsize;

		#ifdef ROVIO_CURL_WriteMemoryVideoFrameCallbackDEBUG
        	printf("input buffer too small skip\n");
		#endif
        return realsize;
    }

	// set the buffer on which we will be working

	if(frame->inputSaveBuffer) {
		frame->inputSaveBuffer = (char*) realloc(frame->inputSaveBuffer, frame->inputSaveBufferSize + realsize);
		memcpy(&(frame->inputSaveBuffer[frame->inputSaveBufferSize]), ptr, realsize);
		frame->inputSaveBufferSize += realsize;

		inputbuf = frame->inputSaveBuffer;
		inputbufsize = frame->inputSaveBufferSize;
	}
	else {
		inputbuf = ptr;
		inputbufsize = realsize;
	}

    // have we found the next image interleave signature in the stream ?

    //char* interleave = strstr((char *)ptr, ROVIO_CAMERA_VIDEO_INTERLEAVE);
    char* interleave = (char *) memmem((char *)inputbuf, inputbufsize, ROVIO_CAMERA_VIDEO_INTERLEAVE, ROVIO_CAMERA_VIDEO_INTERLEAVE_SIZE);

    if ((interleave) && (frame->size < ROVIO_CAMERA_VIDEO_STREAMING_BUFFER_SIZE_TOO_BIG_HACK))
    {
        // copy any remaining data before the next interleave onto end of current buffer

        #ifdef ROVIO_CURL_WriteMemoryVideoFrameCallbackDEBUG
            printf("do decode - level 1\n");
        #endif

        size_t beforeinterleave = (interleave - (char*)inputbuf);
        frame->memory = (char *)ROVIO_CURL_realloc(frame->memory, (frame->size) + beforeinterleave + 1);
        if (!frame->memory){
            frame->size = 0;
            return 0;
        }

        memcpy(&(frame->memory[frame->size]), inputbuf, beforeinterleave);
		frame->size +=  beforeinterleave;
		frame->memory[frame->size] = 0;

        // don't assume the buffer is aligned with the previous interleave signature at the begining
        // so find where it is and attempt any decoding as an offset from that point

        //char* p_interleave = strstr((char *)frame->memory, ROVIO_CAMERA_VIDEO_INTERLEAVE);
        char* p_interleave = (char *) memmem((char *)frame->memory, frame->size, ROVIO_CAMERA_VIDEO_INTERLEAVE, ROVIO_CAMERA_VIDEO_INTERLEAVE_SIZE);

        if (p_interleave)
        {

            #ifdef ROVIO_CURL_WriteMemoryVideoFrameCallbackDEBUG
                printf("do decode - level 2\n");
            #endif

            // attempt to decode previously completed frame (before current interleave position in the stream)

            size_t img_size = frame->size - (p_interleave - frame->memory) - ROVIO_CAMERA_VIDEO_INTERLEAVE_SIZE;

            if (((img_size >= ROVIO_JPEG_MIN_SIZE)) &&
                    (img_size >= (ROVIO_CAMERA_VIDEO_INTERLEAVE_SIZE + ROVIO_CAMERA_VIDEO_STREAMING_BUFFER_SIZE_TOO_SMALL_HACK)) &&
                    (p_interleave[ROVIO_CAMERA_VIDEO_INTERLEAVE_SIZE] & ROVIO_JPEG_SOI_FIRST_BYTE) &&
                    (p_interleave[ROVIO_CAMERA_VIDEO_INTERLEAVE_SIZE + 1] & ROVIO_JPEG_SOI_SECOND_BYTE))
            {
                // use mutex around the image

                #ifdef ROVIO_CURL_WriteMemoryVideoFrameCallbackDEBUG
                    printf("do decode - level 3\n");
                #endif

				#ifdef linux
					pthread_mutex_lock( &(frame->completed));
                #endif
				#ifdef _WIN32
					WaitForSingleObject(frame->completed, INFINITE);
                #endif
				frame->image = cv::Mat(1, img_size, CV_8UC1, &(p_interleave[ROVIO_CAMERA_VIDEO_INTERLEAVE_SIZE]));
                frame->image = cv::imdecode(frame->image, CV_LOAD_IMAGE_UNCHANGED);
				#ifdef linux
					pthread_mutex_unlock( &(frame->completed));
                #endif
				#ifdef _WIN32
					ReleaseMutex(frame->completed);
				#endif
				#ifdef ROVIO_CURL_WriteMemoryVideoFrameCallbackDEBUG
                    printf("decoded - all OK\n");
                #endif
            }
        }

        // copy data occuring at and after the next interleave (including it) and following data
        // to the begining of the current buffer (effectively over-writing previously decoded frame data)

        size_t frominterleave = inputbufsize - beforeinterleave;

        // not the most efficient way to free() then realloc() but seems to keep things clean

        if (frame->memory){free(frame->memory);}
        frame->memory = NULL;

        #ifdef ROVIO_CURL_WriteMemoryVideoFrameCallbackDEBUG
            printf("allocating %d\n", (int) frominterleave);
        #endif
        frame->memory = (char *)ROVIO_CURL_realloc(frame->memory, frominterleave + 1);
        if (!frame->memory){
            frame->size = 0;
            return 0;
        }

        memcpy(frame->memory, interleave, frominterleave);
        frame->size = frominterleave;
        frame->memory[frame->size] = 0;

    }
    else
    {
        // check the buffer size and rescue things if they seem to have gone bad
        // because probably the video has gone bad

        if (frame->size > ROVIO_CAMERA_VIDEO_STREAMING_BUFFER_SIZE_TOO_BIG_HACK)
        {
            #ifdef ROVIO_CURL_WriteMemoryVideoFrameCallbackDEBUG
                printf("do reset - bad stuff\n");
            #endif
            frame->size = 0;

            // not the most efficient way to free() then realloc() but seems to keep things clean

            if (frame->memory){free(frame->memory);} // free memory as ROVIO_CULRL_realloc() handles NULL pointers
            frame->memory = NULL;
        }


        // when no interleave signature is found just copy across all of the data onto the end of the current
        // buffer

        #ifdef ROVIO_CURL_WriteMemoryVideoFrameCallbackDEBUG
            printf("copy copy copy\n");
        #endif

        frame->memory = (char *)ROVIO_CURL_realloc(frame->memory, (frame->size) + inputbufsize + 1);
        if (!frame->memory){
            frame->size = 0;
            return 0;
        }
        memcpy(&(frame->memory[frame->size]), inputbuf, inputbufsize);
        frame->size += inputbufsize;
        frame->memory[frame->size] = 0;
    }

    #ifdef ROVIO_CURL_WriteMemoryVideoFrameCallbackDEBUG
        printf("memory buffer size = %d, real size (newly added) = %d\n", (int) frame->size, (int) inputbufsize);
    #endif

	// clean frame->inputSaveBuffer

	if(frame->inputSaveBuffer) {
        frame->inputSaveBufferSize = 0;
        free(frame->inputSaveBuffer);
        frame->inputSaveBuffer = NULL;
	}

    return realsize;
}

// *****************************************************************************

void Rovio::getBaseStationSignalInfo(int &x, int &y, float &theta, int &room, int &ss)
{
    x = y = theta = 0;

    // set up a memory write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, ROVIO_CURL_WriteMemoryCallback);
    curl_easy_setopt(curlCom,CURLOPT_WRITEDATA, (void *) &buffer);

    // send the command

    sendToRobot((char *) "rev.cgi?Cmd=nav&action=1");

    // extract information from the buffer
    if (buffer.memory)
    {
        char* ptr = strstr(buffer.memory, "x=");
        if (ptr)
        {
            ptr += strlen("x=");
            if (ptr < (buffer.memory + buffer.size))
            {
                sscanf_s(ptr, "%d|", &x);
            }
        }

		ptr = strstr(buffer.memory, "y=");
        if (ptr)
        {
            ptr += strlen("y=");
            if (ptr < (buffer.memory + buffer.size))
            {
                sscanf_s(ptr, "%d|", &y);
            }
        }

		ptr = strstr(buffer.memory, "theta=");
        if (ptr)
        {
            ptr += strlen("theta=");
            if (ptr < (buffer.memory + buffer.size))
            {
                sscanf_s(ptr, "%f|", &theta);
            }
        }

		ptr = strstr(buffer.memory, "room=");
        if (ptr)
        {
            ptr += strlen("room=");
            if (ptr < (buffer.memory + buffer.size))
            {
                sscanf_s(ptr, "%d|", &room);
            }
        }

		ptr = strstr(buffer.memory, "ss=");
        if (ptr)
        {
            ptr += strlen("ss=");
            if (ptr < (buffer.memory + buffer.size))
            {
                sscanf_s(ptr, "%d|", &ss);
            }
        }
    }

    if (verboseMode)
    {
        printf("ROVIO: Base station signal returned : x=%d, y=%d, theta=%f\n", x, y, theta);
    }

    // reset buffer

    clearBuffer();

}

// *****************************************************************************

// return WLAN information from all

bool Rovio::getWlanScan(std::vector<ROVIOWifiAccessPoint> &result)
{
	char *ptr_mac, *ptr_essid, *ptr_quality;

    ROVIOWifiAccessPoint current;

    // clear prior list
    result.clear();

    // set up a memory write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, ROVIO_CURL_WriteMemoryCallback);
    curl_easy_setopt(curlCom,CURLOPT_WRITEDATA, (void *) &buffer);

    // send the command

    sendToRobot((char *) "ScanWlanEXT.cgi");

    // extract information from the buffer
    if (buffer.memory)
    {
		ptr_mac = ptr_essid = ptr_quality =  buffer.memory;

		do {

			// get the mac adress
			ptr_mac = strstr(ptr_mac, "MAC = ");
			if (ptr_mac)
			{
				ptr_mac += strlen("MAC = ");
				if (ptr_mac < (buffer.memory + buffer.size))
				{
					sscanf_s(ptr_mac, "%17c", &(current.mac));
					current.mac[17] = '\0';
				}
				else
					ptr_mac = NULL;
			}

			// get the mac ESSID
			ptr_essid = strstr(ptr_essid, "ESSID = ");
			if (ptr_essid && ptr_mac)
			{
				ptr_essid += strlen("ESSID = ");
				if (ptr_essid < (buffer.memory + buffer.size))
				{
					char *end = (char*) memchr(ptr_essid, '\n', (buffer.memory + buffer.size) - ptr_essid);
					memcpy(&(current.essid), ptr_essid, end - ptr_essid);
					current.essid[end - ptr_essid] = '\0';
				}
			}

			// get the quality
			ptr_quality = strstr(ptr_quality, "Quality = ");
			if (ptr_quality && ptr_mac)
			{
				ptr_quality += strlen("Quality = ");
				if (ptr_quality < (buffer.memory + buffer.size))
				{
					sscanf_s(ptr_quality, "%d", &(current.quality));
				}
			}

			// do not record the last result as it has no quality attibute

			if (!(ptr_mac && !ptr_quality))
				result.push_back(current);

		} while(ptr_mac && ptr_essid && ptr_quality);
    }

    if (verboseMode)
    {
        printf("ROVIO: Scan WLAN recieved %s \n", buffer.memory);
    }

    // reset buffer

    clearBuffer();

	return true;
}

// *****************************************************************************
