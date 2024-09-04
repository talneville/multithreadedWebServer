#ifndef __REQUEST_H__

typedef struct stats{
    int total_requests_counter;
    int static_counter;
    int dynamic_counter;
}Stats;

void requestHandle(int fd, struct timeval arrival, Stats* stats);

#endif
