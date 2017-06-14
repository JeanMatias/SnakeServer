#pragma once

#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h> 
#include "SnakeDLL.h"

typedef struct {
	int pid;
	int tid;
	int remoto;
	HANDLE hEventoResposta;
	HANDLE hMemResposta;
	Resposta *vistaResposta;
	HANDLE hPipe;
}Cliente;

//Estrutura para manutenção dos objectos no jogo
typedef struct {
	int Tipo;				//Tipo de Objecto (1-Alimento, 2-Gelo, 3-Granada, 4-Vodka, 5-Oleo, 6-Cola, 7-OVodka, 8-OOleo, 9-OCola)
	int linha;				//Posição no mapa
	int coluna;				//Posição no mapa
	int segundosRestantes;	//Segundos que restam ao objecto para este desaparecer
	int apanhado;           //Flag para saber se objecto foi apanhado para caso tenha sido não tentar apaga-lo do mapa. 
}Objecto;

typedef struct {
	TCHAR username[SIZE_USERNAME];
	int pontuacaoTotal;
	int numJogos;
	int numVitorias;
}Jogador;

typedef struct {
	int pid;
	int tid;
	int jogador;
	TCHAR username[SIZE_USERNAME];
	int tamanho;
	int porAparecer;
	int comeuGelo;
	int duracaoEfeito;
	int pontuacao;
	int direcao;
	int estadoJogador;
	int mudouEstado;
	int posicoesCobra[MAX_COLUNAS * MAX_LINHAS][2];
}Cobras;

//mais tarde ver a necessidade de sincronização a aceder esta estrutura no servidor
typedef struct {
	int pidCriador;
	int tidCriador;
	int estadoJogo;
	int vagasJogadores;
	int cobrasVivas;
	int tipoJogo;
	ConfigInicial config;
	ConfigObjecto configObjectos[NUMTIPOOBJECTOS];
	Cobras jogadores[MAXJOGADORES];
	Objecto objectosMapa[MAXOBJECTOS];
}Jogo;

/* ----------------------------------------------------- */
/*  VARIÁVEIS GLOBAIS									 */
/* ----------------------------------------------------- */
Jogador *listaJogadores;//para usar mais tarde com o Registo
Jogo jogo;
Cliente clientes[MAXCLIENTES];

/* ----------------------------------------------------- */
/*  PROTOTIPO DE FUNÇÕES								 */
/* ----------------------------------------------------- */
DWORD WINAPI moveCobras(LPVOID param);
DWORD WINAPI gestorObjectos(LPVOID param);
DWORD WINAPI atendeClienteRemoto(LPVOID param);
DWORD WINAPI criaPipe(LPVOID param);
void lePedidoDaFila(Pedido *param);
void resetDadosJogo();
void preparaMapaJogo();
void criaCobra(TCHAR username[SIZE_USERNAME], int vaga, int pid, int tid, int jogador);
int Cria_Jogo(ConfigInicial param, int pid, int tid, TCHAR username[SIZE_USERNAME],ConfigObjecto objectosConfig[NUMTIPOOBJECTOS]);
int AssociaJogo(TCHAR username[SIZE_USERNAME], int pid, int tid, int jogador);
int IniciaJogo(int pid, int tid);
void mudaDirecaoJogador(int direcao, int pid, int tid, int jogador);
int procuraJogador(int pid, int tid, int jogador);
void criaObjectosMapaInicial(void);
int procuraObjecto(int linha, int coluna);
void geraObjecto(int indice);
int registaCliente(int pid, int tid, int remoto);
int procuraCliente(int pid, int tid);
int criaMemoriaPartilhadaResposta(int pid, int tid, int indice);
int criaMemoriaPartilhada(void);
void notificaCliente(int indice, Resposta resp);
int trataColisao(int linha, int coluna, int indiceCobra);
int acabouJogo();
void atendeSair(int pid, int tid);

