#include <gst/gst.h>
//#include <stdlib.h>

typedef struct _player {
  GMainLoop *loop;

  GstBus *bus;

  GstElement *pipeline;
  GstElement *source;
  GstElement *decoder;
  GstElement *audioconv;
  GstElement *resampler;
  GstElement *sink;
} Player;

static void sigint_handler(gpointer data)
{
  Player *player = data;
  g_print("\b\bTi\nGood bye!\n");
  g_main_loop_quit(player->loop);
}

static gboolean bus_watcher(GstBus *bus, GstMessage *message, gpointer data)
{
  Player *player = data;
  switch(GST_MESSAGE_TYPE(message))
  {
    case GST_MESSAGE_ERROR:
    {
      GError *err;
      gchar *debug;
      gst_message_parse_error (message, &err, &debug);
      g_print ("Error: %s\n", err->message);
      g_error_free (err);
      g_free (debug);
      g_print("\b\bTi\nGood bye!\n");
      g_main_loop_quit (player->loop);
      break;
    }
    case GST_MESSAGE_EOS:
    {
      /* end-of-stream */
      g_print("\b\bTi\nGood bye!\n");
      g_main_loop_quit (player->loop);
      break;
    }
    default:
      /* unhandled message */
      break;
  }
  return TRUE;
}

static gboolean cb_print_position(GstElement *pipeline)
{
  gint64 pos, len;

  if (gst_element_query_position (pipeline, GST_FORMAT_TIME, &pos)
      && gst_element_query_duration (pipeline, GST_FORMAT_TIME, &len)) {
    g_print ("Time: %" GST_TIME_FORMAT "\b\b\b\b\b\b\b\b / %" GST_TIME_FORMAT "\b\b\b\b\b\b\b\b         \r", GST_TIME_ARGS (pos), GST_TIME_ARGS (len));
  }
  /* call me again */
  return TRUE;
}

static void print_one_tag_line(const gchar *tag, const GValue *val)
{
  if (G_VALUE_HOLDS_STRING (val)) {
    g_print ("\t%10s : %s\n", tag, g_value_get_string (val));
  } else if (G_VALUE_HOLDS_UINT (val)) {
    g_print ("\t%10s : %u\n", tag, g_value_get_uint (val));
  } else if (G_VALUE_HOLDS_DOUBLE (val)) {
    g_print ("\t%10s : %g\n", tag, g_value_get_double (val));
  } else if (G_VALUE_HOLDS_BOOLEAN (val)) {
    g_print ("\t%10s : %s\n", tag,
        (g_value_get_boolean (val)) ? "true" : "false");
  } else if (GST_VALUE_HOLDS_BUFFER (val)) {
    GstBuffer *buf = gst_value_get_buffer (val);
    guint buffer_size = gst_buffer_get_size (buf);
    g_print ("\t%10s : buffer of size %u\n", tag, buffer_size);
  } else if (GST_VALUE_HOLDS_DATE_TIME (val)) {
    GstDateTime *dt = g_value_get_boxed (val);
    gchar *dt_str = gst_date_time_to_iso8601_string (dt);
    g_print ("\t%10s : %s\n", tag, dt_str);
    g_free (dt_str);
  } else {
    g_print ("\t%10s : tag of type ’%s’\n", tag, G_VALUE_TYPE_NAME (val));
  }
}

static void print_one_tag (const GstTagList *list, const gchar *tag, gpointer user_data)
{
  int i, num;
  num = gst_tag_list_get_tag_size (list, tag);
  for (i = 0; i < num; ++i) {
    const GValue *val;
    /* Note: when looking for specific tags, use the gst_tag_list_get_xyz() API,
     * * we only use the GValue approach here because it is more generic */
    val = gst_tag_list_get_value_index (list, tag, i);
    if(g_strrstr(tag, "artist"))
      print_one_tag_line("Artist", val);
    else if(g_strrstr(tag, "title"))
      print_one_tag_line("Song", val);
    else if(g_strrstr(tag, "genre"))
      print_one_tag_line("Genre", val);
    else if(g_strrstr(tag, "datetime"))
      print_one_tag_line("Year", val);
    else if(g_strrstr(tag, "album"))
      print_one_tag_line("Album", val);
  }
}

static void cb_newpad(GstElement *decoder, GstPad *pad, gpointer data)
{
  Player *player = data;

  GstCaps *caps;
  GstStructure *str;
  GstPad *audiopad;

  audiopad = gst_element_get_static_pad(player->audioconv, "sink");
  if (GST_PAD_IS_LINKED (audiopad)) {
    g_object_unref (audiopad);
    return;
  }

  caps = gst_pad_query_caps(pad, NULL);
  str = gst_caps_get_structure(caps, 0);
  gst_caps_unref(caps);
  if (!g_strrstr(gst_structure_get_name(str), "audio"))
  {
    gst_object_unref(audiopad);
    return;
  }

  gst_pad_link(pad, audiopad);

  g_object_unref(audiopad);
}

static gboolean player_free(Player *player)
{
  gst_element_set_state(player->pipeline, GST_STATE_NULL);
  gst_object_unref(player->pipeline);
  g_free(player);
}

gint main(gint argc, gchar *argv[])
{
  if(argc<2)
  {
    g_print("\tUsage: %s <path to audiofile>\n", argv[0]);
    return 1;
  }

  Player *player = g_new(Player, 1);

  guint bus_watch_id;

  GstMessage *msg;
  GstStateChangeReturn ret;

  gst_init(&argc, &argv);
  player->loop = g_main_loop_new(NULL, FALSE);

  g_unix_signal_add(SIGINT, sigint_handler, player);

  player->source = gst_element_factory_make("filesrc", "filesrc0");
  g_object_set(G_OBJECT(player->source), "location", argv[1], NULL);
  player->decoder = gst_element_factory_make("decodebin", "decoder0");
  g_signal_connect(player->decoder, "pad-added", G_CALLBACK(cb_newpad), player);
  player->audioconv = gst_element_factory_make("audioconvert", "audioconv0");
  player->resampler = gst_element_factory_make("audioresample", "resampler0");
  player->sink = gst_element_factory_make("autoaudiosink", "sink0");
  player->pipeline = gst_pipeline_new("player-pipeline");

  if (!player->pipeline || !player->source || !player->decoder || 
      !player->audioconv || !player->resampler || !player->sink)
  {
    g_printerr("Not all elements could be created.\n");
    return -1;
  }

  gst_bin_add_many (GST_BIN(player->pipeline), player->source, player->decoder, 
      player->audioconv, player->resampler, player->sink, NULL);

  if (!gst_element_link_many(player->source, player->decoder, NULL))
  {
    g_printerr("Source and Decoder could not be linked.\n");
    gst_object_unref(player->pipeline);
    g_free(player);
    return -1;
  }

  if (!gst_element_link_many(player->audioconv, player->resampler, player->sink, NULL))
  {
    g_printerr("Audioconvert, Resampler and Sink could not be linked.\n");
    gst_object_unref(player->pipeline);
    g_free(player);
    return -1;
  }

  player->bus = gst_element_get_bus(player->pipeline);
  bus_watch_id = gst_bus_add_watch(player->bus, bus_watcher, player);
  gst_object_unref(player->bus);

  ret = gst_element_set_state(player->pipeline, GST_STATE_PLAYING);

  GstTagList *tags = NULL;
  msg = gst_bus_timed_pop_filtered (GST_ELEMENT_BUS (player->pipeline), GST_CLOCK_TIME_NONE, GST_MESSAGE_TAG);
  gst_message_parse_tag (msg, &tags);
  g_print ("\n");
  gst_tag_list_foreach (tags, print_one_tag, NULL);
  g_print ("\n");
  gst_tag_list_unref (tags);

  msg = gst_bus_timed_pop_filtered (GST_ELEMENT_BUS (player->pipeline), GST_CLOCK_TIME_NONE, 
      GST_MESSAGE_ASYNC_DONE | GST_MESSAGE_ERROR);

  if (GST_MESSAGE_TYPE (msg) == GST_MESSAGE_ERROR)
    g_error ("Got error");

  gst_message_unref (msg);
  g_timeout_add (100, (GSourceFunc) cb_print_position, player->pipeline);
  
  g_main_loop_run(player->loop);
  GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(player->pipeline), 0, "dump_pipeline");

  if(ret == GST_STATE_CHANGE_FAILURE)
  {
    g_printerr("Unable to set pipeline to the playing state.\n");
    gst_object_unref(player->pipeline);
    g_free(player);
    return -1;
  }

  player_free(player);
  return 0;
}
