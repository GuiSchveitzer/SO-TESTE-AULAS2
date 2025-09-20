#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
    printf("=== DEMONSTRAÇÃO DE TIPOS ===\n\n");
    
    printf("Tamanhos dos tipos:\n");
    printf("sizeof(int)     = %zu bytes\n", sizeof(int));
    printf("sizeof(size_t)  = %zu bytes\n", sizeof(size_t));
    printf("sizeof(ssize_t) = %zu bytes\n", sizeof(ssize_t));
    
    printf("\nValores máximos/mínimos:\n");
    printf("size_t máximo:  %zu (sempre positivo)\n", (size_t)-1);
    printf("ssize_t máximo: %ld\n", (ssize_t)(((size_t)-1) >> 1));
    printf("ssize_t mínimo: %ld (pode ser negativo)\n", (ssize_t)(~(((size_t)-1) >> 1)));
    
    // Demonstração de uso
    printf("\n=== TESTE DE LEITURA ===\n");
    printf("Digite algo e pressione Enter: ");
    
    char buffer[100];
    ssize_t bytes_lidos = read(0, buffer, sizeof(buffer) - 1);  // 0 = stdin
    
    if (bytes_lidos == -1) {
        perror("Erro na leitura");
    } else if (bytes_lidos == 0) {
        printf("EOF (fim de arquivo)\n");
    } else {
        buffer[bytes_lidos] = '\0';  // Adiciona terminador
        printf("Lidos %ld bytes: %s", bytes_lidos, buffer);
    }
    
    return 0;
}
