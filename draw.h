#ifndef _DRAW_H_
#define _DRAW_H_

void addEdge(void);
void addLine(Point *p1, Point *p2, char *symbol);
void addSnake(Snake *snake, char *symbol);
void addFood(Food *food, char *symbol);
void addWarn_str(char *warn);
void addWarn_char(char warn);
void addWarn_int(int warn);

#endif
