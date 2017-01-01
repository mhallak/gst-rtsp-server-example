# gst-rtsp-server-example
The goal is to stream video of a camera in gazebo simulation system

Install:

1. Download gstreamer rtsp server: I used the following:
      From https://gstreamer.freedesktop.org/src/gst-rtsp-server/
      gst-rtsp-server-1.4.5.tar.xz
2. Gstreamer: install version 1.4
      From https://gstreamer.freedesktop.org/src/gstreamer
      gstreamer-1.4.0.tar.xz
3. Gstreamer plugins: install plugins base-good-bad and ugly:
      From https://gstreamer.freedesktop.org/src/gst-plugins-[base, bad, good, ugly]
      gst-plugins-[base/bad/good/ugly]-1.4.5.tar.xz
4. Install Opencv-2 from http://docs.opencv.org/trunk/d7/d9f/tutorial_linux_install.html
   Check as follows:
   
   michele@apollo:~$ dpkg -s libopencv
   
libopencv2.4-java         libopencv-features2d-dev  libopencv-legacy-dev      libopencv-stitching-dev
libopencv2.4-jni          libopencv-flann2.4        libopencv-ml2.4           libopencv-superres2.4
libopencv-calib3d2.4      libopencv-flann-dev       libopencv-ml-dev          libopencv-superres-dev
libopencv-calib3d-dev     libopencv-gpu2.4          libopencv-objdetect2.4    libopencv-ts2.4
libopencv-contrib2.4      libopencv-gpu-dev         libopencv-objdetect-dev   libopencv-ts-dev
libopencv-contrib-dev     libopencv-highgui2.4      libopencv-ocl2.4          libopencv-video2.4
libopencv-core2.4         libopencv-highgui-dev     libopencv-ocl-dev         libopencv-video-dev
libopencv-core-dev        libopencv-imgproc2.4      libopencv-photo2.4        libopencv-videostab2.4
libopencv-dev             libopencv-imgproc-dev     libopencv-photo-dev       libopencv-videostab-dev
libopencv-features2d2.4   libopencv-legacy2.4       libopencv-stitching2.4    
   
   Note that the include files are in ls /usr/include/opencv and the shared libraries are installed in /usr/lib/x86_64-linux-gnu/libopencv_*
  
5. clone the repository and put it in ~/gst-rtsp-server-1.4.5/examples
6. build: 
      ~/gst-rtsp-server-1.4.5/configure
      ~/gst-rtsp-server-1.4.5/make

Run:

1. Copy tmp_jpg to /tmp
2. The relevant examples are:

              test-appsrc.c      ==> run 
              test-appsrc-jpg.c  ==> run
                       change the line 61:
                            if (Image_Count > 299) Image_Count=0; 
                        with:
                            if (Image_Count > 10) Image_Count=0;)
3. gazebo-streamer.c is a mixed of the previous example and crashes (:+(
              
