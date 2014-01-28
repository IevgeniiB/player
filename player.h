#include <gst/gst.h>
typedef struct _keyboard_cb {
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

/**
 * Structure, that holds all plugins
 */
typedef struct _player {
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

