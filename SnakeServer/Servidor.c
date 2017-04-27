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
HANDLE	hMemoria;
HANDLE	hSemMemoria;
HANDLE  hEventoMemoria;
MemGeral *vistaPartilha;

/* ----------------------------------------------------- */
/*  PROTOTIPOS FUNÇÕES DAS THREADS						 */
/* ----------------------------------------------------- */
void preparaMemoriaPartilhada(void);
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

	preparaMemoriaPartilhada();
	inicializaMemoriaPartilhada();
	

	while (1) {

		esperaPorActualizacao();

		leMemoriaPartilhada(&aux);

		_tprintf(TEXT("NumClientes: %d \n Estado:%d"), aux.numClientes, aux.estadoJogo);
		
	}
	_gettch();


	fechaMemoriaPartilhada();
	
	return 0;
}


/* ----------------------------------------------------- */
/*  Funções para a DLL									 */
/* ----------------------------------------------------- */
void preparaMemoriaPartilhada(void){

	hMemoria = CreateFileMapping((HANDLE)INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, SIZE_MEM_GERAL, NOME_MEM_GERAL);

	vistaPartilha = (MemGeral*)MapViewOfFile(hMemoria, FILE_MAP_ALL_ACCESS, 0, 0, SIZE_MEM_GERAL);

	hEventoMemoria = CreateEvent(NULL, TRUE, FALSE, EVNT_MEM_GERAL);
	hSemMemoria = CreateSemaphore(NULL, MAXCLIENTES, MAXCLIENTES, SEM_MEM_GERAL);
}

void inicializaMemoriaPartilhada(void) {
	//Inicialização da Memoria Partilhada
	vistaPartilha->numClientes = 0;
	vistaPartilha->estadoJogo = CRIACAOJOGO;
}

void esperaPorActualizacao(void) {
	WaitForSingleObject(hEventoMemoria, INFINITE);
}

void leMemoriaPartilhada(MemGeral* param) {
	
	WaitForSingleObject(hSemMemoria, INFINITE);

	param->numClientes = vistaPartilha->numClientes;
	param->estadoJogo = vistaPartilha->estadoJogo;

	ReleaseSemaphore(hSemMemoria, 1, NULL);
}

void fechaMemoriaPartilhada(void) {
	CloseHandle(hMemoria);
	CloseHandle(hSemMemoria);
	CloseHandle(hEventoMemoria);
	UnmapViewOfFile(vistaPartilha);
}

