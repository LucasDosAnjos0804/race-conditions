#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#define FALSE 0
#define TRUE 1

#define N 2

#define SIZE_TURN sizeof(int)
#define SIZE_INTERESTED sizeof(int)*N

#define SIZE_BOOK sizeof(char)*15 

#define LOOP 5

int *turn;
int *interested;

char *book; //variável compartilhada

void err_sys(const char* X);

void enter_region(int process);

void leave_region(int process);

int
main()
{
    int pid,i;

    if ((turn       = mmap(NULL, SIZE_TURN      , PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0)) == MAP_FAILED || 
        (interested = mmap(NULL, SIZE_INTERESTED, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0)) == MAP_FAILED ||
        (book       = mmap(NULL, SIZE_BOOK      , PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0)) == MAP_FAILED
        )
        err_sys("mmap failed");

    if ((pid = fork()) < 0) err_sys("fork failed");
    else if (pid == 0) {  
        for (i = 0; i<LOOP; i++){     // loop auxiliar de tarefa
            enter_region(0);
            *book = "Era uma vez!";
            leave_region(0);
        }
    } else {
        for (i = 0; i<LOOP; i++){     // loop auxiliar de tarefa 
            enter_region(1);
            *book = "numa terra distante";
            leave_region(1);
        }
    }

    return 0;
}


void
enter_region(int process)
{
    int other;

    other = 1 - process; 
    interested[process] = TRUE;
    *turn = process;
    printf("... process %d enter_region\n",process);

    while (*turn == process && interested[other] == TRUE){
        printf("... process %d waiting!\n",process);
        // sleep(1);
        // se eh a vez do processo e hah algum outro processo interessado
        // o prcesso espera nesse loop
    }
    
    printf("... process %d passed!\n",process);

}

void
leave_region(int process)
{
    interested[process] = FALSE;
    printf("... process %d leave_region\n",process);
}

void 
err_sys(const char* x) 
{ 
    perror(x); 
    exit(1); 
}