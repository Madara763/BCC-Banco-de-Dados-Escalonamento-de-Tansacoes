# Projeto de Análise de Escalonamento de Transações

Este projeto implementa dois algoritmos fundamentais para a análise de escalonamentos de transações concorrentes em sistemas de banco de dados:  O
Algoritmo de Teste de Seriabilidade por Conflito e o Algoritmo de Teste de Equivalência por Visão. desenvolvido em C, o sistema visa proporcionar uma compreensão prática dos gargalos no processamento de transações e a validação de suas propriedades de concorrência.  

## Objetivo

O principal objetivo é detectar e classificar escalonamentos de transações como serializáveis por conflito ou equivalentes por visão, fornecendo uma ferramenta para compreender o comportamento de transações concorrentes em ambientes de banco de dados.  

## Funcionalidades

O programa recebe uma sequência de operações de transações (leitura, escrita, commit) e, ao identificar um bloco de transações commitadas, analisa e imprime o status de seriabilidade e equivalência de visão para esse escalonamento.  

As análises realizadas são:

    Seriabilidade por Conflito (SS/NS):

        Constrói um grafo de precedência das transações.

        Detecta ciclos no grafo utilizando Busca em Profundidade (DFS).

        Classifica o escalonamento como SS (Serializável por Conflito) se não houver ciclos, ou NS (Não Serializável) caso contrário.

    Equivalência por Visão (SV/NV):

        Identifica as relações "reads-from" (leitura de onde) e "final-writes" (últimas escritas) do escalonamento original.

        Gera todas as permutações seriais possíveis das transações.

        Para cada permutação serial, simula o escalonamento e compara suas propriedades de visão com as do escalonamento original.

        Classifica o escalonamento como SV (Equivalente por Visão) se encontrar pelo menos uma permutação serial com visão equivalente, ou NV (Não Equivalente) caso contrário.

### Como Compilar e Executar
#### Compilação

Para compilar o projeto, navegue até a pasta raiz e utilize o Makefile fornecido:

make

Isso criará o executável escalona.

#### Execução

O programa espera a entrada de dados via stdin e produz a saída via stdout. Não são esperadas opções de linha de comando.  

./escalona < entrada.in > saida.out  

    entrada.in: Arquivo de texto contendo as operações das transações, formatado conforme a especificação.  

    saida.out: Arquivo de texto onde o programa registrará os resultados das análises.  

#### Formato da Entrada:  

Cada linha da entrada representa uma operação e possui 4 campos separados por espaço:  
<tempo_chegada> <id_transacao> <tipo_operacao> <atributo>  

    tempo_chegada: Inteiro, indica o tempo da operação. As linhas são ordenadas por este campo.

    id_transacao: Inteiro, identificador único da transação.

    tipo_operacao: Caractere ('R' para leitura, 'W' para escrita, 'C' para commit).

    atributo: String, nome do atributo lido/escrito.
 
#### Formato da Saída:  

Cada linha de saída representa um escalonamento processado e possui 4 campos separados por espaço: 

<id_escalonamento> <lista_transacoes> <resultado_seriabilidade> <resultado_visao>  

    id_escalonamento: Inteiro, identificador sequencial do escalonamento.

    lista_transacoes: Lista de IDs das transações que compõem o escalonamento, separadas por vírgula (ex: 1,2).

    resultado_seriabilidade: SS (Serializável por Conflito) ou NS (Não Serializável).

    resultado_visao: SV (Equivalente por Visão) ou NV (Não Equivalente).

### Exemplo de Uso:  

Considerando o exemplo de entrada e saída do enunciado:  

entrada.in:  
```
1 1 R X
2 2 R X
3 2 W X
4 1 W X
5 2 C -
6 1 C -
7 3 R X
8 3 R Y
9 4 R X
10 3 W Y
11 4 C -
12 3 C -
```
#### Comando de Execução:  

./escalona < entrada.in  

Saída Esperada no stdout (ou saida.out):  
```
1 1,2 NS NV  
2 3,4 SS SV  
```
#### Estrutura do Projeto:  
```
    escalona.c: Contém a função main responsável pela leitura da entrada, agrupamento das operações em escalonamentos e chamada das funções de análise.

    equivalencia.h: Arquivo de cabeçalho que define as estruturas de dados (Operacao, Escalonamento, ReadsFrom, FinalWrite) e declara as funções utilizadas para a verificação de seriabilidade por conflito e equivalência por visão, além de funções auxiliares.

    equivalencia.c: Implementa as funções declaradas em equivalencia.h, incluindo os algoritmos de detecção de ciclo (DFS) para seriabilidade por conflito, e as funções de simulação de escalonamentos seriais e comparação de visões para equivalência por visão.

    Makefile: Define as regras de compilação do projeto.
```
