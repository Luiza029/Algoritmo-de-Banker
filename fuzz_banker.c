/*
 * fuzz_banker.c - compara o banker com uma implementacao de referencia
 * usando entradas aleatorias (gerado com auxilio de IA).
 *
 * Uso (o testar_banker.sh compila e chama sozinho):
 *   gcc -Wall -O2 fuzz_banker.c -o fuzz_banker
 *   ./fuzz_banker <caminho_do_banker> <pasta_temporaria> <quantidade>
 *
 * Para cada caso: gera customer.txt, commands.txt e os argumentos,
 * roda o banker, calcula a saida esperada e compara linha por linha.
 */

#define _GNU_SOURCE
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_R 5
#define MAX_C 8
#define MAX_CMDS 30
#define TAM_SAIDA 65536

static int n, m;
static int avail[MAX_R], maxi[MAX_C][MAX_R], alloc[MAX_C][MAX_R], need[MAX_C][MAX_R];

static char esperado[TAM_SAIDA];
static int pos;

static void escreve(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    pos += vsnprintf(esperado + pos, TAM_SAIDA - pos, fmt, ap);
    va_end(ap);
}

static void escreve_vetor(const int *v)
{
    for (int j = 0; j < n; j++)
        escreve(j == 0 ? "%d" : " %d", v[j]);
}

static int seguro(void)
{
    int work[MAX_R], fim[MAX_C] = {0}, mudou = 1;
    memcpy(work, avail, sizeof(work));
    while (mudou) {
        mudou = 0;
        for (int i = 0; i < m; i++) {
            if (fim[i])
                continue;
            int cabe = 1;
            for (int j = 0; j < n; j++)
                if (need[i][j] > work[j]) { cabe = 0; break; }
            if (cabe) {
                for (int j = 0; j < n; j++)
                    work[j] += alloc[i][j];
                fim[i] = 1;
                mudou = 1;
            }
        }
    }
    for (int i = 0; i < m; i++)
        if (!fim[i])
            return 0;
    return 1;
}

/* executa um comando na referencia e acrescenta a saida esperada */
static void referencia(char tipo, int c, const int *r)
{
    if (tipo == '*') {
        escreve("MAXIMUM | ALLOCATION | NEED\n");
        for (int i = 0; i < m; i++) {
            escreve_vetor(maxi[i]); escreve(" | ");
            escreve_vetor(alloc[i]); escreve(" | ");
            escreve_vetor(need[i]); escreve("\n");
        }
        escreve("AVAILABLE "); escreve_vetor(avail); escreve("\n");
        return;
    }
    if (tipo == 'L') {
        for (int j = 0; j < n; j++) {
            if (r[j] > alloc[c][j]) {
                escreve("The customer %d cannot release the resources ", c);
                escreve_vetor(r);
                escreve(" because it holds only ");
                escreve_vetor(alloc[c]);
                escreve("\n");
                return;
            }
        }
        for (int j = 0; j < n; j++) {
            alloc[c][j] -= r[j]; need[c][j] += r[j]; avail[j] += r[j];
        }
        escreve("Release from customer %d the resources ", c);
        escreve_vetor(r); escreve("\n");
        return;
    }
    for (int j = 0; j < n; j++) {
        if (r[j] > need[c][j]) {
            escreve("The customer %d request ", c); escreve_vetor(r);
            escreve(" was denied because exceed its maximum allocation\n");
            return;
        }
    }
    for (int j = 0; j < n; j++) {
        if (r[j] > avail[j]) {
            escreve("The resources "); escreve_vetor(avail);
            escreve(" are not enough to customer %d request ", c); escreve_vetor(r);
            escreve("\n");
            return;
        }
    }
    for (int j = 0; j < n; j++) {
        avail[j] -= r[j]; alloc[c][j] += r[j]; need[c][j] -= r[j];
    }
    if (seguro()) {
        escreve("Allocate to customer %d the resources ", c); escreve_vetor(r); escreve("\n");
    } else {
        for (int j = 0; j < n; j++) {
            avail[j] += r[j]; alloc[c][j] -= r[j]; need[c][j] += r[j];
        }
        escreve("The customer %d request ", c); escreve_vetor(r);
        escreve(" was denied because result in an unsafe state\n");
    }
}

static int sorteia(int a, int b) { return a + rand() % (b - a + 1); }

int main(int argc, char *argv[])
{
    if (argc != 4) {
        fprintf(stderr, "uso: %s <banker> <pasta> <quantidade>\n", argv[0]);
        return 2;
    }
    const char *banker = argv[1];
    int total = atoi(argv[3]);
    if (chdir(argv[2]) != 0) {
        perror("chdir");
        return 2;
    }

    int falhas = 0;
    static char obtido[TAM_SAIDA];

    for (int k = 0; k < total; k++) {
        srand(k + 1);                           /* mesmo caso sempre gera a mesma entrada */
        n = sorteia(1, MAX_R);
        m = sorteia(1, MAX_C);
        for (int j = 0; j < n; j++)
            avail[j] = sorteia(0, 12);
        for (int i = 0; i < m; i++)
            for (int j = 0; j < n; j++) {
                maxi[i][j] = sorteia(0, 10);
                need[i][j] = maxi[i][j];
                alloc[i][j] = 0;
            }

        FILE *f = fopen("customer.txt", "w");
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n; j++)
                fprintf(f, j == 0 ? "%d" : ",%d", maxi[i][j]);
            fprintf(f, "\n");
        }
        fclose(f);

        /* argumentos: ./banker a1 a2 ... (antes de a referencia alterar o avail) */
        char cmd[512];
        int p = snprintf(cmd, sizeof(cmd), "'%s'", banker);
        for (int j = 0; j < n; j++)
            p += snprintf(cmd + p, sizeof(cmd) - p, " %d", avail[j]);
        snprintf(cmd + p, sizeof(cmd) - p, " > saida.log 2>&1");

        pos = 0;
        esperado[0] = '\0';
        f = fopen("commands.txt", "w");
        int ncmd = sorteia(1, MAX_CMDS);
        for (int t = 0; t < ncmd; t++) {
            int x = rand() % 100, r[MAX_R];
            if (x < 10) {
                fprintf(f, "*\n");
                referencia('*', 0, r);
                continue;
            }
            char tipo = x < 35 ? 'L' : 'Q';
            int c = rand() % m;
            fprintf(f, "%s %d", tipo == 'L' ? "RL" : "RQ", c);
            for (int j = 0; j < n; j++) {
                r[j] = sorteia(0, 4);
                fprintf(f, " %d", r[j]);
            }
            fprintf(f, "\n");
            referencia(tipo, c, r);
        }
        fclose(f);

        remove("result.txt");
        int ret = system(cmd);

        size_t lidos = 0;
        f = fopen("result.txt", "r");
        if (f) {
            lidos = fread(obtido, 1, TAM_SAIDA - 1, f);
            fclose(f);
        }
        obtido[lidos] = '\0';

        if (ret != 0 || f == NULL || strcmp(obtido, esperado) != 0) {
            falhas++;
            if (falhas <= 3) {
                printf("FALHA no caso %d (retorno %d)\n", k, ret);
                char *a = esperado, *b = obtido;
                int linha = 1;
                while (*a && *a == *b) {
                    if (*a == '\n') linha++;
                    a++; b++;
                }
                printf("  primeira diferenca na linha %d do result.txt\n", linha);
            }
        }
    }
    printf("%d casos aleatorios, %d falhas\n", total, falhas);
    return falhas == 0 ? 0 : 1;
}
