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

double DELAY = 0.1;

struct control_buttons default_controls[CONTROLS] = {{KEY_DOWN, KEY_UP, KEY_LEFT, KEY_RIGHT}, {'S', 'W', 'A', 'D'}, {'s', 'w', 'a', 'd'}}; // инициализация структуры control_buttons вариантами кнопок для ручного управления дроном.

void initTail(tail_t t[], size_t size) // создаем будущую цепочку тележек - пока пустые ячейки (с одинаковыми нулевыми координатами)
{
    tail_t init_t = {0, 0};
    for (size_t i = 0; i < size; i++)
        t[i] = init_t;
}

void initHead(drone_t* head, int x, int y) // создаем дрон (задаем координаты и направление движения по умолчанию)
{
    head -> x = x;
    head -> y = y;
    head -> direction = RIGHT;
}

void initDrone(drone_t* head[], size_t size, int x, int y, int i) // выделяем необходимую память и прикрепляем к дрону цепочку тележек
{
    head[i] = (drone_t*)malloc(sizeof(drone_t));
    tail_t* tail = (tail_t*) malloc(MAX_TAIL_SIZE * sizeof(tail_t));
    initTail(tail, MAX_TAIL_SIZE);
    initHead(head[i], x, y);
    head[i]->tail = tail;
    head[i]->tsize = size + 1;
    head[i]->controls = default_controls;
}

void initPumpkins(struct pumpkin f[], size_t size) // создаем будущие тыквы
{
    struct pumpkin init = {0, 0, 0, 0, 0};
    int max_y = 0, max_x = 0;
    getmaxyx(stdscr, max_y, max_x);
    for(size_t i = 0; i < size; i++)
        f[i] = init;
}

void go(drone_t* head) // перемещается дрон
{
    char ch = '@';
    int max_x = 0, max_y = 0;
    getmaxyx(stdscr, max_y, max_x);
    mvprintw(head -> y, head -> x, " ");
    switch (head -> direction)
    {
        case LEFT:
            if (head -> x <= 0)
                head -> x = max_x;
            mvprintw(head -> y, --(head -> x), "%c", ch);
            break;
        case RIGHT:
            if (head -> x >= max_x)
                head -> x = 0;
            mvprintw(head -> y, ++(head -> x), "%c", ch);
            break;
        case UP:
            if (head -> y <= MIN_Y)
                head -> y = max_y;
            mvprintw(--(head -> y), head -> x, "%c", ch);
            break;
        case DOWN:
            if (head -> y >= max_y)
                head -> y = MIN_Y;
            mvprintw(++(head -> y), head -> x, "%c", ch);
            break;
        default:
            break;
    }
    refresh();
}

void goTail(drone_t* head) // вслед за дроном перемещается цепочка тележек
{
    char ch = '*';
    mvprintw(head -> tail[head -> tsize - 1].y,head -> tail[head -> tsize - 1].x, " ");
    for (size_t i = head -> tsize - 1; i > 0; i--)
    {
        head -> tail[i] = head -> tail[i - 1];
        if ((head -> tail[i].y) || (head -> tail[i].x))
            mvprintw(head -> tail[i].y, head -> tail[i].x, "%c", ch);
    }
    head -> tail[0].x = head -> x;
    head -> tail[0].y = head -> y;
}

int checkDirection(drone_t* drone, int32_t key) // проверка, не является ли направление противоположным текущему
{
    for (int i = 0; i < CONTROLS; i++)
    {    
        if ((key == drone -> controls[i].down && drone -> direction == UP) ||
        (key == drone -> controls[i].up && drone -> direction == DOWN) ||
        (key == drone -> controls[i].left && drone -> direction == RIGHT) ||
        (key == drone -> controls[i].right && drone -> direction == LEFT))
            return 0;
    }
    return 1;
}

void changeDirection(drone_t* drone, const int32_t key) // изменить направление движения вручную (клавишей)
{
    if (!checkDirection(drone, key))
        return;

    for (int i = 0; i < CONTROLS; i++)
    {
        if (key == drone -> controls[i].down)
            drone -> direction = DOWN;
        else if (key == drone -> controls[i].up)
            drone -> direction = UP;
        else if (key == drone -> controls[i].right)
            drone -> direction = RIGHT;
        else if (key == drone -> controls[i].left)
            drone -> direction = LEFT;
    }
}

void putPumpkinSeed(struct pumpkin* fp) // назначаем координаты, внешний вид и прочие характеристики тыкве и размещаем её на поле
{
    int max_x = 0, max_y = 0;
    char spoint[2] = {0};
    getmaxyx(stdscr, max_y, max_x);
    mvprintw(fp->y, fp->x, " ");
    fp->x = rand() % (max_x - 1);
    fp->y = rand() % (max_y - 2) + 2;
    fp->put_time = time(NULL);
    fp->point = '$';
    fp->enable = 1;
    spoint[0] = fp -> point;
    mvprintw(fp->y, fp->x, "%s", spoint);
}

void putPumpkin(struct pumpkin f[], size_t number_of_seeds) // разместить все тыквы по порядку
{
    for(size_t i = 0; i < number_of_seeds; i++)
        putPumpkinSeed(&f[i]);
}


void refreshPumpkin(struct pumpkin f[], int npumpkins) // обновить тыквы
{
    int max_x = 0, max_y = 0;
    getmaxyx(stdscr, max_y, max_x);
    for(size_t i = 0; i < npumpkins; i++)
    {
        if(f[i].put_time)
        {
            if(!f[i].enable || (time(NULL) - f[i].put_time) > FOOD_EXPIRE_SECONDS)
                putPumpkinSeed(&f[i]);
        }
    }
}

_Bool haveCollect(struct drone_t* head, struct pumpkin f[]) // собрал ли дрон данную тыкву
{
    for (size_t i = 0; i < MAX_PUMPKINS_COUNT; i++)
    {
        if (f[i].enable && head->x == f[i].x && head->y == f[i].y)
        {
            f[i].enable = 0;
            return 1;
        }
    }    
    return 0;
}

void addTrolley(struct drone_t *head) // добавить еще одну тележку
{
    if (head == NULL || head -> tsize > MAX_TAIL_SIZE)
    {
        mvprintw(0, 0, "Can't add trolley");
        return;
    }
    head -> tsize++;
}

void printLevel(struct drone_t* head) // печать количества собранных тыкв текущего дрона
{
    int max_x = 0, max_y = 0;
    getmaxyx(stdscr, max_y, max_x);
    mvprintw(0, max_x - 20, "Count of trolleys: %zu", head -> tsize - 1);    
}

void printExit(struct drone_t* head) // печать итогового количества собранных текущим дроном тыкв
{
    // int max_x = 0, max_y = 0;
    // getmaxyx(stdscr, max_y, max_x);
    // mvprintw(max_y / 2 - 1, max_x / 2 - 5, "Your LEVEL is %zu", ); 
    // refresh();
    // getchar();

    // size_t res = 0;
    
    printf("%zu   ", head->tsize - 1);
        
        // printf("Total count of trolleys: %zu  ", res);

    // printf("Total count of trolleys: %zu ", head -> tsize);
}

void printCrush() // печать сообщения об аварии
{
    int max_x = 0, max_y = 0;
    getmaxyx(stdscr, max_y, max_x);
    mvprintw(max_y / 2 - 1, max_x / 2 - 5, "Crash!");
    refresh();
    getchar();
} 

int findDistance(const drone_t drone, const struct pumpkin f){ // вычисляет количество ходов до тыквы
    return (abs(drone.x - f.x) + abs(drone.y - f.y));
}

void autoChangeDirection(drone_t* drone, struct pumpkin pumpkin[], int pumpkinCount)
{
    int pointer = 0;
    for (int i = 0; i < pumpkinCount; i++) // ищем ближайшую тыкву
        pointer = (findDistance(*drone, pumpkin[i]) < findDistance(*drone, pumpkin[pointer])) ? i : pointer;
    
    if (drone->direction == RIGHT || drone->direction == LEFT && (drone->y != pumpkin[pointer].y)) //горизонтальное движение
    {
        drone->direction = (pumpkin[pointer].y > drone->y) ? DOWN : UP;
    } else if (drone->direction == DOWN || drone->direction == UP && (drone->x != pumpkin[pointer].x))//вертикальное движение
    {
        drone->direction = (pumpkin[pointer].x > drone->x) ? RIGHT : LEFT;    
    }
}

void update(struct drone_t* head, struct pumpkin f[], const int32_t key) // обновление состояния: дрона, тележек и тыкв
{
    int max_x = 0, max_y = 0;
    getmaxyx(stdscr, max_y, max_x);
    autoChangeDirection(head, f, SEED_NUMBER);
    go(head);
    goTail(head);
    refreshPumpkin(f, SEED_NUMBER);// обновляем зрелые тыквы
    if (haveCollect(head, f))
    {
        addTrolley(head);
        printLevel(head);
        DELAY -= 0.0001;
    }    
}

void updateManual(struct drone_t* head, struct pumpkin f[], const int32_t key) // обновление состояния: дрона, тележек и тыкв (при ручном управлении)
{
    int max_x = 0, max_y = 0;
    getmaxyx(stdscr, max_y, max_x);
    changeDirection(head, key);
    go(head);
    goTail(head);
    refreshPumpkin(f, SEED_NUMBER);// обновляем еду
    if (haveCollect(head, f))
    {
        addTrolley(head);
        printLevel(head);
        DELAY -= 0.0001;
    }    
}

_Bool isCrush(drone_t* drone) // проверка, не столкнулся ли дрон с тележками
{
    for (size_t i = 1; i < drone->tsize; i++)
    {
        if (drone->x == drone->tail[i].x && drone->y == drone->tail[i].y)
            return 1;
    }
    return 0;
}

void repairSeed(struct pumpkin f[], size_t npumpkins, struct drone_t* head) // проверка корректности выставления тыквы
{
    for (size_t i = 0; i < head->tsize; i++) // если тележки совпадают с тыквой
    {
        for (size_t j = 0; j < npumpkins; j++)
        {
            if (f[j].x == head->tail[i].x && f[j].y == head->tail[i].y && f[j].enable)
            {
                mvprintw(2, 0, "Repair trolley pumpkin %zu", j);
                putPumpkinSeed(&f[j]);
            }
        }
    }

    for (size_t i = 0; i < npumpkins; i++) // если две тыквы на одной точке
    {
        for (size_t j = 0; j < npumpkins; j++)
        {
            if (i != j && f[i].enable && f[j].enable && f[j].x == f[i].x && f[j].y == f[i].y)
            {
                mvprintw(1, 1, "Repair same pumpkin %zu", j); 
                putPumpkinSeed(&f[j]);
            }
            
        }
    }
    
}

