#include "player.h"

/**
 * Creates player with default plugins for filesrc or http audio
 */ 
gboolean player_init(Player *player, const gchar *arg)
{
  if(player_init_priv(player, arg))
    return TRUE;
  g_printerr("Error in player_init");
  return FALSE;
}

/**
 * Detects in which of two states player is, and change it 
 * to the opposite state 
 */
gboolean player_play_pause(Player *player)
{
  if(player_play_pause_priv(player))
    return TRUE;
  return FALSE;
}

/**
 * Changes player state to PLAY 
 */
gboolean player_play(Player *player)
{
  if(player_play_priv(player))
    return TRUE;
  return FALSE;
}

/**
 * Changes player state to PAUSE 
 */
gboolean player_pause(Player *player)
{
  if(player_pause_priv(player))
    return TRUE;
  return FALSE;
}

/**
 * Thread function to handle keyboard key presses
 */
gboolean player_key_handle_init(Player* player)
{
  if(!player_key_handle_init_priv(player))
  {
    g_printerr("Player key handle init failed");
    return FALSE;
  }
  return TRUE;
}

/**
 * Handling function for linux signal SIGINT 
 */
void sigint_handler(Player *player)
{
  sigint_handler_priv(player);
}

/**
 * Prints file tags to the console
 */
gboolean print_tags(Player *player)
{
  if(!print_tags_priv(player))
  {
    g_printerr("Printing tags failed");
    return FALSE;
  }
  return TRUE;
}

/**
 * Callback for time tracking in console
 */
gboolean cb_print_position(Player *player)
{
  if(!cb_print_position_priv(player))
  {
    g_printerr("Fail cb_print_position");
    return FALSE;
  }
  return TRUE;
}

/**
 * Destroying player 
 */
gboolean player_free(Player *player)
{
  if(!player_free_priv(player))
  {
    g_printerr("Player freeing failed");
    return FALSE;
  }
  return TRUE;
}

/**
 * Get current playback volume
 * return value is in range (1-1000)
 * default volume value is 100
 */
gint player_get_volume(Player *player)
{
  return player_get_volume_priv(player);
}

/**
 * Set playback volume
 * volume must be in range (1-1000)
 * default volume value is 100
 */
gboolean player_set_volume(Player *player, gint volume)
{
  if(player_set_volume_priv(player, volume))
    return TRUE;
  return FALSE;
}

/**
 * Mute the volume playback
 */
gboolean player_mute(Player *player)
{
  if(player_mute_priv(player))
    return TRUE;
  return FALSE;
}

/**
 * Unmute the volume playback
 */
gboolean player_unmute(Player *player)
{
  if(player_unmute_priv(player))
    return TRUE;
  return FALSE;
}

/**
 * If volume is muted - unmute the volume
 * else - mute the volume
 */
gboolean player_mute_auto(Player *player)
{
  if(player_mute_auto_priv(player))
    return TRUE;
  return FALSE;
}
