#ifndef _PLAYER_PRIV_H_
#define _PLAYER_PRIV_H_

#include <gst/gst.h>
#include <gst/pbutils/pbutils.h>
#include "player.h"

#define BACKSPACE "\b\b\b\b\b\b\b\b"
#define SEEK_STEP 5e9

typedef struct {
  void (*up)(void*);
  void (*down)(void*);
  void (*right)(void*);
  void (*left)(void*);
  void (*insert)(void*);
  void (*delete)(void*);
  void (*pageup)(void*);
  void (*pagedown)(void*);
  void (*home)(void*);
  void (*end)(void*);

  void *data;
} Keyboard_cb;

int getkey();
void *keyboard_thread(void*);
void sigint_handler_priv(gpointer);
static bus_watcher(GstBus *, GstMessage *, gpointer);
gint64 player_get_duration_priv(Player *);
gboolean cb_print_position_priv(Player *);
static gchar * create_one_tag_line(const gchar *, const GValue *);
static void create_one_tag (const GstTagList *, const gchar *, gpointer);
static void print_one_tag (const GstTagList *, const gchar *, gpointer);
static void cb_newpad(GstElement *, GstPad *, gpointer);
static gboolean file_is_audio(const gchar *);
static void on_discovered_cb (GstDiscoverer *, GstDiscovererInfo *, GError *, gboolean *);
static void on_finished_cb (GstDiscoverer *, GMainLoop *);
static void player_print_song(gpointer, gpointer);
gchar * player_get_tags_priv(Player *);
gboolean player_init_priv(Player *, const gchar *);
gboolean player_print_tags_priv(Player *);
gboolean player_play_pause_priv(Player *);
gboolean player_play_priv(Player *);
gboolean player_pause_priv(Player *);
gboolean player_key_handle_init_priv(Player *);
gboolean player_free_priv(Player *);
gint player_get_volume_priv(Player *);
gboolean player_set_volume_priv(Player *, gint);
gboolean player_mute_priv(Player *);
gboolean player_unmute_priv(Player *);
gboolean player_mute_auto_priv(Player *);
gboolean player_seek_priv(Player *, gint64);
gboolean player_init_playlist_from_dir_priv(Player *);
gboolean player_next_priv(Player *);
gboolean player_prev_priv(Player *);
void player_print_playlist_priv(Player *);

#endif
