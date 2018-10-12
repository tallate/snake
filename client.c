#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "snake.h"
#include "socklib.h"
#include "draw.h"
#include <unistd.h>
#include <pthread.h>
#include <locale.h>
#include <curses.h>
#include <signal.h>
#include <fcntl.h>
#include <aio.h>

#define HOST "127.0.1.1"
#define LOGIN_PORT 2222 /* 登录请求端口 */
#define OP_PORT 1234 /* 传输用户操作 */
#define OBJ_PORT 4321 /* 传输蛇和食物对象 */
#define error_exit(m, x) { perror(m); exit(x); }
#define MAX_SNAKE 3

/* id为当前蛇的标志，若为-1说明死了（见download_obj） */
int id;
Food *food = NULL;
Snake *snakes[MAX_SNAKE] = { 0 };
struct aiocb kbcbuf;

void setup();
void wrapup();
int login();
int getOp();
void setup_aio_buffer();
void download_obj(int signum);
void process_input(int signum);

int main(int ac, char *av[]) {
    setup();

    /* 登录 */
    id = login();
    if(id == -1) {
        fprintf(stderr, "login failed\n");
        exit(1);
    }

    /* 初始化定时器 */
    signal(SIGALRM, download_obj);
    set_ticker(1000 / TICKS_PER_SEC);

    /* 设置异步IO */
    signal(SIGIO, process_input);
    setup_aio_buffer();
    aio_read(&kbcbuf);

    while(id != -1) {
        pause();
    }

    wrapup();
    return 0;
}

void setup() {
    /* 设置区域 */
    setlocale(LC_ALL, "");

    /* 初始化curses */
    initscr();
    clear();
    noecho();
    crmode();

    /* 画边界 */
    addEdge();

    /* 初始化食物 */
    food = newFood();
}

void wrapup() {
    int i;
    if(food) { deleteFood(food); }
    for(i = 0; i < MAX_SNAKE; i++) {
        if(snakes[i]) { deleteSnake(snakes[i]); }
    }
    set_ticker(0);
    endwin();
}

/* 登录
 * 若成功返回id，失败返回-1
 */
int login() {
    int fd;

    fd = connect_to_server(HOST, LOGIN_PORT);
    if(fd == -1) {
        error_exit("connect to server failed", 2);
    }

    FILE *fp = fdopen(fd, "r");
    int id;
    fscanf(fp, "%d", &id);
    fclose(fp);
    return id;
}

void download_obj(int signum) {
    signal(SIGALRM, SIG_IGN);

    FILE *fp;
    char buf[BUFSIZ] = { 0 };
    int fd, x, y, i;

    /* 清空上次的屏幕 */
    addFood(food, BLANK);
    for(i = 0; i < MAX_SNAKE; i++) {
        if(snakes[i]) {
            addSnake(snakes[i], BLANK);
            deleteSnake(snakes[i]);
            snakes[i] = NULL;
        }
    }

    /* 连接到服务器 */
    fd = connect_to_server(HOST, OBJ_PORT);
    if(fd == -1) {
        error_exit("connect to server failed", 2);
    }
    fp = fdopen(fd, "r");

    /* 读取食物 */
    fgets(buf, BUFSIZ, fp);
    sscanf(buf, "%d %d", &x, &y);
    if(x != -1) {
        if(! food->pos) {
            food->pos = newPoint(x, y);
        } else {
            food->pos->x = x;
            food->pos->y = y;
        }
    } else if(food->pos) {
        /* 若服务器告知食物消失，则将食物pos删除
            注意这个色块会被蛇覆盖 */
        deletePoint(food->pos);
        food->pos = NULL;
    }
    addFood(food, WHITE_SYMBOL);

    /* 读取蛇体 */
    while(fgets(buf, BUFSIZ, fp)) {
        int offset, cur_id;
        char *bufp = buf;
        sscanf(bufp, "%d%n", &cur_id, &offset);
        bufp += offset;
        snakes[cur_id] = newSnake();
        while(sscanf(bufp, "%d %d%n", &x, &y, &offset) == 2) {
            enqueueHead(snakes[cur_id], newPoint(x, y));
            bufp += offset;
        }
        deleteBody(dequeueTail(snakes[cur_id]));
        deleteBody(dequeueTail(snakes[cur_id]));
        addSnake(snakes[cur_id], WHITE_SYMBOL);
    }

    fclose(fp);

    /* 蛇死了 */
    if(! snakes[id]) {
        id = -1;
    }

    signal(SIGALRM, download_obj);
}

void process_input(int signum) {
    int fd;
    int isDir = 1;
    int cmd;
    char *cp = (char*) kbcbuf.aio_buf;
    int op;
    
    if(aio_error(&kbcbuf) != 0) {
        error_exit("reading failed", 4);
    } else if(aio_return(&kbcbuf) == 0) {
        aio_read(&kbcbuf);
        return ;
    }

    /* 处理输入 */
    op = *cp;
    switch(op) {
    case 'a':
    case 'A': cmd = LEFT; break;
    case 'd':
    case 'D': cmd = RIGHT; break;
    case 'w':
    case 'W': cmd = UP; break;
    case 's':
    case 'S': cmd = DOWN; break;
    case 27: cmd = 27; break;
    default: isDir = 0; break;
    }

    /* 将按键送往server */
    if(isDir) {
        fd = connect_to_server(HOST, OP_PORT);
        if(fd == -1) {
            error_exit("connect to server failed", 2);
        }
        FILE *fp = fdopen(fd, "w");
        fprintf(fp, "%d %d", id, cmd);
        fclose(fp);
    }
    aio_read(&kbcbuf);
}

void setup_aio_buffer() {
    static char input[1];

    kbcbuf.aio_fildes = 0;
    kbcbuf.aio_buf = input;
    kbcbuf.aio_nbytes = 1;
    kbcbuf.aio_offset = 0;

    kbcbuf.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
    kbcbuf.aio_sigevent.sigev_signo = SIGIO;
}

