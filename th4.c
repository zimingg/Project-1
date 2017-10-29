#include "p1fxns.h"
#include "taskqueue.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>



#define STRING_LENGTH 1000
#define COMMAND_LIST_LENGTH 300
#define PRINTINFO_SIZE 5024
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
        }
        if (p1strneq(argv[i], "--number=", 9) == 1){
            input->nprocesses = p1atoi((char* )argv[i] + 9);
        }
        if (p1strneq(argv[i], "--processors=", 13) == 1){
            input->nprocessors = p1atoi((char* )argv[i] + 13);
        }
        if (p1strneq(argv[i], "--command=", 10) == 1){
            p1strcpy(input->command,(char* )argv[i] + 10);
        }
    }
    if (input->quantum == 0 || input->nprocesses == 0 || input->nprocessors == 0){
        p1perror(1, "No input value! Fail!\n");
        return NULL;
    }
    return input;
}
void printInfo(pid_t pid){
    char path_status[PRINTINFO_SIZE];
    char str_pid[PRINTINFO_SIZE];
    char str_status[PRINTINFO_SIZE] = "/status";
    p1strcpy(path_status, "/proc/");
    p1itoa(pid, str_pid);
    p1strcat(path_status, str_pid);
    p1strcat(path_status, str_status);
    
    //p1putstr(1,path);
    p1putstr(1,"**************PROCESS******************");
    p1putstr(1,"\n");
    p1putstr(1,"Process's pid: ");
    p1putstr(1,str_pid);
    p1putstr(1,"\n");
    
    char path_cmdline[PRINTINFO_SIZE];
    
    char str_cmdline[PRINTINFO_SIZE] = "/cmdline";
    p1strcpy(path_cmdline, "/proc/");
    
    p1strcat(path_cmdline, str_pid);
    p1strcat(path_cmdline, str_cmdline);
    
    //p1putstr(1,path);
    
    int fd = open(path_cmdline, O_RDONLY);
    
    if(fd > 0){
        char inbuffer[100];
        p1putstr(1,"Process's ");
        p1getline(fd, inbuffer, 100);
        p1putstr(1,inbuffer);

        close(fd);
    }
    
    fd = open(path_status, O_RDONLY);
    
    if(fd > 0){

        char inbuffer[100];
        for (i=1; i<20;i++){
            p1getline(fd, inbuffer, 100);
            if (i == 1){ p1putstr(1,inbuffer);  }
            if (i == 2){  p1putstr(1,inbuffer); }
            if (i == 6){ p1putstr(1,inbuffer);  }
            if (i == 10){ p1putstr(1,inbuffer);  }
            if (i == 12){p1putstr(1,inbuffer);  }
            if (i == 13){  p1putstr(1,inbuffer); }
        }
        close(fd);
    }

    
    char path_io[PRINTINFO_SIZE];
    
    char str_io[PRINTINFO_SIZE] = "/io";
    p1strcpy(path_io, "/proc/");
    
    p1strcat(path_io, str_pid);
    p1strcat(path_io, str_io);
    
    //p1putstr(1,path);
    
    fd = open(path_io, O_RDONLY);
    
    if(fd > 0){
        char inbuffer[100];
        p1getline(fd, inbuffer, 100);
        
        //p1putstr(1,"Process' ");
        p1putstr(1,inbuffer);
        //p1putstr(1,"\n");
        p1getline(fd, inbuffer, 100);
        p1putstr(1,inbuffer);
        //p1putstr(1,"\n");
        p1getline(fd, inbuffer, 100);
        p1putstr(1,inbuffer);
        //p1putstr(1,"\n");
        p1getline(fd, inbuffer, 100);
        p1putstr(1,inbuffer);
        p1getline(fd, inbuffer, 100);
        p1putstr(1,inbuffer);
        p1getline(fd, inbuffer, 100);
        p1putstr(1,inbuffer);
        //p1putstr(1,"\n");
        //printf("%s\n", inbuffer);
        
    }
    close(fd);

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
    command_length = 1;
    char b[100];
    int a = p1getword(command, 0, b);
    if (a == -1){
        command_length = 0;
    }
    while((a = p1getword(command, a, b))!= -1){
        command_length++;
    }
    
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
    return words;
}

void signal_handler(int sig_get){
    
    if(sig_get == SIGUSR1){
        wakeup_sig_got = 1;
    }
}

void sigalrm_handler(int sig_get){
    if(sig_get == SIGALRM){
//        p1putstr(1, "Alarm! Hi! \n");
//        p1putstr(1, "finished procs is: \n");
//        p1putint(1, finished_procs);
//        p1putstr(1, "   \n");
        p1putstr(1, "number_in_exe_list is: \n");
        p1putint(1, number_in_exe_list);
        p1putstr(1, "   \n");
         p1putstr(1, "   \n");
        p1putstr(1, ">>>>>>>>>>>>>>>>>>>>>>>>NEW PAIR<<<<<<<<<<<<<<<<<<<<   \n");
         p1putstr(1, "   \n");
        int n;
        for(n=0; n<number_in_exe_list;n++){
            
            printInfo(executing_now_list[n]);
            
        }
        
        
        
        //if privious, stop them
        if(number_in_exe_list != 0){
            //stop them
            //p1putstr(1, "Start stop! \n");
            for(i = 0; i < number_in_exe_list; i++){
                if(executing_now_list[i]!=-1){
                    kill(executing_now_list[i],SIGSTOP);
                    //p1putstr(1, "stoped one! \n");
                }
            }
        }
        //add items in exe list to tq
        for(i = 0; i < number_in_exe_list; i++){
            if(executing_now_list[i] != -1){
                for(j = 0; j < np; j++ ){
                    if(p_list[j].id == executing_now_list[i]){
                        addProc(tq, &p_list[j]);
//                        p1putstr(1, "size of tq is: \n");
//                        p1putint(1, tq->size);
//                        p1putstr(1, "   \n");
                        
                    }
                }
            }
        }
        
        //reset to 0 and -1
        number_in_exe_list = 0;
        for(i = 0; i < np; i++){
            executing_now_list[i] = -1;
        }
//        p1putstr(1, "Reseted!! number_in_exe_list is: \n");
//        p1putint(1, number_in_exe_list);
//        p1putstr(1, "   \n");
        
        //pick to execute!
//        p1putstr(1, "pick to execute! \n");
//        p1putstr(1, "number_of_proessors is: \n");
//        p1putint(1, number_of_proessors);
//        p1putstr(1, "   \n");
        
        
        for(i = 0; i < number_of_proessors; i++){
            
            Process * p = deleteProc(tq);
            if (p!=NULL){
                executing_now_list[i] = p->id;
                number_in_exe_list++;
               // p1putstr(1, "pick one! \n");
            }
        }
//        p1putstr(1, "pick finished! \n");
//        p1putstr(1, "Start to SIGCONT! lol! \n");
        for(i = 0; i < number_in_exe_list; i++){
            kill(executing_now_list[i],SIGCONT);
            //p1putstr(1, "SIGCONT one! \n");
        }
        
    }
    else{
        p1perror(1, "Called with Error");
    }
    
}

void sigchld_handler(int sig_get){
    
    if(sig_get == SIGCHLD){
        //p1putstr(1, "child sig got!\n");
        pid_t child_pid;
        int status;
        while((child_pid = waitpid(-1, &status, WNOHANG)) > 0){
            if(WIFEXITED(status)){
               
                for(i = 0; i < number_of_proessors; i++ ){
                    if(executing_now_list[i] == child_pid){
                        executing_now_list[i] = -1;
                        number_in_exe_list--;
                        //                        break;
                    }
                }
                finished_procs++;
                //p1putstr(1, "child process finished!\n");
            }
        }
    }
    else{
        p1perror(1, "Terminated with Error");
    }
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
            ((Process*) p_list+i)->id = proc;
            ((Process*) p_list+i)->status = -1;
        }
        else{
            //child
            while(!wakeup_sig_got){
                sleep(1);
            }
            if(execvp(*words, words)<0){
                p1putstr(1, "execvp fail");
            }
            exit(1);
        }
    }
}

void wait_for_process(int np){
    int n;
    for (i = 0; i < np; i++ ){
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
        p1putstr(1, "signal did not handled!!");
        return 0;
    }
    if(signal(SIGALRM, sigalrm_handler) == SIG_ERR){
        p1putstr(1, "signal did not handled!!");
        return 0;
    }
    if(signal(SIGCHLD, sigchld_handler) == SIG_ERR){
        p1putstr(1, "signal did not handled!!");
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
    
    executing_now_list = (int*)malloc((input->nprocesses)*sizeof(int));
    for(i = 0; i < input->nprocesses; i++){
        executing_now_list[i] = -1;
    }
    number_of_proessors = input->nprocessors;
    
    struct timeval t1;
    struct timeval t2;
    gettimeofday(&t1, 0);
    
    //p1putstr(1, "start fork! \n");
    
    fork_and_create_processes(p_list, np, words);
    for (i = 0; i < np; i++){
        executing_now_list[i] = p_list[i].id;
        number_in_exe_list++;
    }
//    p1putstr(1, "number_in_exe_list is: \n");
//    p1putint(1, number_in_exe_list);
//    p1putstr(1, "   \n");
    
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
    
    //p1putint(1, elapsed);
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
