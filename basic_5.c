#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <sys/select.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define WIDTH 400
#define HEIGHT 600
#define MAX_ENEMY_BOMBS 20
#define MAX_ENEMIES 20
#define MAX_PLAYER_BOMBS 20

Display* display;
Window mainwin;
int screen;

void drawAll();

typedef struct {
  int x;
  int y;
  int w;
  int h;
  bool active;
  bool enemy_goingLeft;
  int  enemy_bombFrequency;
  int  bomb_Speed;
} entity_ST;

entity_ST enemies[MAX_ENEMIES];
entity_ST player[1];
entity_ST playerBombs[MAX_PLAYER_BOMBS];
entity_ST enemiesBombs[MAX_ENEMY_BOMBS];
GC gcontext, gc_enemies, gc_enemiesBombs, gc_playerBombs;

//*******************************************************

void createWin() {
  display = XOpenDisplay("");
  screen = DefaultScreen(display);
  mainwin = XCreateSimpleWindow(display, RootWindow(display, screen),
                                10, 10, WIDTH, HEIGHT, 2,
                                BlackPixel(display, screen),
                                WhitePixel(display, screen));
  XMapWindow(display, mainwin);
  XFlush(display);
}

void checkColision(entity_ST* first_ent, int first_num,
                   entity_ST* second_ent, int second_num) {

  int firstPlusWidth;
  int firstPlusHeight;
  int secondPlusWidth;
  int secondPlusHeight;

  for (int i = 0; i < first_num; i++) {
    if (first_ent[i].active == true) {
      firstPlusWidth = first_ent[i].x + first_ent[i].w;
      firstPlusHeight = first_ent[i].y + first_ent[i].h;
    } else {
      continue;
    }

    for (int j = 0; j < second_num; j++) {
      if  (second_ent[j].active == true)  {
        secondPlusWidth = second_ent[j].x + second_ent[j].w;
        secondPlusHeight = second_ent[j].y + second_ent[j].h;

        //--- START CODE

        if ( (first_ent[i].x > second_ent[j].x) &&
             (first_ent[i].x < secondPlusWidth) &&
             (first_ent[i].y > second_ent[j].y) &&
             (first_ent[i].y < secondPlusHeight )) {
          printf("Colision 1\n");
          first_ent[i].active = false;
          second_ent[j].active = false;
          break;
        }

        if ( (firstPlusWidth > second_ent[j].x) &&
             (firstPlusWidth < secondPlusWidth) &&
             (first_ent[i].y > second_ent[j].y) &&
             (first_ent[i].y < secondPlusHeight) ) {
          printf("Colision 2\n");
          first_ent[i].active = false;
          second_ent[j].active = false;
          break;
        }

        if ( (first_ent[i].x > second_ent[j].x) &&
             (first_ent[i].x < secondPlusWidth) &&
             (firstPlusHeight > second_ent[j].y) &&
             (firstPlusHeight < secondPlusHeight) ) {
          printf("Colision 3\n");
          first_ent[i].active = false;
          second_ent[j].active = false;
          break;
        }

        if ( (firstPlusWidth > second_ent[j].x) &&
             (firstPlusWidth < secondPlusWidth) &&
             (firstPlusHeight > second_ent[j].y) &&
             (firstPlusHeight < secondPlusHeight) ) {
          printf("Colision 4\n");
          first_ent[i].active = false;
          second_ent[j].active = false;
          break;
        }

        //--- END
      } else {
        continue;
      }
    }
  }
  if (player[0].active == false) {
    printf("Ouch player down!\n");
    player[0].active = true;
  }
}


void drawPlayer() {
  Pixmap player_bitmap;
  unsigned int bitmap_width, bitmap_height;
  int hotspot_x, hotspot_y;
  int rc;
  rc = XReadBitmapFile(display, RootWindow(display, screen),
		       "player.bmp", &bitmap_width, &bitmap_height,
		       &player_bitmap, &hotspot_x, &hotspot_y);
  
  XCopyPlane(display, player_bitmap, mainwin, gcontext, 0, 0,
	     bitmap_width, bitmap_height, player[0].x, player[0].y, 1);

  //XFillRectangle(display, mainwin, gcontext,
  //               player[0].x, player[0].y, player[0].w, player[0].h);
}

void createEnemy() {
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (enemies[i].active == false) {
      enemies[i].w = (rand() % 10) + 20;
      enemies[i].h = (rand() % 10) + 20;
      enemies[i].x = (rand() % WIDTH);
      enemies[i].y = (rand() % 50);
      enemies[i].active = true;
      enemies[i].enemy_bombFrequency = 100;
      break;
    }
    if (i == 19) {
      printf("Enemies full\n");
    }
  }
}

void createEnemyBomb(int enemy_num) {
  int bomb_posX, bomb_posY;
  bomb_posX = enemies[enemy_num].x + (enemies[enemy_num].w / 2);
  bomb_posY = enemies[enemy_num].y + enemies[enemy_num].h + 5;
  for (int i = 0; i < MAX_ENEMY_BOMBS; i++) {
    if (enemiesBombs[i].active == false) {
      enemiesBombs[i].x = bomb_posX;
      enemiesBombs[i].y = bomb_posY;
      enemiesBombs[i].w = 10;
      enemiesBombs[i].h = 10;
      enemiesBombs[i].active = true;
      enemiesBombs[i].bomb_Speed = (rand() % 6) + 1;
      break;
    }
  }
  
}

void movePlayer(int x, int y) {
  player[0].x = player[0].x + x;
  player[0].y = player[0].y + y;
  checkColision(enemies, MAX_ENEMIES, player, 1);
  drawAll();
  //checkColision_Player(player, 1, enemies, 20);
}

void moveBombs() {
  for (int i = 0; i < MAX_PLAYER_BOMBS; i++) {
    if (playerBombs[i].active == true) {
      playerBombs[i].y -= 3;
      if (playerBombs[i].y < -playerBombs[i].h) playerBombs[i].active = false;
    }
  }

  for (int i = 0; i < MAX_ENEMY_BOMBS; i++) {
    if (enemiesBombs[i].active == true) {
      enemiesBombs[i].y += enemiesBombs[i].bomb_Speed;
      if (enemiesBombs[i].y > HEIGHT) enemiesBombs[i].active = false;
    }
  }
}

void moveEnemies() {
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (enemies[i].active == true) {
      if (enemies[i].enemy_goingLeft == true) {
	enemies[i].x -= 3;	
      } else {
	enemies[i].x += 3;		
      }
      enemies[i].y += 3;
      enemies[i].enemy_bombFrequency -= 10;
      if (enemies[i].enemy_bombFrequency < 0) {
	createEnemyBomb(i);
	enemies[i].enemy_bombFrequency = 400;
      };

      
      if (enemies[i].x < 0) enemies[i].enemy_goingLeft = false;
      if (enemies[i].x > (WIDTH - enemies[i].w))
	enemies[i].enemy_goingLeft = true;
      if (enemies[i].y > HEIGHT) enemies[i].active = false;

    }
  }
  checkColision(enemies, MAX_ENEMIES, player, 1);
}

void drawEnemies() {
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (enemies[i].active == true) {
      XFillRectangle(display, mainwin, gc_enemies,
                     enemies[i].x, enemies[i].y, enemies[i].w, enemies[i].h);
    }
  }
}

void drawBombs() {
  for (int i = 0; i < MAX_PLAYER_BOMBS; i++) {
    if (playerBombs[i].active == true) {
      XFillRectangle(display, mainwin, gc_playerBombs,
                     playerBombs[i].x, playerBombs[i].y,
                     playerBombs[i].w, playerBombs[i].h);
    }

    for (int i = 0; i < MAX_ENEMY_BOMBS; i++) {
      if (enemiesBombs[i].active == true) {
        XFillRectangle(display, mainwin, gc_enemiesBombs,
                       enemiesBombs[i].x, enemiesBombs[i].y,
                       enemiesBombs[i].w, enemiesBombs[i].h);
      }
    }
  }
}

void drawAll() {
  XClearWindow(display, mainwin);
  drawEnemies();
  drawPlayer();
  drawBombs();
  XFlush(display);
}
/***********************************************/
void initialSetup() {
  srand(time(NULL));
  gcontext = XCreateGC(display, mainwin, 0, 0);
  gc_enemies = XCreateGC(display, mainwin, 0, 0);
  gc_enemiesBombs = XCreateGC(display, mainwin, 0, 0);
  gc_playerBombs = XCreateGC(display, mainwin, 0, 0);

  player[0].w = 40;
  player[0].h = 20;
  player[0].x = WIDTH / 2;
  player[0].y = HEIGHT - player[0].h;
  player[0].active = true;
  XSelectInput(display, mainwin, KeyPressMask);
  for (int i = 0; i < MAX_ENEMY_BOMBS; i++) {
    enemiesBombs[i].active = false;
  }
}

void initColors() {
  
  int return_value;
  XColor mycolor, mycolor_exact;
  Colormap mycolormap;
  mycolormap = DefaultColormap(display, screen);

  return_value = XAllocNamedColor(display, mycolormap, "red",
				  &mycolor_exact, &mycolor);
  XSetForeground(display, gcontext, mycolor.pixel);

  return_value = XAllocNamedColor(display, mycolormap, "green",
				  &mycolor_exact, &mycolor);
  XSetForeground(display, gc_enemies, mycolor.pixel);
  
  return_value = XAllocNamedColor(display, mycolormap, "blue",
				  &mycolor_exact, &mycolor);
  XSetForeground(display, gc_playerBombs, mycolor.pixel);

  return_value = XAllocNamedColor(display, mycolormap, "yellow",
				  &mycolor_exact, &mycolor);
  XSetForeground(display, gc_enemiesBombs, mycolor.pixel);

  //*** Player Background (yellow)
  XSetBackground(display, gcontext, mycolor.pixel);
  //***

  return_value = XAllocNamedColor(display, mycolormap, "olive drab",
				  &mycolor_exact, &mycolor);

  //**************************************

  XSetWindowBackground(display, mainwin, mycolor.pixel);
}
int main() {

  createWin();
  initialSetup();
  initColors();
  
  int ret_key, inc_t, inc_spawn;
  inc_t = 0;
  inc_spawn = 0;
  XEvent ev;
  KeySym* key_symbol;

  //XAutoRepeatOn(display);
  while (1) {
    usleep(10000);
    inc_t++;
    inc_spawn++;
    if (inc_t >= 3) {
      moveBombs();
      moveEnemies();
      checkColision(playerBombs, MAX_PLAYER_BOMBS, enemies, MAX_ENEMIES);
      checkColision(enemiesBombs, MAX_ENEMY_BOMBS, player, 1);
      checkColision(enemiesBombs, MAX_ENEMY_BOMBS, playerBombs, MAX_PLAYER_BOMBS);
      drawAll();
      inc_t = 0;
    }
    if (inc_spawn >= 100) {
      createEnemy();
      inc_spawn = 0;
    }

    while (XPending(display)) {
      XNextEvent(display, &ev);

      switch (ev.type) {
      case KeyPress:
        key_symbol = XGetKeyboardMapping(display, ev.xkey.keycode, 1, &ret_key);

        if (key_symbol[0] == XK_a) {
          createEnemy();
        }

        if (key_symbol[0] == XK_b) {
          for (int i = 0; i < 20; i++) {
            printf("rand %i: %i\n", i, rand() % 50);
          }
        }

        if (key_symbol[0] == XK_Left) {
          movePlayer(-10, 0);
        }

        if (key_symbol[0] == XK_Right) {
          movePlayer(10, 0);
        }

        if (key_symbol[0] == XK_Up) {
          movePlayer(0, -10);
        }

        if (key_symbol[0] == XK_Down) {
          movePlayer(0, 10);
        }

        if (key_symbol[0] == XK_q) {
          printf("exit...\n");
          exit(0);
        }

        if (key_symbol[0] == XK_z) {
          for (int i = 0; i < MAX_PLAYER_BOMBS; i++) {
            if (playerBombs[i].active == false) {
              playerBombs[i].w = 15;
              playerBombs[i].h = 15;
              playerBombs[i].x = player[0].x + (player[0].w / 2) -
                                 (playerBombs[i].w / 2);
              playerBombs[i].y = player[0].y - 20;
              playerBombs[i].active = true;
              break;
            }
          }
        }

        if (key_symbol[0] == XK_x) {
          int numBombs = 0;
          for (int i = 0; i < MAX_PLAYER_BOMBS; i++) {
            if (playerBombs[i].active == true) {
              playerBombs[i].y = playerBombs[i].y - 5;
              numBombs++;
            }
          }

          printf("bombs: %i\n", numBombs);
          drawAll();
        }

        break;
      }
    }
  }

  sleep(3);
  return (0);
}
