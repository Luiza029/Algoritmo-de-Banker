#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if(argc < 2){
        printf("Erro: ./banker <arg1> <arg2> <arg3>\n");
        return 1;
    }

    int n_recursos = argc - 1;
    int *vet = malloc(n_recursos * sizeof(int));
    char *endptr;

    if(vet == NULL){
        printf("Erro: Vetor nao alocado\n");
        return -1;
    }

    for(int i=0; i<n_recursos; i++){
        vet[i] = strtol(argv[i+1], &endptr, 10);

        if(endptr == argv[i+1] || *endptr != '\0' || vet[i] < 0){
            printf("Erro: Entrada invalida, digite apenas inteiros nao negativos\n");
            return 1;
            free(vet);
        }
    }

    for(int i=0; i<n_recursos; i++){
        printf("%d \n", vet[i]);
    }

    return 0;
}