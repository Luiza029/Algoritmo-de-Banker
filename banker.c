#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "banker.h"

int **ler_customers(int n_recursos, int *n_clientes){
    FILE *arq = fopen("customer.txt", "r");

    if(arq == NULL){
        printf("Erro: Arquivo nao encontrado\n");
        return NULL;
    }

    int countLinha = 0;
    char linha[256];
    while(fgets(linha, sizeof(linha), arq) != NULL){
        if(linha[0] != '\n' && linha[0] != '\r'){
            countLinha++;
        }
    }

    printf("%d\n", countLinha);
    if(countLinha == 0){
        printf("Erro: Nenhum linha lida no arquivo\n");
        fclose(arq);
        return NULL;
    }

    rewind(arq);
    int cliente = 0;

    int **max = criar_matriz(countLinha, n_recursos);

    if(max == NULL){
        fclose(arq);
        return NULL;
    }

    while(fgets(linha, sizeof(linha), arq) != NULL){
        if(linha[0] == '\n' || linha[0] == '\r'){
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
                liberar_matriz(max, countLinha);
                return NULL;
            }

            if(count >= n_recursos){
                printf("Erro: customer.txt linha %d tem mais de %d recursos\n", cliente + 1, n_recursos);
                fclose(arq);
                liberar_matriz(max, countLinha);
                return NULL;
            }

            max[cliente][count] = valor;
            count++;
            pedaco = strtok(NULL, ",\r\n");
        }

        if(count != n_recursos){
            printf("Erro: customer.txt linha %d tem %d recursos, esperado %d\n", cliente + 1, count, n_recursos);
            fclose(arq);
            liberar_matriz(max, countLinha);
            return NULL;
        }
        cliente++;
    }

    *n_clientes = cliente;
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
        matriz[i] = calloc(colunas, sizeof(int));

        if(matriz[i] == NULL){
            liberar_matriz(matriz, i);
            return NULL;
        }
    }

    return matriz;
}

void liberar_matriz(int **m, int linhas){
    if(m == NULL) return;

    for(int i=0; i<linhas; i++){
        free(m[i]);
    }

    free(m);
}

void imprimir_vetor(int *v, int n){
    printf("%d", v[0]);

    for(int i=1; i<n; i++){
        printf(" %d", v[i]);
    }
}

void imprimir_estado(int ** max, int **allocation, int **need, int *available, int n_clientes, int n_recursos){
    printf("MAXIMUM | ALLOCATION | NEED\n");

    for(int i=0; i<n_clientes; i++){
        imprimir_vetor(max[i], n_recursos);
        printf(" | ");
        imprimir_vetor(allocation[i], n_recursos);
        printf(" | ");
        imprimir_vetor(need[i], n_recursos);
        printf("\n");
    }

    printf("AVAILABLE ");
    imprimir_vetor(available, n_recursos);
    printf("\n");
}

int processar_comandos(int n_clientes, int n_recursos){
    FILE *arq = fopen("commands.txt", "r");

    if(arq == NULL){
        printf("Erro: Arquivo nao encontrado\n");
        return 0;
    }

    char linhaComando[256];
    char tipo;
    int cliente;
    int recursos[n_recursos];
    int countLinha = 0;

    while(fgets(linhaComando, sizeof(linhaComando), arq) != NULL){
        if(linhaComando[0] == '\n' || linhaComando[0] == '\r'){
            continue;
        }

        if(ler_comando(linhaComando, n_clientes, n_recursos, &tipo, &cliente, recursos) == 0){
            fclose(arq);
            return 0;
        }

        countLinha++;
    }

    if(countLinha == 0){
        printf("Erro: Nenhum comando lido no arquivo\n");
        fclose(arq);
        return 0;
    }

    rewind(arq);

     while(fgets(linhaComando, sizeof(linhaComando), arq) != NULL){
        if(linhaComando[0] == '\n' || linhaComando[0] == '\r'){
            continue;
        }

        if(ler_comando(linhaComando, n_clientes, n_recursos, &tipo, &cliente, recursos) == 0){
            fclose(arq);
            return 0;
        }

        if(tipo == '*'){
            printf("tipo=*\n");
        } 
        
        else {
            printf("tipo=%c cliente=%d recursos: ", tipo, cliente);
            imprimir_vetor(recursos, n_recursos);
            printf("\n");
        }
     }

    fclose(arq);
    return 1;
}

int ler_comando(char *linha, int n_clientes, int n_recursos,char *tipo, int *cliente, int *recursos){
    char *pedaco = strtok(linha, " \r\n");

    if(pedaco == NULL){
        printf("Erro: Linha em branco\n");
        return 0;
    }

    if(strcmp(pedaco, "*") == 0){
        *tipo = '*';

        pedaco = strtok(NULL, " \r\n");

        if(pedaco != NULL){
            printf("Erro: Argumentos demais no [*]\n");
            return 0;
        }

        return 1;
    }

    else if(strcmp(pedaco, "RQ") == 0){
        *tipo = 'Q';
    }

    else if(strcmp(pedaco, "RL") == 0){
        *tipo = 'L';
    }

    else{
        printf("Erro: Comando desconhecido\n");
        return 0;
    }

    pedaco = strtok(NULL, " \r\n");
    if(pedaco == NULL){
        printf("Erro: Comando incomplemto\n");
        return 0;
    }

    char *fim;
    long valor = strtol(pedaco, &fim, 10);

    if(fim == pedaco || *fim != '\0'){
        printf("Erro: commands.txt tem cliente invalido: [%s]\n", pedaco);
        return 0;
    }

    if(valor < 0  || valor >= n_clientes){
        printf("Erro: commands.txt tem cliente inexistente: %ld\n", valor);
        return 0;
    }
    
    *cliente = valor;

    int count = 0;
    pedaco = strtok(NULL, " \r\n");
    while(pedaco != NULL){
        char *fim;
        long valor = strtol(pedaco, &fim, 10);

        if(fim == pedaco || *fim != '\0' || valor <0){
            printf("Erro: commands.txt tem valor invalido: [%s]\n", pedaco);
            return 0;
        }

        if(count >= n_recursos){
            printf("Erro: commands.txt tem mais de %d recursos no comando\n", n_recursos);
            return 0;
        }

        recursos[count] = valor;
        count++;
        pedaco = strtok(NULL, " \r\n");
    }   

    if(count != n_recursos){
        printf("Erro: Recusos a menos\n");
        return 0;
    }

    return 1; 
}