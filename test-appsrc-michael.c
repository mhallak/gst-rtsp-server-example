
/* GStreamer
 * Copyright (C) 2008 Wim Taymans <wim.taymans at gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <gst/gst.h>
//#include <gst/app/gstappsrc.h>


#include <gst/rtsp-server/rtsp-server.h>

typedef struct
{
  gboolean white;
  GstClockTime timestamp;
} MyContext;

/* called when we need to give data to appsrc */
static void
need_data (GstElement * appsrc, guint unused, MyContext * ctx)
{
  GstBuffer *gst_buffer;
//  guint size;
  GstFlowReturn ret;
  int num;
int even;
GstMemory *memory;
 GstMapInfo info;
 int i;
 guint8 *inbuf_u;
 int file_size_u;
 FILE *fd_u;

 static int nd_counter = 0;
 static FILE *fd = 0;
 static int file_size;
 static guint8 *inbuf;

 static FILE *fd2 = 0;
 static int file_size2;
 static guint8 *inbuf2;

printf("[%s]: nd_counter=[%d]\n", __func__, nd_counter++);


	if (!fd)
	{
		fd = fopen("/tmp/cam_sensor1-286.jpg", "rb");
		if (!fd)
		{
			printf("ERROR: can't open file fd=%d\n", fd);
			return;
		}

		fseek(fd,0,SEEK_END);
		file_size = ftell(fd);
		fseek(fd,0,SEEK_SET);
		printf("file size=[%d]\n", file_size);

	    inbuf = (void *) malloc(file_size);

	}

	if (!fd2)
	{
		fd2 = fopen("/tmp/cam_sensor1-06.jpg", "rb");
		if (!fd2)
		{
			printf("ERROR: can't open file\n");
			return;
		}

		fseek(fd2,0,SEEK_END);
		file_size2 = ftell(fd2);
		fseek(fd2,0,SEEK_SET);
		printf("file size=[%d]\n", file_size2);

	    inbuf2 = (void *) malloc(file_size2);

	}




  if (nd_counter < 20)
  {
	  fd_u = fd;
	  inbuf_u = inbuf;
	  file_size_u = file_size;
  }
  else
  {
	  fd_u = fd2;
	  inbuf_u = inbuf2;
	  file_size_u = file_size2;
  }

  if (nd_counter > 40)
	  nd_counter = 0;


  fseek(fd_u,0,SEEK_SET);
  num = fread(inbuf_u, 1, file_size_u, fd_u);
  if (num != file_size_u)
	  printf("num=[%d]\n", num);




#if 0

  /* make empty buffer */
  	  gst_buffer = gst_buffer_new ();
  	  memory = gst_allocator_alloc (NULL, file_size, NULL);

  	  /* get access to the memory in write mode */
  	  gst_memory_map (memory, &info, GST_MAP_WRITE);

  	 /* add the buffer */
  	  gst_buffer_append_memory (gst_buffer, memory);


  	  for(i=0;i<info.size; i++)
      {
          /// info.data[i]= inbuf[i];
      }



  	  //  gst_buffer = gst_buffer_new_wrapped(inbuf, file_size);

#endif

#if 1

//  	  gst_buffer = gst_buffer_new_allocate(NULL,file_size,NULL);
//  	  gst_buffer_fill(gst_buffer, 0, inbuf, file_size);
  	  gst_buffer = gst_buffer_new_wrapped(inbuf_u, file_size_u);


#endif




#if 0
  gst_buffer = gst_buffer_new_allocate(NULL,file_size,NULL);
  gst_buffer_fill(gst_buffer, 0, inbuf, file_size);
  gst_app_src_push_buffer(appsrc, gst_buffer);
#endif

#if 0
  gst_buffer = gst_buffer_new();
  memory = gst_allocator_alloc (NULL, file_size, NULL);
    gst_buffer_insert_memory (gst_buffer, -1, memory);
     gst_memory_map (memory, &info, GST_MAP_WRITE);

    for(i=0;i<info.size; i++)
    {
        info.data[i]= inbuf[i];
    }

 ret = gst_app_src_push_buffer((GstAppSrc *) 													appsrc, gst_buffer);
#endif




  /* this makes the image black/white */
//  gst_buffer_memset (buffer, 0, ctx->white ? 0xff : 0x0, size);

  ctx->white = !ctx->white;

  /* increment the timestamp every 1/2 second */
  GST_BUFFER_PTS (gst_buffer) = ctx->timestamp;
//  GST_BUFFER_DURATION (gst_buffer) = gst_util_uint64_scale_int (1, GST_SECOND, 2);
 ctx->timestamp += 25 * 1000000;
 //ctx->timestamp += GST_BUFFER_DURATION (gst_buffer);

  g_signal_emit_by_name (appsrc, "push-buffer", gst_buffer, &ret);
}

/* called when a new media pipeline is constructed. We can query the
 * pipeline and configure our appsrc */
static void
media_configure (GstRTSPMediaFactory * factory, GstRTSPMedia * media,
    gpointer user_data)
{
  GstElement *element, *appsrc;
  MyContext *ctx;

  /* get the element used for providing the streams of the media */
  element = gst_rtsp_media_get_element (media);

  /* get our appsrc, we named it 'mysrc' with the name property */
  appsrc = gst_bin_get_by_name_recurse_up (GST_BIN (element), "mysrc");

  /* this instructs appsrc that we will be dealing with timed buffer */
  gst_util_set_object_arg (G_OBJECT (appsrc), "format", "time");
  /* configure the caps of the video */
  g_object_set (G_OBJECT (appsrc), "caps",
      gst_caps_new_simple (
    		  //      		"video/x-jpeg", // "video/x-raw",
//    		  "video/x-jpeg",
    		  "image/jpeg",
    		  //          "format", G_TYPE_STRING, "RGB16",
    	   "format", G_TYPE_STRING, "RGB",
          "width", G_TYPE_INT, 1024,// 384,
          "height", G_TYPE_INT, 768, // 288,
          "framerate", GST_TYPE_FRACTION, 0, 1,
          NULL),
      NULL);
//       	 "framerate", GST_TYPE_FRACTION, 1, 1, NULL),

  ctx = g_new0 (MyContext, 1);
  ctx->white = FALSE;
  ctx->timestamp = 0;
  /* make sure ther datais freed when the media is gone */
  g_object_set_data_full (G_OBJECT (media), "my-extra-data", ctx,
      (GDestroyNotify) g_free);

  /* install the callback that will be called when a buffer is needed */
  g_signal_connect (appsrc, "need-data", (GCallback) need_data, ctx);
}

int
main (int argc, char *argv[])
{
  GMainLoop *loop;
  GstRTSPServer *server;
  GstRTSPMountPoints *mounts;
  GstRTSPMediaFactory *factory;

  gst_init (&argc, &argv);

  loop = g_main_loop_new (NULL, FALSE);

  /* create a server instance */
  server = gst_rtsp_server_new ();

  /* get the mount points for this server, every server has a default object
   * that be used to map uri mount points to media factories */
  mounts = gst_rtsp_server_get_mount_points (server);

  /* make a media factory for a test stream. The default media factory can use
   * gst-launch syntax to create pipelines.
   * any launch line works as long as it contains elements named pay%d. Each
   * element with pay%d names will be a stream */
  factory = gst_rtsp_media_factory_new ();
  gst_rtsp_media_factory_set_launch (factory,
//	      "( appsrc name=mysrc ! videoconvert ! x264enc ! rtph264pay name=pay0 pt=96 )");
//		  "( appsrc name=mysrc ! decodebin ! videoconvert ! x264enc ! rtph264pay name=pay0 pt=96 )");
//		  "( appsrc name=mysrc ! image/jpeg, mapping=/stream1 ! rtph264pay name=pay0 pt=96 )");
//	      "( appsrc name=mysrc ! decodebin ! videoconvert ! jpegenc ! rtph264pay name=pay0 pt=96 )");
//		  "( appsrc name=mysrc ! decodebin ! videoconvert ! x264enc ! rtph264pay name=pay0 pt=96 )");
//		  "( appsrc name=mysrc ! avimux ! avidemux ! rtpjpegpay name=pay0 pt=96 )");
//		  "( appsrc name=mysrc ! rtpjpegpay name=pay0 pt=96 )");
		  "( appsrc name=mysrc ! decodebin ! videoconvert ! x264enc ! rtph264pay name=pay0 pt=96 )");


  /* notify when our media is ready, This is called whenever someone asks for
   * the media and a new pipeline with our appsrc is created */
  g_signal_connect (factory, "media-configure", (GCallback) media_configure,
      NULL);

  /* attach the test factory to the /test url */
  gst_rtsp_mount_points_add_factory (mounts, "/test", factory);

  /* don't need the ref to the mounts anymore */
  g_object_unref (mounts);

  /* attach the server to the default maincontext */
  gst_rtsp_server_attach (server, NULL);

  /* start serving */
  g_print ("stream ready at rtsp://127.0.0.1:8554/test\n");
  g_main_loop_run (loop);

  return 0;
}