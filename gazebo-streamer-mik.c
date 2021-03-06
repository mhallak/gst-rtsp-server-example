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
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
static GMainLoop *loop;
static int Image_Count = 0;
typedef struct
{
  gboolean white;
  GstClockTime timestamp;
} MyContext;

/* called when we need to give data to appsrc */
static void
need_data (GstElement * appsrc, guint unused, MyContext * ctx)
{
  // static gboolean white = FALSE;
  //static GstClockTime timestamp = 0;
 
  volatile GstBuffer *buffer, *new_buffer;
  volatile guint size,depth,height,width,step,channels;
  GstFlowReturn ret;
  volatile IplImage* img;
  volatile guchar *data1;
  GstMapInfo map;
  char tmp[1024];
  volatile gsize bufsize;

  volatile guint8 *inbuf_u = 0;

  //img=cvLoadImage("frame1.jpg",CV_LOAD_IMAGE_COLOR); 
  //img=cvLoadImage("/tmp/cam_sensor1-175.jpg",CV_LOAD_IMAGE_COLOR); 



  snprintf(tmp, sizeof(tmp), "/tmp/cam_sensor1-%02d.jpg",Image_Count);
  g_print("file=[%s], need_data has been called with Image_Count=%d\n", tmp, Image_Count);

  Image_Count++;
  if (Image_Count > 10) Image_Count=0;

  
  img=cvLoadImage(tmp,CV_LOAD_IMAGE_COLOR); 
  if (img == (IplImage *) NULL) size=384 * 288 * 2;
  else {
    height    = img->height;  
    width     = img->width;
    step      = img->widthStep;
    channels  = img->nChannels;
    depth     = img->depth;
    data1      = (guchar *)img->imageData;
   // size = height*width*channels;
    size      = img->imageSize;
  }

#if 0
  buffer = gst_buffer_new_allocate (NULL, size, NULL);
  g_print("allocated size %d\n", size);
  gst_buffer_map (buffer, &map, GST_MAP_WRITE);
  bufsize = gst_buffer_get_size( buffer );
  memcpy( (guchar *)map.data, data1,  gst_buffer_get_size( buffer ) );
  /* this makes the image black/white */
  //7ngst_buffer_memset (buffer, 0, ctx->white ? 0xff : 0x0, size);
  //white = ctx->white;
  //timestamp = ctx->timestamp;
  inbuf_u = (guint8 *)malloc(size);
  memcpy(inbuf_u, data1, size);
#endif

#if 1
//  if  (!inbuf_u)
	  inbuf_u = (guint8 *)malloc(size);

  if (!inbuf_u)
	  for (;;)
		  printf("ERROR: malloc failed \n");

  memcpy(inbuf_u, data1, size);
#endif

  new_buffer=gst_buffer_new_wrapped(inbuf_u, size);


  
  ctx->white = !ctx->white;


  GST_BUFFER_PTS (new_buffer) = ctx->timestamp;
  //Mik GST_BUFFER_DURATION (buffer) = gst_util_uint64_scale_int (1, GST_SECOND, 2);
  /*Mik*/ ctx->timestamp += 25 * 1000000;
  //Mik ctx->timestamp += GST_BUFFER_DURATION (buffer);

  g_signal_emit_by_name (appsrc, "push-buffer", new_buffer, &ret);

  if (ret != GST_FLOW_OK) {
    /* something wrong, stop pushing */
    g_main_loop_quit (loop);
  }
  
}

/* called when a new media pipeline is constructed. We can query the
 * pipeline and configure our appsrc */
static void
media_configure (GstRTSPMediaFactory * factory, GstRTSPMedia * media,
    gpointer user_data)
{
  GstElement *element, *appsrc;
  MyContext *ctx;
  //GstElement *element, *pipeline, *appsrc, *conv, *videosink;
  g_print("media_configure has been called\n");

  /* get the element used for providing the streams of the media */
  element = gst_rtsp_media_get_element (media);

  /* get our appsrc, we named it 'mysrc' with the name property */
  appsrc = gst_bin_get_by_name_recurse_up (GST_BIN (element), "mysrc");

  /* this instructs appsrc that we will be dealing with timed buffer */
  gst_util_set_object_arg (G_OBJECT (appsrc), "format", "time");

  /* configure the caps of the video */
     g_object_set (G_OBJECT (appsrc), "caps",
        gst_caps_new_simple ("video/x-raw",
                     "format", G_TYPE_STRING, "RGB",
                     "width", G_TYPE_INT, 640,
                     "height", G_TYPE_INT, 480,
                     "framerate", GST_TYPE_FRACTION, 0, 1,
                     NULL), NULL);
                     /***/
  ctx = g_new0 (MyContext, 1);
  ctx->white = FALSE;
  ctx->timestamp = 0;
   /* setup appsrc 
  g_object_set (G_OBJECT (appsrc),
        "stream-type", 0,
        "format", GST_FORMAT_TIME, NULL);*/
  printf("gazebo-streamer....!!!\n");
  
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
		  "( appsrc name=mysrc ! videoconvert ! x264enc ! rtph264pay name=pay0 pt=96 )");


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


