#!/bin/bash
# testar_banker.sh - testes do algoritmo do banqueiro (gerado com auxilio de IA)
#
# Uso: coloque este arquivo (e o fuzz_banker.py, opcional) na pasta do projeto e rode:
#   chmod +x testar_banker.sh && ./testar_banker.sh
#
# Para ver cada teste que passou: VERBOSO=1 ./testar_banker.sh
# Para mais casos aleatorios:      ./testar_banker.sh 5000
#
# Os testes rodam numa pasta temporaria: o seu customer.txt, commands.txt
# e result.txt NAO sao alterados.

DIR_PROJETO="$(cd "$(dirname "$0")" && pwd)"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
PASSOU=0; FALHOU=0

ok()   { PASSOU=$((PASSOU+1)); if [ -n "$VERBOSO" ]; then echo "    ok: $1"; fi; return 0; }
erro() { FALHOU=$((FALHOU+1)); echo "    FALHOU: $1"; }
passo(){ echo; echo "=== $1"; }

passo "1. compilacao (make clean + make, sem warnings)"
cd "$DIR_PROJETO" || exit 1
make clean > /dev/null 2>&1
if ! make > "$TMP/build.log" 2>&1; then
    cat "$TMP/build.log"; echo "    ERRO: nao compilou"; exit 1
fi
if grep -qi "warning" "$TMP/build.log"; then cat "$TMP/build.log"; erro "compilou com warnings"; else ok "compilou"; fi
[ -x banker ] || { echo "    ERRO: make nao gerou o executavel banker"; exit 1; }
cp banker "$TMP/"
cd "$TMP" || exit 1

CUST="7,5,3\n3,2,2\n9,0,2\n2,2,2\n4,3,3\n"
CMDS="RQ 0 0 2 1\nRQ 2 3 0 2\nRQ 1 2 0 0\nRQ 4 0 0 2\nRL 0 0 1 1\nRQ 3 2 1 1\n*\nRQ 4 4 0 0\nRQ 1 1 0 2\nRQ 0 0 2 0\nRQ 2 0 1 0\n"
cat > esperado.txt <<'FIM'
Allocate to customer 0 the resources 0 2 1
Allocate to customer 2 the resources 3 0 2
Allocate to customer 1 the resources 2 0 0
Allocate to customer 4 the resources 0 0 2
Release from customer 0 the resources 0 1 1
Allocate to customer 3 the resources 2 1 1
MAXIMUM | ALLOCATION | NEED
7 5 3 | 0 1 0 | 7 4 3
3 2 2 | 2 0 0 | 1 2 2
9 0 2 | 3 0 2 | 6 0 0
2 2 2 | 2 1 1 | 0 1 1
4 3 3 | 0 0 2 | 4 3 1
AVAILABLE 3 3 2
The resources 3 3 2 are not enough to customer 4 request 4 0 0
Allocate to customer 1 the resources 1 0 2
The customer 0 request 0 2 0 was denied because result in an unsafe state
The customer 2 request 0 1 0 was denied because exceed its maximum allocation
FIM

passo "2. exemplo do PDF (result.txt deve ser identico)"
printf "$CUST" > customer.txt; printf "$CMDS" > commands.txt; rm -f result.txt
./banker 10 5 7 > saida.log 2>&1; e=$?
if [ $e -eq 0 ] && diff result.txt esperado.txt > diff.log 2>&1; then
    ok "identico ao PDF"
else
    erro "result.txt diferente do esperado (exit=$e)"; cat diff.log 2>/dev/null | head -10
fi
[ -s saida.log ] && erro "imprimiu algo no terminal num caso sem erro: $(head -1 saida.log)" || ok "terminal limpo"

# caso de erro: espera codigo de saida 1, uma mensagem e NENHUM result.txt
deve_falhar() {
    desc="$1"; c="$2"; m="$3"; shift 3
    printf "$c" > customer.txt; printf "$m" > commands.txt; rm -f result.txt
    ./banker "$@" > saida.log 2>&1; e=$?
    if [ $e -eq 139 ] || [ $e -eq 134 ]; then erro "$desc: o programa QUEBROU (exit $e)"; return; fi
    if [ $e -eq 0 ]; then erro "$desc: deveria dar erro, mas saiu com 0"; return; fi
    if [ ! -s saida.log ]; then erro "$desc: deu erro sem nenhuma mensagem"; return; fi
    if [ -s result.txt ]; then erro "$desc: gerou result.txt mesmo com erro"; return; fi
    ok "$desc -> $(head -1 saida.log)"
}
# caso valido: espera codigo 0 e o result.txt com o conteudo dado
deve_passar() {
    desc="$1"; c="$2"; m="$3"; esp="$4"; shift 4
    printf "$c" > customer.txt; printf "$m" > commands.txt; rm -f result.txt
    ./banker "$@" > saida.log 2>&1; e=$?
    if [ $e -ne 0 ]; then erro "$desc: deveria funcionar, mas saiu com $e: $(head -1 saida.log)"; return; fi
    printf "$esp" > esp.txt
    if diff result.txt esp.txt > /dev/null 2>&1; then ok "$desc"; else erro "$desc: result.txt diferente"; diff result.txt esp.txt | head -6; fi
}

passo "3. argumentos invalidos"
deve_falhar "sem argumentos"          "$CUST" "$CMDS"
deve_falhar "argumentos a menos"      "$CUST" "$CMDS" 10 5
deve_falhar "argumentos a mais"       "$CUST" "$CMDS" 10 5 7 1
deve_falhar "letra"                   "$CUST" "$CMDS" 10 x 7
deve_falhar "numero com lixo (5x)"    "$CUST" "$CMDS" 10 5x 7
deve_falhar "negativo"                "$CUST" "$CMDS" 10 -5 7
deve_falhar "numero enorme"           "$CUST" "$CMDS" 99999999999 5 7
deve_falhar "argumento vazio"         "$CUST" "$CMDS" 10 "" 7

passo "4. customer.txt invalido"
deve_falhar "vazio"                   ""                    "*\n" 10 5 7
deve_falhar "so linhas em branco"     "\n\n"                "*\n" 10 5 7
deve_falhar "letra"                   "7,5,3\n3,x,2\n"      "*\n" 10 5 7
deve_falhar "negativo"                "7,5,3\n3,-2,2\n"     "*\n" 10 5 7
deve_falhar "recursos a menos"        "7,5\n"               "*\n" 10 5 7
deve_falhar "recursos a mais"         "7,5,3,1\n"           "*\n" 10 5 7
deve_falhar "virgula no fim"          "7,5,3,\n"            "*\n" 10 5 7
deve_falhar "virgula no comeco"       ",7,5,3\n"            "*\n" 10 5 7
deve_falhar "virgulas duplas"         "7,,5,3\n"            "*\n" 10 5 7
deve_falhar "numero enorme"           "99999999999,5,3\n"   "*\n" 10 5 7
deve_falhar "linha so com espacos"    "7,5,3\n   \n"        "*\n" 10 5 7
deve_falhar "ponto e virgula"         "7;5;3\n"             "*\n" 10 5 7
printf "$CMDS" > commands.txt; rm -f customer.txt result.txt
./banker 10 5 7 > saida.log 2>&1; e=$?
[ $e -ne 0 ] && [ -s saida.log ] && ok "customer.txt inexistente" || erro "customer.txt inexistente (exit=$e)"

passo "5. commands.txt invalido (nada pode ser executado)"
deve_falhar "vazio"                   "$CUST" ""                         10 5 7
deve_falhar "so linhas em branco"     "$CUST" "\n\n"                     10 5 7
deve_falhar "comando desconhecido"    "$CUST" "XX 0 1 2 3\n"             10 5 7
deve_falhar "rq minusculo"            "$CUST" "rq 0 0 2 1\n"             10 5 7
deve_falhar "RQ sozinho"              "$CUST" "RQ\n"                     10 5 7
deve_falhar "RL sozinho"              "$CUST" "RL\n"                     10 5 7
deve_falhar "cliente inexistente"     "$CUST" "RQ 9 0 0 1\n"             10 5 7
deve_falhar "cliente negativo"        "$CUST" "RQ -1 0 0 1\n"            10 5 7
deve_falhar "cliente letra"           "$CUST" "RQ a 0 0 1\n"             10 5 7
deve_falhar "cliente com lixo (1x)"   "$CUST" "RQ 1x 0 0 1\n"            10 5 7
deve_falhar "recursos a menos"        "$CUST" "RQ 0 0 2\n"               10 5 7
deve_falhar "recursos a mais"         "$CUST" "RQ 0 0 2 1 5\n"           10 5 7
deve_falhar "recurso letra"           "$CUST" "RQ 0 0 x 1\n"             10 5 7
deve_falhar "recurso negativo"        "$CUST" "RQ 0 0 -2 1\n"            10 5 7
deve_falhar "recurso enorme"          "$CUST" "RQ 0 0 99999999999 1\n"   10 5 7
deve_falhar "* com sobra"             "$CUST" "* 1 2\n"                  10 5 7
deve_falhar "linha so com espacos"    "$CUST" "RQ 0 0 2 1\n   \n"        10 5 7
deve_falhar "erro so na ultima linha" "$CUST" "RQ 0 0 2 1\n*\nXX\n"      10 5 7
printf "$CUST" > customer.txt; rm -f commands.txt result.txt
./banker 10 5 7 > saida.log 2>&1; e=$?
[ $e -ne 0 ] && [ -s saida.log ] && [ ! -e result.txt ] && ok "commands.txt inexistente" || erro "commands.txt inexistente (exit=$e)"
printf "$CUST" > customer.txt; printf "$CMDS" > commands.txt; rm -f result.txt; mkdir result.txt
./banker 10 5 7 > saida.log 2>&1; e=$?
[ $e -eq 1 ] && ok "result.txt impossivel de criar" || erro "result.txt impossivel de criar (exit=$e)"
rmdir result.txt

passo "6. casos validos e de borda"
deve_passar "arquivos do Windows (CRLF)" "7,5,3\r\n3,2,2\r\n" "RQ 0 1 1 1\r\n*\r\n" \
  "Allocate to customer 0 the resources 1 1 1\nMAXIMUM | ALLOCATION | NEED\n7 5 3 | 1 1 1 | 6 4 2\n3 2 2 | 0 0 0 | 3 2 2\nAVAILABLE 9 4 6\n" 10 5 7
deve_passar "sem \\n na ultima linha" "7,5,3\n3,2,2" "RQ 1 1 0 0\n*" \
  "Allocate to customer 1 the resources 1 0 0\nMAXIMUM | ALLOCATION | NEED\n7 5 3 | 0 0 0 | 7 5 3\n3 2 2 | 1 0 0 | 2 2 2\nAVAILABLE 9 5 7\n" 10 5 7
deve_passar "linha em branco no meio" "$CUST" "RQ 0 0 2 1\n\nRL 0 0 2 1\n" \
  "Allocate to customer 0 the resources 0 2 1\nRelease from customer 0 the resources 0 2 1\n" 10 5 7
deve_passar "RL mais do que o cliente tem" "$CUST" "RL 0 1 1 1\n" \
  "The customer 0 cannot release the resources 1 1 1 because it holds only 0 0 0\n" 10 5 7
deve_passar "pedido zerado" "$CUST" "RQ 0 0 0 0\n" "Allocate to customer 0 the resources 0 0 0\n" 10 5 7
deve_passar "pede o maximo e devolve" "$CUST" "RQ 0 7 5 3\nRL 0 7 5 3\n" \
  "Allocate to customer 0 the resources 7 5 3\nRelease from customer 0 the resources 7 5 3\n" 10 5 7
deve_passar "1 recurso e 1 cliente" "5\n" "RQ 0 3\nRQ 0 3\n*\n" \
  "Allocate to customer 0 the resources 3\nThe customer 0 request 3 was denied because exceed its maximum allocation\nMAXIMUM | ALLOCATION | NEED\n5 | 3 | 2\nAVAILABLE 2\n" 5
deve_passar "1 cliente, pedido inseguro" "5\n" "RQ 0 3\n" \
  "The customer 0 request 3 was denied because result in an unsafe state\n" 4
deve_passar "recursos zerados" "0,0\n" "*\n" \
  "MAXIMUM | ALLOCATION | NEED\n0 0 | 0 0 | 0 0\nAVAILABLE 0 0\n" 0 0

if command -v valgrind > /dev/null; then
    passo "7. valgrind (memoria nao inicializada e vazamentos)"
    printf "$CUST" > customer.txt; printf "$CMDS" > commands.txt
    if valgrind -q --leak-check=full --error-exitcode=9 ./banker 10 5 7 > valgrind.log 2>&1; then
        ok "sem erros de memoria"
    else
        erro "valgrind encontrou problemas"; head -15 valgrind.log
    fi
    printf "7,5,3\n3,x,2\n" > customer.txt
    valgrind -q --leak-check=full --error-exitcode=9 ./banker 10 5 7 > valgrind.log 2>&1
    [ $? -ne 9 ] && ok "sem vazamento num caso de erro" || { erro "vazamento num caso de erro"; head -15 valgrind.log; }
else
    passo "7. valgrind"
    echo "    (pulado: valgrind nao instalado; para instalar: sudo apt install valgrind)"
fi

passo "8. comparacao com implementacao de referencia (entradas aleatorias)"
if [ -f "$DIR_PROJETO/fuzz_banker.py" ] && command -v python3 > /dev/null; then
    mkdir fuzz
    if python3 -I "$DIR_PROJETO/fuzz_banker.py" "$TMP/banker" "$TMP/fuzz" "${1:-2000}" > fuzz.log 2>&1 && grep -q " 0 falhas" fuzz.log; then
        ok "$(head -1 fuzz.log)"; echo "    $(head -1 fuzz.log)"
    else
        erro "diferencas na comparacao"; head -10 fuzz.log
    fi
else
    echo "    (pulado: precisa do fuzz_banker.py na pasta do projeto e do python3)"
fi

echo
echo "RESULTADO: $PASSOU verificacoes passaram, $FALHOU falharam"
[ $FALHOU -eq 0 ] && echo "TUDO PASSOU" || echo "ALGO FALHOU (veja acima)"
exit $FALHOU
