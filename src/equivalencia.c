/*
  Biblioteca implementa funcoes de verificacao de equivalencia de transacoes em BD.
  Criada para trabalho da materia de BD de BCC na UFPR.
  Criado por Davi Lazzarin e Mardoqueu Nunes
  Data: 16/06/25
*/
#include "equivalencia.h"

// --- FUNÇÕES AUXILIARES ---

// Procura um transacao pelo id no vetor de transacoes do escalonamento.
// Retorna o indice se encontrar e -1 se não encontrar.
int busca_ind_trans(Escalonamento *s, int trans_id){
  
  for (int i = 0; i < s->trans_count; i++){ //verifica ate o maximo de transacoes
    if (s->transactions[i] == trans_id) //se achar o id
      return i;
  }
  return -1; //se nao achar o id em todo o vetor
}

// Adiciona uma transação à lista do escalonamento se ela ainda não existir.
void add_transicao_se_n_existe(Escalonamento *s, int trans_id){

  if (busca_ind_trans(s, trans_id) == -1){ //Procura se essa transacao ja existe, caso nao, cria ela
    if (s->trans_count < MAX_TRANS_ESCALONAMENTO)    {
      s->transactions[s->trans_count] = trans_id;
      s->committed[s->trans_count] = false;
      s->trans_count++;
    }
  }
}

// Verifica se todos as transações ativas no escalonamento já fizeram commit.
bool todas_commitadas(Escalonamento *s){

  if (s->trans_count == 0)//Se nao tem nenhuma transacao
    return false;

  for (int i = 0; i < s->trans_count; i++){
    if (!s->committed[i])//se alguma nao foi commitada ainda ja retorna falso
      return false; 
    
  }
  return true;
}

// Funcao auxiliar para trocar dois inteiros
void swap(int *a, int *b){
  int temp = *a;
  *a = *b;
  *b = temp;
}

// --- ALGORITMO 1: SERIABILIDADE POR CONFLITO ---

/*
  Realiza uma Busca em Profundidade para detectar ciclos em um grafo.
  O estado de um nó é representado por:
  - visitado[i] == false: nó não visitado.
  - visitado[i] == true && recursion_stack[i] == false: nó visitado e fora da pilha de recursão.
  - visitado[i] == true && recursion_stack[i] == true: nó na pilha de recursão atual (ciclo).
*/
bool dfs_cycle_check(int u, int trans_count, int graph[trans_count][trans_count], bool visitado[], bool recursion_stack[]){
  visitado[u] = true;
  recursion_stack[u] = true;

  for (int v = 0; v < trans_count; v++){
    if (graph[u][v]){ // Se existe uma aresta de u para v
      if (!visitado[v]){
        if (dfs_cycle_check(v, trans_count, graph, visitado, recursion_stack))
          return true;
        
      }
      else if (recursion_stack[v])
        // Se v já foi visitado e está na pilha de recursão tem um ciclo.
        return true;
      
    }
  }
  recursion_stack[u] = false; // Remove u da pilha de recursão ao retroceder
  return false;
}

/**
 * Verifica se um escalonamento é serializável por conflito.
 * Constrói um grafo de precedência e procura por ciclos.
 * Retorna true se for serializável (sem ciclos), false caso contrário.
 */
bool eh_serializavel_por_conflito(Escalonamento *s){
  if (s->trans_count == 0)
    return true;

  // Cria um grafo usando matriz de adjacencia booleana
  // Alocado na pilha pq o count eh limitado por um define
  // Nao vai ter uma matriz gigante na pilha da funcao
  int graph[s->trans_count][s->trans_count];
  memset(graph, 0, sizeof(graph)); //inicia tudo com 0

  // Constroi o grafo 
  for (int i = 0; i < s->op_count; i++){
    for (int j = i + 1; j < s->op_count; j++){
      Operacao op1 = s->ops[i];
      Operacao op2 = s->ops[j];

      // Conflitos so ocorrem entre transacoes diferentes no mesmo atributo
      if (op1.transaction_id != op2.transaction_id && strcmp(op1.attribute, op2.attribute) == 0){
        
        // Pelo menos uma deve ser escrita
        if (op1.type == 'W' || op2.type == 'W'){
          
          //pega os indices
          int t1_idx = busca_ind_trans(s, op1.transaction_id);
          int t2_idx = busca_ind_trans(s, op2.transaction_id);
          
          //salva a aresta no grafo
          if (t1_idx != -1 && t2_idx != -1)
            graph[t1_idx][t2_idx] = 1;
          
        }
      }
    }
  }

  // Verifica se tem ciclos no grafo
  bool visitado[s->trans_count];
  bool recursion_stack[s->trans_count];
  memset(visitado, false, sizeof(visitado));
  memset(recursion_stack, false, sizeof(recursion_stack));

  for (int i = 0; i < s->trans_count; i++){
    if (!visitado[i]){
      if (dfs_cycle_check(i, s->trans_count, graph, visitado, recursion_stack))
        return false; // Ciclo detectado
      
    }
  }

  return true; // Nenhum ciclo encontrado, eh serializavel
}


// --- ALGORITMO 2: EQUIVALÊNCIA POR VISÃO ---

// Analisa um escalonamento (original ou serial simulado) e extrai suas propriedades de visão
void salva_propriedades(Operacao ops[], int op_count, ReadsFrom reads_from[], int *rf_count, FinalWrite final_writes[], int *fw_count){
  *rf_count = 0;
  *fw_count = 0;

  // Encontra relaoees readsfrom
  for (int i = 0; i < op_count; i++){
    if (ops[i].type == 'R'){ //se eh uma leitura

      int writer_trans = -1;
      // Procura a escrita mais recente ANTES da leitura atual
      for (int j = i - 1; j >= 0; j--){
        if (ops[j].type == 'W' && strcmp(ops[j].attribute, ops[i].attribute) == 0){
          writer_trans = ops[j].transaction_id;
          break;
        }
      }

      // Se não encontrou nenhuma escrita antes, a leitura eh do "BD inicial".
      if (writer_trans != -1){ //Se encontrou, salva o leitor e o escritor
        reads_from[*rf_count].reader_trans = ops[i].transaction_id;
        reads_from[*rf_count].writer_trans = writer_trans;
        (*rf_count)++;
      }
    }
  }

  // Encontra "escritas finais"
  char unique_attrs[QUANT_MAX_ATR][MAX_TAM_ATR];
  int unique_attr_count = 0;

  //percorre as operacoes
  for (int i = 0; i < op_count; i++){
    if (ops[i].type == 'W'){ //se for uma escrita
      bool found = false;
      //Adiciona verifica se ja exite na matriz
      for (int j = 0; j < unique_attr_count; j++){ 
        if (strcmp(unique_attrs[j], ops[i].attribute) == 0){
          found = true;
          break;
        }
      }
      // caso nao, adiciona ele
      if (!found && unique_attr_count < QUANT_MAX_ATR)
        strcpy(unique_attrs[unique_attr_count++], ops[i].attribute);
      
    }
  }

  //Para cada atributo q foi ecrito
  for (int i = 0; i < unique_attr_count; i++){
    int last_writer = -1;
    for (int j = op_count - 1; j >= 0; j--){//percorre o vetor de atributos e acha qual foi o ultimo id a alterar ele
      if (ops[j].type == 'W' && strcmp(ops[j].attribute, unique_attrs[i]) == 0){ 
        last_writer = ops[j].transaction_id; //se achar salva o id
        break;
      }
    }
    // tendo achado um id, salva em final_writes quem foi o ultimo a alterar aquele atributo
    if (last_writer != -1){
      strcpy(final_writes[*fw_count].attribute, unique_attrs[i]);
      final_writes[*fw_count].final_writer_trans = last_writer;
      (*fw_count)++;
    }
  }
}

// Compara as propriedades de visao de dois escalonamentos
bool compara_visoes(ReadsFrom rf1[], int rf1_count, FinalWrite fw1[], int fw1_count,
                   ReadsFrom rf2[], int rf2_count, FinalWrite fw2[], int fw2_count){
  
  //Verifica se o num de operacoes sao diferentes, se for ja deu errado
  if (rf1_count != rf2_count || fw1_count != fw2_count) 
    return false;

  // Os dois blocos fazem a mesma coisa
  // procuram se os pares de rf e fw sao equivalentes nas duas visoes, se algum nao for ja deu errado

  for (int i = 0; i < rf1_count; i++){
    bool found = false;
    for (int j = 0; j < rf2_count; j++){
      if (rf1[i].reader_trans == rf2[j].reader_trans && rf1[i].writer_trans == rf2[j].writer_trans){
        found = true;
        break;
      }
    }
    //ao menos um nao estava igual, retorna falso, nao eh equivalente
    if (!found) 
      return false; 
  }

  for (int i = 0; i < fw1_count; i++){
    bool found = false;
    for (int j = 0; j < fw2_count; j++){
      if (strcmp(fw1[i].attribute, fw2[j].attribute) == 0 && fw1[i].final_writer_trans == fw2[j].final_writer_trans){
        found = true;
        break;
      }
    }
    //ao menos um nao estava igual, retorna falso, nao eh equivalente
    if (!found)
      return false;
  }

  //Todos estavam iguais
  return true;
}

// Gera todas as permutações de transações e as testa
// Funcao recursiva, testa todas as combinacoes de ordenacao das transacoes
// Usa backtracking para continuar as combinacoes
void gera_e_testa_permutacoes(int l, int r, Escalonamento *s, int perm_trans_ids[], ReadsFrom original_rf[], 
                                    int original_rf_count, FinalWrite original_fw[], int original_fw_count, bool *found_equivalent){
  
  if (*found_equivalent)
    return; // se já achou, para de gerar
  
  //Verifica a visao atual (base da recursao)
  if (l == r){
    if (checa_equivalencia_permutacao(s, perm_trans_ids, original_rf, original_rf_count, original_fw, original_fw_count))
      *found_equivalent = true;
    
  }
  else{
    // Cria outra visao
    for (int i = l; i <= r; i++){
      swap(&perm_trans_ids[l], &perm_trans_ids[i]);
      gera_e_testa_permutacoes(l + 1, r, s, perm_trans_ids, original_rf, original_rf_count, original_fw, original_fw_count, found_equivalent);
      swap(&perm_trans_ids[l], &perm_trans_ids[i]); // backtrack
    }
  }
}

// Para uma dada permutação, simula o escalonamento serial e compara as visões
bool checa_equivalencia_permutacao(Escalonamento *s, int permutation[], ReadsFrom original_rf[], int original_rf_count, FinalWrite original_fw[], int original_fw_count){
  Operacao serial_ops[MAX_OPS];
  int serial_op_count = 0;

  // Constroi o escalonamento serial simulado a partir da permutação
  for (int i = 0; i < s->trans_count; i++){
    int current_trans_id = permutation[i];
    for (int j = 0; j < s->op_count; j++){
      if (s->ops[j].transaction_id == current_trans_id)
        serial_ops[serial_op_count++] = s->ops[j];
      
    }
  }

  // Salva as propriedades do escalonamento simulado
  ReadsFrom serial_rf[MAX_OPS];
  int serial_rf_count = 0;
  FinalWrite serial_fw[QUANT_MAX_ATR];
  int serial_fw_count = 0;
  
  salva_propriedades(serial_ops, serial_op_count, serial_rf, &serial_rf_count, serial_fw, &serial_fw_count);

  // Compara com o original
  return compara_visoes(original_rf, original_rf_count, original_fw, original_fw_count,
                       serial_rf, serial_rf_count, serial_fw, serial_fw_count);
}


//Verifica se um escalonamento é equivalente por visão a algum escalonamento serial.

bool eh_visao_equivalente(Escalonamento *s){
  if (s->trans_count == 0)
    return true;

  // Salva propriedades originais
  ReadsFrom original_rf[MAX_OPS];
  int original_rf_count = 0;
  FinalWrite original_fw[QUANT_MAX_ATR];
  int original_fw_count = 0;

  salva_propriedades(s->ops, s->op_count, original_rf, &original_rf_count, original_fw, &original_fw_count);

  // Gerar permutacaes e testar cada uma
  // Cria um vetor com os id das transacoes
  int perm_trans_ids[s->trans_count];
  for (int i = 0; i < s->trans_count; i++) 
    perm_trans_ids[i] = s->transactions[i];
  
  
  //Gera todas as combinacoes de transferencias seriais para ver se alguma eh equiuvalente
  bool found_equivalent = false;
  gera_e_testa_permutacoes(0, s->trans_count - 1, s, perm_trans_ids, original_rf, original_rf_count, original_fw, original_fw_count, &found_equivalent);

  return found_equivalent;
}

// --- FUNÇÃO DE ANÁLISE E IMPRESSÃO ---

// Analisa um escalonamento completo e imprime o resultado dos dois algoritmos
void gera_escalonamento(Escalonamento *s){
  
  if (s->op_count == 0)
    return;

  // Printa id e lista de transacoes desse escalonamento
  printf("%d ", s->id);
  for (int i = 0; i < s->trans_count; i++)
    printf("%d%s", s->transactions[i], (i == s->trans_count - 1) ? "" : ",");
  

  // Analisa e imprime os resultados
  bool ss = eh_serializavel_por_conflito(s);
  printf(" %s", ss ? "SS" : "NS");

  bool sv = eh_visao_equivalente(s);
  printf(" %s\n", sv ? "SV" : "NV");
}
