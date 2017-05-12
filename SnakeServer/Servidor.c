#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h> 
#include "..\..\DLL\DLL\DLL.h"

/* ----------------------------------------------------- */
/*  VARIÁVEIS GLOBAIS									 */
/* ----------------------------------------------------- */
Jogador *listaJogadores;
void criaCobra(TCHAR username[SIZE_USERNAME], int vaga);
long random_at_most(long max);
/* ----------------------------------------------------- */
/*  Função MAIN											 */
/* ----------------------------------------------------- */
int _tmain(int argc, LPTSTR argv[]) {
	MemGeral  aux;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif
	
	preparaMemoriaPartilhada();
	inicializaMemoriaPartilhada();
	

	while (1) {

		esperaPorActualizacao();

		leMemoriaPartilhada(&aux);

		_tprintf(TEXT("Estado:%d \nUsername:%s\nCodigoMSG:%d"), aux.estadoJogo,aux.mensagem.username,aux.mensagem.codigoMsg);
		switch (aux.mensagem.codigoMsg)
		{
		case CRIARJOGO://criar Mapa do jogo
			preparaMapaJogo(aux);
		default:
			break;
		}
	}
	_gettch();


	fechaMemoriaPartilhada();
	
	return 0;
}

//Gera as posições da cobra no mapa verificando se há colisões com paredes e fazendo a respectiva alteração á cobra
void criaCobra(TCHAR username[SIZE_USERNAME],int vaga) {
	int posXGerada, posYGerada, dirGerada;
	//Gera posições até encontrar uma vaga;
	while (1) {
		posXGerada = random_at_most((long)vistaPartilhaGeral->config.C);
		posYGerada = random_at_most((long)vistaPartilhaGeral->config.L);
		if (vistaPartilhaGeral->mapa[posYGerada][posXGerada] == ' ')
			break;
	}

	//Na posição 0 do array de posições ficam as Linhas e na 1 ficam as Colunas
	vistaPartilhaGeral->jogadores[vaga].posicoesCobra[0][0] = posYGerada;
	vistaPartilhaGeral->jogadores[vaga].posicoesCobra[0][1] = posXGerada;

	dirGerada = random_at_most(3)+1;
	vistaPartilhaGeral->jogadores[vaga].direcao = dirGerada;

	vistaPartilhaGeral->jogadores[vaga].porAparecer = vistaPartilhaGeral->config.T - 1;
	vistaPartilhaGeral->jogadores[vaga].estadoJogador = VIVO;
	vistaPartilhaGeral->jogadores[vaga].pontuacao = 0;
	vistaPartilhaGeral->jogadores[vaga].tamanho = vistaPartilhaGeral->config.T;
	_tcscpy_s(vistaPartilhaGeral->jogadores[vaga].username, SIZE_USERNAME, username);
}

int AssociaJogo(int numJogadores, TCHAR username1[SIZE_USERNAME], TCHAR username2[SIZE_USERNAME]) {
	for (int i = 0; i < MAXCLIENTES; i++) {
		WaitForSingleObject(hSemMemoria, INFINITE);
	}
	if ((vistaPartilhaGeral->estadoJogo != ASSOCIACAOJOGO) && ((vistaPartilhaGeral->config.N - vistaPartilhaGeral->vagasJogadores) < numJogadores)) {
		ReleaseSemaphore(hSemMemoria, MAXCLIENTES, NULL);
		return 0;
	}

	if (numJogadores == 1) {
		criaCobra(username1, vistaPartilhaGeral->vagasJogadores);
		vistaPartilhaGeral->vagasJogadores++;
	}
	else {
		criaCobra(username1, vistaPartilhaGeral->vagasJogadores);
		vistaPartilhaGeral->vagasJogadores++;
		criaCobra(username2, vistaPartilhaGeral->vagasJogadores);
		vistaPartilhaGeral->vagasJogadores++;
	}

	SetEvent(hEventoMemoria);
	ResetEvent(hEventoMemoria);
	ReleaseSemaphore(hSemMemoria, MAXCLIENTES, NULL);
	return 1;
}

// Assumes 0 <= max <= RAND_MAX
// Returns in the closed interval [0, max]
long random_at_most(long max) {
	unsigned long
		// max <= RAND_MAX < ULONG_MAX, so this is okay.
		num_bins = (unsigned long)max + 1,
		num_rand = (unsigned long)RAND_MAX + 1,
		bin_size = num_rand / num_bins,
		defect = num_rand % num_bins;

	long x;
	do {
		x = (long)random();
	}
	// This is carefully written not to overflow
	while (num_rand - defect <= (unsigned long)x);

	// Truncated division is intentional
	return x / bin_size;
}



