#pragma once

/* ----------------------------------------------------- */
/*  CONSTANTES											 */
/* ----------------------------------------------------- */
#define MAXUTILIZADOR	30								// Max chars do utilizador
#define MAXMENSAGEM		100								// Max chars da mensagem
#define MAXJOGADORES	4
#define TIMEOUTPIPE		1000
#define TIMEOUTVAGA		30000
#define SIZEMENSAGEM	sizeof(Msg)
#define NOMEMEMORIA		TEXT("SharedMem")					// Memoria Partilhada
#define SEMAFORO_MEM	TEXT("SemaforoSharedMem")
#define EVENTO_MEM		TEXT("EventoSharedMem")
#define PIPE_ATENDE		TEXT("\\\\.\\pipe\\atende")		// Servidor para o cliente

#define CIMA			1
#define BAIXO			2
#define ESQUERDA		3
#define DIREITA			4

#define CRIARJOGO		5
#define JUNTARJOGO		6
#define ACTUALIZAMAPA	7

#define ALIMENTO		1 
#define GELO			2
#define GRANADA			3 
#define VODKA			4 
#define OLEO			5 
#define COLA			6 
#define O_VODKA			7 
#define O_OLEO			8 
#define O_COLA			9 

/* ----------------------------------------------------- */
/*  TIPOS												 */
/* ----------------------------------------------------- */
typedef struct {
	TCHAR utilizador[MAXUTILIZADOR];
	int codigoMsg;
}Msg;

typedef struct {
	char utilizador[MAXUTILIZADOR];
	int pontuacao;
	int direcao;
	int ultimoSegmento;
}Cobras;

typedef struct {
	int T;			//Tamanho inicial das Serpentes
	int A;			//Numero de Serpentes Automáticas
	int O;			//Numero de Objectos
	int N;			//Numero maximo de jogadores
}ConfigInicial;

typedef struct {
	int Tipo;		//Tipo de Objecto (1-Alimento, 2-Gelo, 3-Granada, 4-Vodka, 5-Oleo, 6-Cola, 7-OVodka, 8-OOleo, 9-OCola)
	int S;			//Segundos que fica no mapa
}Objecto;


