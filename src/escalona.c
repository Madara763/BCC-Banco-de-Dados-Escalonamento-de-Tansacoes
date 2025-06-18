#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "equivalencia.h"


int main(){

  //Inicializacao de variaveis
  Escalonamento escalonamento_atual;
  memset(&escalonamento_atual, 0, sizeof(Escalonamento));

  int id_atual = 1;
  escalonamento_atual.id = id_atual;

  //leitura da entrada
  Operacao op; //salva a operacao descrita em uma linha

  while (scanf("%d %d %c %s", &op.time, &op.transaction_id, &op.type, op.attribute) != EOF){ //enquanto tiver coisa pra ler
  
    // Adiciona a operação e a transação ao escalonamento atual
    escalonamento_atual.ops[escalonamento_atual.op_count] = op;
    escalonamento_atual.op_count++;
    add_transicao_se_n_existe(&escalonamento_atual, op.transaction_id);

    // Se for um commit, marca a transação como commitada
    if (op.type == 'C'){

      int id_aux = busca_ind_trans(&escalonamento_atual, op.transaction_id);
      if (id_aux != -1)
        escalonamento_atual.committed[id_aux] = true;

      //(nao sei se isso eh valido por falta de info na descricao do trabalho)
      //else {perror("Entrada errada, operacao inexistente dando commit\n"); exit(0);} //A entrada esta errada 
    }

    // Se todas as transacoes do escalonamento deram commit, fecha um bloco de transacoes e processa elas
    if (todas_commitadas(&escalonamento_atual)){

      // Verifica os tipos de escalonamento e imprime a saida
      gera_escalonamento(&escalonamento_atual);

      // Reseta para o proximo escalonamento
      memset(&escalonamento_atual, 0, sizeof(Escalonamento));
      id_atual++;
      escalonamento_atual.id = id_atual;
    }
  }//while

  return 0;
}