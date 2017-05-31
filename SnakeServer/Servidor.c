#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h> 
#include "..\..\SnakeDLL\SnakeDLL\SnakeDLL.h"


//mais tarde ver a necessidade de sincronização a aceder esta estrutura no servidor
typedef struct {
	int pidCriador;
	int estadoJogo;
	int vagasJogadores;
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

DWORD WINAPI moveCobras(LPVOID param);
void lePedidoDaFila(Pedido *param);
void resetDadosJogo();
void preparaMapaJogo();
void criaCobra(TCHAR username[SIZE_USERNAME], int vaga, int pid, int jogador);
int Cria_Jogo(ConfigInicial param, int pid);
int AssociaJogo(TCHAR username[SIZE_USERNAME], int pid, int jogador);
int IniciaJogo(int pid);
void mudaDirecaoJogador(int direcao, int pid, int jogador);
int procuraJogador(int pid, int jogador);


/* ----------------------------------------------------- */
/*  Função MAIN											 */
/* ----------------------------------------------------- */
int _tmain(int argc, LPTSTR argv[]) {
	
	Pedido aux;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif
	

	srand((int)time(NULL));

	if (preparaMemoriaPartilhada() == -1) {
		_tprintf(TEXT("ERRO"));
		exit(-1);
	}

	resetDadosJogo();

	while (1) {

		lePedidoDaFila(&aux);

		_tprintf(TEXT("Pid:%d CodigoMSG:%d\n"), aux.pid,aux.codigoPedido);
		switch (aux.codigoPedido)
		{
		case CRIARJOGO:
			Cria_Jogo(aux.config, aux.pid);
			//criar Mapa do jogo
			preparaMapaJogo(aux);
			break;
		case ASSOCIAR_JOGADOR1:
			AssociaJogo(aux.username, aux.pid, JOGADOR1);
			break;
		case ASSOCIAR_JOGADOR2:
			AssociaJogo(aux.username, aux.pid, JOGADOR2);
			break;
		case INICIARJOGO:
			IniciaJogo(aux.pid);
			break;
		case CIMA:
		case BAIXO:
		case ESQUERDA:
		case DIREITA:
			mudaDirecaoJogador(aux.codigoPedido, aux.pid, aux.jogador);
			break;
		default:
			break;
		}
	}
	_gettch();


	fechaMemoriaPartilhada();
	
	return 0;
}

//Mete dados do jogo para ser possivel aceitar pedidos de criação de jogo.
void resetDadosJogo() {
	jogo.estadoJogo = CRIACAOJOGO;
	jogo.vagasJogadores = 0;
}

void lePedidoDaFila(Pedido *param){

	WaitForSingleObject(hPodeLerPedido, INFINITE);

	param->pid = vistaPartilhaGeral->fila.pedidos[vistaPartilhaGeral->fila.frente].pid;
	param->jogador = vistaPartilhaGeral->fila.pedidos[vistaPartilhaGeral->fila.frente].jogador;
	param->codigoPedido = vistaPartilhaGeral->fila.pedidos[vistaPartilhaGeral->fila.frente].codigoPedido;
	param->config = vistaPartilhaGeral->fila.pedidos[vistaPartilhaGeral->fila.frente].config;
	_tcscpy_s(param->username , SIZE_USERNAME, vistaPartilhaGeral->fila.pedidos[vistaPartilhaGeral->fila.frente].username);
	for (int i = 0; i < NUMTIPOOBJECTOS; i++)
		param->objectosConfig[i] = vistaPartilhaGeral->fila.pedidos[vistaPartilhaGeral->fila.frente].objectosConfig[i];

	vistaPartilhaGeral->fila.frente++;

	//chegou ao fim da fila temos de voltar a por desde o inicio da fila
	if (vistaPartilhaGeral->fila.frente == MAX_PEDIDOS) {
		vistaPartilhaGeral->fila.frente = 0;
	}

	ReleaseSemaphore(hPodeEscreverPedido, 1, NULL);
}

/*----------------------------------------------------------------- */
/*  THREAD - Função que movimenta uma cobra no mapa				 	*/
/* ---------------------------------------------------------------- */
DWORD WINAPI moveCobras(LPVOID param) {
	int posArrayAux, auxLinha, auxColuna;
	int idCobraNoMapa, posCobra = (int)param;

	while (1) {
		Sleep(LENTIDAO * RAPIDO);
		//mexer so as cobras vivas
		if (jogo.jogadores[posCobra].estadoJogador == VIVO) {
			idCobraNoMapa = (posCobra + 1) * 100;
			for (int i = 0; i < MAXCLIENTES; i++) {
				WaitForSingleObject(hSemaforoMapa, INFINITE);
			}
			switch (jogo.jogadores[posCobra].direcao)
			{
			case CIMA://Coluna mantem-se, muda de linha(-1)
				if (jogo.jogadores[posCobra].porAparecer != 0) {
					posArrayAux = jogo.jogadores[posCobra].tamanho - jogo.jogadores[posCobra].porAparecer;
					//Buscar a posição da cabeça
					auxLinha = jogo.jogadores[posCobra].posicoesCobra[posArrayAux - 1][0];
					auxColuna = jogo.jogadores[posCobra].posicoesCobra[posArrayAux - 1][1];
					jogo.jogadores[posCobra].posicoesCobra[posArrayAux][0] = auxLinha-1;
					jogo.jogadores[posCobra].posicoesCobra[posArrayAux][1] = auxColuna;
					jogo.jogadores[posCobra].porAparecer--;
					vistaPartilhaGeral->mapa[auxLinha - 1][auxColuna] = idCobraNoMapa;
				}
				else {
					posArrayAux = jogo.jogadores[posCobra].tamanho - 1;
					//Apagar a cauda do mapa
					auxLinha = jogo.jogadores[posCobra].posicoesCobra[0][0];
					auxColuna = jogo.jogadores[posCobra].posicoesCobra[0][1];
					vistaPartilhaGeral->mapa[auxLinha][auxColuna] = ESPACOVAZIO;
					//Buscar a posição da cabeça
					auxLinha = jogo.jogadores[posCobra].posicoesCobra[posArrayAux][0];
					auxColuna = jogo.jogadores[posCobra].posicoesCobra[posArrayAux][1];
					//Mover todas as posições da cobra no array sobrepondo as antigas
					for (int z = 0; z < posArrayAux; z++) {
						jogo.jogadores[posCobra].posicoesCobra[z][0] = jogo.jogadores[posCobra].posicoesCobra[z+1][0];
						jogo.jogadores[posCobra].posicoesCobra[z][1] = jogo.jogadores[posCobra].posicoesCobra[z+1][1];
					}
					jogo.jogadores[posCobra].posicoesCobra[posArrayAux][0] = auxLinha - 1;
					jogo.jogadores[posCobra].posicoesCobra[posArrayAux][1] = auxColuna;
					vistaPartilhaGeral->mapa[auxLinha - 1][auxColuna] = idCobraNoMapa;		
				}					
				break;
			case BAIXO://Coluna mantem-se, muda de linha(+1)
				if (jogo.jogadores[posCobra].porAparecer != 0) {
					posArrayAux = jogo.jogadores[posCobra].tamanho - jogo.jogadores[posCobra].porAparecer;
					//Buscar a posição da cabeça
					auxLinha = jogo.jogadores[posCobra].posicoesCobra[posArrayAux - 1][0];
					auxColuna = jogo.jogadores[posCobra].posicoesCobra[posArrayAux - 1][1];
					jogo.jogadores[posCobra].posicoesCobra[posArrayAux][0] = auxLinha + 1;
					jogo.jogadores[posCobra].posicoesCobra[posArrayAux][1] = auxColuna;
					jogo.jogadores[posCobra].porAparecer--;
					vistaPartilhaGeral->mapa[auxLinha + 1][auxColuna] = idCobraNoMapa;
				}
				else {
					posArrayAux = jogo.jogadores[posCobra].tamanho - 1;
					//Apagar a cauda do mapa
					auxLinha = jogo.jogadores[posCobra].posicoesCobra[0][0];
					auxColuna = jogo.jogadores[posCobra].posicoesCobra[0][1];
					vistaPartilhaGeral->mapa[auxLinha][auxColuna] = ESPACOVAZIO;
					//Buscar a posição da cabeça
					auxLinha = jogo.jogadores[posCobra].posicoesCobra[posArrayAux][0];
					auxColuna = jogo.jogadores[posCobra].posicoesCobra[posArrayAux][1];
					//Mover todas as posições da cobra no array sobrepondo as antigas
					for (int z = 0; z < posArrayAux; z++) {
						jogo.jogadores[posCobra].posicoesCobra[z][0] = jogo.jogadores[posCobra].posicoesCobra[z + 1][0];
						jogo.jogadores[posCobra].posicoesCobra[z][1] = jogo.jogadores[posCobra].posicoesCobra[z + 1][1];
					}
					jogo.jogadores[posCobra].posicoesCobra[posArrayAux][0] = auxLinha + 1;
					jogo.jogadores[posCobra].posicoesCobra[posArrayAux][1] = auxColuna;
					vistaPartilhaGeral->mapa[auxLinha + 1][auxColuna] = idCobraNoMapa;
				}
				break;
			case ESQUERDA://Linha mantem-se, muda de coluna(-1)
				if (jogo.jogadores[posCobra].porAparecer != 0) {
					posArrayAux = jogo.jogadores[posCobra].tamanho - jogo.jogadores[posCobra].porAparecer;
					//Buscar a posição da cabeça
					auxLinha = jogo.jogadores[posCobra].posicoesCobra[posArrayAux - 1][0];
					auxColuna = jogo.jogadores[posCobra].posicoesCobra[posArrayAux - 1][1];
					jogo.jogadores[posCobra].posicoesCobra[posArrayAux][0] = auxLinha;
					jogo.jogadores[posCobra].posicoesCobra[posArrayAux][1] = auxColuna - 1;
					jogo.jogadores[posCobra].porAparecer--;
					vistaPartilhaGeral->mapa[auxLinha][auxColuna - 1] = idCobraNoMapa;
				}
				else {
					posArrayAux = jogo.jogadores[posCobra].tamanho - 1;
					//Apagar a cauda do mapa
					auxLinha = jogo.jogadores[posCobra].posicoesCobra[0][0];
					auxColuna = jogo.jogadores[posCobra].posicoesCobra[0][1];
					vistaPartilhaGeral->mapa[auxLinha][auxColuna] = ESPACOVAZIO;
					//Buscar a posição da cabeça
					auxLinha = jogo.jogadores[posCobra].posicoesCobra[posArrayAux][0];
					auxColuna = jogo.jogadores[posCobra].posicoesCobra[posArrayAux][1];
					//Mover todas as posições da cobra no array sobrepondo as antigas
					for (int z = 0; z < posArrayAux; z++) {
						jogo.jogadores[posCobra].posicoesCobra[z][0] = jogo.jogadores[posCobra].posicoesCobra[z + 1][0];
						jogo.jogadores[posCobra].posicoesCobra[z][1] = jogo.jogadores[posCobra].posicoesCobra[z + 1][1];
					}
					jogo.jogadores[posCobra].posicoesCobra[posArrayAux][0] = auxLinha;
					jogo.jogadores[posCobra].posicoesCobra[posArrayAux][1] = auxColuna - 1;
					vistaPartilhaGeral->mapa[auxLinha][auxColuna - 1] = idCobraNoMapa;
				}
				break;
			case DIREITA://Linha mantem-se, muda de coluna(+1)
				if (jogo.jogadores[posCobra].porAparecer != 0) {
					posArrayAux = jogo.jogadores[posCobra].tamanho - jogo.jogadores[posCobra].porAparecer;
					//Buscar a posição da cabeça
					auxLinha = jogo.jogadores[posCobra].posicoesCobra[posArrayAux - 1][0];
					auxColuna = jogo.jogadores[posCobra].posicoesCobra[posArrayAux - 1][1];
					jogo.jogadores[posCobra].posicoesCobra[posArrayAux][0] = auxLinha;
					jogo.jogadores[posCobra].posicoesCobra[posArrayAux][1] = auxColuna + 1;
					jogo.jogadores[posCobra].porAparecer--;
					vistaPartilhaGeral->mapa[auxLinha][auxColuna + 1] = idCobraNoMapa;
				}
				else {
					posArrayAux = jogo.jogadores[posCobra].tamanho - 1;
					//Apagar a cauda do mapa
					auxLinha = jogo.jogadores[posCobra].posicoesCobra[0][0];
					auxColuna = jogo.jogadores[posCobra].posicoesCobra[0][1];
					vistaPartilhaGeral->mapa[auxLinha][auxColuna] = ESPACOVAZIO;
					//Buscar a posição da cabeça
					auxLinha = jogo.jogadores[posCobra].posicoesCobra[posArrayAux][0];
					auxColuna = jogo.jogadores[posCobra].posicoesCobra[posArrayAux][1];
					//Mover todas as posições da cobra no array sobrepondo as antigas
					for (int z = 0; z < posArrayAux; z++) {
						jogo.jogadores[posCobra].posicoesCobra[z][0] = jogo.jogadores[posCobra].posicoesCobra[z + 1][0];
						jogo.jogadores[posCobra].posicoesCobra[z][1] = jogo.jogadores[posCobra].posicoesCobra[z + 1][1];
					}
					jogo.jogadores[posCobra].posicoesCobra[posArrayAux][0] = auxLinha;
					jogo.jogadores[posCobra].posicoesCobra[posArrayAux][1] = auxColuna + 1;
					vistaPartilhaGeral->mapa[auxLinha][auxColuna + 1] = idCobraNoMapa;
				}
				break;
			}
		}
		_tprintf(TEXT("**********ITERAÇÂO DO MAPA**********\n"));
		SetEvent(hEventoMapa);
		ResetEvent(hEventoMapa);
		ReleaseSemaphore(hSemaforoMapa, MAXCLIENTES, NULL);
	}
}

//Gera as posições da cobra no mapa verificando se há colisões com paredes e fazendo a respectiva alteração á cobra
void criaCobra(TCHAR username[SIZE_USERNAME], int vaga, int pid, int jogador) {
	int posXGerada, posYGerada, dirGerada;
	//Gera posições até encontrar uma vaga;
	while (1) {
		posXGerada = random_at_most((long)jogo.config.C);
		posYGerada = random_at_most((long)jogo.config.L);
		if (vistaPartilhaGeral->mapa[posYGerada][posXGerada] == ESPACOVAZIO)
			break;
	}

	//Na posição 0 do array de posições ficam as Linhas e na 1 ficam as Colunas
	jogo.jogadores[vaga].posicoesCobra[0][0] = posYGerada;
	jogo.jogadores[vaga].posicoesCobra[0][1] = posXGerada;
	//falta efectuar as contas com a vaga do array para colocar esse valor no mapa
	vistaPartilhaGeral->mapa[posYGerada][posXGerada] = vaga;

	dirGerada = random_at_most(3) + 1;
	jogo.jogadores[vaga].direcao = dirGerada;

	jogo.jogadores[vaga].porAparecer = jogo.config.T - 1;
	jogo.jogadores[vaga].estadoJogador = VIVO;
	jogo.jogadores[vaga].pontuacao = 0;
	jogo.jogadores[vaga].jogador = jogador;
	jogo.jogadores[vaga].pid = pid;
	jogo.jogadores[vaga].tamanho = jogo.config.T;
	_tcscpy_s(jogo.jogadores[vaga].username, SIZE_USERNAME, username);
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
		x = rand();
	}
	// This is carefully written not to overflow
	while (num_rand - defect <= (unsigned long)x);

	// Truncated division is intentional
	return x / bin_size;
}

int AssociaJogo(TCHAR username[SIZE_USERNAME], int pid, int jogador) {
	
	//Se não existir jogo criado ou não existirem vagas
	if ((jogo.estadoJogo != ASSOCIACAOJOGO) || ((jogo.config.N - jogo.vagasJogadores) == 0)) {
		_tprintf(TEXT("**********ERRO ASSOCIAR JOGO**********\n"));
		return 0;
	}

	criaCobra(username, jogo.vagasJogadores,pid,jogador);
	jogo.vagasJogadores++;
		
	return 1;
}

int IniciaJogo(int pid) {
	DWORD tid;
	HANDLE hThread; 

	//Se não for o computador que criou o jogo não pode dar inicio a este
	if (!(jogo.pidCriador==pid) && (jogo.estadoJogo == ASSOCIACAOJOGO)){
		_tprintf(TEXT("**********ERRO INICIAR JOGO**********\n"));
		return 0;
	}
	//Se for, criar as threads de mover as cobras, de gerir objectos e gerir cobras automaticas e notificar jogadores
	jogo.estadoJogo = DECORRERJOGO;
	for (int i = 0; i < jogo.vagasJogadores; i++) {
		hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)moveCobras, (LPVOID)i, 0, &tid);
	}
	
	
	return 1;
}

int Cria_Jogo(ConfigInicial param, int pid) {
	
	if ((jogo.estadoJogo != CRIACAOJOGO)) {
		return 0;
	}

	jogo.estadoJogo = ASSOCIACAOJOGO;
	jogo.config = param;
	jogo.pidCriador = pid;
	jogo.vagasJogadores = 0;
	vistaPartilhaGeral->colunas = param.C;
	vistaPartilhaGeral->linhas = param.L;

	return 1;
}

void preparaMapaJogo() {
	for (int z = 0; z < vistaPartilhaGeral->colunas; z++) {
		vistaPartilhaGeral->mapa[0][z] = PAREDE;
		vistaPartilhaGeral->mapa[vistaPartilhaGeral->linhas - 1][z] = PAREDE;
		for (int j = 1; j < vistaPartilhaGeral->linhas - 1; j++) {
			if (z == 0 || z == vistaPartilhaGeral->colunas - 1) {
				vistaPartilhaGeral->mapa[j][z] = PAREDE;
			}
			else
				vistaPartilhaGeral->mapa[j][z] = ESPACOVAZIO;
		}
	}
}

/*----------------------------------------------------------------- */
/*  THREAD - Função que gere os objectos no mapa				 	*/
/* ---------------------------------------------------------------- */
DWORD WINAPI gereObjectos(LPVOID param) {
	//Cria os objetos no mapa
	for (int i = 0; i < jogo.config.O; i++) {

	}
	while (1) {
		Sleep(SEGUNDO);

	}
}

//altera a direcao do jogador 1 ou 2 de determinado pid, se a mudança de direção for inversa a actual ignora o movimento
void mudaDirecaoJogador(int direcao, int pid, int jogador) {
	int posicao;
	posicao = procuraJogador(pid, jogador);
	switch (direcao)
	{
	case CIMA:if(jogo.jogadores[posicao].direcao !=BAIXO)
		jogo.jogadores[posicao].direcao = direcao;
		break;
	case BAIXO:if (jogo.jogadores[posicao].direcao != CIMA)
		jogo.jogadores[posicao].direcao = direcao;
		break;
	case ESQUERDA:if (jogo.jogadores[posicao].direcao != DIREITA)
		jogo.jogadores[posicao].direcao = direcao;
		break;
	case DIREITA:if (jogo.jogadores[posicao].direcao != ESQUERDA)
		jogo.jogadores[posicao].direcao = direcao;
		break;
	default:
		break;
	}
	
	
}

//procura o jogador mencionado na lista de jogadores em jogo
int procuraJogador(int pid, int jogador) {
	for (int i = 0; i < jogo.config.N; i++) {
		if (jogo.jogadores[i].pid == pid && jogo.jogadores[i].jogador == jogador) {
			_tprintf(TEXT("Posicao do jogador no array:%d\n"), i);
			return i;
		}
			
	}
	
	return -1;
}


