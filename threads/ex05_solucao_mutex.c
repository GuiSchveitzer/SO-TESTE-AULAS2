//
//  Versão com MUTEX para resolver o problema
//
//  Este código garante que o resultado final seja sempre o esperado (30)
//  usando mutex para sincronizar o acesso à variável compartilhada
//

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 3

int variavel_compartilhada = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Inicializa o mutex

void *funcao_thread(void *arg) {
    int *id = (int *)arg;
    int valor_local;
    
    // SEÇÃO CRÍTICA - apenas uma thread pode executar por vez
    pthread_mutex_lock(&mutex);
    
    valor_local = variavel_compartilhada; // Captura o valor de forma protegida
    printf("Thread %d - Antes da modificação: %d\n", *id, valor_local);
    variavel_compartilhada = valor_local + 10; // Modifica usando valor capturado
    printf("Thread %d - Depois da modificação: %d\n", *id, variavel_compartilhada);
    
    pthread_mutex_unlock(&mutex);
    // FIM DA SEÇÃO CRÍTICA

    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int ids[NUM_THREADS];
    int i;

    printf("=== VERSÃO COM MUTEX - RESULTADO SEMPRE CORRETO ===\n");

    for (i = 0; i < NUM_THREADS; i++) {
        ids[i] = i + 1;
        pthread_create(&threads[i], NULL, funcao_thread, &ids[i]);
    }

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Variável compartilhada após as threads modificarem: %d\n", variavel_compartilhada);
    printf("Resultado esperado: 30\n");
    printf("Status: %s\n", (variavel_compartilhada == 30) ? "✓ CORRETO" : "✗ INCORRETO");
    
    pthread_mutex_destroy(&mutex); // Limpa o mutex

    return 0;
}
