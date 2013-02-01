cmake_minimum_required (VERSION 2.6)
set( CMAKE_CXX_FLAGS "-O3 -Wall ${CMAKE_CXX_FLAGS}" )

# SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "CMakeCache.txt;CMakeFiles;cmake_install.cmake")

# linux specific stuff

IF ( UNIX )
   set( CMAKE_PREFIX_PATH "/opt/opencv-2.4.2" )
   set( CMAKE_FIND_LIBRARY_PREFIXES "/usr" )
   set( CURL_INCLUDE_DIR "/usr/include/curl" )
   set( CMAKE_CXX_FLAGS "-lpthread -lcurl")
   MESSAGE( "LINUX CONFIG" )
ENDIF ( UNIX )

# windows (inc. 64-bit specific stuff)

IF ( WIN32 )
   set( CMAKE_PREFIX_PATH "C:/OpenCV2.4/build" )
   set( OpenCV_DIR "C:/OpenCV2.4/build" )
   set( CMAKE_FIND_LIBRARY_PREFIXES "C:/libCURL" )
   set ( CURL_INCLUDE_DIR "C:/libCURL/include" )  
   set ( CURL_LIBS "libCURL.lib" )	
   MESSAGE( "WINDOWS CONFIG" )

ENDIF ( WIN32 )

find_package( OpenCV REQUIRED )
find_package( CURL REQUIRED )

project(rovio_example)
add_executable(rovio_example rovio_example.cpp rovio_cc_lib.cc)
target_link_libraries( rovio_example ${OpenCV_LIBS} ${CURL_LIBS})

project(testharness)
add_executable(testharness testharness.cc rovio_cc_lib.cc)
target_link_libraries( testharness ${OpenCV_LIBS} ${CURL_LIBS})
