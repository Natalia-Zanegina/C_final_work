#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ncurses.h>
#include <time.h>

#include "autopilot.h"
#include "controls.h"
#include "drone_collect.h"
#include "drone_creation.h"
#include "drone_motion.h"
#include "pseudo_graphics.h"
#include "pumpkin.h"

#define MIN_Y 2
#define START_TROLLEYS_COUNT 0
#define MAX_TAIL_SIZE 100
#define MAX_PUMPKINS_COUNT 20
#define CONTROLS 3
#define FOOD_EXPIRE_SECONDS 10
#define SEED_NUMBER 10

extern double DELAY; // задержка при отрисовке


int main(void)
{
    int mode = 0;
    int droneNum = 1;
    printf("Select mode of drone work: 1 - manual, 2 - auto, 3 - cooperative work:\n");
    scanf("%d", &mode);
    if (mode == 3)
    {
        printf("Input number of drones (max 5):\n");
        scanf("%d", &droneNum);
    }
    refresh();

    drone_t* drones[droneNum];
    for (int i = 0; i < droneNum; i++)
        initDrone(drones, START_TROLLEYS_COUNT, 10 + i * 10, 10 + i * 10, i);

    struct pumpkin pumpkin[MAX_PUMPKINS_COUNT];
    
    
    initscr();
    keypad(stdscr, TRUE);
    raw();
    noecho();
    curs_set(FALSE);
    initPumpkins(pumpkin, MAX_PUMPKINS_COUNT);
    mvprintw(0, 1, "Use arrows or WASD keys for control. Press 'F10' for EXIT.");
    timeout(0);
    int key_pressed = 0;

    putPumpkinSeed(pumpkin);
    putPumpkin(pumpkin, 10);


    while (key_pressed != STOP_GAME)
    {
        clock_t begin = clock();
        key_pressed = getch(); // считываем клавишу

        _Bool oneCrushed = 0;
        
        for (int i = 0; i < droneNum; i++)
        {
            if (mode == 1) {
                updateManual(drones[i], pumpkin, key_pressed);
            } else {
                update(drones[i], pumpkin, key_pressed);  
            }

            if (isCrush(drones[i]))
            {
                oneCrushed = 1;
                printCrush();
                break;
            }
            repairSeed(pumpkin, SEED_NUMBER, drones[i]);
        }

        if (oneCrushed)
        {
            printf("Crush! ");
            break;
        }
            
        
        while ((double) (clock() - begin) / CLOCKS_PER_SEC < DELAY)
        {}
        
        refresh();
        
    }

    if (mode == 1 || mode == 2) {
        printExit(drones[0]);
    } else {
        size_t res = 0;
        for (int i = 0; i < droneNum; i++)
        {
            printExit(drones[i]);
            res += drones[i]->tsize;
        }
        printf("Total count of trolleys: %zu  ", res);
    }
    
    for (int i = 0; i < droneNum; i++)
    {
        free(drones[i]->tail);
        free(drones[i]);
    }
    
    endwin();
    return 0;
}