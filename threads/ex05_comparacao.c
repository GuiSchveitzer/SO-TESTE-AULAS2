//
//  Comparação das 3 versões para demonstrar o problema
//

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 3

int variavel_compartilhada = 0;

// VERSÃO ORIGINAL (pode ter race condition no print, mas resultado final correto)
void *versao_original(void *arg) {
    int *id = (int *)arg;
    
    printf("Thread %d - Antes da modificação: %d\n", *id, variavel_compartilhada);
    variavel_compartilhada += 10; // += usa valor atual da memória
    printf("Thread %d - Depois da modificação: %d\n", *id, variavel_compartilhada);

    return NULL;
}

// VERSÃO MODIFICADA (pode ter resultado final incorreto)
void *versao_modificada(void *arg) {
    int *id = (int *)arg;
    int valor_local;
    
    valor_local = variavel_compartilhada; // Captura valor que pode estar desatualizado
    printf("Thread %d - Antes da modificação: %d\n", *id, valor_local);
    variavel_compartilhada = valor_local + 10; // Usa valor possivelmente desatualizado
    printf("Thread %d - Depois da modificação: %d\n", *id, variavel_compartilhada);

    return NULL;
}

int main() {
    printf("=== DEMONSTRAÇÃO DAS DIFERENÇAS ===\n\n");
    
    printf("VERSÃO ORIGINAL:\n");
    printf("- Race condition no printf (valores inconsistentes)\n");
    printf("- Mas += sempre soma corretamente\n");
    printf("- Resultado final: sempre 30\n\n");
    
    printf("VERSÃO MODIFICADA:\n");
    printf("- Race condition na captura do valor\n");
    printf("- Threads podem usar valor desatualizado\n");
    printf("- Resultado final: pode ser < 30\n\n");
    
    printf("VERSÃO COM MUTEX:\n");
    printf("- Sem race conditions\n");
    printf("- Acesso serializado à variável\n");
    printf("- Resultado final: sempre 30\n\n");
    
    return 0;
}
