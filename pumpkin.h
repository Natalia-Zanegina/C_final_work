#define MAX_PUMPKINS_COUNT 20

#ifndef PUMPKIN_H
#define PUMPKIN_H
typedef struct pumpkin
{
    int x;
    int y;
    time_t put_time;
    char point;
    uint8_t enable;
} pumpkin;
#endif

void initPumpkins(struct pumpkin f[], size_t size);
void putPumpkinSeed(struct pumpkin* fp);
void putPumpkin(struct pumpkin f[], size_t number_of_seeds);
void refreshPumpkin(struct pumpkin f[], int npumpkins);
void repairSeed(struct pumpkin f[], size_t npumpkins, drone_t* head);