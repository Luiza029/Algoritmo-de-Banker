#ifndef BANKER_H
#define BANKER_H

int **ler_customers(int n_recursos, int *n_customers);
int **criar_matriz(int linhas, int colunas);
void liberar_matriz(int **m, int linhas);
#endif