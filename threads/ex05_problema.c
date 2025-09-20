#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 3

int variavel_compartilhada = 0;

void *funcao_thread(void *arg) {
    int *id = (int *)arg;
    int valor_local; // Captura o valor para usar na operação
    
    valor_local = variavel_compartilhada; // Lê o valor atual
    printf("Thread %d - Antes da modificação: %d\n", *id, valor_local);
    
    variavel_compartilhada = valor_local + 10; // Usa o valor capturado
    printf("Thread %d - Depois da modificação: %d\n", *id, variavel_compartilhada);

    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int ids[NUM_THREADS];
    int i;

    for (i = 0; i < NUM_THREADS; i++) {
        ids[i] = i + 1;
        pthread_create(&threads[i], NULL, funcao_thread, &ids[i]);
    }

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Variável compartilhada após as threads modificarem: %d\n", variavel_compartilhada);

    return 0;
}
