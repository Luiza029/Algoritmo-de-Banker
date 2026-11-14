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
        return 1;
    }

    for(int i=0; i<n_recursos; i++){
        long valor = strtol(argv[i+1], &endptr, 10);

        if(endptr == argv[i+1] || *endptr != '\0' || available[i] < 0){
            printf("Erro: Entrada invalida, digite apenas inteiros nao negativos\n");
            free(available);
            return 1;
        }

        if(valor > MAX_INSTANCIAS){
            printf("Erro: Numero muito grande [%ld]\n", valor);
            free(available);
            return 1;
        }

        available[i] = valor;
    }

    int n_clientes = 0;
    int **max = ler_customers(n_recursos, &n_clientes);
    
    if(max == NULL){
        free(available);
        return 1;
    }
    
    int **allocation = criar_matriz(n_clientes, n_recursos); 
    if(allocation == NULL){
        liberar_matriz(max, n_clientes);
        free(available);
        return 1;
    }

    int **need = criar_matriz(n_clientes, n_recursos);
    if(need == NULL){
        liberar_matriz(allocation, n_clientes);
        liberar_matriz(max, n_clientes);
        free(available);
        return 1;
    }

    for(int i=0; i<n_clientes; i++){
        for(int j=0; j<n_recursos; j++){
            need[i][j] = max[i][j];
        }
    }

    // TESTE eh_seguro: estado do "*" do PDF (deve ser SEGURO)
    int alloc_teste[5][3] = {{0,1,0},{2,0,0},{3,0,2},{2,1,1},{0,0,2}};
    for(int i = 0; i < 5; i++){
        for(int j = 0; j < 3; j++){
            allocation[i][j] = alloc_teste[i][j];
            need[i][j] = max[i][j] - allocation[i][j];
        }
    }
    available[0] = 3; available[1] = 3; available[2] = 2;

    printf("seguro? %d\n", eh_seguro(available, allocation, need, n_clientes, n_recursos));

    // TESTE eh_seguro: depois de conceder RQ 1 1 0 2 e fingir RQ 0 0 2 0 (deve ser INSEGURO)
    allocation[1][0] = 3; allocation[1][2] = 2;   // cliente 1 ganhou 1 0 2
    allocation[0][1] = 3;                         // cliente 0 "ganhou" 0 2 0
    for(int i = 0; i < 5; i++){
        for(int j = 0; j < 3; j++){
            need[i][j] = max[i][j] - allocation[i][j];
        }
    }
    available[0] = 2; available[1] = 1; available[2] = 0;

    printf("seguro? %d\n", eh_seguro(available, allocation, need, n_clientes, n_recursos));

    allocation[0][1] = 2;
    need[0][1] = 3;

    allocation[0][2] = 1;
    need[0][2] = 2;

    processar_comandos(n_clientes, n_recursos, max, allocation, need, available);

    liberar_matriz(need, n_clientes);
    liberar_matriz(allocation, n_clientes);
    liberar_matriz(max, n_clientes);
    free(available);

    return 0;
}