#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define N1 4 // Tamanho do buffer B1
#define N2 4 // Tamanho do buffer B2
#define NUM_X 2 // Número de processos X
#define NUM_Y 2 // Número de processos Y
#define NUM_Z 1 // Número de processos Z


sem_t ocupado_B1, vazio_B1, mutua_B1;
sem_t ocupado_B2, vazio_B2, mutua_B2;

void* X_produce(void *id);
void* Y_consume_produce(void *id);
void* Z_consume(void *id);

static int inicio_B1 = 0, fim_B1 = 0;
static int inicio_B2 = 0, fim_B2 = 0;

int buffer_B1[N1]; // Buffer B1
int buffer_B2[N2]; // Buffer B2

pthread_t Thr_X[NUM_X], Thr_Y[NUM_Y], Thr_Z[NUM_Z];

int main()
{
    srand(time(NULL));

    // Inicializar semáforos para B1
    sem_init(&ocupado_B1, 0, 0);     
    sem_init(&vazio_B1, 0, N1);      
    sem_init(&mutua_B1, 0, 1);      

    // Inicializar semáforos para B2
    sem_init(&ocupado_B2, 0, 0);     
    sem_init(&vazio_B2, 0, N2);      
    sem_init(&mutua_B2, 0, 1);      

    // Criar threads de processos X, Y e Z
    for (long i = 0; i < NUM_X; i++) {
        pthread_create(&Thr_X[i], NULL, X_produce, (void *) &i);
    }
    for (long i = 0; i < NUM_Y; i++) {
        pthread_create(&Thr_Y[i], NULL, Y_consume_produce, (void *) &i);
    }
    for (long i = 0; i < NUM_Z; i++) {
        pthread_create(&Thr_Z[i], NULL, Z_consume, (void *) &i);
    }

    // Esperar todas as threads terminarem
    for (int i = 0; i < NUM_X; i++) {
        pthread_join(Thr_X[i], NULL);
    }
    for (int i = 0; i < NUM_Y; i++) {
        pthread_join(Thr_Y[i], NULL);
    }
    for (int i = 0; i < NUM_Z; i++) {
        pthread_join(Thr_Z[i], NULL);
    }

    // Destruir semáforos
    sem_destroy(&ocupado_B1);
    sem_destroy(&vazio_B1);
    sem_destroy(&mutua_B1);

    sem_destroy(&ocupado_B2);
    sem_destroy(&vazio_B2);
    sem_destroy(&mutua_B2);
    

    printf("Sistema finalizado!\n");
    return 0;
}


void* X_produce(void *id)
{
    long idX = *(long *)id;
    for (int i = 0; i < N1; i++) {
        sem_wait(&vazio_B1);       
        sem_wait(&mutua_B1);       

        
        fim_B1 = (fim_B1 + 1) % N1;
        buffer_B1[fim_B1] = 1 + rand() % 100;
        printf("Processo X%ld produziu %d e colocou em B1 na posicao %d\n", idX, buffer_B1[fim_B1], fim_B1);

        sem_post(&mutua_B1);        
        sem_post(&ocupado_B1);      
        sleep(1);
    }
    return NULL;
}

// Processo Y (consome de B1 e coloca em B2)
void* Y_consume_produce(void *id)
{
    long idY = *(long *)id;
    for (int i = 0; i < N1; i++) {
        sem_wait(&ocupado_B1);      
        sem_wait(&mutua_B1);        

        // Consumir item de B1
        inicio_B1 = (inicio_B1 + 1) % N1;
        int item = buffer_B1[inicio_B1];
        printf("Processo Y%ld consumiu %d de B1 na posicao %d\n", idY, item, inicio_B1);

        sem_post(&mutua_B1);       
        sem_post(&vazio_B1);        
        
        sem_wait(&vazio_B2);        
        sem_wait(&mutua_B2);        
        fim_B2 = (fim_B2 + 1) % N2;
        buffer_B2[fim_B2] = item;
        printf("Processo Y%ld produziu %d e colocou em B2 na posicao %d\n", idY, buffer_B2[fim_B2], fim_B2);

        sem_post(&mutua_B2);        
        sem_post(&ocupado_B2);      
        sleep(1);
    }
    return NULL;
}


void* Z_consume(void *id)
{
    long idZ = *(long *)id;
    for (int i = 0; i < N2; i++) {
        sem_wait(&ocupado_B2);     
        sem_wait(&mutua_B2);       
        
        inicio_B2 = (inicio_B2 + 1) % N2;
        int item = buffer_B2[inicio_B2];
        printf("Processo Z%ld consumiu %d de B2 na posicao %d\n", idZ, item, inicio_B2);

        sem_post(&mutua_B2);        
        sem_post(&vazio_B2);        
        sleep(1);
    }
    return NULL;
}
