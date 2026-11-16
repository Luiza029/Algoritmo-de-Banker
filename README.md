# Algoritmo do Banqueiro (banker)
## Arquivos

- `main.c`: lê os argumentos, cria as matrizes e coordena a execução.
- `banker.c`: funções do programa (leitura dos arquivos, validação, `request`, `release`, teste de segurança e impressão do estado).
- `banker.h`: protótipos das funções.
- `Makefile`: compila o projeto e gera o executável `banker`.
- `testar_banker.sh`: script de testes automáticos (exemplo do PDF, casos de erro e casos de borda). (gerado com IA)
- `fuzz_banker.c`: compara o programa com uma implementação de referência usando entradas aleatórias; é compilado e chamado pelo `testar_banker.sh`. (gerado com IA)

## Como compilar

```bash
make
```

Para apagar os arquivos gerados (`.o`, `banker` e `result.txt`):

```bash
make clean
```

## Como executar

Cada argumento é o número de instâncias de um tipo de recurso:

```bash
./banker 10 5 7
```

Os arquivos `customer.txt` e `commands.txt` precisam estar na mesma pasta do executável.

## Como Testar
```bash
chmod +x testar_banker.sh
./testar_banker.sh
```

### Saída
O resultado de cada comando é gravado em `result.txt`. Mensagens de erro aparecem no terminal.

## Sistema operacional
Desenvolvido e testado em **Ubuntu 26.04 LTS** rodando no **WSL2** (Windows 11), com **gcc (Ubuntu 15.2.0-16ubuntu1) 15.2.0** e GNU Make.
