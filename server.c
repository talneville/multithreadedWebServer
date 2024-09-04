#include "segel.h"
#include "request.h"
#include "ourSystem.h"
#include "assert.h"

/*
 * server.c: A very, very simple web server
 * To run:  ./server <portnum (above 2000)>
 *
 * Repeatedly handles HTTP requests sent to this port number.
 * Most of the work is done within routines written in request.c
*/


/* _____________DECLARATIONS______________ */
pthread_mutex_t mutex_lock;
pthread_cond_t consumer_allowed;
pthread_cond_t producer_allowed;
// our variable will be the num of requests
// that are allowed in the queue - systemFull() func

typedef struct args {
    int port;
    System system;
}Args;




/* _________________FUNCTIONS________________*/

/* this function parse arguments given in main */
void getargs(int *port, int argc, char *argv[], int* num_of_threads, int* max_num_requests, SchedAlg* alg)
{
    if (argc < 5) {
        fprintf(stderr, "Usage: %s <port> <num_of_threads> <max_num_requests> <SchedAlg>\n", argv[0]);
        exit(1);
    }
    *port = atoi(argv[1]);
    *num_of_threads = atoi(argv[2]);
    *max_num_requests = atoi(argv[3]);
    char* sched_alg = argv[4];
    if(strcmp(sched_alg, "block") == 0) {
        *alg = BLOCK;
    }
    else if (strcmp(sched_alg, "dt") == 0) {
        *alg = DT;
    }
    else if (strcmp(sched_alg, "dh") == 0) {
        *alg = DH;
    }
    else if (strcmp(sched_alg, "random") == 0) {
        *alg = RANDOM;
    }
    else {
        printf("sched alg doesnt exist!");
    }
}

/*
 * the next set of functions are the specific handlers
 * for each algorithm chosen for the system
 */
void blockAlgorithm(System system) {
    assert(system);
    while(systemFull(system)) {
        pthread_cond_wait(&producer_allowed, &mutex_lock);
    }
}


void dhAlgorithm(System system, int conn_fd) {
    assert(system);
    if(systemWaitingRequestsSize(system) == 0) {
        Close(conn_fd);
        return;
    }
    int fd = dropLastUnhandledRequest(system);
    assert(fd >= 0);
    Close(fd);
    // to close the last socket
    //Close(conn_fd);
    // the producer func adds the request to the list
}

void randomAlgorithm(System system, int conn_fd) {
    assert(system);
    int size_to_delete = systemWaitingRequestsSize(system);
    int random;

    if(size_to_delete % 10 == 0) {
        size_to_delete =  size_to_delete * 0.3;
    }
    else {
        size_to_delete = size_to_delete * 0.3 + 1;
    }

    for(int i = 0; i < size_to_delete; i++) {
        if(systemWaitingRequestsSize(system) == 0) {
            break;
        }
        random = rand() % systemWaitingRequestsSize(system) + 1;
        int fd = systemGetByIndexPendingRequests(system,random);
        assert(fd >= 0);
        Close(fd);
        //to close the fd that is being deleted
        if(systemRemoveByIndex(system, random) != SYS_SUCCESS) {
         //   printf("CANNOT REMOVE BY INDEX ERRORRRRRRRR\n");
        }
    }

}

/*
 * this function produces requests into the waiting queue,
 * if there is more requests than the max_num_request
 * this func (the main thread) will give up the lock and wait
 */
void* producer(void* args) {
    int listen_fd = Open_listenfd(((Args*)args)->port);
    int client_len;
    int conn_fd;
    struct sockaddr_in client_addr;
    struct timeval arrival_time;

    SchedAlg alg = systemGetAlgorithm(((Args*)args)->system);

    while(1) {
        client_len = sizeof(client_addr);
        conn_fd = Accept(listen_fd, (SA *)&client_addr, (socklen_t *) &client_len);

        gettimeofday(&arrival_time, NULL);

        pthread_mutex_lock(&mutex_lock);
        if(systemFull(((Args*)args)->system) == true) {
            if(alg == BLOCK) {
                blockAlgorithm(((Args*)args)->system);
               // gettimeofday(&arrival_time, NULL);
                SystemResults res = systemAddRequest(((Args*)args)->system, conn_fd, arrival_time);
                if(res != SYS_SUCCESS) {
                 //   printf("something went wrong: in finish %s", SysResult_To_String(res));
                    //Close(listen_fd);
                   // exit(res);
                }
            }
            else if(alg == DT) {
             //   printf("producer CLOSE\n");
                Close(conn_fd);
            }
            else if(alg == RANDOM) {
                if(systemWaitingRequestsSize(((Args*)args)->system) == 0) {
                    Close(conn_fd);
                }
                else {
                    randomAlgorithm(((Args*)args)->system, conn_fd);
                   // gettimeofday(&arrival_time, NULL);
                    SystemResults res = systemAddRequest(((Args*)args)->system, conn_fd, arrival_time);
                    if(res != SYS_SUCCESS) {
                        //   printf("something went wrong: in finish %s", SysResult_To_String(res));
                      //  Close(listen_fd);
                        //exit(res);
                    }
                }
            }
            else if(alg == DH) {
                if(systemWaitingRequestsSize(((Args*)args)->system) == 0) {
                    Close(conn_fd);
                }
                else {
                    dhAlgorithm(((Args*)args)->system, conn_fd);
                   // gettimeofday(&arrival_time, NULL);
                    SystemResults res = systemAddRequest(((Args*)args)->system, conn_fd, arrival_time);
                    if(res != SYS_SUCCESS) {
                        //  printf("something went wrong: in finish %s", SysResult_To_String(res));
                       // Close(listen_fd);
                       //exit(res);
                    }
                }
            }
        }
        else {
          //  gettimeofday(&arrival_time, NULL);
            SystemResults res = systemAddRequest(((Args*)args)->system, conn_fd, arrival_time);
            if(res != SYS_SUCCESS) {
            //    printf("something went wrong: in finish %s", SysResult_To_String(res));
                //Close(listen_fd);
                //exit(res);
            }
        }

        // block after add and when the queue is full
        pthread_mutex_unlock(&mutex_lock);
        pthread_cond_signal(&consumer_allowed);
    }
   // Close(listen_fd);
    return NULL;
}

/*
void* producer(void* args) {
    int listen_fd = Open_listenfd(((args_t*)args)->port);
    int client_len;
    int conn_fd;
    struct sockaddr_in client_addr;
    bool to_add;

    while(1) {
        client_len = sizeof(client_addr);
        conn_fd = Accept(listen_fd, (SA *)&client_addr, (socklen_t *) &client_len);
        to_add = true;

        pthread_mutex_lock(&mutex_lock);
        if(systemFull(((args_t*)args)->system) == true) {
            algorithmHandler(((args_t*)args)->system, conn_fd);
            to_add = toAddRequest(((args_t*)args)->system);
        }

        // if DT no insert to the list!
        if(to_add) {
            SystemResults res = systemAddRequest(((args_t*)args)->system, conn_fd);
            if(res != SYS_SUCCESS) {
                printf("something went wrong: in finish %s", SysResult_To_String(res));
                exit(res);
            }
        }
        else {
            Close(conn_fd);
        }

        // block after add and when the queue is full
        pthread_mutex_unlock(&mutex_lock);
        pthread_cond_signal(&consumer_allowed);
        to_add = true;
    }
    return NULL;
}
*/


/*
void* producer(void* args) {
    int listen_fd = Open_listenfd(((args_t*)args)->port);
    int client_len;
    int conn_fd;
    struct sockaddr_in client_addr;
    int flag = false;

    while(1) {

        client_len = sizeof(client_addr);
        conn_fd = Accept(listen_fd, (SA *)&client_addr, (socklen_t *) &client_len);

        pthread_mutex_lock(&mutex_lock);
        if(systemFull(((args_t*)args)->system) == true) {
            algorithmHandler(((args_t*)args)->system, conn_fd);
            if(systemGetAlgorithm(((args_t*)args)->system) == DT ){
                flag = true;
            }
            if(systemGetAlgorithm(((args_t*)args)->system) == DH && systemNoRequests(((args_t*)args)->system)){
                flag = true;
                Close(conn_fd);
            }
        }

        if(flag == false) {
            SystemResults res = systemAddRequest(((args_t*)args)->system, conn_fd);
            if(res != SYS_SUCCESS) {
                printf("something went wrong: in finish %s", SysResult_To_String(res));
                exit(res);
            }
        }
        flag = false;

        // block after add and when the queue is full
        pthread_mutex_unlock(&mutex_lock);
        pthread_cond_signal(&consumer_allowed);
    }
    return NULL;
}
*/

/* this function consumes requests and take them out of the running queue */
void* consumer(void* system) {
    assert((System)system);
    Stats stats;
    stats.total_requests_counter = 0;
    stats.dynamic_counter = 0;
    stats.static_counter = 0;

    while(1) {
        int fd;
        pthread_mutex_lock(&mutex_lock);
        while(systemNoRequests((System)system)) {
            pthread_cond_wait(&consumer_allowed, &mutex_lock);
        }
        //remove first from waiting list and adds to running list
        fd = systemDoRequest((System)system);
        if(fd < 0){
         //   printf("ERROORRRR FD _ DO REQUEST - %d", fd);
        }
        struct timeval arrival =  systemGetArrivalTimeByFd(system, fd);
        pthread_mutex_unlock(&mutex_lock);


        requestHandle(fd, arrival, &stats);

        pthread_mutex_lock(&mutex_lock);
        //remove from list
        SystemResults res = systemFinishRequest((System)system, fd);
        if(res != SYS_SUCCESS){
          //  printf("something went wrong: in finish %s", SysResult_To_String(res));
            exit(res);
        }
      //  printf("consumer CLOSE\n");
        Close(fd);
        pthread_mutex_unlock(&mutex_lock);
        pthread_cond_signal(&producer_allowed);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    int port;
    int num_of_threads;
    int max_num_requests;
    SchedAlg alg;

    getargs(&port, argc, argv, &num_of_threads,
            &max_num_requests, &alg);

    /* initialize mutex_lock and cond */
    pthread_mutex_init(&mutex_lock, NULL);
    pthread_cond_init(&consumer_allowed, NULL);
    pthread_cond_init(&producer_allowed, NULL);

    /* initialize lists and system struct */
    System system = systemCreate(alg, max_num_requests);
    if(system == NULL) {
        perror("Failed to allocate\n");
        return 0;
    }

    /* Create threads (consumers) */
    pthread_t* thread_array = malloc(sizeof(pthread_t) * num_of_threads);
    if(thread_array == NULL) {
        perror("Failed to allocate\n");
        return 0;
    }
    for (int i = 0; i < num_of_threads; i++) {
        if(pthread_create(&thread_array[i], NULL, &consumer, (void *)system) != 0) {
            perror("Failed to create thread\n");
            systemDestroy(system);
            free(thread_array);
            return 0;
        }
    }

    /* producer function */
    Args args_for_producer;
    args_for_producer.port = port;
    args_for_producer.system = system;
    producer((void*)&args_for_producer);

    /* join threads */
    for (int i = 0; i < num_of_threads; i++) {
        if (pthread_join(thread_array[i], NULL) != 0) {
            perror("Failed to join thread\n");
            systemDestroy(system);
            free(thread_array);
            return 0;
        }
    }

    /* destroy */
    pthread_mutex_destroy(&mutex_lock);
    pthread_cond_destroy(&consumer_allowed);
    pthread_cond_destroy(&producer_allowed);
    systemDestroy(system);
    free(thread_array);
    return 0;
}


    


 
