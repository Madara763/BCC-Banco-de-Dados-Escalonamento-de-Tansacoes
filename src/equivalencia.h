/*
  Biblioteca implementa funcoes de verificacao de equivalencia de transacoes em BD.
  Criada para trabalho da materia de BD de BCC na UFPR.
  Criado por Davi Lazzarin e Mardoqueu Nunes
  Data: 16/06/25
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifndef _EQUIVALENCIA_H_
#define _EQUIVALENCIA_H_

// ------------- ESTRUTURAS E CONSTANTES -------------
// defines de tamanhos maximos para os escalonamentos
// "facam transacoes pequenas" - Eduardo Almeida
#define MAX_OPS 100
#define MAX_TRANS_ESCALONAMENTO 20

// defines para o tamanho dos nomes dos atributos
// podem ter ate 9 digitos e serem ate 100 atributos diferentes
// necessario para faciliar o uso de strings
#define MAX_TAM_ATR 10
#define QUANT_MAX_ATR 100

// Estrutura para representar uma única operação (R, W, ou C)
typedef struct{
  int time;
  int transaction_id;
  char type; // 'R', 'W', 'C'
  char attribute[MAX_TAM_ATR];
} Operacao;

// Estrutura para representar um escalonamento completo
typedef struct{
  int id;
  Operacao ops[MAX_OPS];
  int op_count;
  int transactions[MAX_TRANS_ESCALONAMENTO];
  bool committed[MAX_TRANS_ESCALONAMENTO];
  int trans_count;
} Escalonamento;

// Structs auxiliares para a verificação de Equivalência por Visão
typedef struct{
  int reader_trans;
  int writer_trans;
} ReadsFrom;

typedef struct{
  char attribute[MAX_TAM_ATR];
  int final_writer_trans;
} FinalWrite;


// --- FUNÇÕES AUXILIARES ---
// Procura um transacao pelo id no vetor de transacoes do escalonamento.
// Retorna o indice se encontrar e -1 se não encontrar.
int busca_ind_trans(Escalonamento *s, int trans_id);


// Adiciona uma transação à lista do escalonamento se ela ainda não existir.
void add_transicao_se_n_existe(Escalonamento *s, int trans_id);

// Verifica se todos as transações ativas no escalonamento já fizeram commit.
bool todas_commitadas(Escalonamento *s);

// Funcao auxiliar para trocar dois inteiros
void swap(int *a, int *b);

// ------------- ALGORITMO 1: SERIABILIDADE POR CONFLITO -------------

/*
  Realiza uma Busca em Profundidade para detectar ciclos em um grafo.
  O estado de um nó é representado por:
  - visitado[i] == false: nó não visitado.
  - visitado[i] == true && recursion_stack[i] == false: nó visitado e fora da pilha de recursão.
  - visitado[i] == true && recursion_stack[i] == true: nó na pilha de recursão atual (ciclo).
*/
bool dfs_cycle_check(int u, int trans_count, int graph[trans_count][trans_count], bool visitado[], bool recursion_stack[]);

/*
 Verifica se um escalonamento é serializável por conflito.
 Constrói um grafo de precedência e procura por ciclos.
 Retorna true se for serializável (sem ciclos), false caso contrário.
*/
bool eh_serializavel_por_conflito(Escalonamento *s);


// ------------- ALGORITMO 2: EQUIVALÊNCIA POR VISÃO -------------

// Analisa um escalonamento (original ou serial simulado) e extrai suas propriedades de visão
void salva_propriedades(Operacao ops[], int op_count, ReadsFrom reads_from[], int *rf_count, FinalWrite final_writes[], int *fw_count);

// Compara as propriedades de visao de dois escalonamentos
bool compara_visoes(ReadsFrom rf1[], int rf1_count, FinalWrite fw1[], int fw1_count,
                   ReadsFrom rf2[], int rf2_count, FinalWrite fw2[], int fw2_count);


// Gera todas as permutações de transações e as testa
// Funcao recursiva, testa todas as combinacoes de ordenacao das transacoes
// Usa backtracking para continuar as combinacoes
void gera_e_testa_permutacoes(int l, int r, Escalonamento *s, int perm_trans_ids[], ReadsFrom original_rf[], 
                                    int original_rf_count, FinalWrite original_fw[], int original_fw_count, bool *found_equivalent);

// Para uma dada permutação, simula o escalonamento serial e compara as visões
bool checa_equivalencia_permutacao(Escalonamento *s, int permutation[], ReadsFrom original_rf[], int original_rf_count, FinalWrite original_fw[], int original_fw_count);


// Verifica se um escalonamento é equivalente por visão a algum escalonamento serial.
bool eh_visao_equivalente(Escalonamento *s);


// ------------- FUNÇÃO DE ANÁLISE E IMPRESSÃO -------------

// Analisa um escalonamento completo e imprime o resultado dos dois algoritmos
void gera_escalonamento(Escalonamento *s);


#endif