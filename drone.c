#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ncurses.h>
#include <time.h>

#define MIN_Y 2
#define START_TROLLEYS_COUNT 0
#define MAX_TAIL_SIZE 100
#define MAX_PUMPKINS_COUNT 20
#define CONTROLS 3
#define FOOD_EXPIRE_SECONDS 10
#define SEED_NUMBER 10

double DELAY = 0.1;

enum {
    LEFT = 1,
    UP,
    RIGHT,
    DOWN,
    STOP_GAME = KEY_F(10)
};

struct control_buttons
{
    int down;
    int up;
    int left;
    int right;
};

struct control_buttons default_controls[CONTROLS] = {{KEY_DOWN, KEY_UP, KEY_LEFT, KEY_RIGHT}, {'S', 'W', 'A', 'D'}, {'s', 'w', 'a', 'd'}};

typedef struct drone_t
{
    int x;
    int y;
    int direction;
    size_t tsize;
    struct tail_t* tail;
    struct control_buttons* controls;
} drone_t;


typedef struct tail_t
{
    int x;
    int y;
} tail_t;

struct pumpkin
{
    int x;
    int y;
    time_t put_time;
    char point;
    uint8_t enable;
} pumpkin[MAX_PUMPKINS_COUNT]; 

void initTail(tail_t t[], size_t size)
{
    tail_t init_t = {0, 0};
    for (size_t i = 0; i < size; i++)
        t[i] = init_t;
}

void initHead(drone_t* head, int x, int y)
{
    head -> x = x;
    head -> y = y;
    head -> direction = RIGHT;
}

void initDrone(drone_t* head[], size_t size, int x, int y, int i)
{
    head[i] = (drone_t*)malloc(sizeof(drone_t));
    tail_t* tail = (tail_t*) malloc(MAX_TAIL_SIZE * sizeof(tail_t));
    initTail(tail, MAX_TAIL_SIZE);
    initHead(head[i], x, y);
    head[i]->tail = tail;
    head[i]->tsize = size + 1;
    head[i]->controls = default_controls;
}

void initPumpkins(struct pumpkin f[], size_t size)
{
    struct pumpkin init = {0, 0, 0, 0, 0};
    int max_y = 0, max_x = 0;
    getmaxyx(stdscr, max_y, max_x);
    for(size_t i = 0; i < size; i++)
        f[i] = init;
}

void go(drone_t* head)
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

void goTail(drone_t* head)
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

int checkDirection(drone_t* drone, int32_t key)
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

void changeDirection(drone_t* drone, const int32_t key)
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

void putPumpkinSeed(struct pumpkin* fp)
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

void putPumpkin(struct pumpkin f[], size_t number_of_seeds)
{
    for(size_t i = 0; i < number_of_seeds; i++)
        putPumpkinSeed(&f[i]);
}

void refreshPumpkin(struct pumpkin f[], int npumpkins)
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

_Bool haveCollect(struct drone_t* head, struct pumpkin f[])
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

void addTrolley(struct drone_t *head)
{
    if (head == NULL || head -> tsize > MAX_TAIL_SIZE)
    {
        mvprintw(0, 0, "Can't add trolley");
        return;
    }
    head -> tsize++;
}



void printLevel(struct drone_t* head)
{
    int max_x = 0, max_y = 0;
    getmaxyx(stdscr, max_y, max_x);
    mvprintw(0, max_x - 20, "Count of trolleys: %zu", head -> tsize - 1);    
}

void printExit(struct drone_t* head)
{
    printf("Count of trolleys: %zu   ", head->tsize - 1);
}


void printCrush()
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

void update(struct drone_t* head, struct pumpkin f[], const int32_t key)
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

void updateManual(struct drone_t* head, struct pumpkin f[], const int32_t key)
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

_Bool isCrush(drone_t* drone)
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
            // printExit(drones[i]);
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
