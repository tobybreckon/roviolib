// *****************************************************************************

// Rovio API C++ Class Library : C++ Interface to Rovio Robot API

// Dependancies : OpenCV 2.1 (or later), libCurl 7.18 (or later)

// Author : Toby Breckon, toby.breckon@cranfield.ac.uk

// Copyright (c) 2011 Toby Breckon, School of Engineering, Cranfield University
// License : GPL - http://www.gnu.org/copyleft/gpl.html

// *****************************************************************************

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

To use both rovio_example.cpp and testharness.cc you will need to set your own robot settings
as follows in the top of both files:

#define TESTHARNESS_ROVIO_HOST "robot.domain.tld" // or IP address
#define TESTHARNESS_ROVIO_USER "username"
#define TESTHARNESS_ROVIO_PASS "password"

Note that this library is dependant on OpenCV 2.1 (or later), libCurl 7.18 (or later).

Both rovio_example.cpp and testharness.cc are mutually exclusive (both define void main())
and cannot be used together in the same project/build.

*** TO USE : MS Windows (with Visual Studio 2008 onwards)

- Setup Visual Studio for OpenCV (using local instructions or
http://opencv.willowgarage.com/wiki/VisualC++)

- Setup Visual Studio for libCURL (using local instructions at:
http://www.cranfield.ac.uk/~toby.breckon/teaching/dip/rovio/libcurl_vs2010_setup.pdf
or see:http://curl.haxx.se/libcurl/)

- Add files rovio_cc_lib.cc and rovio_cc_lib.h to Visual Studio project
(also add EITHER of testharness.cc and the rovio_example.cpp if required)

- Add directory containing rovio_cc_lib.h to Visual Studio "VC++ Directories -> Include"

- Build project, .... connect to robot.

**** TO USE : Linux /MacOS (with gcc etc.)

Ensure you have both OpenCV and libCURL installed and configured for use with pkg-config.

A CMakefile and Makefile is provided for linux (and *nix / MacOS) users. By default this builds
both the testharness and the rovio_example in the current directory using both library
dependancy configurations provided by pkg-config

To use this library add the rovio_cc_lib.cc and rovio_cc_lib.h files to any project/build
you are using. We assume you know what your doing in an varient of software build environment
we use.

Build project, .... connect to robot.


*** Previously Asked Questions (PAQ)

Q. Why as this is a C++ library does it use char* for value returns from methods ?

A. The underlying URL handler is libCURL which is a C library. It seemed easier than converting
everything. We use this datatype internally to the library for this reason too.

--

Q. Why does the library not implement all of the functionality in the Rovio API 1.3 ?

A. The sub-set of functionality is targeted towards using rovio for computer vision tasks. Feel
free to implement missing functionality using my examples and send me the functions for
inclusion in the library (provided the programming / interface style matches the existing code
and it is documented).

--

Q. Why does libCURL / OpenCV not work properly together for me on my system?

A. Try the simple example http://www.cranfield.ac.uk/~toby.breckon/teaching/dip/rovio/imagetest.cc
first. When this works get back with any issues related to the library itself - libCURL and OpenCV
config support is non something that is on offer. Don't ask.

--

Q. How well tested in the library ?

A. We have 5 rovios (~2010 models, firmware 5.03+) we use for teaching. We test over all 5. Many
people do the testing. But every robot is different .... (this is standard robotics stuff).

--

Q. What do I do if I find a bug in the library ?

A. Use the testharness to isolate the conditons under which it occurs. Find a fix. Send it to me. Thanks.
[We'll credit you in the source code of future releases and you gain eternal open source fame. You'll feel
great about it.]

--

Q. Can I use this library in my own project outside of Cranfield University?

A. Yes. The code is copyright (c) Toby Breckon, Cranfield University 2010/2011 onwards. It's licensed under
the GPL - read and understand what this means from the URL at the top of this file.

Contributions to the project are under any copyright asserted by the contributor.

Licensing terms maybe made available upon request.

--

Q. Why this this code licensed under the GPL ?

A. This is because we need to make it publicly available for our teaching use whilst protecting the fact that
it is not used by a third party to create a closed source product and make commerical gain from code that is
the property of the copyright holder. This would not be fair or correct.

***
