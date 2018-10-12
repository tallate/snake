#ifndef _SNAKE_H_
#define _SNAKE_H_

/* 蛇体 */
#define BLANK "  "
#define WHITE_SYMBOL "■ "
#define BLACK_SYMBOL "□ "
#define SYMBOL_WIDTH 2

/* 时钟滴答/秒 */
#define TICKS_PER_SEC 8

/* 边界 */
#define TOP_EDGE 4
#define BOT_EDGE 20
#define LEFT_EDGE 10
#define RIGHT_EDGE 70
#define WIN_HEIGHT 16
#define WIN_WIDTH 60

/* 蛇头起始点 */
#define X_INIT 0
#define Y_INIT 1

/* 标识蛇的运动方向
 */
typedef enum Dir {
    LEFT = 0,
    RIGHT = 2,
    UP = 1,
    DOWN = 3
} Dir;
#define IS_HORIZONTAL(dir) (dir % 2 == 0)
#define IS_VERTICAL(dir) (dir % 2 != 0)

typedef struct Point {
    int x, y;
} Point;

typedef struct PointList {
    Point *pos;
    struct PointList *next;
    struct PointList *prev;
} Body;

typedef struct Snake {
    Body *head;
    Body *tail;
    Dir oldDir;
    Dir dir;
    int isAlive;
} Snake;

typedef struct Food {
    Point *pos;
    int h;
    int count;
} Food;

/* 构造器
 */
Point *newPoint(int x, int y);
Body *newBody(Point *p);
Snake *newSnake(void);
Food *newFood(void);
/* 回收器
 */
void deletePoint(Point *p);
void deleteBody(Body *b);
void deleteSnake(Snake *s);
void deleteFood(Food *f);

/* 坐标转换 */
int shift_posx(int x);
int shift_posy(int y);

/* 判断两个点连线是否水平 */
int isHorizontal(Point *p1, Point *p2);
/* 判断两个点是否相邻 */
int isAdjacent(Point *p1, Point *p2);
/* 判断点是否在（垂直或水平）区间内 */
int isInInterval(int x, int y, Point *begin, Point *end);

/* 连接/断开两段Body，以前面的作为头 */
void connectBody(Body *b1, Body *b2);
void disconnectBody(Body *b1, Body *b2);
/* 蛇头和蛇尾的出入队操作 */
void enqueueHead(Snake *snake, Point *newP);
Body *dequeueTail(Snake *snake);
/* 选取蛇身的第一截和最后截 */
Body *snake_body_first(Snake *snake);
Body *snake_body_last(Snake *snake);

/* 蛇增长（头部）和收缩（尾部） */
void snake_grow(Snake *snake);
void snake_shrink(Snake *snake);

/* 使蛇前进 */
void snake_move(Snake *snake);

/* 判断蛇是否转向 */
int isRedir(Snake *snake);
/* 改变蛇的前进方向
 * 不允许转向后方或前方，不允许转向后瞬间转向
 */
void snake_redir(Snake *snake, Dir newDir);

/* 为食物创建新位置
 * 若计时器到了，判断pos是否为NULL，为其分配位置
 */
void food_locate(Food *food);
/* 蛇吃食物？
 */
void snake_eat_food(Snake *snake, Food *food);
/* 蛇撞蛇？
 * 撞到返回1
 */
int snake_hit_snake(Snake *snake1, Snake *snake2);

#endif
