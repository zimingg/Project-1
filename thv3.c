/*
 authorship statement:
 
 Name: Ziming Guo
 DuckID: zimingg
 Title of the assignment: CIS 415 Project 1
 
 This is my own work.
 
 */


#include "p1fxns.h"
#include "taskqueue.h"
//#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/time.h>



#define STRING_LENGTH 1000
#define COMMAND_LIST_LENGTH 300

typedef struct task_queue {
    int size;
    Node * node;
}TaskQueue;


int i;
int j;
int command_length;
int wakeup_sig_got = 0;
int finished_procs = 0;
int number_in_exe_list = 0;
int number_of_proessors = 0;
int *executing_now_list;
int np;


TaskQueue* tq;


typedef struct inputInfo{
    int quantum;
    int nprocesses;
    int nprocessors;
    char command[STRING_LENGTH];
    
}InputInfo;

typedef struct process{
    pid_t id;
    int status;
    
}Process;

Process* p_list;


InputInfo* parseInputData(int argc, const char * argv[]){
    
    InputInfo *input = (InputInfo*)malloc(sizeof(InputInfo));
    //printf("  lol %d \n",input->nprocesses);
    
    
    char *p;
    int val = -1;
    
    //get environment variables.
    
    if ((p = getenv("TH_QUANTUM_MSEC")) != NULL){
        val = p1atoi(p);
        input->quantum = val;
        
    }
    if ((p = getenv("TH_NPROCESSES")) != NULL){
        val = p1atoi(p);
        input->nprocesses = val;
        
    }
    if ((p = getenv("TH_NPROCESSORS")) != NULL){
        val = p1atoi(p);
        input->nprocessors = val;
        
        
    }
    
    //get argvs
    
    for (i = 1; i < argc; i++){
        
        if (p1strneq("--quantum=",argv[i], 10) == 1){
            
            input->quantum = p1atoi((char* )argv[i] + 10);
            //printf("%s  !%d\n",argv[i], p1atoi((char* )argv[i] + 10));
        }
        
        if (p1strneq(argv[i], "--number=", 9) == 1){
            
            input->nprocesses = p1atoi((char* )argv[i] + 9);
            //printf("%s  !%d\n",argv[i], p1atoi((char* )argv[i] + 9));
        }
        
        if (p1strneq(argv[i], "--processors=", 13) == 1){
            
            input->nprocessors = p1atoi((char* )argv[i] + 13);
            //printf("%s  !%d\n",argv[i], p1atoi((char* )argv[i] + 13));
        }
        
        if (p1strneq(argv[i], "--command=", 10) == 1){
            
            
            p1strcpy(input->command,(char* )argv[i] + 10);
            //printf("%s (%s) \n",argv[i],input->command);
        }
        
        
    }
    
    
    //printf("%d  %d  %d\n",input->quantum, input->nprocesses, input->nprocessors);
    
    
    if (input->quantum == 0 || input->nprocesses == 0 || input->nprocessors == 0){
        p1perror(1, "No input value! Fail!\n");
        return NULL;
    }
    
    
    return input;
    
    
}

void free_words(char** words){
    
    for (i = 0; i < (command_length + 1); i++){
        free(words[i]);
        words[i] = NULL;
    }
    free(words);
    words = NULL;
    
}

char** get_words(InputInfo *input){
    
    
    char * command = input->command;
    //char * command = "ls -a -l";
    command_length = 1;
    char b[100];
    
    
    
    
    int a = p1getword(command, 0, b);
    if (a == -1){
        command_length = 0;
    }
    while((a = p1getword(command, a, b))!= -1){
        
        command_length++;
        //printf(" command length is %d %s \n", command_length,b);
        
    }
    //printf(" COMMAND_LENGTH is %d \n", command_length);
    
    char **words = (char**)calloc(command_length+1, sizeof(char*));
    if (words == NULL){
        return NULL;
    }
    else{
        for (i = 0; i < command_length; i++){
            words[i] = (char*)calloc(100,sizeof(char));
            if(words[i] == NULL){
                free_words(words);
                return NULL;
            }
        }
    }
    
    
    
    i = 0;
    a = p1getword(command, 0, words[0]);
    if(a!=-1){
        i++;
    }
    while((a = p1getword(command, a, words[i]))!=-1){
        i++;
    }
    words[command_length] = NULL; //words is got
    
    //printf(" yo: %s %s %s %s \n", words[0],words[1],words[2],words[3]);
    
    
    return words;
    
    
    
}

void signal_handler(int sig_get){
    
    if(sig_get == SIGUSR1){
        wakeup_sig_got = 1;
    }
}

void sigalrm_handler(int sig_get){
    
    if(sig_get == SIGALRM){
        p1putstr(1, "Alarm! Hi! \n");
        p1putstr(1, "finished procs is: \n");
        p1putint(1, finished_procs);
        p1putstr(1, "   \n");

        
        p1putstr(1, "number_in_exe_list is: \n");
        p1putint(1, number_in_exe_list);
        p1putstr(1, "   \n");
 
        
        //if privious, stop them
        if(number_in_exe_list != 0){
            //stop them
            p1putstr(1, "Start stop! \n");
            for(i = 0; i < number_in_exe_list; i++){
                if(executing_now_list[i]!=-1){
                    kill(executing_now_list[i],SIGSTOP);
                    p1putstr(1, "stoped one! \n");
                }
            }
        }
        //add items in exe list to tq
        for(i = 0; i < number_in_exe_list; i++){
            if(executing_now_list[i] != -1){
                for(j = 0; j < np; j++ ){
                    if(p_list[j].id == executing_now_list[i]){
                        addProc(tq, &p_list[j]);
                        p1putstr(1, "size of tq is: \n");
                        p1putint(1, tq->size);
                        p1putstr(1, "   \n");
                        
                    }
                }
            }
        }
 
        //reset to 0 and -1
        number_in_exe_list = 0;
        for(i = 0; i < np; i++){
            executing_now_list[i] = -1;
        }
        p1putstr(1, "Reseted!! number_in_exe_list is: \n");
        p1putint(1, number_in_exe_list);
        p1putstr(1, "   \n");

        //pick to execute!
        p1putstr(1, "pick to execute! \n");
        p1putstr(1, "number_of_proessors is: \n");
        p1putint(1, number_of_proessors);
        p1putstr(1, "   \n");

        
        for(i = 0; i < number_of_proessors; i++){
            
            Process * p = deleteProc(tq);
            if (p!=NULL){
                executing_now_list[i] = p->id;
                number_in_exe_list++;
                p1putstr(1, "pick one! \n");
            }
        }
        p1putstr(1, "pick finished! \n");
        p1putstr(1, "Start to SIGCONT! lol! \n");
        for(i = 0; i < number_in_exe_list; i++){
            kill(executing_now_list[i],SIGCONT);
            p1putstr(1, "SIGCONT one! \n");
        }
//
//        //then pick first n items in the queue and cont them, if m items in queue < n, pick m items
//        
//        //delete them from queue
//        
        
    }
    else{
         p1perror(1, "Called with Error");
    }
    
}

void sigchld_handler(int sig_get){
    
    if(sig_get == SIGCHLD){
        p1putstr(1, "child sig got!\n");
        pid_t child_pid;
        int status;
        while((child_pid = waitpid(-1, &status, WNOHANG)) > 0){
            //printf("child id is %d \n", child_pid);
            if(WIFEXITED(status)){
                //printf("child process finished!\n");
                
                
//                finished_procs++;
//                //printf("%d %d child process finished! \n", finished_procs, number_in_exe_list);
//                p1putstr(1, "child process finished!\n");
                for(i = 0; i < number_of_proessors; i++ ){
                    if(executing_now_list[i] == child_pid){
                        executing_now_list[i] = -1;
                        number_in_exe_list--;
                        
//                        break;
                    }
                    
                }
                finished_procs++;
                //printf("%d %d child process finished! \n", finished_procs, number_in_exe_list);
                p1putstr(1, "child process finished!\n");
                
            }
            
            
            
            
        }
    }
    else{
         p1perror(1, "Terminated with Error");
    }
    
    
    
    //if end, ok
    
    
    //if stop, add to the queue
    
    
}





void fork_and_create_processes(Process * p_list, int np, char ** words){
    
    pid_t proc;
    
    
    for(i = 0; i < np; i++){
        
        //fork()
        
        proc = fork();
        
        if (proc < 0){
            //fail, need to free!
            exit(0);
        }
        else if (proc > 0){
            //prarent
            //printf("Parents: %d \n", getpid());
            ((Process*) p_list+i)->id = proc;  //should be proc
            ((Process*) p_list+i)->status = -1;
        }
        else{
            //child
            //printf("childs: %d \n", getpid());
            while(!wakeup_sig_got){
                sleep(1);
            }
            //printf("childs wake up: id is %d \n", getpid());
            if(execvp(*words, words)<0){
                p1perror(1, "execvp fail\n");
            }
            exit(1);
        }
        
    }
    
}

void wait_for_process(int np){
    
    int n;
    
    for (i = 0; i < np; i++ ){
        //int n = ((Process*)p_list+i)->status;
        wait(&n);
    }
    
}


void send_signal(int np, int signal, Process* p_list){
    
    for (i = 0; i < np; i++ ){
        kill(((Process*)p_list+i)->id,signal);
    }
}


void time_print(int a, char* b,int copy, int prosors){
    int fake_a = a;
    int counter = 0;
    if(fake_a<100000){
        counter++;
        fake_a= fake_a*10;
    }
    int y = a/1000000;
    int yu = a % 1000000;
    int m = yu;
    while(m > 999){
        m = m/10;
    }
    for(i = 0; i< counter; i++){
        m = m/10;
    }
    //printf(" %d  %d %d\n",y, m, counter);
    p1putstr(1,"The elapsed time to execute ");
    p1putint(1, copy);
    p1putstr(1," copies of \"");
    p1putstr(1,b);
    p1putstr(1,"\" on ");
    p1putint(1, prosors);
    p1putstr(1," processors is ");
    char* num = (char*)malloc(sizeof(char)*100);
    char* pack = (char*)malloc(sizeof(char)*100);
    p1itoa(y,num);
    p1strpack(num, -7, ' ', pack);
    p1putstr(1,pack);
    p1putstr(1,".");
    p1itoa(m,num);
    p1strpack(num, -3, '0', pack);
    p1putstr(1,pack);
    p1putstr(1,"sec. \n");
    free(num);
    free(pack);
    num = NULL;
    pack = NULL;
    
}



int main(int argc, const char * argv[]) {
    
    
    if(signal(SIGUSR1, signal_handler) == SIG_ERR){
        p1perror(1, "signal did not handled!!\n");
        return 0;
    }
    
    if(signal(SIGALRM, sigalrm_handler) == SIG_ERR){
        p1perror(1, "signal did not handled!!\n");
        return 0;
    }
    
    if(signal(SIGCHLD, sigchld_handler) == SIG_ERR){
        p1perror(1, "signal did not handled!!\n");
        return 0;
    }
    
    
    InputInfo *input = parseInputData(argc, argv);
    if(input == NULL){
        return 0;
    }
    np = input->nprocesses;
    
    char ** words = get_words(input);
    if(words == NULL){
        free(input);
        input = NULL;
        return 0;
    }
    
    p_list = (Process*) malloc(sizeof(Process)*(input->nprocesses));
    if(p_list == NULL){
        free(input);
        input = NULL;
        free_words(words);
        return 0;
    }
    
    tq = Create();
    if(tq == NULL){
        free(input);
        input = NULL;
        free_words(words);
        free(p_list);
        p_list = NULL;
        return 0;
    }
    //printf("init tq size: %d \n", tq->size);
    p1putstr(1, "init tq size:  ");
    p1putint(1,tq->size);
    p1putstr(1, "\n");
    
    
    executing_now_list = (int*)malloc((input->nprocesses)*sizeof(int));
    for(i = 0; i < input->nprocesses; i++){
        executing_now_list[i] = -1;
    }
    
    
    number_of_proessors = input->nprocessors;
    
    struct timeval t1;
    struct timeval t2;
    gettimeofday(&t1, 0);
    
    p1putstr(1, "start fork! \n");

    fork_and_create_processes(p_list, np, words);
    for (i = 0; i < np; i++){
        executing_now_list[i] = p_list[i].id;
        number_in_exe_list++;
    }
    p1putstr(1, "number_in_exe_list is: \n");
    p1putint(1, number_in_exe_list);
    p1putstr(1, "   \n");
   
    send_signal(np, SIGUSR1, p_list);
    send_signal(np, SIGSTOP, p_list);
    //send_signal(np, SIGCONT, p_list);
    //send_signal(np, SIGSTOP, p_list);
    
    //alarm(1);
    // Setitimer
    int quant = input->quantum;
    struct itimerval itv;
    if(quant >=1000){
        
        itv.it_value.tv_sec = quant/1000;
        itv.it_value.tv_usec = (quant % 1000) * 1000;
        itv.it_interval.tv_sec = quant/1000;
        itv.it_interval.tv_usec = (quant % 1000) * 1000;
        setitimer(ITIMER_REAL, &itv, NULL);
    }
    else{
        itv.it_value.tv_sec = 0;
        itv.it_value.tv_usec = quant * 1000;
        itv.it_interval.tv_sec = 0;
        itv.it_interval.tv_usec = quant * 1000;
        setitimer(ITIMER_REAL, &itv, NULL);
    }
    
    //wait_for_process(np);
    while(finished_procs < np){    ///while
        sleep(1);
    }
    
    gettimeofday(&t2, 0);
    
    unsigned long elapsed = (t2.tv_sec-t1.tv_sec)*1000000 + t2.tv_usec-t1.tv_usec; //It counts microseconds.
    time_print(elapsed, input->command, input->nprocesses, input->nprocessors);
    
    
    //free part
    
    free(p_list);
    p_list = NULL;
    free(input);
    input = NULL;
    
    for (i = 0; i < (command_length + 1); i++){
        free(words[i]);
        words[i] = NULL;
    }
    free(words);
    words = NULL;
    free(tq);
    tq = NULL;
    free(executing_now_list);
    executing_now_list = NULL;
    
    
    return 0;
}
