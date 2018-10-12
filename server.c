#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "snake.h"
#include "socklib.h"
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>

#define LOGIN_PORT 2222 /* 用户登录端口 */
#define OP_PORT 1234 /* 传输用户的操作 */
#define OBJ_PORT 4321 /* 传输蛇和食物对象 */
#define error_exit(m, x) { perror(m); exit(x); }
#define MAX_SNAKE 3 /* 登太多会溢出的 */

Food *food = NULL;
Snake *snakes[MAX_SNAKE] = { 0 };
pthread_mutex_t food_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t snakes_lock = PTHREAD_MUTEX_INITIALIZER;

void setup(void);
void wrapup(void);
void update(int signum);
void *handle_login(void *s);
void *handle_op(void *s);
void *handle_obj(void *s);
void process_login(int fd);
void process_op(int fd);
void process_obj(int fd);

int main(int ac, char *av[]) {
    int login_sock, op_sock, obj_sock;
    pthread_t login_thread, op_thread, obj_thread;

    /* 初始化工作 */
    setup();

    /* 创建服务器socket */
    if((login_sock = make_server_socket(LOGIN_PORT)) == -1) {
        error_exit("make server socket failed", 2);
    }
    if((op_sock = make_server_socket(OP_PORT)) == -1) {
        error_exit("make server socket failed", 2);
    }
    if((obj_sock = make_server_socket(OBJ_PORT)) == -1) {
        error_exit("make server socket failed", 2);
    }

    pthread_create(&login_thread, NULL, handle_login, &login_sock);
    pthread_create(&op_thread, NULL, handle_op, &op_sock);
    pthread_create(&obj_thread, NULL, handle_obj, &obj_sock);

    pthread_join(login_thread, NULL);
    pthread_join(op_thread, NULL);
    pthread_join(obj_thread, NULL);

    /* 收尾工作 */
    wrapup();
}

void setup() {
    food = newFood();

    signal(SIGALRM, update);
    set_ticker(1000 / TICKS_PER_SEC);
}

void wrapup() {
    int id;
    for(id = 0; id < MAX_SNAKE; id++) {
        if(snakes[id]) {
            deleteSnake(snakes[id]);
        }
    }
    deleteFood(food);
    set_ticker(0);
}

void update(int signum) {
    int id, ob;
    signal(SIGALRM, SIG_IGN);

    pthread_mutex_lock(&food_lock);
    pthread_mutex_lock(&snakes_lock);

    //puts("update locked");
    
    food_locate(food);
    for(id = 0; id < MAX_SNAKE; id++) {
        for(ob = 0; snakes[id] && ob < MAX_SNAKE; ob++) {
            if(snakes[ob] && snake_hit_snake(snakes[id], snakes[ob])) {
                printf("碰撞%d->%d\n", id, ob);
                deleteSnake(snakes[id]);
                snakes[id] = NULL;
            }
        }

        if(snakes[id]) {
            snake_eat_food(snakes[id], food);
            snake_move(snakes[id]);
        }
    }

    //puts("update unlocked");
    pthread_mutex_unlock(&food_lock);
    pthread_mutex_unlock(&snakes_lock);

    signal(SIGALRM, update);
}

void *handle_login(void *s) {
    int sock = * (int*) s;
    int fd;
    while(1) {
        fd = accept(sock, NULL, NULL);
        if(fd == -1) {
            error_exit("accept socket", 3);
        }
        process_login(fd);
        close(fd);
    }
}
void *handle_op(void *s) {
    int sock = * (int*) s;
    int fd;
    while(1) {
        fd = accept(sock, NULL, NULL);
        if(fd == -1) {
            error_exit("accept socket", 3);
        }
        process_op(fd);
        close(fd);
    }
}
void *handle_obj(void *s) {
    int sock = * (int*) s;
    int fd;
    while(1) {
        fd = accept(sock, NULL, NULL);
        if(fd == -1) {
            error_exit("accept socket", 3);
        }
        process_obj(fd);
        close(fd);
    }
}

/* 初始化蛇，并将最新的蛇id返回 */
void process_login(int fd) {
    int id = -1;
    FILE *fp = fdopen(fd, "w");

    pthread_mutex_lock(&snakes_lock);
    // puts("process_login locked");

    /* 找出下一个可用id */
    while(snakes[++id]);
    /* 分配空间并传回其id */
    snakes[id] = newSnake();

    // puts("process_login unlocked");
    pthread_mutex_unlock(&snakes_lock);

    fprintf(fp, "%d", id);
    printf("login: %d\n", id);
    fclose(fp);
}
/* 读取操作，并调整对应的蛇
 * id dir
 */
void process_op(int fd) {
    FILE *fp = fdopen(fd, "r");
    int id;
    int cmd;
    fscanf(fp, "%d %d", &id, &cmd);
    // printf("snake %d move: %d\n", id, newDir);
    fclose(fp);

    pthread_mutex_lock(&snakes_lock);
    // puts("process_op locked");

    /* 接受命令（调整方向/退出），为什么要判断是否存在呢，
        是因为服务端蛇死亡后，客户端可能来不及反应 */
    if(snakes[id]) {
        if(cmd == 27) {
            printf("%d号蛇退出\n", id);
            deleteSnake(snakes[id]);
            snakes[id] = NULL;
        } else {
            snake_redir(snakes[id], cmd);
        }
    }

    // puts("process_op unlocked");
    pthread_mutex_unlock(&snakes_lock);
}
/* 接受请求，返回食物和蛇的数据
 * 第一行为食物，之后的是蛇，比如：
 * x y
 * id x y x y x y
 * id x y x y
 */
void process_obj(int fd) {
    FILE *fp = fdopen(fd, "w");
    if(food->pos) {
        fprintf(fp, "%d %d\n", food->pos->x, food->pos->y);
    } else {
        fprintf(fp, "-1 -1\n");
    }

    pthread_mutex_lock(&food_lock);
    pthread_mutex_lock(&snakes_lock);
    // puts("process_obj locked");

    /* 蛇体 */
    int id;
    for(id = 0; id < MAX_SNAKE; id++) {
        if(snakes[id]) {
            fprintf(fp, "%d", id);
            Body *head = snakes[id]->head, 
                    *next = snake_body_first(snakes[id]);
            fprintf(fp, " %d %d", head->pos->x, head->pos->y);
            while(next) {
                fprintf(fp, " %d %d", next->pos->x, next->pos->y);
                head = next;
                next = head->next;
            }
            fprintf(fp, "\n");
        }
    }
    fclose(fp);

    // puts("process_obj unlocked");
    pthread_mutex_unlock(&food_lock);
    pthread_mutex_unlock(&snakes_lock);
}


