#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <pthread.h>
#include "player_priv.h"


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
              // "Insert"
              getkey();
              break;
            case 0x33:
              // "Delete"
              getkey();
              break;
            case 0x35:
              // "Page Up"
              key_cb->pageup(key_cb->data);
              getkey();
              break;
            case 0x36:
              // "Page Down"
              key_cb->pagedown(key_cb->data);
              getkey();
              break;
            case 0x41:
              // "Up"
              key_cb->up(key_cb->data);
              getkey();
              break;
            case 0x42:
              // "Down"
              key_cb->down(key_cb->data);
              getkey();
              break;
            case 0x43:
              // "Right"
              key_cb->right(key_cb->data);
              getkey();
              break;
            case 0x44:
              // "Left"
              key_cb->left(key_cb->data);
              getkey();
              break;
            default:
              break;
              // "Default case"
          }
          continue;
        }
        else if(key == 0x4f)
        {
          key = getkey();
          switch(key)
          {
            case 0x48:
              // "Home"
              key_cb->home(key_cb->data);
              break;
            case 0x46:
              // "End"
              key_cb->end(key_cb->data);
              break;
            default:
              break;
              // "Default case"
          }
          continue;
        }
        else if(key == -1)
          goto key_handle;
        else
        {
          //printf("After escape: 0x%02x\n", key);
        }
      }
    }
    if(!player->thread_run)
    {
      free(arg);
      g_thread_exit(NULL);
    }
  }
}
