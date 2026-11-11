#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    int **max = malloc(countLinha * sizeof(int*));
    for(int i=0; i<countLinha; i++){
        max[i] = malloc(n_recursos * sizeof(int));
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