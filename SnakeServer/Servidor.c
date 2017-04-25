#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "TiposConstantes.h"

/* ----------------------------------------------------- */
/*  VARI�VEIS GLOBAIS									 */
/* ----------------------------------------------------- */
HANDLE	hMemoria;
HANDLE	hSemMemoria;
HANDLE  hEventoMemoria;

/* ----------------------------------------------------- */
/*  PROTOTIPOS FUN��ES DAS THREADS						 */
/* ----------------------------------------------------- */


/* ----------------------------------------------------- */
/*  Fun��o MAIN											 */
/* ----------------------------------------------------- */
int _tmain(int argc, LPTSTR argv[]) {
	Msg *partilha, aux;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	_tcscpy_s(aux.username, sizeof(aux.username), TEXT("Server"));
	aux.codigoMsg = ACTUALIZAMAPA;	

	hEventoMemoria = CreateEvent(NULL, TRUE, FALSE, EVNT_MEM_GERAL);
	hSemMemoria = CreateSemaphore(NULL, 2, 2, SEM_MEM_GERAL);

	hMemoria = CreateFileMapping((HANDLE)INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, SIZE_MEM_GERAL, NOME_MEM_GERAL);

	partilha = (Msg*)MapViewOfFile(hMemoria, FILE_MAP_ALL_ACCESS, 0, 0, SIZE_MEM_GERAL);

	partilha->codigoMsg = aux.codigoMsg;
	_tcscpy_s(partilha->username, SIZE_USERNAME, aux.username);
	_gettch();

	CloseHandle(hMemoria);
	UnmapViewOfFile(partilha);

	return 0;
}
