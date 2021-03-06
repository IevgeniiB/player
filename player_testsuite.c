#include <stdlib.h>
#include "player.h"

gint main(gint argc, gchar *argv[])
{
  gchar *directory;
  Player *player = g_new(Player, 1);
  if(argc>2)
    g_print("===> Player currently work's only with one argument\n");
  if(argc<2)
  {
    directory = g_get_current_dir();
    directory = g_strconcat(directory, "/", NULL);
  }
  else
    directory = argv[1];

  gst_init(&argc, &argv);
  if(!player_init(player, directory))
  {
    g_printerr("Terminate\n");
    g_free(player);
    return -1;
  }
  g_unix_signal_add(SIGINT, sigint_handler, player);

  GstStateChangeReturn ret;
  ret = gst_element_set_state(player->pipeline, GST_STATE_PLAYING);

  if(ret == GST_STATE_CHANGE_FAILURE)
  {
    g_printerr("Unable to set pipeline to the playing state.\n");
    gst_object_unref(player->pipeline);
    g_free(player);
    return -1;
  }

  g_timeout_add (100, (GSourceFunc) cb_print_position, player);
  
  player_key_handle_init(player);

  g_main_loop_run(player->loop);
  GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(player->pipeline), 0, "dump_pipeline");

  player_free(player);
  return 0;
}
