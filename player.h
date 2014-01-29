#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <stdlib.h>
#include <gst/gst.h>

/**
 * Structure, that holds all plugins
 */
typedef struct {
  GMainLoop *loop;
  GThread *thread;
  gboolean thread_run;

  GstBus *bus;

  GstElement *pipeline;
  GstElement *source;
  GstElement *decoder;
  GstElement *audioconv;
  GstElement *resampler;
  GstElement *volume;
  GstElement *sink;
} Player;

gboolean player_init(Player *player, const gchar *arg);
void play_pause(Player *player);
gboolean player_free(Player *player);
gboolean player_key_handle_init(Player* player);
void sigint_handler(Player *player);
gboolean print_tags(Player *player);
gboolean cb_print_position(Player *player);

#endif
