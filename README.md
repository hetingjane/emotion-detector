# Sample Apps for Affdex SDK for Windows and Linux (contains server and client)

Welcome to our repository on GitHub! Here you will find example code to get you started with our Affdex Linux SDK 4.0.0, Affdex Windows SDK 3.4 and begin emotion-enabling you own app! Documentation for the SDKs is available on the <a href=http://developer.affectiva.com/>Affectiva's Developer Portal</a>.

Build Status
-------------
- Windows: [![Build status](https://ci.appveyor.com/api/projects/status/github/Affectiva/cpp-sdk-samples?branch=master&svg=true)](https://ci.appveyor.com/project/ahamino/win-sdk-samples)
- Ubuntu: [![Build Status](https://travis-ci.org/Affectiva/cpp-sdk-samples.svg?branch=master)](https://travis-ci.org/Affectiva/cpp-sdk-samples)

Dependencies
------------

##### Windows
- Affdex SDK 3.4 (64 bit)
- Visual Studio 2013 or higher

##### Linux
- Ubuntu 16.04 with GCC v5.4.1
- CentOS 7 with GCC v4.8.x
- Affdex SDK 4.0.0
- CMake 3.5 or higher

##### Additional dependencies

- OpenCV 2.4
- Boost 1.63
- libuuid
- libcurl
- libopenssl

Installation
------------

- Download Affdex SDK for Linux [from here](https://affectiva.readme.io/docs/getting-started-with-the-affectiva-sdk-for-linux#section-1-download-and-extract-the-sdk-archive)
- Download Affdex SDK for Windows [from here](https://affectiva.readme.io/docs/getting-started-with-the-emotion-sdk-for-windows#section-1-download-and-run-the-sdk-installer)


##### Windows
- Install the SDK using MSI installer.
- The additional dependencies get installed automatically by NuGet.

##### Linux

*Installation Guide for dependencies CMake v3.8.1 and Boost v1.63 for CentOS 7 and Ubuntu 16.04*

- Boost

```
$ wget https://sourceforge.net/projects/boost/files/boost/1.63.0/boost_1_63_0.tar.gz
$ tar -xzvf boost_1_63_0.tar.gz -C $HOME
$ cd boost_1_63_0
$ ./bootstrap.sh
$ sudo ./b2 -j $(nproc) cxxflags=-fPIC threading=multi runtime-link=shared \
      --with-log --with-serialization --with-system --with-date_time \
      --with-filesystem --with-regex --with-timer --with-chrono --with-thread \
      --with-program_options install
```

- CMAKE

```
$ wget https://cmake.org/files/v3.8/cmake-3.8.1.tar.gz \
$ tar -xvf cmake-3.8.1.tar.gz && rm cmake-3.8.1.tar.gz
$ cd $SRC_DIR/cmake-3.8.1/
$ ./bootstrap --system-curl && \
    make -j$(nproc) && \
    make install && \
    rm -rf $SRC_DIR/cmake-3.8.1

```

- Building the SDK on Ubuntu 16.04

```bashrc
$ sudo apt-get install build-essential libopencv-dev libcurl4-openssl uuid-dev
$ wget https://download.affectiva.com/linux/gcc-5.4/affdex-cpp-sdk-4.0-75-ubuntu-xenial-xerus-x86_64bit.tar.gz
$ mkdir $HOME/affdex-sdk
$ tar -xzvf affdex-cpp-sdk-4.0-75-ubuntu-xenial-xerus-x86_64bit.tar.gz -C $HOME/affdex-sdk
$ export AFFDEX_DATA_DIR=$HOME/affdex-sdk/data
$ git clone https://github.com/Affectiva/cpp-sdk-samples.git $HOME/sdk-samples
$ mkdir $HOME/build
$ cd $HOME/build
$ cmake -DOpenCV_DIR=/usr/ -DBOOST_ROOT=/usr/ -DAFFDEX_DIR=$HOME/affdex-sdk $HOME/sdk-samples
$ make
$ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$HOME/affdex-sdk/lib

# The SDK statically links a forked version of OpenCV, so if you run into double free or corruption error
# then you will need to preload the OpenCV library installed from package manager
# Use this command to find the path of libopencv_core.so.2.4

$ ldconfig -p | grep libopencv_core.so.2.4
$ export LD_PRELOAD=/path/to/libopencv_core.so.2.4

```

- Building the SDK on CentOS 7

```bashrc
$ sudo yum install libcurl-devel.x86_64 libuuid-devel.x86_64 opencv-devel
$ wget https://download.affectiva.com/linux/centos-4.8/affdex-cpp-sdk-4.0-2941-centos-7-x86_64bit.tar.gz
$ mkdir $HOME/affdex-sdk
$ tar -xzvf affdex-cpp-sdk-4.0-2941-centos-7-x86_64bit.tar.gz -C $HOME/affdex-sdk
$ export AFFDEX_DATA_DIR=$HOME/affdex-sdk/data
$ git clone https://github.com/Affectiva/cpp-sdk-samples.git $HOME/sdk-samples
$ mkdir $HOME/build
$ cd $HOME/build
$ cmake -DOpenCV_DIR=/usr/ -DBOOST_ROOT=/usr/ -DAFFDEX_DIR=$HOME/affdex-sdk $HOME/sdk-samples
$ make
$ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$HOME/affdex-sdk/lib

# The SDK statically links a forked version of OpenCV, so if you run into double free or corruption error
# then you will need to preload the OpenCV library installed from package manager
# Use this command to find the path of libopencv_core.so.2.4

$ ldconfig -p | grep libopencv_core.so.2.4
$ export LD_PRELOAD=/path/to/libopencv_core.so.2.4
```

OpenCV-webcam-demo (c++)
------------------

Project for demoing the [FrameDetector class](http://developer.affectiva.com/v3_2/cpp/analyze-frames/). It grabs frames from the camera, analyzes them and displays the results on screen.

The following command line arguments can be used to run it:

    -h [ --help ]                        Display this help message.
    -d [ --data ] arg (=data)            Path to the data folder
    -r [ --resolution ] arg (=640 480)   Resolution in pixels (2-values): width
                                         height
    --pfps arg (=30)                     Processing framerate.
    --cfps arg (=30)                     Camera capture framerate.
    --bufferLen arg (=30)                process buffer size.
    --cid arg (=0)                       Camera ID.
    --faceMode arg (=0)                  Face detector mode (large faces vs small
                                        faces).
    --numFaces arg (=1)                  Number of faces to be tracked.
    --draw arg (=1)                      Draw metrics on screen.

Video-demo (c++)
----------

Project for demoing the Windows SDK [VideoDetector class](http://developer.affectiva.com/v3_2/cpp/analyze-video/) and [PhotoDetector class](http://developer.affectiva.com/v3_2/cpp/analyze-photo/). It process video or image files, displays the emotion metrics and exports the results in a csv file.

The following command line arguments can be used to run it:

    -h [ --help ]                        Display this help message.
    -d [ --data ] arg (=data)            Path to the data folder
    -i [ --input ] arg                   Video or photo file to process.
    --pfps arg (=30)                     Processing framerate.
    --draw arg (=1)                      Draw video on screen.
    --faceMode arg (=1)                  Face detector mode (large faces vs small
                                         faces).
    --numFaces arg (=1)                  Number of faces to be tracked.
    --loop arg (=0)                      Loop over the video being processed.


For an example of how to use Affdex in a C# application .. please refer to [AffdexMe](https://github.com/affectiva/affdexme-win)

Docker Build Instructions
-------------------------

The Dockerfile's are located in the [docker](docker) directory. To build the docker image please refer to the files for instructions.
