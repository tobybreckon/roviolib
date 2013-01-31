roviolib - Rovio Robot C++ API Library
========

 ***************************************************************************** 
 Rovio API C++ Class Library : C++ Interface to Rovio Robot API 
 
 Dependancies : OpenCV 2.1 (or later), libCurl 7.18 (or later) 
 
 Author : Toby Breckon, toby.breckon@cranfield.ac.uk 
 
 Copyright (c) 2011-13 Toby Breckon, School of Engineering, Cranfield University
 
 License : GPL - http://www.gnu.org/copyleft/gpl.html
 
 *****************************************************************************

The library is provided as a single C++ source file and header as follows:
- rovio_cc_lib.cc
- rovio_cc_lib.h

The library is documented (i.e. usage how-to) in the header file rovio_cc_lib.h

In addition an example control and image display program for the Rovio is provided
as follows:
- rovio_example.cpp

This basic example makes use of some of the library functionality to do basic search
and exploration of an environment whilst plotting the wifi signal strength levels (as
a simple example).

In addition the library has a seperate test harness as follows:
- testharness.cc

This is designed to exercise each and every method in the library with both valid and
invalid options. It is not intended to control the robot in a useful way. Note the
on/off test switches (#define TESTHARNESS_ ....) at the top of the file to turn on/off
test harness functionality. Use this example to isolate bugs in the library as required.
