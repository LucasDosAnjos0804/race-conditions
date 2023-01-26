#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>

#define SIGHUP 1

#define TRUE 1
#define N 4

#define SIZE_BUFFER sizeof(int)*N
#define SIZE_COUNT sizeof(int)

void wake_up(int id);
void sighup();

void err_sys(const char* X);

int produce_item();
void insert_item(int item);
int remove_item();
void consume_item(int item);


int *buffer;

int *count;

int
main()
{
    int pid, item;

    if ((buffer = mmap(NULL, SIZE_BUFFER , PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0)) == MAP_FAILED || 
        (count  = mmap(NULL, SIZE_COUNT  , PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0)) == MAP_FAILED )
        err_sys("mmap failed");
    
    if ((pid = fork()) < 0) err_sys("fork error");
    else if (pid) { // parent / producer
        
        while (TRUE) {
            sleep(1);
            item = produce_item();
            printf("... item %d produzido\n",item);

            if (*count == N) {
                printf("... producer vai dormir\n");
                signal(SIGHUP, sighup);
                pause();
            }
            insert_item(item);
            *count = *count + 1;    
            printf("... item %d inserido no buffer\n",item);
            if (*count == 1) {
                printf("... acordar consumer\n");
                wake_up(pid);
            }
        }

    } else {        // child  / consumer

        while (TRUE) {
            sleep(1);
            if (*count == 0) {
                printf("... consumer vai dormir\n");
                signal(SIGHUP, sighup);
                pause();
            }
            item = remove_item();
            *count = *count - 1;
            printf("... item %d removido do buffer\n",item);
            if (*count == N-1) {
                printf("... acordar producer\n");
                wake_up(getppid());

            }
            consume_item(item);
            printf("... item %d consumido\n",item);
        }
    }

    return 0;

}

int
produce_item() // gera numeros aleatorios entre 1 e 100
{
    return 1+(random()%99);
}

void
insert_item(int item) // insere um item no buffer usando count como index
{
    buffer[*count] = item;
}

int
remove_item() // remove o ultimo item adicionado no buffer
{
    int r = buffer[*count-1];
    buffer[*count-1] = 0;
    
    return r;
}

void
consume_item(int item)
{
    //printf("... item %d consumido\n",item);
}

void
wake_up(int id)
{
    kill(id,SIGHUP);
}

void
sighup()
{
    signal(SIGHUP,sighup);
}

void 
err_sys(const char* x) 
{ 
    perror(x); 
    exit(1); 
}