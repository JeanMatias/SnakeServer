#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "TiposConstantes.h"

/* ----------------------------------------------------- */
/*  VARIÁVEIS GLOBAIS									 */
/* ----------------------------------------------------- */

/* ----------------------------------------------------- */
/*  VARIÁVEIS GLOBAIS DLL								 */
/* ----------------------------------------------------- */
HANDLE hMemoria;
HANDLE hSemMemoria;
HANDLE hEventoMemoria;
HANDLE hFicheiro;
MemGeral *vistaPartilhaGeral;
/* ----------------------------------------------------- */
/*  PROTOTIPOS FUNÇÕES DA DLL							 */
/* ----------------------------------------------------- */
void preparaMemoriaPartilhada(void);
void preparaMapaJogo(MemGeral param);
void inicializaMemoriaPartilhada(void);
void esperaPorActualizacao(void);
void leMemoriaPartilhada(MemGeral* param);
void fechaMemoriaPartilhada(void);

/* ----------------------------------------------------- */
/*  Função MAIN											 */
/* ----------------------------------------------------- */
int _tmain(int argc, LPTSTR argv[]) {
	MemGeral  aux;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	hFicheiro=CreateFile(FILE_MAP_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

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


/* ----------------------------------------------------- */
/*  Funções para a DLL									 */
/* ----------------------------------------------------- */
void preparaMemoriaPartilhada(void){

	hMemoria = CreateFileMapping(hFicheiro, NULL, PAGE_READWRITE, 0, SIZE_MEM_GERAL, NOME_MEM_GERAL);
	
	vistaPartilhaGeral = (MemGeral*)MapViewOfFile(hMemoria, FILE_MAP_ALL_ACCESS, 0, 0, SIZE_MEM_GERAL);

	hEventoMemoria = CreateEvent(NULL, TRUE, FALSE, EVNT_MEM_GERAL);
	hSemMemoria = CreateSemaphore(NULL, MAXCLIENTES, MAXCLIENTES, SEM_MEM_GERAL);

	if (hMemoria == NULL || hEventoMemoria == NULL || hSemMemoria == NULL) {
		_tprintf(TEXT("[Erro] Criação de objectos do Windows(%d)\n"), GetLastError());
		return -1;
	}

}

void preparaMapaJogo(MemGeral param) {

	for (int i = 0; i < MAXCLIENTES - 1; i++) {
		WaitForSingleObject(hSemMemoria, INFINITE);
	}

	for (int z = 0; z < param.config.C; z++) {
		vistaPartilhaGeral->mapa[0][z] = TEXT('#');
		vistaPartilhaGeral->mapa[param.config.L-1][z] = TEXT('#');
		for (int j = 1; j < param.config.L-1; j++) {
			if (z == 0 || z == param.config.C-1) {
				vistaPartilhaGeral->mapa[j][z] = TEXT('#');
			}
			else
				vistaPartilhaGeral->mapa[j][z] = TEXT(' ');
		}
	}
	SetEvent(hEventoMemoria);
	ResetEvent(hEventoMemoria);
	ReleaseSemaphore(hSemMemoria, MAXCLIENTES, NULL);	
}

void inicializaMemoriaPartilhada(void) {
	//Inicialização da Memoria Partilhada
	vistaPartilhaGeral->estadoJogo = CRIACAOJOGO;
}

void esperaPorActualizacao(void) {
	WaitForSingleObject(hEventoMemoria, INFINITE);
}

void leMemoriaPartilhada(MemGeral* param) {
	
	WaitForSingleObject(hSemMemoria, INFINITE);

	param->estadoJogo = vistaPartilhaGeral->estadoJogo;
	param->mensagem.codigoMsg = vistaPartilhaGeral->mensagem.codigoMsg;
	_tcscpy_s(param->mensagem.username, SIZE_USERNAME, vistaPartilhaGeral->mensagem.username);
	param->config.C = vistaPartilhaGeral->config.C;
	param->config.L = vistaPartilhaGeral->config.L;

	ReleaseSemaphore(hSemMemoria, 1, NULL);
}

void fechaMemoriaPartilhada(void) {
	CloseHandle(hMemoria);
	CloseHandle(hSemMemoria);
	CloseHandle(hEventoMemoria);
	UnmapViewOfFile(vistaPartilhaGeral);
}


/*			AFAZER
	Funçao de leitura do mapa para a memoria dinamica
	Criar Projecto da DLL
	Associar Clientes ao Jogo (Escrever na memoria dinamica das cobras)
	


*/

