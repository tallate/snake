#include "snake.h"
#include "utils.h"
#include <stdlib.h>

Point *newPoint(int x, int y) {
    Point *p = (Point*) malloc(sizeof(Point));
    p->x = x;
    p->y = y;
    return p;
}
Body *newBody(Point *p) {
    Body *b = (Body*) malloc(sizeof(Body));
    b->pos = p;
    b->next = NULL;
    b->prev = NULL;
    return b;
}
/* 创建一个蛇，它只有两个节点分别是头和尾
 * 初始头在起点，尾出了边界，方向朝右
 */
Snake *newSnake() {
    Snake *s = (Snake*) malloc(sizeof(Snake));
    s->head = newBody(newPoint(X_INIT, Y_INIT));
    s->tail = newBody(newPoint(X_INIT, Y_INIT - 1));
    connectBody(s->head, s->tail);
    s->oldDir = RIGHT;
    s->dir = RIGHT;
    s->isAlive = 1;
    return s;
}
/* 创建一个食物
 * 食物刚开始没有实体（位置），每10次时钟滴答出现（或者改变位置）
 */
Food *newFood() {
    Food *f = (Food*) malloc(sizeof(Food));
    f->pos = NULL;
    f->h = f->count = 30;
}

void deletePoint(Point *p) {
    free(p);
}
void deleteBody(Body *b) {
    deletePoint(b->pos);
    free(b);
}
void deleteSnake(Snake *s) {
    Body *cur = s->head, *next;
    while(cur) {
        next = cur->next;
        deleteBody(cur);
        cur = next;
    }
}
void deleteFood(Food *f) {
    if(f->pos) {
        deletePoint(f->pos);
    }
    free(f);
}

/* 坐标转换，到屏幕内 */
int shift_posx(int x) {
    while(x < 0) {
        x += WIN_HEIGHT;
    }
    return x % WIN_HEIGHT;
}
int shift_posy(int y) {
    while(y < 0) {
        y += (WIN_WIDTH / SYMBOL_WIDTH);
    }
    return y % (WIN_WIDTH / SYMBOL_WIDTH);
}

/* 判断两个点是否呈水平 */
int isHorizontal(Point *p1, Point *p2) {
    return p1->x == p2->x;
}
/* 判断两个点是否相邻 */
int isAdjacent(Point *p1, Point *p2) {
    int delt_x = abs(p1->x - p2->x);
    int delt_y = abs(p1->y - p2->y);
    return delt_x + delt_y == 1;
}
/* 判断点是否在区间内
 * 假设这两个点一定水平或垂直
 */
int isInInterval(int x, int y, Point *begin, Point *end) {
    int begin_posx = shift_posx(begin->x);
    int begin_posy = shift_posy(begin->y);
    int end_posx = shift_posx(end->x);
    int end_posy = shift_posy(end->y);
    x = shift_posx(x), y = shift_posy(y);
/*
// 这种判断方式在蛇从右边穿回左边(循环)的时候有点问题
    int isIn = (x - begin_posx) * (x - end_posx) <= 0 &&
                (y - begin_posy) * (y - end_posy) <= 0;
    if(((begin_posx < end_posx) != (begin->x < end->x))
            || ((begin_posy < end_posy) != (begin->y < end->y))) {
        return ! isIn;
    } else {
        return isIn;
    }
*/  
    if((x == begin_posx && y == begin_posy) || (x == end_posx && y == end_posy)) {
        // printf("1\n");
        return 1;
    } else if(x == begin_posx /*&& begin_posx == end_posx*/) {
        int isIn = (y - begin_posy) * (y - end_posy) < 0;
        int isInBorder = (begin_posy < end_posy) == (begin->y < end->y);
        if(isIn == isInBorder) {
            // printf("2\n");
            return 1;
        }
    } else if(y == begin_posy /*&& begin_posy == end_posy*/) {
        int isIn = (x - begin_posx) * (x - end_posx) < 0;
        int isInBorder = (begin_posx < end_posx) == (begin->x < end->x);
        if(isIn == isInBorder) {
            // printf("3\n");
            return 1;
        }
    }
    return 0;
}

/* 连接身体，即连接双向链表 */
void connectBody(Body *b1, Body *b2) {
    b1->next = b2;
    b2->prev = b1;
}
void disconnectBody(Body *b1, Body *b2) {
    b1->next = b2->prev = NULL;
}

/* 对蛇头和尾进行出入队操作 */
void enqueueHead(Snake *snake, Point *newP) {
    Body *oldHead = snake->head;
    snake->head = newBody(newP);
    connectBody(snake->head, oldHead);
}
Body *dequeueTail(Snake *snake) {
    Body *tail = snake->tail;
    Body *bodylast = snake_body_last(snake);
    disconnectBody(bodylast, tail);
    snake->tail = bodylast;
    return tail;
}


/* 身体的首节和末节 */
Body *snake_body_first(Snake *snake) {
    return snake->head->next;
}
Body *snake_body_last(Snake *snake) {
    return snake->tail->prev;
}

/* 蛇的增长（头部） */
void snake_grow(Snake *snake) {
    int tmp_x, tmp_y;
    tmp_x = snake->head->pos->x, tmp_y = snake->head->pos->y;
    switch(snake->dir) {
    case LEFT: tmp_y -= 1; break;
    case RIGHT: tmp_y += 1; break;
    case UP: tmp_x -= 1; break;
    case DOWN: tmp_x += 1; break;
    default: return ; /* sth fucked */
    }

    if(isRedir(snake)) {
        /* 若已转向，则将新头补上，并改变oldDir */
        enqueueHead(snake, newPoint(tmp_x, tmp_y));
        snake->oldDir = snake->dir;
    } else {
        /* 若无转向，直接更新到头 */
        snake->head->pos->x = tmp_x;
        snake->head->pos->y = tmp_y;
    }
}

/* 收缩（尾部） */
void snake_shrink(Snake *snake) {
    Body *tail = snake->tail;
    Body *lastBody = snake_body_last(snake);
    if(isAdjacent(tail->pos, lastBody->pos)) {
        /* 若尾与身体最后截相邻，则去掉尾部 */
        free(dequeueTail(snake));
    } else {
        /* 不相邻，直接更新到尾部 */
        int delt;
        if((delt = lastBody->pos->x - tail->pos->x) != 0) {
            tail->pos->x += delt / abs(delt);
        }
        if((delt = lastBody->pos->y - tail->pos->y) != 0) {
            tail->pos->y += delt / abs(delt);
        }
    }
}

/* 蛇移动 */
void snake_move(Snake *snake) {
    /* 头部（增长） */
    snake_grow(snake);

    /* 尾部（收缩） */
    snake_shrink(snake);
}

int isRedir(Snake *snake) {
    return snake->oldDir != snake->dir;
}

void snake_redir(Snake *snake, Dir newDir) {
    int isOldDirHori = IS_HORIZONTAL(snake->dir);
    int isNewDirHori = IS_HORIZONTAL(newDir);
    int isSnakeHori = isHorizontal(snake->head->pos, snake_body_first(snake)->pos);
    if((isOldDirHori != isNewDirHori) &&
            (isNewDirHori != isSnakeHori)) {
        snake->dir = newDir;
    }
}


void food_locate(Food *food) {
    if(food->count == 0) {
        int x = rand_interval(0, WIN_HEIGHT);
        int y = rand_interval(0, WIN_WIDTH / SYMBOL_WIDTH);
        if(food->pos == NULL) {
            food->pos = newPoint(x, y);
        } else {
            food->pos->x = x;
            food->pos->y = y;
        }
        food->count = food->h;
    } else {
        food->count--;
    }
}

/* 判断蛇是否吃到食物
 * 即判断蛇下一步是否能走到食物位置
 */
void snake_eat_food(Snake *snake, Food *food) {
    if(food->pos != NULL) {
        int x = snake->head->pos->x, y = snake->head->pos->y;
        switch(snake->dir) {
        case LEFT: y--; break;
        case RIGHT: y++; break;
        case UP: x--; break;
        case DOWN: x++; break;
        default: return ;
        }

        if(shift_posx(x) == food->pos->x && 
                shift_posy(y) == food->pos->y) {
            /* 若食物在蛇的前进方向上则吃掉 */
            snake_grow(snake);
            deletePoint(food->pos);
            food->pos = NULL;
        }
    }
}

/* 判断蛇1是否会撞上蛇2
 * 即判断蛇1下一步是否会走到蛇2的身体的某节位置，自撞也是说得通的
 */
int snake_hit_snake(Snake *snake1, Snake *snake2) {
    int x = snake1->head->pos->x, y = snake1->head->pos->y;
    switch(snake1->dir) {
    case LEFT: y--; break;
    case RIGHT: y++; break;
    case UP: x--; break;
    case DOWN: x++; break;
    default: return 0;
    }

    Body *begin = snake2->head, *end = begin->next;
    while(end) {
        /* 是否在区间内 */
        if(isInInterval(x, y, begin->pos, end->pos)) {
            return 1;
        }
        /* 取下一截 */
        begin = end;
        end = begin->next;
    }
    return 0;
}


