#include "player_priv.h"
#include <string.h>

void sigint_handler_priv(gpointer data)
{
  Player *player = data;
  g_print("\b\bTi\nGood bye!\n");
  player->thread_run = FALSE;
  g_main_loop_quit(player->loop);
}

static gboolean bus_watcher(GstBus *bus, GstMessage *message, gpointer data)
{
  Player *player = data;
  switch(GST_MESSAGE_TYPE(message))
  {
    case GST_MESSAGE_TAG:
    {
      gchar *new_tags;
      GstTagList *tags = NULL;
      GString *tag_string;
      tag_string = g_string_new("");

      gst_message_parse_tag (message, &tags);
      gst_tag_list_foreach (tags, create_one_tag, tag_string);
      gst_tag_list_unref (tags);

      new_tags = g_string_free(tag_string, FALSE);
      if(g_strcmp0(new_tags, player->tags) && strlen(new_tags) > 0)
      {
        player->tags = new_tags;
        player_print_song(player->playlist->data, NULL);
        g_print("--------------------------------\n");
        g_print("%s", player->tags);
        g_print("--------------------------------\n");
      }
      break;
    }
    case GST_MESSAGE_ERROR:
    {
      GError *err;
      gchar *debug;
      gst_message_parse_error (message, &err, &debug);
      g_print ("Error: %s\n", err->message);
      g_error_free (err);
      g_free (debug);
      g_print("\b\bTi\nGood bye!\n");
      player->thread_run = FALSE;
      g_main_loop_quit (player->loop);
      break;
    }
    case GST_MESSAGE_EOS:
    {
      /* end-of-stream */
      player_next_priv(player);
      break;
    }
    default:
      /* unhandled message */
      break;
  }
  return TRUE;
}

gint64 player_get_duration_priv(Player *player)
{
  gint64 len;
  gst_element_query_duration(player->pipeline, GST_FORMAT_TIME, &len);
  return len;
}

gboolean cb_print_position_priv(Player *player)
{
  gint64 pos, len;
  GstElement *pipeline = player->pipeline;

  if (gst_element_query_position (pipeline, GST_FORMAT_TIME, &pos)
      && gst_element_query_duration (pipeline, GST_FORMAT_TIME, &len)) {
    //FIXME A HUGE nail
    g_print ("Time: %" GST_TIME_FORMAT BACKSPACE " / %" GST_TIME_FORMAT BACKSPACE "         \r", 
        GST_TIME_ARGS (pos), GST_TIME_ARGS (len));
  }
  /* call me again */
  return TRUE;
}

static gchar * create_one_tag_line(const gchar *tag, const GValue *val)
{
  GString *res;
  res = g_string_new("");

  if (G_VALUE_HOLDS_STRING (val)) {
    g_string_printf(res, "%" INDENT "s : %s\n", tag, g_value_get_string (val));
  } else if (G_VALUE_HOLDS_UINT (val)) {
    g_string_printf(res, "%" INDENT "s : %u\n", tag, g_value_get_uint (val));
  } else if (G_VALUE_HOLDS_DOUBLE (val)) {
    g_string_printf(res, "%" INDENT "s : %g\n", tag, g_value_get_double (val));
  } else if (G_VALUE_HOLDS_BOOLEAN (val)) {
    g_string_printf(res, "%" INDENT "s : %s\n", tag,
        (g_value_get_boolean (val)) ? "true" : "false");
  } else if (GST_VALUE_HOLDS_BUFFER (val)) {
    GstBuffer *buf = gst_value_get_buffer (val);
    guint buffer_size = gst_buffer_get_size (buf);
    g_string_printf(res, "%" INDENT "s : buffer of size %u\n", tag, buffer_size);
  } else if (GST_VALUE_HOLDS_DATE_TIME (val)) {
    GstDateTime *dt = g_value_get_boxed (val);
    gchar *dt_str = gst_date_time_to_iso8601_string (dt);
    g_string_printf(res, "%" INDENT "s : %s\n", tag, dt_str);
    g_free (dt_str);
  } else {
    g_string_printf(res, "%" INDENT "s : tag of type ’%s’\n", tag, G_VALUE_TYPE_NAME (val));
  }

  return g_string_free(res, FALSE);
}

static void create_one_tag (const GstTagList *list, const gchar *tag, gpointer user_data)
{
  int i, num;
  GString *res;
  res = user_data;

  num = gst_tag_list_get_tag_size (list, tag);

  for (i = 0; i < num; ++i) {
    const GValue *val;
    /* Note: when looking for specific tags, use the gst_tag_list_get_xyz() API,
     * * we only use the GValue approach here because it is more generic */
    val = gst_tag_list_get_value_index (list, tag, i);
    if(g_strrstr(tag, "artist"))
      g_string_append(res, create_one_tag_line("Artist", val));
    else if(g_strrstr(tag, "title"))
      g_string_append(res, create_one_tag_line("Song", val));
    else if(g_strrstr(tag, "genre"))
      g_string_append(res, create_one_tag_line("Genre", val));
    else if(g_strrstr(tag, "datetime"))
      g_string_append(res, create_one_tag_line("Year", val));
    else if(g_strrstr(tag, "album"))
      g_string_append(res, create_one_tag_line("Album", val));
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

static void on_discovered_cb (GstDiscoverer *discoverer, GstDiscovererInfo *info, GError *err, gboolean *data) {
  GstDiscovererResult result;
  const gchar *uri;
  const GstTagList *tags;
  GstDiscovererStreamInfo *sinfo;
  GstCaps *sinfo_caps;
  gchar *nick;

  uri = gst_discoverer_info_get_uri (info);
  result = gst_discoverer_info_get_result (info);

  if (result != GST_DISCOVERER_OK) {
    *data = FALSE;
    return;
  }

  sinfo = gst_discoverer_info_get_stream_info (info);
  if (!sinfo)
  {
    *data = FALSE;
    gst_discoverer_stream_info_unref(sinfo);
    return;
  }

  sinfo_caps = gst_discoverer_stream_info_get_caps(sinfo);
  nick = gst_caps_to_string(sinfo_caps);

  if(g_strrstr(nick, "video") || g_strrstr(nick, "image"))
  {
    *data = FALSE;
  }
  else
    *data = TRUE;

  g_free(nick);
  gst_caps_unref(sinfo_caps);
  gst_discoverer_stream_info_unref(sinfo);
}

static void on_finished_cb (GstDiscoverer *discoverer, GMainLoop *mainloop) {
  g_main_loop_quit(mainloop);
}

static gboolean file_is_audio(const gchar *filename)
{
  GstDiscoverer *discoverer;
  GMainLoop *mainloop;
  GError *error = NULL;
  gchar *uri;
  gboolean *is_audio;
  is_audio = g_new(gboolean, 1);

  if(filename == NULL)
    return FALSE;

  uri = gst_filename_to_uri(filename, &error);
  discoverer = gst_discoverer_new (5 * GST_SECOND, &error);

  if (!discoverer) {
    g_print("Error creating discoverer instance: %s\n", error->message);
    g_clear_error(&error);
    return FALSE;
  }

  mainloop = g_main_loop_new (NULL, FALSE);
  g_signal_connect (discoverer, "discovered", G_CALLBACK (on_discovered_cb), is_audio);
  g_signal_connect (discoverer, "finished", G_CALLBACK (on_finished_cb), mainloop);

  gst_discoverer_start(discoverer);

  if (!gst_discoverer_discover_uri_async (discoverer, uri)) {
    g_print ("Failed to start discovering URI '%s'\n", uri);
    g_object_unref (discoverer);
    return FALSE;
  }

  g_main_loop_run (mainloop);

  gst_discoverer_stop (discoverer);

  g_object_unref (discoverer);
  g_main_loop_unref (mainloop);

  if(*is_audio)
  {
    g_free(is_audio);
    return TRUE;
  }
  g_free(is_audio);
  return FALSE;
}

gboolean player_next_priv(Player *player)
{
  GList *list;
  GstState state;

  list = player->playlist;
  list = g_list_next(list);
  if(!list)
    list = g_list_first(player->playlist);

  if(!list)
    return FALSE;

  gst_element_get_state(player->pipeline, &state, NULL, GST_CLOCK_TIME_NONE);
  gst_element_set_state(player->pipeline, GST_STATE_READY);
  g_object_set(G_OBJECT(player->source), "location", list->data, NULL);

  gst_element_set_state(player->pipeline, state);
  player->playlist = list;
  list = NULL;
  g_free(list);
}

gboolean player_prev_priv(Player *player)
{
  GList *list;
  GstState state;

  list = player->playlist;
  list = g_list_previous(list);
  if(!list)
    list = g_list_last(player->playlist);

  if(!list)
    return FALSE;

  gst_element_get_state(player->pipeline, &state, NULL, GST_CLOCK_TIME_NONE);
  gst_element_set_state(player->pipeline, GST_STATE_READY);
  g_object_set(G_OBJECT(player->source), "location", list->data, NULL);
  gst_element_set_state(player->pipeline, state);
  player->playlist = list;
  list = NULL;
  g_free(list);
}

gboolean player_init_playlist_from_dir_priv(Player *player)
{
  GDir *dir;
  gchar *file;
  gchar *filename;
  guint size;
  const gchar *dir_name = g_path_get_dirname(player->init_song);
  gchar *current;
  if(player->playlist != NULL)
  {
    current = player->playlist->data;
  }
  else
  {
    current = NULL;
  }
  player->playlist = NULL;

  size = g_list_length(player->playlist);
  dir = g_dir_open(dir_name, 0, NULL);
  file = g_strconcat(dir_name, "/", filename = g_strdup(g_dir_read_name(dir)), NULL);

  while(filename)
  {
    if(file_is_audio(file) && g_strcmp0(file, player->init_song))
    {
      player->playlist = g_list_append(player->playlist, g_strdup(file));
    }
    
    g_free(file);
    file = g_strconcat(dir_name, "/", filename = g_strdup(g_dir_read_name(dir)), NULL);
  }
  g_free(file);

  if(g_list_length(player->playlist) > size)
  {
    player->playlist = g_list_sort(player->playlist, (GCompareFunc)g_ascii_strcasecmp);
    if(file_is_audio(player->init_song))
      player->playlist = g_list_prepend(player->playlist, g_strdup(player->init_song));
    if(file_is_audio(current)){
      while(g_strcmp0(player->playlist->data, current))
      {
        player->playlist = g_list_next(player->playlist);
      }
    }
    g_print("---------New playlist-----------\n");
    player_print_playlist_priv(player);
    g_print("--------------------------------\n");
    return TRUE;
  }
  return FALSE;
}

static void player_print_song(gpointer fullname, gpointer user_data)
{
  gchar *name = g_path_get_basename(fullname);
  //FIXME A HUGE nail
  g_print("%s                                     \n", name);
  g_free(name);
}

void player_print_playlist_priv(Player *player)
{
  GList *list;
  list = player->playlist;
  player->playlist = g_list_first(player->playlist);
  g_list_foreach(player->playlist, (GFunc)player_print_song, NULL);
  player->playlist = list;
}

static void player_show_playlist_priv(Player *player)
{
  g_print("------------Playlist------------\n");
  player_print_playlist_priv(player);
  g_print("--------------------------------\n");
}


gboolean player_init_priv(Player *player, const gchar *arg)
{
  player->thread_run = TRUE;
  player->loop = g_main_loop_new(NULL, FALSE);
  player->init_song = g_strdup(arg);
  player->tags = g_strdup("");

  if(g_file_test(arg, G_FILE_TEST_EXISTS) && file_is_audio(arg))
  {
    player->playlist = g_list_append(player->playlist, g_strdup(arg));
    g_print("------------Playlist------------\n");
    player_print_playlist_priv(player);
    g_print("--------------------------------\n");
  }
  else
  {
    player_init_playlist_from_dir_priv(player);
    if(!player->playlist)
    {
      g_print("No available audio\n");
      return FALSE;
    }
    player->init_song = player->playlist->data;
  }

  if(g_strrstr(arg, "http"))
  {
    player->source = gst_element_factory_make("souphttpsrc", "httpsrc0");
  }
  else
  {
    player->source = gst_element_factory_make("filesrc", "filesrc0");
  }

  g_object_set(G_OBJECT(player->source), "location", player->playlist->data, NULL);
  player->decoder = gst_element_factory_make("decodebin", "decoder0");
  g_signal_connect(player->decoder, "pad-added", G_CALLBACK(cb_newpad), player);
  player->audioconv = gst_element_factory_make("audioconvert", "audioconv0");
  player->resampler = gst_element_factory_make("audioresample", "resampler0");
  player->volume = gst_element_factory_make("volume", "volume0");
  player->sink = gst_element_factory_make("autoaudiosink", "sink0");
  player->pipeline = gst_pipeline_new("player-pipeline");

  if (!player->pipeline || !player->source || !player->decoder || 
      !player->audioconv || !player->resampler || !player->volume || !player->sink)
  {
    g_printerr("Not all elements could be created.\n");
    return FALSE;
  }

  gst_bin_add_many (GST_BIN(player->pipeline), player->source, player->decoder, 
      player->audioconv, player->resampler, player->volume, player->sink, NULL);

  if (!gst_element_link_many(player->source, player->decoder, NULL))
  {
    g_printerr("Source and Decoder could not be linked.\n");
    gst_object_unref(player->pipeline);
    g_free(player);
    return FALSE;
  }

  if (!gst_element_link_many(player->audioconv, player->resampler, player->volume, player->sink, NULL))
  {
    g_printerr("Audioconvert, Resampler and Sink could not be linked.\n");
    gst_object_unref(player->pipeline);
    g_free(player);
    return FALSE;
  }

  guint bus_watch_id;
  player->bus = gst_element_get_bus(player->pipeline);
  bus_watch_id = gst_bus_add_watch(player->bus, bus_watcher, player);
  gst_object_unref(player->bus);

  return TRUE;
}

gchar * player_get_tags_priv(Player *player)
{
  return player->tags;
}

gboolean player_print_tags_priv(Player *player)
{
  gchar * tag_string = player_get_tags_priv(player);
  g_print ("\n");
  g_print("%s", tag_string);
  g_print ("\n");
  g_free(tag_string);
  return TRUE;
}

gboolean player_play_pause_priv(Player *player)
{
  if(GST_STATE(player->pipeline) == GST_STATE_PLAYING)
  {
    gst_element_set_state(player->pipeline, GST_STATE_PAUSED);
    return TRUE;
  }
  else if(GST_STATE(player->pipeline) == GST_STATE_PAUSED)
  {
    gst_element_set_state(player->pipeline, GST_STATE_PLAYING);
    return TRUE;
  }
  else
  {
    g_printerr("play-pause exception");
    return FALSE;
  }
}

void volume_increase(Player* player)
{
  player_set_volume_priv(player, player_get_volume_priv(player) + 10);
}

void volume_decrease(Player* player)
{
  player_set_volume_priv(player, player_get_volume_priv(player) - 10);
}

void seek_forw(Player *player)
{
  gint64 pos;
  gst_element_query_position(player->pipeline, GST_FORMAT_TIME, &pos);
  player_seek_priv(player, pos + SEEK_STEP);
}


void seek_back(Player *player)
{
  gint64 pos;
  gst_element_query_position(player->pipeline, GST_FORMAT_TIME, &pos);
  player_seek_priv(player, pos - SEEK_STEP);
}

gboolean player_key_handle_init_priv(Player *player)
{
  Keyboard_cb *key_cb = (Keyboard_cb *)calloc(1, sizeof(Keyboard_cb));
  key_cb->end = (void *)player_play_pause_priv;
  key_cb->pageup = (void *)volume_increase;
  key_cb->pagedown = (void *)volume_decrease;
  key_cb->home = (void *)player_mute_auto_priv;
  key_cb->right = (void *)seek_forw;
  key_cb->left = (void *)seek_back;
  key_cb->up = (void *)player_prev_priv;
  key_cb->down = (void *)player_next_priv;
  key_cb->insert = (void *)player_init_playlist_from_dir_priv;
  key_cb->delete = (void *)player_delete_current_priv;
  key_cb->enter = (void *)player_show_playlist_priv;

  key_cb->data = player;

  player->thread = g_thread_new("player_thread", keyboard_thread, key_cb);

  return TRUE;
}

gboolean player_free_priv(Player *player)
{
  gst_element_set_state(player->pipeline, GST_STATE_NULL);
  gst_object_unref(player->pipeline);
  g_free(player);
  return TRUE;
}

gboolean player_play_priv(Player *player)
{
  if(GST_STATE(player->pipeline) == GST_STATE_PLAYING)
  {
    g_print("Already in playing state\n");
    return FALSE;
  }
  else if(GST_STATE(player->pipeline) == GST_STATE_PAUSED)
  {
    gst_element_set_state(player->pipeline, GST_STATE_PLAYING);
    return TRUE;
  }
  else
  {
    g_printerr("Unexpected state of pipeline (play function)");
    return FALSE;
  }
}

gboolean player_pause_priv(Player *player)
{
  if(GST_STATE(player->pipeline) == GST_STATE_PAUSED)
  {
    g_print("Already in paused state\n");
    return FALSE;
  }
  else if(GST_STATE(player->pipeline) == GST_STATE_PLAYING)
  {
    gst_element_set_state(player->pipeline, GST_STATE_PAUSED);
    return TRUE;
  }
  else
  {
    g_printerr("Unexpected state of pipeline (pause function)");
    return FALSE;
  }
}

gint player_get_volume_priv(Player *player)
{
  gdouble volume;
  g_object_get(player->volume, "volume", &volume, NULL);
  return (gint)(volume * 100);
}

gboolean player_set_volume_priv(Player *player, gint volume)
{
  if(volume < 0 || volume > 100)
  {
    return FALSE;
  }
  gdouble new_volume = volume / 100.0;
  g_object_set(player->volume, "volume", (gdouble)volume/100.0, NULL);

  return TRUE;
}

gboolean player_mute_priv(Player *player)
{
  gboolean mute;
  g_object_get(player->volume, "mute", &mute, NULL);
  if(mute)
  {
    g_printerr("Player is already muted\n");
    return FALSE;
  }
  g_object_set(player->volume, "mute", TRUE, NULL);
  return TRUE;
}

gboolean player_unmute_priv(Player *player)
{
  gboolean mute;
  g_object_get(player->volume, "mute", &mute, NULL);
  if(!mute)
  {
    g_printerr("Player is already unmuted\n");
    return FALSE;
  }
  g_object_set(player->volume, "mute", FALSE, NULL);
  return TRUE;
}

gboolean player_mute_auto_priv(Player *player)
{
  gboolean mute;
  g_object_get(player->volume, "mute", &mute, NULL);
  g_object_set(player->volume, "mute", !mute, NULL);
  return TRUE;
}

gboolean player_delete_current_priv(Player *player)
{
  player_next_priv(player);
  player->playlist = g_list_remove_link(player->playlist, player->playlist->prev);
  g_print("------------Updated-------------\n");
  player_print_playlist_priv(player);
  g_print("--------------------------------\n");

  return TRUE;
}

gboolean player_seek_priv (Player *player, gint64 pos)
{
  GstElement *pipeline = player->pipeline;
  gint64 len;

  gst_element_query_duration (pipeline, GST_FORMAT_TIME, &len);
  if (pos > len)
    pos = len;
  if (pos < 0)
    pos = 0;
  if (!gst_element_seek (pipeline, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
        GST_SEEK_TYPE_SET, pos, GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE))
    return FALSE;
  return TRUE;
}
