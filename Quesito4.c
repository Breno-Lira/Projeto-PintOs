#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>


// Número de leitores e escritores
#define NUM_READERS 6
#define NUM_WRITERS 4

// Número de operações que cada leitor/escritor realizará
#define NUM_READS 5
#define NUM_WRITES 5
#define MAX_DATA 100
int shared_data[MAX_DATA];
int data_count=0;

// Locks para sincronização
pthread_rwlock_t rwlock;      // Lock de leitura/escrita
pthread_mutex_t read_try;     // Mutex para controlar a entrada de leitores


void* reader(void* arg) {
    int reader_id = *((int*)arg);
    for(int i = 0; i < NUM_READS; i++) {
        // Tentativa de leitura
        pthread_mutex_lock(&read_try);               // Verifica se há escritores esperando
        pthread_rwlock_rdlock(&rwlock);             // Adquire o lock de leitura
        pthread_mutex_unlock(&read_try);             // Permite que outros leitores ou escritores tentem acessar

        // Seção crítica de leitura
        int random_data = rand() % data_count;
        printf("Leitor %d está lendo o valor %d\n", reader_id, shared_data[random_data]);
        sleep(rand() % 2 + 1);                       

        // Saída da seção crítica de leitura
        pthread_rwlock_unlock(&rwlock);             // Libera o lock de leitura

        
        sleep(2);
    }
    pthread_exit(NULL);
}


void* writer(void* arg) {
    int writer_id = *((int*)arg);
    for(int i = 0; i < NUM_WRITES; i++) {
        // Tentativa de escrita
        pthread_mutex_lock(&read_try);               // Indica que um escritor deseja escrever
        pthread_rwlock_wrlock(&rwlock);             // Adquire o lock de escrita

        // Seção crítica de escrita
        shared_data[data_count] = rand() % 100;                           
        printf("Escritor %d está escrevendo o valor %d\n", writer_id, shared_data[data_count]);
        sleep(rand() % 3 + 1);  
        data_count++;                     

        // Saída da seção crítica de escrita
        pthread_rwlock_unlock(&rwlock);             // Libera o lock de escrita
        pthread_mutex_unlock(&read_try);             // Permite que leitores ou outros escritores acessem

        
        sleep(2);
    }
    pthread_exit(NULL);
}

int main() {
    srand(time(NULL)); // Inicializa a semente do gerador de números aleatórios

    // Inicializa os locks
    if(pthread_rwlock_init(&rwlock, NULL) != 0) {
        perror("Falha ao inicializar o rwlock");
        exit(EXIT_FAILURE);
    }

    if(pthread_mutex_init(&read_try, NULL) != 0) {
        perror("Falha ao inicializar o mutex");
        exit(EXIT_FAILURE);
    }

    pthread_t readers[NUM_READERS], writers[NUM_WRITERS];
    int reader_ids[NUM_READERS], writer_ids[NUM_WRITERS];

    // Cria as threads de escritores
    for(int i = 0; i < NUM_WRITERS; i++) {
        writer_ids[i] = i + 1;
        if(pthread_create(&writers[i], NULL, writer, &writer_ids[i]) != 0) {
            perror("Falha ao criar a thread do escritor");
            exit(EXIT_FAILURE);
        }
    }

    // Cria as threads de leitores
    for(int i = 0; i < NUM_READERS; i++) {
        reader_ids[i] = i + 1;
        if(pthread_create(&readers[i], NULL, reader, &reader_ids[i]) != 0) {
            perror("Falha ao criar a thread do leitor");
            exit(EXIT_FAILURE);
        }
    }

    for(int i = 0; i < NUM_WRITERS; i++) {
        pthread_join(writers[i], NULL);
    }
    // Espera que todas as threads terminem
    for(int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }

    // Destroi os locks
    pthread_rwlock_destroy(&rwlock);
    pthread_mutex_destroy(&read_try);

    printf("Operações de leitura/escrita concluídas.\n");
    return 0;
}