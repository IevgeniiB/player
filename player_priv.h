#ifndef _PLAYER_PRIV_H_
#define _PLAYER_PRIV_H_

#include <gst/gst.h>
#include "player.h"

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
gboolean cb_print_position_priv(Player *);
static void print_one_tag_line(const gchar *, const GValue *);
static void print_one_tag (const GstTagList *, const gchar *, gpointer);
static void cb_newpad(GstElement *, GstPad *, gpointer);
gboolean player_init_priv(Player *, const gchar *);
gboolean print_tags_priv(Player *);
void play_pause_priv(void *);
gboolean player_key_handle_initi_priv(Player *);
gboolean player_free_priv(Player *);

#endif
