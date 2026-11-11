#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "banker.h"

int **ler_customers(int n_recursos, int *n_customers){
    FILE *arq = fopen("customer.txt", "r");

    if(arq == NULL){
        printf("Erro: Arquivo nao encontrado\n");
        return NULL;
    }

    int countLinha = 0;
    char linha[256];
    while(fgets(linha, sizeof(linha), arq) != NULL){
        countLinha++;
    }

    if(countLinha == 0){
        printf("Erro: Nenhum linha lida no arquivo\n");
        fclose(arq);
        return NULL;
    }

    rewind(arq);
    int cliente = 0;

    int **max = criar_matriz(countLinha, n_recursos);

    if(max == NULL){
        printf("Erro: Erro ao alocar matriz\n");
        fclose(arq);
        return NULL;
    }

    while(fgets(linha, sizeof(linha), arq) != NULL){
        if(linha[0] == '\n' && linha[0] == '\r'){
            continue;
        }
        int count = 0;
        char *pedaco = strtok(linha, ",\r\n");
       
        while(pedaco != NULL){
            char *fim;
            long valor = strtol(pedaco, &fim, 10);

            if(fim == pedaco || *fim != '\0' || valor <0){
                printf("Erro: customer.txt linha %d tem valor invalido: %s\n", cliente + 1, pedaco);
                fclose(arq);
                return NULL;
            }

            if(count >= n_recursos){
                printf("Erro: customer.txt linha %d tem mais de %d recursos\n", cliente + 1, n_recursos);
                fclose(arq);
                return NULL;
            }

            max[cliente][count] = valor;
            count++;
            pedaco = strtok(NULL, ",\r\n");
        }

        if(count != n_recursos){
            printf("Erro: customer.txt linha %d tem %d recursos, esperado %d\n", cliente + 1, count, n_recursos);
            fclose(arq);
            return NULL;
        }
        cliente++;
    }

    *n_customers = cliente;
    fclose(arq);
    return max;
}

int **criar_matriz(int linhas, int colunas){
    int **matriz = calloc(linhas, sizeof(int*));

    if(matriz == NULL){
        printf("Erro: Erro ao alocar matriz\n");
        return NULL;
    }

    for(int i=0; i<linhas; i++){
        matriz[i] = calloc(colunas, sizeof(int*));
    }

    return matriz;
}