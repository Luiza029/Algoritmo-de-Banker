#ifndef BANKER_H
#define BANKER_H

int **ler_customers(int n_recursos, int *n_clientes);
int **criar_matriz(int linhas, int colunas);
void liberar_matriz(int **m, int linhas);
void imprimir_vetor(int *v, int n);
void imprimir_estado(int ** max, int **allocation, int **need, int *available, int n_clientes, int n_recursos);
int ler_comando(char *linha, int n_clientes, int n_recursos,char *tipo, int *cliente, int *recursos);
int processar_comandos(int n_clientes, int n_recursos);
#endif