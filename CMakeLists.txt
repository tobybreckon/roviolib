# RovioLib CMake File - http://www.cmake.org/
# Toby Breckon, 2013

cmake_minimum_required (VERSION 2.6)
set( CMAKE_CXX_FLAGS "-O3 -Wall ${CMAKE_CXX_FLAGS}" )

# SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "CMakeCache.txt;CMakeFiles;cmake_install.cmake")

# linux specific stuff

IF ( UNIX )
   MESSAGE( "USING LINUX CONFIG" )
   set( CMAKE_PREFIX_PATH "/opt/opencv" )
   set( CMAKE_LIBRARY_PATH "/opt/opencv" )
   MESSAGE( "Assuming OpenCV in " ${CMAKE_PREFIX_PATH} " unless detected otherwise" )
   set( CMAKE_FIND_LIBRARY_PREFIXES "/usr" )
   set( CURL_INCLUDE_DIR "/usr/include/curl" )
   set( CMAKE_CXX_FLAGS "-lpthread -lcurl")
ENDIF ( UNIX )

# windows (inc. 64-bit specific stuff)

IF ( WIN32 )
   MESSAGE( "USING MS WINDOWS 32/64 CONFIG" )

   set( OpenCV_DIR "C:/OpenCV2.4/build" )
   set( CMAKE_PREFIX_PATH "C:/OpenCV2.4/build" )
   MESSAGE( "Assuming OpenCV in " ${OpenCV_DIR} " unless detected otherwise")

   MESSAGE( "Assuming libCURL in C:/libCURL or C:/Program Files/libCURL unless detected otherwise")
   set( CMAKE_LIBRARY_PATH "C:/libCURL;C:/Program Files/libCURL" )
   set ( CURL_INCLUDE_DIR "C:/libCURL/include" )  
   set ( CURL_LINK_DIR "C:/libCURL/lib" )
   include_directories(${CURL_INCLUDE_DIR})
   link_directories(${CURL_LINK_DIR})

   set ( CURL_INCLUDE_DIR "C:/Program Files/libCURL/include" )  
   set ( CURL_LINK_DIR "C:/Program Files/libCURL/lib" )
   include_directories(${CURL_INCLUDE_DIR})
   link_directories(${CURL_LINK_DIR})

   set ( CURL_LIBS "libCURL.lib" )	

ENDIF ( WIN32 )

find_package( OpenCV REQUIRED )
find_package( CURL REQUIRED )

project(rovio_example)
add_executable(rovio_example rovio_example.cpp rovio_cc_lib.cc)
target_link_libraries( rovio_example ${OpenCV_LIBS} ${CURL_LIBS})

project(testharness)
add_executable(testharness testharness.cc rovio_cc_lib.cc)
target_link_libraries( testharness ${OpenCV_LIBS} ${CURL_LIBS})
