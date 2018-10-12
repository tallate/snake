#include <sys/time.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "utils.h"

int abs(int x) {
    return x > 0 ? x : -x;
}

int rand_interval(int l, int r) {
    srand(time(0));
    int delt = r - l + 1;
    return rand() % delt + l;
}

/* 设置定时器
 * interval: 毫秒单位的间隔时间
 */
int set_ticker(int interval) {
    struct itimerval timeset;

    int sec = interval / 1000;
    int msec = (interval % 1000) * 1000; /* 微秒 */
    timeset.it_interval.tv_sec = sec;
    timeset.it_interval.tv_usec = msec;
    timeset.it_value.tv_sec = sec;
    timeset.it_value.tv_usec = msec;

    return setitimer(ITIMER_REAL, &timeset, NULL);
}
