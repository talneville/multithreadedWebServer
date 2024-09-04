#include <stdlib.h>
#include <stdio.h>
#include "mylist.h"
#include "ourSystem.h"
#include "sys/time.h"
#include "sys/types.h"

#define FAIL -2


struct System_t {
    myList waiting_list;
    myList running_list;
    int max_num_requests;
    SchedAlg alg;
};

char* list_success = "LIST_SUCCESS";
char* list_out_of_memory = "LIST_OUT_OF_MEMORY";
char* list_null_argument = "LIST_NULL_ARGUMENT";
char* list_bad_argument = "LIST_BAD_ARGUMENT";
char* list_element_does_not_exists = "LIST_ELEMENT_DOES_NOT_EXISTS";
char* list_element_already_exists = "LIST_ELEMENT_ALREADY_EXISTS";
char* list_error = "LIST_ERROR";
char* a_really_bad_bug = "REALLY_BAD_ERROR - JUST_GIVE_UP";

static char* ListResult_To_String(ListResult res) {
    switch (res)  {
        case LIST_SUCCESS :
            return list_success;
        case LIST_OUT_OF_MEMORY :
            return list_out_of_memory;
        case LIST_NULL_ARGUMENT :
            return list_null_argument;
        case LIST_BAD_ARGUMENT :
            return list_bad_argument;
        case LIST_ELEMENT_DOES_NOT_EXISTS : 
            return list_element_does_not_exists;
        case LIST_ELEMENT_ALREADY_EXISTS : 
            return list_element_already_exists;
        case LIST_ERROR: 
            return list_error;
        default:
            return a_really_bad_bug;
    }
}


char* sys_success = "SYS_SUCCESS";
char* sys_out_of_memory = "SYS_OUT_OF_MEMORY";
char* sys_null_argument = "SYS_NULL_ARGUMENT";
char* sys_bad_argument = "SYS_BAD_ARGUMENT";
char* sys_element_does_not_exists = "SYS_ELEMENT_DOES_NOT_EXISTS";
char* sys_element_already_exists = "SYS_ELEMENT_ALREADY_EXISTS";
char* sys_error = "SYS_ERROR";
char* sys_really_bad_bug = "GO_TO_SYS, SYS == SLEEP";

char* SysResult_To_String(SystemResults res) {
    switch (res)  {
        case SYS_SUCCESS :
            return sys_success;
        case SYS_OUT_OF_MEMORY :
            return sys_out_of_memory;
        case SYS_NULL_ARGUMENT :
            return sys_null_argument;
        case SYS_BAD_ARGUMENT :
            return sys_bad_argument;
        case SYS_ELEMENT_DOES_NOT_EXISTS : 
            return sys_element_does_not_exists;
        case SYS_ELEMENT_ALREADY_EXISTS : 
            return sys_element_already_exists;
        case SYS_ERROR: 
            return sys_error;
        default:
            return sys_really_bad_bug;
    }
}


static SystemResults ListResult_To_SysResult(ListResult res) {
    switch (res)  {
        case LIST_SUCCESS :
            return SYS_SUCCESS;
        case LIST_OUT_OF_MEMORY :
            return SYS_OUT_OF_MEMORY;
        case LIST_NULL_ARGUMENT :
            return SYS_NULL_ARGUMENT;
        case LIST_BAD_ARGUMENT :
            return SYS_BAD_ARGUMENT;
        case LIST_ELEMENT_DOES_NOT_EXISTS : 
            return SYS_ELEMENT_DOES_NOT_EXISTS;
        case LIST_ELEMENT_ALREADY_EXISTS : 
            return SYS_ELEMENT_ALREADY_EXISTS;
        case LIST_ERROR: 
            return SYS_ERROR;
        default:
            return SYS_ERROR;
    }
}

System systemCreate(SchedAlg alg, int max_num_requests) {
    if(alg != BLOCK && alg != DT && alg != DH && alg != RANDOM) {
        return NULL;
    }

    if(max_num_requests <= 0 ) {
        return NULL;
    }

    System system = (System)malloc(sizeof(*system));
    if(system == NULL) {
        return NULL;
    }
    
    system->waiting_list = ListCreate();
    if(system->waiting_list == NULL) {
        free(system);
        return NULL;
    }

    system->running_list = ListCreate();
    if(system->running_list == NULL) {
        free(system);
        free(system->waiting_list);
        return NULL;
    }

    system->alg = alg;
    system->max_num_requests = max_num_requests;

    return system;
}

/* adds request to waiting list */
SystemResults systemAddRequest(System system, int fd, struct timeval arrival_time) {
    if(system == NULL) {
        return SYS_NULL_ARGUMENT;
    }
    ListResult res = insert(system->waiting_list, fd, arrival_time);
    if(res != LIST_SUCCESS){
        return ListResult_To_SysResult(res);
    }
    return SYS_SUCCESS;
}

int systemDoRequest(System system){
    if(system == NULL) {
        return SYS_NULL_ARGUMENT;
    }

    int fd = getFdByIndex(system->waiting_list, 1);
    struct timeval arrival = getArrivalTimeByIndex(system->waiting_list, 1);
    removeByIndex(system->waiting_list, 1);
  
    if(fd == FAIL){
        return SYS_ERROR;
    }
    ListResult res = insert(system->running_list, fd, arrival);
    if(res != LIST_SUCCESS){
       // printf("something went wrong: in insert %s", ListResult_To_String(res));
        return ListResult_To_SysResult(res);
    }
    
   // printf("running list:\n");
 //   printNodes(system->running_list);
    return fd;
}

SystemResults systemFinishRequest(System system, int fd){
    if(system == NULL) {
        return SYS_ERROR;
    }
 //   printf("running list (before remove):\n");
   // printNodes(system->running_list);
    
    ListResult res = removeByFd(system->running_list, fd);
    if(res != LIST_SUCCESS){
    //    printf("something went wrong: in removeByFd %s", ListResult_To_String(res));
        return ListResult_To_SysResult(res);
    }
    
  //  printf("running list:\n");
    //printNodes(system->running_list);
    return SYS_SUCCESS;
}

bool systemNoRequests(System system) {
    if(system == NULL) {
        return false;
    }
    return listIsEmpty(system->waiting_list);
}

bool systemFull(System system) {
    if(system == NULL) {
        return false;
    }
    return (listSize(system->waiting_list) + listSize(system->running_list) >= system->max_num_requests);
}

void systemDestroy(System system) {
    if(system == NULL) {
        return;
    }
    ListDestroy(system->waiting_list);
    ListDestroy(system->running_list);
}

SchedAlg systemGetAlgorithm(System system) {
  /*
    if(system == NULL) {
        return
    }
*/
  return system->alg;
}

int dropLastUnhandledRequest(System system) {
    if(system == NULL) {
        return FAIL;
    }
    int fd = getFdByIndex(system->waiting_list, 1);
    removeByIndex(system->waiting_list, 1);
    return fd;
}

int systemSize(System system) {
    return listSize(system->waiting_list) + listSize(system->running_list);
}

int systemWaitingRequestsSize(System system) {
    return listSize(system->waiting_list);
}

SystemResults systemRemoveByIndex(System system, int index) {
    return ListResult_To_SysResult(removeByIndex(system->waiting_list, index));
}

int systemGetByIndexPendingRequests(System system, int index) {
    return getFdByIndex(system->waiting_list, index);
}
struct timeval systemGetArrivalTimeByFd(System system, int fd) {
    return getArrivalTimeByFd(system->running_list, fd);
}