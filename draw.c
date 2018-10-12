#include "snake.h"
#include <curses.h>

static int _warn_pos_x = 0, _warn_pos_y = 0;
void addWarn_str(char *warn) {
    mvaddstr(_warn_pos_x++, _warn_pos_y, warn);
    refresh();
    if(_warn_pos_x >= LINES) {
        _warn_pos_x = 0;
        _warn_pos_y += 5;
    }
}
void addWarn_char(char warn) {
    mvaddch(_warn_pos_x++, _warn_pos_y, warn);
    refresh();
    if(_warn_pos_x >= LINES) {
        _warn_pos_x = 0;
        _warn_pos_y += 5;
    }
}
void addWarn_int(int warn) {
    static char buf[BUFSIZ];
    sprintf(buf, "%d", warn);
    mvaddstr(_warn_pos_x++, _warn_pos_y, buf);
    refresh();
    if(_warn_pos_x >= LINES) {
        _warn_pos_x = 0;
        _warn_pos_y += 5;
    }
}

/* 画点
 * y坐标从左边界开始向右以SYMBOL_WIDTH为单位计算
 */
#define addPoint(x, y, symbol) { mvaddstr(shift_posx(x) + TOP_EDGE, \
                                          shift_posy(y) * SYMBOL_WIDTH + LEFT_EDGE, \
                                          symbol); refresh(); }

/* 只支持横线或竖线
 * 需要判断p1和p2的相对位置
 */
void addLine(Point *p1, Point *p2, char *symbol) {
    int i;
    int begin, end;
    if(p1->x == p2->x) {
        begin = p1->y < p2->y ? p1->y : p2->y;
        end = p1->y > p2->y ? p1->y : p2->y;
        for(i = begin; i <= end; i++) {
            addPoint(p1->x, i, symbol);
        }
    } else if(p1->y == p2->y) {
        begin = p1->x < p2->x ? p1->x : p2->x;
        end = p1->x > p2->x ? p1->x : p2->x;
        for(i = begin; i <= end; i++) {
            addPoint(i, p1->y, symbol);
        }
    }
    refresh();
}

/* 画蛇
 * 因为蛇至少有2段（头尾），所以没有特殊情况
 */
void addSnake(Snake *snake, char *symbol) {
    Body *cur = snake->head;
    Body *next = cur->next;

    while(next) {
        addLine(cur->pos, next->pos, symbol);
        cur = next;
        next = cur->next;
    }
    move(LINES - 1, COLS - 1);
    refresh();
}

void addFood(Food *food, char *symbol) {
    /* 画食物 */
    if(food->pos) {
        addPoint(food->pos->x, food->pos->y, symbol);
        refresh();
    }
}

void addEdge() {
    int l = LEFT_EDGE - 2, u = TOP_EDGE - 2, 
        r = RIGHT_EDGE + 2, b = BOT_EDGE + 2;
    int x, y;
    y = l;
    for(x = u; x <= b; x++) {
        mvaddstr(x, y, BLACK_SYMBOL);
    }
    y = r;
    for(x = u; x <= b; x++) {
        mvaddstr(x, y, BLACK_SYMBOL);
    }
    x = u;
    for(y = l; y <= r; y += 2) {
        mvaddstr(x, y, BLACK_SYMBOL);
    }
    x = b;
    for(y = l; y <= r; y += 2) {
        mvaddstr(x, y, BLACK_SYMBOL);
    }
    refresh();
}

