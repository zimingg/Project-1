#include "p1fxns.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#define STRING_LENGTH 300
#define COMMAND_LIST_LENGTH 100

int i;
int command_length;

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
        p1perror(1, "No value! Fail!\n");
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
            printf("p: %d \n", getpid());
            ((Process*) p_list+i)->id = getpid();  //should be proc
            ((Process*) p_list+i)->status = -1;
        }
        else{
            //child
            execvp(*words, words);
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

int main(int argc, const char * argv[]) {
    
    
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
    wait_for_process(np);
   
    
    gettimeofday(&t2, 0);
    
    unsigned long elapsed = (t2.tv_sec-t1.tv_sec)*1000000 + t2.tv_usec-t1.tv_usec; //It counts microseconds.
    printf("!!: %lu \n",elapsed);
    
    
    
    
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
    

    
    //printf("argc is %d, the first one is %s \n",input->quantum,p);
    return 0;
}
