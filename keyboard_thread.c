#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <pthread.h>
#include "player.h"


int getkey() {
  int character;
  struct termios orig_term_attr;
  struct termios new_term_attr;

  /* set the terminal to raw mode */
  tcgetattr(fileno(stdin), &orig_term_attr);
  memcpy(&new_term_attr, &orig_term_attr, sizeof(struct termios));
  new_term_attr.c_lflag &= ~(ECHO|ICANON);
  new_term_attr.c_cc[VTIME] = 0;
  new_term_attr.c_cc[VMIN] = 0;
  tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);

  /* read a character from the stdin stream without blocking */
  /*   returns EOF (-1) if no character is available */
  character = fgetc(stdin);

  /* restore the original terminal attributes */
  tcsetattr(fileno(stdin), TCSANOW, &orig_term_attr);

  return character;
}
/*
void print(const char *string)
{
  printf("hello, %s", string);
}
*/
void *keyboard_thread(void *arg)
{
  Keyboard_cb *key_cb = arg;
  Player *player = key_cb->data;
  int key;

  for (;;) {
    key = getkey();
    
key_handle:

    if(key != -1) {
      if(key == 0x1b)
      {
        key = getkey();
        if (key == 0x5b)
        {
          key = getkey();
          switch(key)
          {
            case 0x32:
              key_cb->insert(key_cb->data);
              printf("Insert\n");
              getkey();
              break;
            case 0x33:
              printf("Delete\n");
              getkey();
              break;
            case 0x35:
              printf("Page Up\n");
              getkey();
              break;
            case 0x36:
              printf("Page Down\n");
              getkey();
              break;
            case 0x41:
              printf("Up\n");
              break;
            case 0x42:
              printf("Down\n");
              break;
            case 0x43:
              printf("Right\n");
              break;
            case 0x44:
              printf("Left\n");
              break;
            default:
              printf("Default case\n");
          }
          continue;
        }
        else if(key == 0x4f)
        {
          key = getkey();
          switch(key)
          {
            case 0x48:
              printf("Home\n");
              break;
            case 0x46:
              key_cb->end(key_cb->data);
              break;
            default:
              printf("Default case\n");
          }
          continue;
        }
        else if(key == -1)
          goto key_handle;
        else
          printf("After escape: 0x%02x\n", key);
      }
      printf("0x%02x\n", key);
    }
    if(!player->thread_run)
      g_thread_exit(NULL);
  }
}
/*
int main()
{
  Keyboard_cb *key_cb = (Keyboard_cb *)calloc(1, sizeof(Keyboard_cb));
  key_cb->insert = print;
  key_cb->data = "i.baliuk";

  pthread_t thread;
  pthread_create(&thread, NULL, keyboard_thread, key_cb);
  pthread_join(thread, NULL);

  return 0;
}*/
