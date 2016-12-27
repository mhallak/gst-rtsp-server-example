#include <gst/gst.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <stdio.h>

static GMainLoop *loop;
static int Image_Count = 0;
static void
cb_need_data (GstElement *appsrc,
          guint       unused_size,
          gpointer    user_data)
{
  static gboolean white = FALSE;
  static GstClockTime timestamp = 0;
  GstBuffer *buffer;
  guint size,depth,height,width,step,channels;
  GstFlowReturn ret;
  IplImage* img;
  guchar *data1;
  GstMapInfo map;
  char tmp[1024];

  //img=cvLoadImage("frame1.jpg",CV_LOAD_IMAGE_COLOR); 
  //img=cvLoadImage("/tmp/cam_sensor1-175.jpg",CV_LOAD_IMAGE_COLOR); 

  printf("cb_need_data has been called with Image_Count=%d\n",Image_Count);
  snprintf(tmp, sizeof(tmp), "/tmp/cam_sensor1-%02d.jpg",Image_Count);
  
  img=cvLoadImage(tmp,CV_LOAD_IMAGE_COLOR); 
  if (img == (IplImage *) NULL) size=1024;
  else {
    height    = img->height;  
    width     = img->width;
    step      = img->widthStep;
    channels  = img->nChannels;
    depth     = img->depth;
    data1      = (guchar *)img->imageData;
    size = height*width*channels;
  }

  buffer = gst_buffer_new_allocate (NULL, size, NULL);
  gst_buffer_map (buffer, &map, GST_MAP_WRITE);
  memcpy( (guchar *)map.data, data1,  gst_buffer_get_size( buffer ) );
  /* this makes the image black/white */
  //gst_buffer_memset (buffer, 0, white ? 0xff : 0x0, size);

  white = !white;

  GST_BUFFER_PTS (buffer) = timestamp;
  GST_BUFFER_DURATION (buffer) = gst_util_uint64_scale_int (1, GST_SECOND, 2);

  timestamp += GST_BUFFER_DURATION (buffer);

  g_signal_emit_by_name (appsrc, "push-buffer", buffer, &ret);

  if (ret != GST_FLOW_OK) {
    /* something wrong, stop pushing */
    g_main_loop_quit (loop);
  }
  Image_Count++;
  if (Image_Count > 299) Image_Count=0;
}

gint
main (gint   argc,
      gchar *argv[])
{
  GstElement *pipeline, *appsrc, *conv, *videosink;
  printf("Welcome to test-appsrc-jpg!!!\n");

  /* init GStreamer */
  gst_init (&argc, &argv);
  loop = g_main_loop_new (NULL, FALSE);
  printf("test-appsrc-jpg....after init!!!\n");

  /* setup pipeline */
  pipeline = gst_pipeline_new ("pipeline");
  appsrc = gst_element_factory_make ("appsrc", "source");
  conv = gst_element_factory_make ("videoconvert", "conv");
  videosink = gst_element_factory_make ("autovideosink", "videosink");
  printf("test-appsrc-jpg....after setting up the pipeline!!!\n");

  /* setup */
  g_object_set (G_OBJECT (appsrc), "caps",
        gst_caps_new_simple ("video/x-raw",
                     "format", G_TYPE_STRING, "RGB",
                     "width", G_TYPE_INT, 640,
                     "height", G_TYPE_INT, 360,
                     "framerate", GST_TYPE_FRACTION, 1, 1,
                     NULL), NULL);
  gst_bin_add_many (GST_BIN (pipeline), appsrc, conv, videosink, NULL);
  gst_element_link_many (appsrc, conv, videosink, NULL);

  //g_object_set (videosink, "device", "/dev/video0", NULL);
  /* setup appsrc */
  g_object_set (G_OBJECT (appsrc),
        "stream-type", 0,
        "format", GST_FORMAT_TIME, NULL);
  printf("test-appsrc-jpg....after adding many!!!\n");

  g_signal_connect (appsrc, "need-data", G_CALLBACK (cb_need_data), NULL);
    printf("test-appsrc-jpg....after adding callback!!!\n");

  /* play */
  gst_element_set_state (pipeline, GST_STATE_PLAYING);
  g_main_loop_run (loop);

  /* clean up */
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (GST_OBJECT (pipeline));
  g_main_loop_unref (loop);

  return 0;
  }