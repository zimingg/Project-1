

typedef struct node Node;
typedef struct task_queue TaskQueue;




TaskQueue * Create();
int addProc(TaskQueue *tq, void * poc);
void* deleteProc(TaskQueue *tq);
