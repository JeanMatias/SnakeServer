#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h> 
#include "..\..\SnakeDLL\\SnakeDLL\SnakeDLL.h"

/* ----------------------------------------------------- */
/*  VARIÁVEIS GLOBAIS									 */
/* ----------------------------------------------------- */
Jogador *listaJogadores;//para usar mais tarde com o Registo
BOOLEAN jogoADecorrer = FALSE;

DWORD WINAPI moveCobras(LPVOID param);

/* ----------------------------------------------------- */
/*  Função MAIN											 */
/* ----------------------------------------------------- */
int _tmain(int argc, LPTSTR argv[]) {
	MemGeral  aux;
	DWORD tid;
	HANDLE hThread;

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
			break;
		case INICIARJOGO:
			if (!jogoADecorrer) {
				//criar threads de interação com o mapa
				hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)moveCobras, NULL, 0, &tid);
				jogoADecorrer = TRUE;
			}
			
		default:
			break;
		}
	}
	_gettch();


	fechaMemoriaPartilhada();
	
	return 0;
}

/*----------------------------------------------------------------- */
/*  THREAD - Função que movimenta as cobras no mapa				 	*/
/* ---------------------------------------------------------------- */
DWORD WINAPI moveCobras(LPVOID param) {
	int posArrayAux, auxLinha, auxColuna;

	while (1) {
		//Aqui deve esperar pelo Tîmer para que este avise que passou um instante(?)
		Sleep(100);
		for (int i = 0; i < MAXCLIENTES; i++) {
			WaitForSingleObject(hSemMemoria, INFINITE);
		}
		for (int j = 0; j < vistaPartilhaGeral->config.N; j++) {
			//mexer so as cobras vivas
			if (vistaPartilhaGeral->jogadores[j].estadoJogador == VIVO) {
				switch (vistaPartilhaGeral->jogadores[j].direcao)
				{
				case CIMA://Coluna mantem-se, muda de linha(-1)
					if (vistaPartilhaGeral->jogadores[j].porAparecer != 0) {
						posArrayAux = vistaPartilhaGeral->jogadores[j].tamanho - vistaPartilhaGeral->jogadores[j].porAparecer;
						//Buscar a posição da cabeça
						auxLinha = vistaPartilhaGeral->jogadores[j].posicoesCobra[posArrayAux - 1][0];
						auxColuna = vistaPartilhaGeral->jogadores[j].posicoesCobra[posArrayAux - 1][1];
						vistaPartilhaGeral->jogadores[j].posicoesCobra[posArrayAux][0] = auxLinha-1;
						vistaPartilhaGeral->jogadores[j].posicoesCobra[posArrayAux][1] = auxColuna;
						vistaPartilhaGeral->jogadores[j].porAparecer--;
						vistaPartilhaGeral->mapa[auxLinha - 1][auxColuna] = j + '0';
					}
					else {
						posArrayAux = vistaPartilhaGeral->jogadores[j].tamanho - 1;
						//Apagar a cauda do mapa
						auxLinha = vistaPartilhaGeral->jogadores[j].posicoesCobra[0][0];
						auxColuna = vistaPartilhaGeral->jogadores[j].posicoesCobra[0][1];
						vistaPartilhaGeral->mapa[auxLinha][auxColuna] = ' ';
						//Buscar a posição da cabeça
						auxLinha = vistaPartilhaGeral->jogadores[j].posicoesCobra[posArrayAux][0];
						auxColuna = vistaPartilhaGeral->jogadores[j].posicoesCobra[posArrayAux][1];
						//Mover todas as posições da cobra no array sobrepondo as antigas
						for (int z = 0; z < posArrayAux; z++) {
							vistaPartilhaGeral->jogadores[j].posicoesCobra[z][0] = vistaPartilhaGeral->jogadores[j].posicoesCobra[z+1][0];
							vistaPartilhaGeral->jogadores[j].posicoesCobra[z][1] = vistaPartilhaGeral->jogadores[j].posicoesCobra[z+1][1];
						}
						vistaPartilhaGeral->jogadores[j].posicoesCobra[posArrayAux][0] = auxLinha - 1;
						vistaPartilhaGeral->jogadores[j].posicoesCobra[posArrayAux][1] = auxColuna;
						vistaPartilhaGeral->mapa[auxLinha - 1][auxColuna] = j + '0';
						
					}					
					break;
				case BAIXO://Coluna mantem-se, muda de linha(+1)
					if (vistaPartilhaGeral->jogadores[j].porAparecer != 0) {
						posArrayAux = vistaPartilhaGeral->jogadores[j].tamanho - vistaPartilhaGeral->jogadores[j].porAparecer;
						//Buscar a posição da cabeça
						auxLinha = vistaPartilhaGeral->jogadores[j].posicoesCobra[posArrayAux - 1][0];
						auxColuna = vistaPartilhaGeral->jogadores[j].posicoesCobra[posArrayAux - 1][1];
						vistaPartilhaGeral->jogadores[j].posicoesCobra[posArrayAux][0] = auxLinha + 1;
						vistaPartilhaGeral->jogadores[j].posicoesCobra[posArrayAux][1] = auxColuna;
						vistaPartilhaGeral->jogadores[j].porAparecer--;
						vistaPartilhaGeral->mapa[auxLinha + 1][auxColuna] = j + '0';
					}
					else {
						posArrayAux = vistaPartilhaGeral->jogadores[j].tamanho - 1;
						//Apagar a cauda do mapa
						auxLinha = vistaPartilhaGeral->jogadores[j].posicoesCobra[0][0];
						auxColuna = vistaPartilhaGeral->jogadores[j].posicoesCobra[0][1];
						vistaPartilhaGeral->mapa[auxLinha][auxColuna] = ' ';
						//Buscar a posição da cabeça
						auxLinha = vistaPartilhaGeral->jogadores[j].posicoesCobra[posArrayAux][0];
						auxColuna = vistaPartilhaGeral->jogadores[j].posicoesCobra[posArrayAux][1];
						//Mover todas as posições da cobra no array sobrepondo as antigas
						for (int z = 0; z < posArrayAux; z++) {
							vistaPartilhaGeral->jogadores[j].posicoesCobra[z][0] = vistaPartilhaGeral->jogadores[j].posicoesCobra[z + 1][0];
							vistaPartilhaGeral->jogadores[j].posicoesCobra[z][1] = vistaPartilhaGeral->jogadores[j].posicoesCobra[z + 1][1];
						}
						vistaPartilhaGeral->jogadores[j].posicoesCobra[posArrayAux][0] = auxLinha + 1;
						vistaPartilhaGeral->jogadores[j].posicoesCobra[posArrayAux][1] = auxColuna;
						vistaPartilhaGeral->mapa[auxLinha + 1][auxColuna] = j + '0';

					}
					break;
				case ESQUERDA://Linha mantem-se, muda de coluna(-1)
					if (vistaPartilhaGeral->jogadores[j].porAparecer != 0) {
						posArrayAux = vistaPartilhaGeral->jogadores[j].tamanho - vistaPartilhaGeral->jogadores[j].porAparecer;
						//Buscar a posição da cabeça
						auxLinha = vistaPartilhaGeral->jogadores[j].posicoesCobra[posArrayAux - 1][0];
						auxColuna = vistaPartilhaGeral->jogadores[j].posicoesCobra[posArrayAux - 1][1];
						vistaPartilhaGeral->jogadores[j].posicoesCobra[posArrayAux][0] = auxLinha;
						vistaPartilhaGeral->jogadores[j].posicoesCobra[posArrayAux][1] = auxColuna - 1;
						vistaPartilhaGeral->jogadores[j].porAparecer--;
						vistaPartilhaGeral->mapa[auxLinha][auxColuna - 1] = j + '0';
					}
					else {
						posArrayAux = vistaPartilhaGeral->jogadores[j].tamanho - 1;
						//Apagar a cauda do mapa
						auxLinha = vistaPartilhaGeral->jogadores[j].posicoesCobra[0][0];
						auxColuna = vistaPartilhaGeral->jogadores[j].posicoesCobra[0][1];
						vistaPartilhaGeral->mapa[auxLinha][auxColuna] = ' ';
						//Buscar a posição da cabeça
						auxLinha = vistaPartilhaGeral->jogadores[j].posicoesCobra[posArrayAux][0];
						auxColuna = vistaPartilhaGeral->jogadores[j].posicoesCobra[posArrayAux][1];
						//Mover todas as posições da cobra no array sobrepondo as antigas
						for (int z = 0; z < posArrayAux; z++) {
							vistaPartilhaGeral->jogadores[j].posicoesCobra[z][0] = vistaPartilhaGeral->jogadores[j].posicoesCobra[z + 1][0];
							vistaPartilhaGeral->jogadores[j].posicoesCobra[z][1] = vistaPartilhaGeral->jogadores[j].posicoesCobra[z + 1][1];
						}
						vistaPartilhaGeral->jogadores[j].posicoesCobra[posArrayAux][0] = auxLinha;
						vistaPartilhaGeral->jogadores[j].posicoesCobra[posArrayAux][1] = auxColuna - 1;
						vistaPartilhaGeral->mapa[auxLinha][auxColuna - 1] = j + '0';

					}
					break;
				case DIREITA://Linha mantem-se, muda de coluna(+1)
					if (vistaPartilhaGeral->jogadores[j].porAparecer != 0) {
						posArrayAux = vistaPartilhaGeral->jogadores[j].tamanho - vistaPartilhaGeral->jogadores[j].porAparecer;
						//Buscar a posição da cabeça
						auxLinha = vistaPartilhaGeral->jogadores[j].posicoesCobra[posArrayAux - 1][0];
						auxColuna = vistaPartilhaGeral->jogadores[j].posicoesCobra[posArrayAux - 1][1];
						vistaPartilhaGeral->jogadores[j].posicoesCobra[posArrayAux][0] = auxLinha;
						vistaPartilhaGeral->jogadores[j].posicoesCobra[posArrayAux][1] = auxColuna + 1;
						vistaPartilhaGeral->jogadores[j].porAparecer--;
						vistaPartilhaGeral->mapa[auxLinha][auxColuna + 1] = j + '0';
					}
					else {
						posArrayAux = vistaPartilhaGeral->jogadores[j].tamanho - 1;
						//Apagar a cauda do mapa
						auxLinha = vistaPartilhaGeral->jogadores[j].posicoesCobra[0][0];
						auxColuna = vistaPartilhaGeral->jogadores[j].posicoesCobra[0][1];
						vistaPartilhaGeral->mapa[auxLinha][auxColuna] = ' ';
						//Buscar a posição da cabeça
						auxLinha = vistaPartilhaGeral->jogadores[j].posicoesCobra[posArrayAux][0];
						auxColuna = vistaPartilhaGeral->jogadores[j].posicoesCobra[posArrayAux][1];
						//Mover todas as posições da cobra no array sobrepondo as antigas
						for (int z = 0; z < posArrayAux; z++) {
							vistaPartilhaGeral->jogadores[j].posicoesCobra[z][0] = vistaPartilhaGeral->jogadores[j].posicoesCobra[z + 1][0];
							vistaPartilhaGeral->jogadores[j].posicoesCobra[z][1] = vistaPartilhaGeral->jogadores[j].posicoesCobra[z + 1][1];
						}
						vistaPartilhaGeral->jogadores[j].posicoesCobra[posArrayAux][0] = auxLinha;
						vistaPartilhaGeral->jogadores[j].posicoesCobra[posArrayAux][1] = auxColuna + 1;
						vistaPartilhaGeral->mapa[auxLinha][auxColuna + 1] = j + '0';
					}
					break;
				}
			}
		}

		SetEvent(hEventoMemoria);
		ResetEvent(hEventoMemoria);
		ReleaseSemaphore(hSemMemoria, MAXCLIENTES, NULL);
	}
}



