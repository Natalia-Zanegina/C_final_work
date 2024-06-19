#ifndef DRONE_H
#define DRONE_H
typedef struct drone_t
{
    int x;
    int y;
    int direction;
    size_t tsize;
    struct tail_t* tail;
    struct control_buttons* controls;
} drone_t;
#endif

#ifndef TAIL_H
#define TAIL_H
typedef struct tail_t
{
    int x;
    int y;
} tail_t;
#endif

void initTail(tail_t t[], size_t size);
void initHead(drone_t* head, int x, int y);
void initDrone(drone_t* head[], size_t size, int x, int y, int i);
