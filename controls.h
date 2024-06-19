#define CONTROLS 3

enum {
    LEFT = 1,
    UP,
    RIGHT,
    DOWN,
    STOP_GAME = KEY_F(10)
}; // ключевые команды для управления дроном

struct control_buttons
{
    int down;
    int up;
    int left;
    int right;
}; // структура контрольных значений, для которых можно назначить определенные клавиши