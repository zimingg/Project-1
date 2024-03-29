/*
 authorship statement:
 
 Name: Ziming Guo
 DuckID: zimingg
 Title of the assignment: CIS 415 Project 1
 
 This is my own work.
 
 */



#include "p1fxns.h"
//#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/time.h>


#define STRING_LENGTH 1000
#define COMMAND_LIST_LENGTH 300

int i;
int command_length;
int wakeup_sig_got = 0;

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
            ((Process*) p_list+i)->id = proc;  //should be proc
            ((Process*) p_list+i)->status = -1;
        }
        else{
            //child
            
            while(!wakeup_sig_got){
                sleep(1);
            }
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


int send_signal(int np, int signal, Process* p_list){
    
    for (i = 0; i < np; i++ ){
        kill(((Process*)p_list+i)->id,signal);
    }

    
    return 0;
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
    
    InputInfo *input = parseInputData(argc, argv);
    if(input == NULL){
        return 0;
    }
    int np = input->nprocesses;
    
    char ** words = get_words(input);
    if(words == NULL){
        free(input);
        input = NULL;
        return 0;
    }
    
    Process* p_list = (Process*) malloc(sizeof(Process)*(input->nprocesses));
    if(p_list == NULL){
        free(input);
        input = NULL;
        free_words(words);
        return 0;
    }
    
    
    
    struct timeval t1;
    struct timeval t2;
    gettimeofday(&t1, 0);
    
    
    fork_and_create_processes(p_list, np, words);
    p1putstr(1, "send USER1\n");
    send_signal(np, SIGUSR1, p_list);
    p1putstr(1, "send stop\n");
    send_signal(np, SIGSTOP, p_list);
    p1putstr(1, "send cont\n");
    send_signal(np, SIGCONT, p_list);
    wait_for_process(np);
    
    
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
    
    
    
    return 0;
}
