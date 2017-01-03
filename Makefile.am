noinst_PROGRAMS = test-video test-ogg test-mp4 test-readme \
		  test-launch test-sdp test-uri test-auth \
		  test-multicast test-multicast2 test-appsrc \
		  test-appsrc-jpg gazebo-streamer

#INCLUDES = -I$(top_srcdir) -I$(srcdir)

AM_CFLAGS = -O0 -g $(GST_OBJ_CFLAGS)
OPENCV_LIBS_LOC = /usr/lib/x86_64-linux-gnu/libopencv_highgui.so.2.4
#OPENCV_LIBS = -l$(OPENCV_LIBS_LOC)/libopencv-highgui2.4 
LDADD = $(GST_OBJ_LIBS) $(GIO_LIBS) $(OPENCV_LIBS_LOC)

if HAVE_LIBCGROUP
noinst_PROGRAMS += test-cgroups
LDADD += $(LIBCGROUP_LIBS)
endif

