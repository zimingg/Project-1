

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>



typedef struct node Node;
typedef struct task_queue TaskQueue;

typedef struct node Node;
typedef struct node {
    Node * next;
    void * data;
}Node;


typedef struct task_queue {
    int size;
    Node * node;
}TaskQueue;


TaskQueue * Create(){
    TaskQueue * new_tq = (TaskQueue*)malloc(sizeof(TaskQueue));
    if (new_tq == NULL){
        return NULL;
    }
    
    new_tq->node = NULL;
    new_tq->size = 0;
    
    return new_tq;
    
}

int addProc(TaskQueue *tq, void * poc){
    Node *node = (Node*)malloc(sizeof(Node));
    if(node == NULL){
        return 0;
    }
    node->next = NULL;
    node->data = poc;
    
    if(tq->size == 0){
        tq->node = node;
        tq->size++;
        return 1;
    }
    
    Node * head = tq->node;
    
    while(head->next!=NULL){
        head = head->next;
    }
    head->next = node;
    tq->size++;
    
    return 1;
}

void* deleteProc(TaskQueue *tq){
    if(tq->size == 0){
        return NULL;
    }
    void* poc = tq->node->data;
    tq->size--;
    Node * temp = tq->node;
    tq->node = tq->node->next;
    free(temp);
    
    return poc;
    
}
