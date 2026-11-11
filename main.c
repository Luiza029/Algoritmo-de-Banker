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
    int *available = malloc(n_recursos * sizeof(int));
    char *endptr;

    if(available == NULL){
        printf("Erro: Vetor nao alocado\n");
        return -1;
    }

    for(int i=0; i<n_recursos; i++){
        available[i] = strtol(argv[i+1], &endptr, 10);

        if(endptr == argv[i+1] || *endptr != '\0' || available[i] < 0){
            printf("Erro: Entrada invalida, digite apenas inteiros nao negativos\n");
            free(available);
            return 1;
        }
    }

    int n_customers = 0;
    int **max = ler_customers(n_recursos, &n_customers);
    
    if(max == NULL){
        free(available);
        return 1;
    }
    
    int **allocation = criar_matriz(n_customers, n_recursos); 
    if(allocation == NULL){
        liberar_matriz(max, n_customers);
        free(available);
        return 1;
    }

    int **need = criar_matriz(n_customers, n_recursos);
    if(need == NULL){
        liberar_matriz(allocation, n_customers);
        liberar_matriz(max, n_customers);
        free(available);
        return 1;
    }

    for(int i=0; i<n_customers; i++){
        for(int j=0; j<n_recursos; j++){
            need[i][j] = max[i][j];
        }
    }

    liberar_matriz(need, n_customers);
    liberar_matriz(allocation, n_customers);
    liberar_matriz(max, n_customers);
    free(available);

    return 0;
}