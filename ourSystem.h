#ifndef _OUR_SYSTEM_H
#define _OUR_SYSTEM_H
#include "sys/types.h"
#include <stdbool.h>

typedef enum Algorithm_t {
    BLOCK,
    DT,
    DH,
    RANDOM
} SchedAlg;

typedef enum Sys_results_t {
    SYS_SUCCESS,
    SYS_OUT_OF_MEMORY,
    SYS_NULL_ARGUMENT,
    SYS_BAD_ARGUMENT,
    SYS_ELEMENT_DOES_NOT_EXISTS,
    SYS_ELEMENT_ALREADY_EXISTS,
    SYS_ERROR
} SystemResults;

typedef struct System_t *System;

System systemCreate(SchedAlg alg, int max_num_requests);
SystemResults systemAddRequest(System system, int fd, struct timeval arrival_time);
int systemDoRequest(System system);
SystemResults systemFinishRequest(System system, int fd);
bool systemNoRequests(System system);
bool systemFull(System system);
char* SysResult_To_String(SystemResults res);
void systemDestroy(System system);
SchedAlg systemGetAlgorithm(System system);
int dropLastUnhandledRequest(System system);
int systemSize(System system);
int systemWaitingRequestsSize(System system);
SystemResults systemRemoveByIndex(System system, int index);
int systemGetByIndexPendingRequests(System system, int index);
struct timeval systemGetArrivalTimeByFd(System system, int fd);



#endif