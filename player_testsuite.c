#include <stdlib.h>
#include "player.h"

gint main(gint argc, gchar *argv[])
{
  if(argc<2)
  {
    g_print("\tUsage: %s <path to audiofile>\n", argv[0]);
    return 1;
  }

  gst_init(&argc, &argv);
  Player *player = g_new(Player, 1);
  if(!player_init(player, argv[1]))
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

  gchar *string;
  string = player_get_tags(player);
  g_print("%s", string);
  g_free(string);

  g_timeout_add (100, (GSourceFunc) cb_print_position, player);
  
  player_key_handle_init(player);

  g_main_loop_run(player->loop);
  GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(player->pipeline), 0, "dump_pipeline");

  player_free(player);
  return 0;
}
