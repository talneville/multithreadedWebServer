#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "mylist.h"
#include "sys/types.h"
#include <sys/time.h>


/* this list is a specific list that was made for this assignment */

/* ________DECLARATIONS_______ */
#define INIT -1
#define FAIL -2
#define FINISH -3
#define LIST_EMPTY -4

typedef struct node_t {
    int fd;
    struct timeval arrival_time;
    struct node_t* prev;
    struct node_t* next;
} *Node;

struct list_t {
    int size;
    Node first;
    Node last;
    Node iterator;
};

/*____________________list__functions______________________*/

myList ListCreate(){
    myList list = malloc(sizeof(*list));
    if (list == NULL) {
        return NULL;
    }
    list->size = 0;
    list->iterator = NULL;

    // create first node
    list->first = malloc(sizeof(struct node_t));
    if(list->first == NULL){
        free(list);
        return NULL;
    }
    list->first->fd = INIT;
    list->first->arrival_time.tv_sec = 0;
    list->first->arrival_time.tv_usec = 0;


    list->last = malloc(sizeof(struct node_t));
    if(list->last == NULL){
        free(list->first);
        free(list);
        return NULL;
    }
    list->last->fd = INIT;
    list->last->arrival_time.tv_sec = 0;
    list->last->arrival_time.tv_usec = 0;

    //pointer games
    list->first->prev = NULL;
    list->first->next = list->last;
    list->last->prev = list->first;
    list->last->next = NULL;    

    return list;
}

/* 
    new func - static - delete node
    in this implementation there always first and last - therefor prev and next will be allocated
    this function WILL NOT delete first and last node.
    returns: a pointer to the next node after the one deleted
*/
static Node deleteNode(myList list, Node to_delete){
    if(list == NULL || to_delete == NULL){
        return NULL;
    }
    if(to_delete == list->first || to_delete == list->last){
        return NULL;
    }

    /* if the next is NULL it means that this is the first or last node */
    assert(to_delete->prev != NULL);
    assert(to_delete->next != NULL);

    Node prev = to_delete->prev;
    Node next = to_delete->next;
    
    prev->next = next;
    next->prev = prev;
    free(to_delete);

    return next;
}

void ListDestroy(myList list){
    if(list == NULL){
        return;
    }
    Node temp = list->first->next;
    while(temp){
        temp = deleteNode(list, temp);
        list->size--;
    }
    free(list->first);
    free(list->last);
    free(list);
}

/* this function wil remove the first Node and return its fd 
   returns -2 on error */

int dequeue(myList list){
    if(list == NULL) {
        return FAIL;
    }
    if(listIsEmpty(list)) {
        return FAIL;
    }
  //  int fd = list->first->next;
    Node to_delete = list->first->next;
    int fd = to_delete->fd;
    deleteNode(list, to_delete);
    list->size--;
    return fd;
}


static Node createNode(int fd, struct timeval new_time_val){
    Node node = malloc(sizeof(*node));
    if (node == NULL){
        return NULL;
    }
    node->fd = fd;
    node->arrival_time = new_time_val;
    node->prev = NULL;
    node->next = NULL;
    return node;
}

/* will insert from end because we want the list to be ordered by arrival time */
ListResult insert(myList list, int fd, struct timeval new_time_val) {
    if (list == NULL){
        return LIST_NULL_ARGUMENT;
    }
    list->iterator = NULL;
    if (fd < 0){
        return LIST_BAD_ARGUMENT;
    }
    if(contains(list,fd) == true){
        return LIST_ELEMENT_ALREADY_EXISTS;
    }
    Node new_node = createNode(fd, new_time_val);
    if (new_node == NULL){
        return LIST_OUT_OF_MEMORY;
    }
    Node prev = list->last->prev;
    prev->next = new_node;
    new_node->prev = prev;
    new_node->next = list->last;
    list->last->prev = new_node;
    list->size++;

    return LIST_SUCCESS;
}

/* returns false on error */
bool contains(myList list, int fd){
    if (list == NULL){
        return false;
    }
    if (fd < 0){
        return false;
    }
    Node current = list->first->next;
    while (current != list->last){
        if(current->fd == fd){
            return true;
        }
        current = current->next;
    }
    return false;
}

int getFdByIndex(myList list, int index){
    if (list == NULL){
        return FAIL;
    }
    if (index <= 0 || index > listSize(list)){
        return FAIL;
    }
    if(listIsEmpty(list)) {
        return LIST_EMPTY;
    }

    Node current = list->first->next;
    int i = 1;
    while (current != list->last){
        if(i == index){
            return current->fd;
        }
        current = current->next;
        i++;
    }
    return FAIL;
}

struct timeval getArrivalTimeByIndex(myList list, int index){
    struct timeval init;
    init.tv_sec = 0;
    init.tv_usec = 0;
    if (list == NULL){
        return init;
    }
    if (index <= 0 || index > listSize(list)){
        return init;
    }
    if(listIsEmpty(list)) {
        return init;
    }

    Node current = list->first->next;
    int i = 1;
    while (current != list->last){
        if(i == index){
            return current->arrival_time;
        }
        current = current->next;
        i++;
    }
    return init;
}

struct timeval getArrivalTimeByFd(myList list, int fd){
    struct timeval init;
    init.tv_sec = 0;
    init.tv_usec = 0;
    if (list == NULL){
        return init;
    }
    if (fd < 0){
        return init;
    }
    if(listIsEmpty(list)) {
        return init;
    }

    Node current = list->first->next;
    int i = 1;
    while (current != list->last){
        if(current->fd == fd){
            return current->arrival_time;
        }
        current = current->next;
        i++;
    }
    return init;
}

ListResult removeByIndex(myList list, int index){
    if (list == NULL){
        return LIST_NULL_ARGUMENT;
    }
    if (index <= 0 || index > listSize(list)){
        return LIST_BAD_ARGUMENT;
    }
    if(listIsEmpty(list)) {
        return LIST_SUCCESS;
    }

    Node current = list->first->next;
    int i = 1;
    while (current != list->last){
        if(i == index){
            deleteNode(list, current);
            list->size--;
            return LIST_SUCCESS;
        }
        current = current->next;
        i++;
    }
    assert(0);
    return LIST_ERROR;
}

ListResult removeByFd(myList list, int fd){
    if (list == NULL){
        return LIST_NULL_ARGUMENT;
    }
    if (fd < 0){
        return LIST_BAD_ARGUMENT;
    }
    Node current = list->first->next;
    while (current != list->last){
        if(current->fd == fd){
            deleteNode(list, current);
            list->size--;
            return LIST_SUCCESS;
        }
        current = current->next;
    }
    return LIST_ERROR;
}

/*____________________print______________________*/

int printNodes(myList list){
    if(list == NULL){
        return FAIL;
    }
    if(listIsEmpty(list)){
     //   printf("list is empty\n");
        return LIST_SUCCESS;
    }
   // printf("list has %d elements\n",list->size);
    Node current = list->first->next;
    int i=1;
    while (current != list->last){
      //  printf("Node %d with fd %d\n",i,current->fd);
        i++;
        current = current->next;
    }
    return LIST_SUCCESS;
}

/*____________________for__iterators______________________*/
int ListGetFirst(myList list){
    if(list == NULL){
        return FAIL;
    }
    if(listIsEmpty(list)){
        return FAIL;
    }
    list->iterator = list->first->next;    
    return list->iterator->fd;
}

int ListGetNext(myList list){
    if(list == NULL){
        return FAIL;
    }
    if(listIsEmpty(list)){
        return FAIL;
    }
    if(list->iterator == NULL){
        return FAIL;
    }
    list->iterator = list->iterator->next;
    if(list->iterator == list->last){
        return FINISH;
    }
    return list->iterator->fd;
}

/*____________________list__attributes______________________*/
int listSize(myList list){
    return list->size;
}

bool listIsEmpty(myList list){
    return listSize(list) == 0;
}

