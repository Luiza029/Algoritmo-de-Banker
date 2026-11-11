#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "banker.h"

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
            free(vet);
            return 1;
        }
    }

    int n_customers = 0;
    int **max = ler_customers(n_recursos, &n_customers);

    if(max == NULL){
        free(vet);
        return 1;
    }

    printf("Clientes lidos: %d\n", n_customers);
    for(int i=0; i<n_customers; i++){
        for(int j=0; j<n_recursos; j++){
            printf("%d ", max[i][j]);
        }

        printf("\n");
    }

    for(int i=0; i<n_customers; i++){
        free(max[i]);
    }

    free(max);
    free(vet);

    return 0;
}