#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/mman.h>

#define TRUE 1
#define N 10

#define INIT_MUTEX 1
#define INIT_EMPTY N
#define INIT_FULL  0

#define SIZE_BUFFER sizeof(int)*N
#define SIZE_SEMAPHORE sizeof(sem_t)

int  produce_item();
void insert_item(int item);
int  remove_item();
void consume_item(int item);

void err_sys(const char* x);

sem_t *sem_mutex; // referencia ao semaforo de acesso a regiao critica
sem_t *sem_empty; // referencia ao semaforo de lugares vazios no buffer
sem_t *sem_full;  // referencia ao semaforo de lugares preenchidos no buffer

int full;

int *buffer;

int
main()
{
    int pid,item;

    if ((buffer    = mmap(NULL, SIZE_BUFFER   , PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0)) == MAP_FAILED ||
        (sem_mutex = mmap(NULL, SIZE_SEMAPHORE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0)) == MAP_FAILED ||
        (sem_empty = mmap(NULL, SIZE_SEMAPHORE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0)) == MAP_FAILED ||
        (sem_full  = mmap(NULL, SIZE_SEMAPHORE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0)) == MAP_FAILED )
        err_sys("mmap failed");

    sem_init(sem_mutex,1,INIT_MUTEX);    // inicializacoes de semaforos
    sem_init(sem_empty,1,INIT_EMPTY);    // **
    sem_init(sem_full ,1,INIT_FULL);     // **

     
    if ((pid = fork()) < 0 ) {
        err_sys("fork error");

    } else if (pid) {  // parent / producer
        while (TRUE) {
            sleep(1);
            item = produce_item();
            printf("... PRODUCER produziu um item\n");
            sem_wait(sem_empty);
            sem_wait(sem_mutex);
            printf("... PRODUCER entrou na regiao critica\n");
            insert_item(item);
            printf("... PRODUCER inseriu um item no buffer\n");
            sem_post(sem_mutex);
            sem_post(sem_full);
            printf("... PRODUCER saiu da regiao critica\n");
        }

    } else {         // child  / consumer
        while (TRUE)
        {
            sleep(1);
            sem_wait(sem_full);
            sem_wait(sem_mutex);
            printf("... consumer entrou na região critica\n");
            item = remove_item();
            printf("... consumer removeu um item do buffer\n");
            sem_post(sem_mutex);
            sem_post(sem_empty);
            printf("... consumer saiu da regiao critica\n");
            consume_item(item);
            printf("... consumer consumiu um item\n");
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
    sem_getvalue(sem_full,&full); // pega o valor do semaforo e o coloca em full
    buffer[full] = item;
}

int
remove_item() // remove o ultimo item adicionado no buffer
{
    sem_getvalue(sem_full,&full);
    int r = buffer[full];
    buffer[full] = 0;
    
    return r;
}

void
consume_item(int item)
{
    //printf("... item %d consumido\n",item);
}

void 
err_sys(const char* x) 
{ 
    perror(x); 
    exit(1); 
}