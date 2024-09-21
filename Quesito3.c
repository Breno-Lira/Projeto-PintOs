#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_DATA 100 // Define o tamanho máximo do vetor de dados

sem_t rw_mutex; // Semáforo para controlar o acesso ao recurso compartilhado
sem_t mutex;    // Semáforo para controlar o acesso ao contador de escritores
int write_count = 0; // Contador de leitores
int data [MAX_DATA];  // Recurso compartilhado (simulando uma variável que será lida/escrita)
int data_count = 0;


void *writer(void *arg) {
    int id = *(int *)arg;

    while (1) {
        
        sem_wait(&mutex); // Garante acesso ao contador de escritores
        write_count++;
        if (write_count == 1) {
            sem_wait(&rw_mutex); // Se for o primeiro escritores, bloqueia os leitores
        }
        sem_post(&mutex);


        // Seção crítica (escrita)
        sem_wait(&mutex); // Protege o contador de escritores e data_count
        data[data_count] = rand() % 100;
        printf("Escritor %d está escrevendo o valor: %d\n", id, data[data_count]);
        data_count++;
        sem_post(&mutex);
        sleep(1); 


        sem_wait(&mutex);
        write_count--;
        if (write_count == 0) {
            sem_post(&rw_mutex); // Se for o último escritor, libera os leitores
        }
        sem_post(&mutex);
        sleep(2); 
    }

    pthread_exit(0);
}

void *reader(void *arg) {
    int id = *(int *)arg;

    while (1) {
        
        sem_wait(&rw_mutex);

        // Seção crítica (leitura)
        int random_number = rand() % data_count;
        printf("Leitor %d está lendo o valor: %d\n", id, data[random_number]);
        sleep(1); 

        sem_post(&rw_mutex);

        sleep(1); 
    }

    pthread_exit(0);
}

int main() {
    int i;
    pthread_t r_threads[5], w_threads[2];
    int reader_ids[5], writer_ids[2];

    // Inicializa os semáforos
    sem_init(&rw_mutex, 0, 1);
    sem_init(&mutex, 0, 1);

    // Cria threads de escritores
    for (i = 0; i < 2; i++) {
        writer_ids[i] = i + 1;
        pthread_create(&w_threads[i], NULL, writer, &writer_ids[i]);
    }

    // Cria threads de leitores
    for (i = 0; i < 5; i++) {
        reader_ids[i] = i + 1;
        pthread_create(&r_threads[i], NULL, reader, &reader_ids[i]);
    }

    // Junta as threads (nunca ocorre na prática aqui, pois os loops são infinitos)
    for (i = 0; i < 5; i++) {
        pthread_join(r_threads[i], NULL);
    }

    for (i = 0; i < 2; i++) {
        pthread_join(w_threads[i], NULL);
    }

    // Destrói os semáforos
    sem_destroy(&rw_mutex);
    sem_destroy(&mutex);

    return 0;
}