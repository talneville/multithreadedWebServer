#ifndef _MYLIST_H
#define _MYLIST_H

#include <stdbool.h>
#include "sys/types.h"

/*____________________structs______________________*/

typedef struct list_t* myList;

typedef enum ListResult_t {
    LIST_SUCCESS,
    LIST_OUT_OF_MEMORY,
    LIST_NULL_ARGUMENT,
    LIST_BAD_ARGUMENT,
    LIST_ELEMENT_DOES_NOT_EXISTS,
    LIST_ELEMENT_ALREADY_EXISTS,
    LIST_EMPTY,
    LIST_ERROR
} ListResult;



/*____________________list__functions______________________*/

myList ListCreate();
void ListDestroy(myList list);

// ListResult dequeue(myList list);
ListResult insert(myList list, int fd, struct timeval new_time_val);
int getFdByIndex(myList list, int index);
struct timeval getArrivalTimeByIndex(myList list, int index);
struct timeval getArrivalTimeByFd(myList list, int fd);


bool contains(myList list, int fd);
ListResult removeByIndex(myList list, int index);
ListResult removeByFd(myList list, int fd);
int printNodes(myList list);

/*____________________for__iterators______________________*/
int ListGetFirst(myList list);
int ListGetNext(myList list);

/*____________________list__attributes______________________*/
bool listIsEmpty(myList list);
int listSize(myList list);

/*!
* Macro for iterating over the list.
* Declares a new iterator for the loop.
*/
#define LIST_FOREACH(type, iterator, list) \
    for(type iterator = (type) ListGetFirst(list) ; \
        iterator >= 0 ;\
        iterator = ListGetNext(list))

#endif