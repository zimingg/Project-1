#include <unistd.h>
#include <stdlib.h>
#include "p1fxns.h"
#include <string.h>
#include <errno.h>

int main(){
    int i;
    for (i = 0; i < 10; i++){
        
        sleep(1);
        printf("out : %d\n", i);
    }
    
    
}
