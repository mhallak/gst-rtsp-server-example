#include <gst/gst.h>
#include <gst/app/gstappsrc.h>

static GMainLoop *loop;

    static void
cb_need_data (GstAppSrc *appsrc,
        guint       unused_size,
        gpointer    user_data)
{
    g_print("In %s\n", __func__);
    static gboolean white = FALSE;
    static GstClockTime timestamp = 0;
    GstBuffer *buffer;
    guint size;
    GstFlowReturn ret;

    size = 385 * 288 * 2;

    buffer = gst_buffer_new_allocate (NULL, size, NULL);

    /* this makes the image black/white */
    gst_buffer_memset (buffer, 0, white ? 0xff : 0x0, size);

    white = !white;

    GST_BUFFER_PTS (buffer) = timestamp;
    GST_BUFFER_DURATION (buffer) = gst_util_uint64_scale_int (1, GST_SECOND, 2);

    timestamp += GST_BUFFER_DURATION (buffer);

    //g_signal_emit_by_name (appsrc, "push-buffer", buffer, &ret);
    //ret = gst_app_src_push_buffer(appsrc, buffer);
    g_signal_emit_by_name (appsrc, "push-buffer", buffer, &ret);

    if (ret != GST_FLOW_OK) {
        /* something wrong, stop pushing */
        g_main_loop_quit (loop);
    }
}

static void cb_enough_data(GstAppSrc *src, gpointer user_data)
{
    g_print("In %s\n", __func__);
}

static gboolean cb_seek_data(GstAppSrc *src, guint64 offset, gpointer user_data)
{
    g_print("In %s\n", __func__);
    return TRUE;
}

    gint
main (gint   argc,
        gchar *argv[])
{
    GstElement *pipeline, *appsrc, *conv, *videosink, *payloader, *udpsink, *videoenc;

    /* init GStreamer */
    gst_init (&argc, &argv);
    loop = g_main_loop_new (NULL, FALSE);

    /* setup pipeline */
    pipeline = gst_pipeline_new ("pipeline");
    appsrc = gst_element_factory_make ("appsrc", "source");
    conv = gst_element_factory_make ("videoconvert", "conv");
    videoenc = gst_element_factory_make("avenc_mpeg4", "ffenc_mpeg4");
    videosink = gst_element_factory_make ("xvimagesink", "videosink");
    payloader = gst_element_factory_make("rtpmp4vpay", "rtpmp4vpay");
    g_object_set(G_OBJECT(payloader),
            "config-interval", 60,
            NULL);
    udpsink = gst_element_factory_make("udpsink", "udpsink");
    g_object_set(G_OBJECT(udpsink),
            "host", "127.0.0.1",
            "port", 5000,
            NULL);
    GstElement *freeze = gst_element_factory_make("imagefreeze", "imagefreeze0");
    GstElement *colorspace = gst_element_factory_make("ffmpegcolorspace", "colorspace");

    /* setup */
    g_object_set (G_OBJECT (appsrc), "caps",
            gst_caps_new_simple ("video/x-raw",
                "format", G_TYPE_STRING, "RGB16",
                "width", G_TYPE_INT, 384,
                "height", G_TYPE_INT, 288,
                "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
                "framerate", GST_TYPE_FRACTION, 0, 1,
                NULL), NULL);

#if 0
    // THIS WORKS!
    gst_bin_add_many (GST_BIN (pipeline), appsrc, conv, videosink, NULL);
    gst_element_link_many (appsrc, conv, videosink, NULL);
#elif 0
    // WOKRS!
    gst_bin_add_many (GST_BIN (pipeline), appsrc, conv, freeze, videosink, NULL);
    gst_element_link_many (appsrc, conv, freeze, videosink, NULL);
#else
    // WORKS!
    gst_bin_add_many (GST_BIN (pipeline), appsrc, conv, videoenc, payloader, udpsink, NULL);
    gst_element_link_many (appsrc, conv, videoenc, payloader, udpsink, NULL);
#endif

    /* setup appsrc */
    g_object_set (G_OBJECT (appsrc),
            "stream-type", 0,
            "is-live", TRUE,
            "format", GST_FORMAT_TIME, NULL);
    GstAppSrcCallbacks cbs;
    cbs.need_data = cb_need_data;
    cbs.enough_data = cb_enough_data;
    cbs.seek_data = cb_seek_data;
    g_signal_connect (appsrc, "need-data", G_CALLBACK (cb_need_data), NULL);
    //gst_app_src_set_callbacks(GST_APP_SRC_CAST(appsrc), &cbs, NULL, NULL);

    /* play */
    gst_element_set_state (pipeline, GST_STATE_PLAYING);
    g_main_loop_run (loop);

    /* clean up */
    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref (GST_OBJECT (pipeline));
    g_main_loop_unref (loop);

    return 0;
}