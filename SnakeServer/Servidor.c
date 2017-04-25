#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "TiposConstantes.h"

/* ----------------------------------------------------- */
/*  VARIÁVEIS GLOBAIS									 */
/* ----------------------------------------------------- */
HANDLE	hMemoria;
HANDLE	hSemMemoria;
HANDLE  hEventoMemoria;

/* ----------------------------------------------------- */
/*  PROTOTIPOS FUNÇÕES DAS THREADS						 */
/* ----------------------------------------------------- */


/* ----------------------------------------------------- */
/*  Função MAIN											 */
/* ----------------------------------------------------- */
int _tmain(int argc, LPTSTR argv[]) {
	Msg *partilha, aux;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	_tcscpy_s(aux.utilizador, sizeof(aux.utilizador), TEXT("Server"));
	aux.codigoMsg = ACTUALIZAMAPA;	

	hEventoMemoria = CreateEvent(NULL, TRUE, FALSE, EVENTO_MEM);
	hSemMemoria = CreateSemaphore(NULL, 2, 2, SEMAFORO_MEM);

	hMemoria = CreateFileMapping((HANDLE)INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, SIZEMENSAGEM, NOMEMEMORIA);

	partilha = (Msg*)MapViewOfFile(hMemoria, FILE_MAP_ALL_ACCESS, 0, 0, SIZEMENSAGEM);

	
	
	partilha->codigoMsg = aux.codigoMsg;
	_tcscpy_s(partilha->utilizador, MAXUTILIZADOR, aux.utilizador);
	_gettch();

	CloseHandle(hMemoria);
	UnmapViewOfFile(partilha);

	return 0;
}
