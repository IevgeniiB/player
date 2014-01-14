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
