#include <curses.h>                              
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "prep.h"
#include "worm.h"
#include "worm_model.h"
#include "board_model.h"
#include "messages.h"
#include "options.h"

void initializeColors();
void readUserInput(struct worm* aworm, enum GameStates* agame_state);
enum ResCodes doLevel(struct game_options* somegops);

void initializeColors() 
{
    start_color();
    init_pair(COLP_USER_WORM, COLOR_GREEN, COLOR_BLACK);
    init_pair(COLP_FREE_CELL, COLOR_BLACK, COLOR_BLACK);
    init_pair(COLP_FOOD_1, COLOR_YELLOW, COLOR_BLACK);
    init_pair(COLP_FOOD_2, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(COLP_FOOD_3, COLOR_CYAN, COLOR_BLACK);
    init_pair(COLP_BARRIER, COLOR_RED, COLOR_BLACK);
}

void readUserInput(struct worm* aworm, enum GameStates* agame_state) 
{
    int ch; 
    if ((ch = getch()) > 0) {
        switch(ch) 
        {
            case 'q' :   
                *agame_state = WORM_GAME_QUIT;
                break;
            case KEY_UP :
                setWormHeading(aworm, WORM_UP);
                break;
            case KEY_DOWN :
                setWormHeading(aworm, WORM_DOWN);
                break;
            case KEY_LEFT :
                setWormHeading(aworm, WORM_LEFT);
                break;
            case KEY_RIGHT :
                setWormHeading(aworm, WORM_RIGHT);
                break;
            case 's' :
                nodelay(stdscr, FALSE);  
                break;
            case ' ' :
                nodelay(stdscr, TRUE);
                break;
            case 'g':
                growWorm(aworm, BONUS_3);
                break;
        }
    }
    return;
}

enum ResCodes doLevel(struct game_options* somegops) 
{
    struct worm userworm;
    struct board theboard;
    enum GameStates agame_state;
    enum ResCodes res_code; 
    bool end_level_loop;   
    struct pos bottomLeft;

    napms(somegops->nap_time);
    agame_state = WORM_GAME_ONGOING;
    res_code = initializeBoard(&theboard);
    if(res_code != RES_OK)
    {
      return res_code;
    }
    res_code = initializeLevel(&theboard);
    if(res_code != RES_OK)
    {
      return res_code;
    }
    bottomLeft.y = getLastRowOnBoard(&theboard);
    bottomLeft.x = 0;
    res_code = initializeWorm(&userworm, WORM_LENGTH, WORM_INITIAL_LENGTH, bottomLeft, WORM_RIGHT, COLP_USER_WORM);
    if ( res_code != RES_OK) 
    {
        return res_code;
    }
    showWorm(&theboard, &userworm);
    refresh();
    end_level_loop = false;
    while(!end_level_loop) 
    {
        readUserInput(&userworm, &agame_state); 
        if (agame_state == WORM_GAME_QUIT) 
        {
            end_level_loop = true;
            continue;
        }
        cleanWormTail(&theboard, &userworm);
        moveWorm(&theboard, &userworm, &agame_state);
        if (agame_state != WORM_GAME_ONGOING) 
        {
            end_level_loop = true;
            continue;
        }
        showWorm(&theboard, &userworm);
        showStatus(&theboard, &userworm);
        napms(NAP_TIME);
        refresh();
        if(getNumberOfFoodItems(&theboard) == 0)
        {
          end_level_loop = true;
        }
    }
    res_code = RES_OK;
    switch(agame_state)
    {
      case WORM_GAME_ONGOING:
        if(getNumberOfFoodItems(&theboard) == 0)
        {
          showDialog("Sie haben diese Runde erfolgreich beendet !!!", "Bitte Taste druecken");
        }
        else
        {
          showDialog("Interner Fehler!", "Bitte Taste druecken");
          res_code = RES_INTERNAL_ERROR;
        }
        break;
      case WORM_GAME_QUIT:
        showDialog("Sie haben die aktuelle Runde abgebrochen!", "Bitte Taste Druecken");
        break;
      case WORM_CRASH:
        showDialog("Sie haben das Spiel verloren," "weil sie in die Barriere gefahren sind", "Bitte Taste druecken");
      case WORM_OUT_OF_BOUNDS:
        showDialog("Sie haben das Spiel verloren," "weil sie das Spielfeld verlassen haben", "Bitte Taste druecken");
        break;
      case WORM_CROSSING:
        showDialog("Sie haben das Spiel verloren," "weil sie einen Wurm gekreuzt haben", "Bitte Taste druecken");
        break; 
      default: 
        showDialog("Interner Fehler!", "Bitte Taste druecken");
        res_code = RES_INTERNAL_ERROR;
    }
    cleanupBoard(&theboard);
    return res_code;
} 

enum ResCodes playGame(int argc, char* argv[])
{
  enum ResCodes res_code;
  struct game_options thegops;
  res_code = readCommandLineOptions(&thegops, argc, argv);
  if(res_code != RES_OK)
  {
    return res_code;
  }
  if(thegops.start_single_step)
  {
    nodelay(stdscr, FALSE);
  }
  res_code = doLevel(&thegops);
  return res_code;
}

int main(int argc, char* argv[])
    {
    int res_code;
    initializeCursesApplication();
    initializeColors();
    if ( LINES < ROWS_RESERVED + MIN_NUMBER_OF_ROWS || COLS < MIN_NUMBER_OF_COLS )
    {
        cleanupCursesApp();
        printf("Das Fenster ist zu klein: wir brauchen mindestens %dx%d\n",
                MIN_NUMBER_OF_COLS, MIN_NUMBER_OF_ROWS + ROWS_RESERVED );
        res_code = RES_FAILED;
    }
    else
    {
        //nodelay(stdscr, false);
        res_code = playGame(argc, argv);
        cleanupCursesApp();
    }
    return res_code;
}

