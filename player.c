#include "player.h"

gboolean player_init(Player *player, const gchar *arg)
{
  if(player_init_priv(player, arg))
    return TRUE;
  g_printerr("Error in player_init");
  return FALSE;
}

void play_pause(Player *player)
{
  play_pause_priv(player);
}

gboolean player_free(Player *player)
{
  if(!player_free_priv(player))
  {
    g_printerr("Player freeing failed");
    return FALSE;
  }
  return TRUE;
}

gboolean player_key_handle_init(Player* player)
{
  if(!player_key_handle_initi_priv(player))
  {
    g_printerr("Player key handle init failed");
    return FALSE;
  }
  return TRUE;
}

void sigint_handler(Player *player)
{
  sigint_handler_priv(player);
}

gboolean print_tags(Player *player)
{
  if(!print_tags_priv(player))
  {
    g_printerr("Printing tags failed");
    return FALSE;
  }
  return TRUE;
}

gboolean cb_print_position(Player *player)
{
  if(!cb_print_position_priv(player))
  {
    g_printerr("Fail cb_print_position");
    return FALSE;
  }
  return TRUE;
}
