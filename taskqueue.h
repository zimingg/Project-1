/*
 authorship statement:
 
 Name: Ziming Guo
 DuckID: zimingg
 Title of the assignment: CIS 415 Project 1
 
 This is my own work.
 
 */


typedef struct node Node;
typedef struct task_queue TaskQueue;




TaskQueue * Create();
int addProc(TaskQueue *tq, void * poc);
void* deleteProc(TaskQueue *tq);
