#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define BUFFER_SIZE 4 // Tamanho do buffer
int buffer[BUFFER_SIZE]; // Buffer compartilhado
int count = 0; // Número de itens no buffer

// Variáveis de sincronização
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; //Garante que apenas uma thread seja processada
pthread_cond_t cond_producer = PTHREAD_COND_INITIALIZER; //Gerencia as condições em que os produtores atuam
pthread_cond_t cond_consumer = PTHREAD_COND_INITIALIZER; //Gerencia as condições em que os consumidores atuam

// Função do produtor
void* producer(void* arg) {
    int item;
    for (int i = 0; i < 4; i++) {
        item = i;  // Exemplo de item produzido

        pthread_mutex_lock(&mutex); //Garante que a thread produtora tenha acesso exclusivo ao buffer

        // Espera enquanto o buffer está cheio
        while (count == BUFFER_SIZE) {
            pthread_cond_wait(&cond_producer, &mutex); //Bloqueia o produtor até que o consumidor sinalize que o buffer tem espaço
        }

        // Insere item no buffer
        buffer[count] = item;
        count++;
        printf("Produtor produziu item %d\n", item);
        sleep(1);

        // Sinaliza que o consumidor pode consumir
        pthread_cond_signal(&cond_consumer);

        pthread_mutex_unlock(&mutex); // Mutex liberado para que outras thread acessem o buffer
    }
    return NULL;
}

// Função do consumidor
void* consumer(void* arg) {
    int item;
    for (int i = 0; i < 4; i++) {
        pthread_mutex_lock(&mutex);

        // Espera enquanto o buffer está vazio
        while (count == 0) {
            pthread_cond_wait(&cond_consumer, &mutex);
        }

        // Remove item do buffer
        item = buffer[count - 1];
        count--;
        printf("Consumidor consumiu item %d\n", item);
        sleep(1);

        // Sinaliza que o produtor pode produzir
        pthread_cond_signal(&cond_producer);

        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    pthread_t prod, cons;

    // Cria threads de produtor e consumidor
    pthread_create(&prod, NULL, producer, NULL);
    pthread_create(&cons, NULL, consumer, NULL);

    // Espera as threads terminarem
    pthread_join(prod, NULL);
    pthread_join(cons, NULL);

    printf("\nProcesso finalizado!\n");

    return 0;
}
