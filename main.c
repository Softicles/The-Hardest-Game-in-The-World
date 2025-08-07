#include "main.h"
#include "background.h"
#include <stdio.h>
#include <stdlib.h>
#include "garbage.h"
#include "winning.h"
#include "losing.h"
#include "backplay.h"
#include "gba.h"

// GAME: THE HARDEST GAME IN THE WORLD
MAINSQUARE player;
OBSTABLE obs[5];

int spacing = 4;
int v_obs1 = 1;
int v_obs2 = -1;

enum gba_state {
  START,
  PLAY,
  WIN,
  LOSE,
};

int collision (MAINSQUARE player, OBSTABLE obst);
int initialize (MAINSQUARE *player, OBSTABLE obs[5]);

int main(void) {
  // Manipulate REG_DISPCNT here to set Mode 3. //
  REG_DISPCNT = MODE3 | BG2_ENABLE;
  // Save current and previous state of button input.
  u32 previousButtons = BUTTONS;
  u32 currentButtons = BUTTONS;

  // Load initial application state
  enum gba_state state = START;
  drawFullScreenImageDMA(background);
  int backstart = 1;
  int backlosing = 0;
  int backwinning = 0;
  int backplaying = 0;
  int dead_num = 0;

  initialize(&player, obs);
  
  while (1) {
    currentButtons = BUTTONS; // Load the current state of the buttons

    waitForVBlank();

    switch (state) {
      case START:
        backlosing = 0;
        backwinning = 0;
        backplaying = 0;
        // Draw the background screen
        
        if (!backstart) {
          drawFullScreenImageDMA(background);
          backstart = 1;
        }
        // Draw the game title
        // Draw instruction
        drawCenteredString(100, 40, 150, 30, "Press ENTER to start", MAGENTA);
        drawCenteredString(50, 20, 200, 40, "THE HARDEST GAME IN THE WORLD", MAGENTA);
        if (KEY_JUST_PRESSED(BUTTON_START, currentButtons, previousButtons)) {
          state = PLAY;
        }
        break;
      case PLAY:
        // reset backstart, backlosing, backwinning
        backstart = 0;
        backlosing = 0;
        backwinning = 0;
        if (!backplaying) {
          drawFullScreenImageDMA(backplay);
          char num_dead_str[20];
          sprintf(num_dead_str, "FAILS: %d", dead_num);
          drawCenteredString(40, 135, 40, 20, num_dead_str, WHITE);
          backplaying = 1;
        }

        // draw player
        drawRectDMA(player.row, player.col, player.width, player.height, BLUE);
        // logic for player
        if (KEY_DOWN(BUTTON_UP, currentButtons)) {
          if (player.row - 1 < 60) {
            player.row = 60;
          } else {
            player.row -= 1;
          }
        }
        
        if (KEY_DOWN(BUTTON_DOWN, currentButtons)) {
          if (player.row  + 1 > 105) {
            player.row = 105;
          }
          else {
            player.row += 1;
          }
        }

        if (KEY_DOWN(BUTTON_LEFT, currentButtons)) {
          if (player.col - 1 < 60) {
            player.col = 60;
          }
          else {
            if (player.col - 1 < 80) {
              player.col = 80;
            }
            else {
              player.col -= 1;
            }
          }
        }
        
        if (KEY_DOWN(BUTTON_RIGHT, currentButtons)) {
          if (player.col + 1 > 175) {
            player.col = 175;
          } else {
            if (player.col + 1 > 160) {
              player.col = 160;
              state = WIN;
            } else {
              player.col++;     
            }
          }
        }

        if(KEY_JUST_PRESSED(BUTTON_SELECT, currentButtons, previousButtons)) {
          v_obs1 = 1;
          v_obs2 = -1;
          initialize(&player, obs);
          state = START;
        }

        if (KEY_JUST_PRESSED(BUTTON_START, currentButtons, previousButtons)) {
          if (v_obs2 > 0 && v_obs2 < 2) {
            v_obs2++;
          } else if (v_obs2 < 0 && v_obs2 > -2) {
            v_obs2--;
          } else if (v_obs1 > 0 && v_obs1 < 2) {
            v_obs1++;
          } else if (v_obs1 < 0 && v_obs1 > -2) {
            v_obs1--;
          }
        }
        // draw starting base
        drawRectDMA(60, 60, 20, 50, GREEN);

        // draw finish base
        drawRectDMA(60, 160, 20, 50, GREEN);

        // draw playzone
        drawRectDMA(60, 80, 80, 50, YELLOW);

        // draw player
        drawRectDMA(player.row, player.col, player.width, player.height, BLUE);
        
        // draw obstacles
        for (int i = 0; i < 5; i++) {
          drawRectDMA(obs[i].row, obs[i].col, obs[i].width, obs[i].height, RED);
        }

        // obstacles logic
        for (int i = 0; i < 5; i++) { // set up obs 0, 2, 4
          if (obs[i].col + v_obs1 > 154) {
            v_obs1 = -v_obs1;
          }
          if (obs[i].col + v_obs1 < 80) {
            v_obs1 = -v_obs1;
          }
          obs[i].col += v_obs1;
          i++;
        }

        for (int i = 1; i < 5; i++) { // set up obs 1, 3
          if (obs[i].col + v_obs2 > 154) {
            v_obs2 = -v_obs2;
          }
          if (obs[i].col + v_obs2 < 80) { 
            v_obs2 = -v_obs2;
          }
          obs[i].col += v_obs2;
          i++;
        }
        
        for (int i = 0; i < 5; i++) {
          if (collision(player, obs[i])) {
            state = LOSE;
            dead_num++;
          }
        }

        break;
      case WIN:
        backstart = 0;
        backlosing = 0;
        backplaying = 0;
        dead_num = 0;
        if (!backwinning) {
          fillScreenDMA(BLACK);
          drawImageDMA(50, 51, 137, 60, winning);
          backwinning = 1;
        }
        drawCenteredString(110, 45, 150, 30, "Press ENTER to play again!", WHITE);
        drawCenteredString(120, 43, 150, 30, "Too easy? Try press ENTER twice!", WHITE);
        drawCenteredString(20, 30, 100, 20, "Press BACKSPACE", WHITE);
        drawCenteredString(30, 9, 100, 20, "to exit!", WHITE);
        if(KEY_JUST_PRESSED(BUTTON_START, currentButtons, previousButtons)) {
          initialize(&player, obs);
          state = PLAY;
        }
        if(KEY_JUST_PRESSED(BUTTON_SELECT, currentButtons, previousButtons)) {
          v_obs1 = 1;
          v_obs2 = -1;
          initialize(&player, obs);
          state = START;
        }

        break;
      case LOSE:
        backwinning = 0;
        backstart = 0;
        backplaying = 0;
        if (!backlosing) {
          fillScreenDMA(BLACK);
          drawImageDMA(10, 51, 137, 137, losing);
          backlosing = 1;
        }

        drawCenteredString(110, 45, 150, 30, "Press ENTER to play again!", WHITE);
        drawCenteredString(20, 30, 100, 20, "Press BACKSPACE", WHITE);
        drawCenteredString(30, 9, 100, 20, "to exit!", WHITE);

        if(KEY_JUST_PRESSED(BUTTON_START, currentButtons, previousButtons)) {
          state = PLAY;
          initialize(&player, obs);
        }
        if(KEY_JUST_PRESSED(BUTTON_SELECT, currentButtons, previousButtons)) {
          v_obs1 = 1;
          v_obs2 = -1;
          initialize(&player, obs);
          state = START;
        }

        break;
    }

    previousButtons = currentButtons; // Store the current state of the buttons
  }
  return 0;
}

// collision logic
int collision(MAINSQUARE player, OBSTABLE obst) {
  return ((player.col + player.width) > obst.col && (player.row + player.height > obst.row) 
      && (player.col < obst.col + obst.width) && (player.row < obst.row + obst.height));
  }

int initialize(MAINSQUARE *player, OBSTABLE obs[]) {
  player->row = 100;
  player->col = 60;
  player->width = 5;
  player->height = 5;

  for (int i = 0; i < 5; i++) { // set up obs 0, 2, 4
    obs[i].width = 6;
    obs[i].height = 6;
    obs[i].row = 62 + (spacing + obs[i].width) * i;
    obs[i].col = 80;
    i++;
  }

  for (int i = 1; i < 5; i++) { // set up obs 0, 2, 4
    obs[i].width = 6;
    obs[i].height = 6;
    obs[i].row = 62 + (spacing + obs[i].width) * i;
    obs[i].col = 154;
    i++;
  }
  return 0;
}