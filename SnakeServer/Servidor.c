#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
//#include "TiposConstantes.h"
#include "..\..\DLL\DLL\DLL.h"

/* ----------------------------------------------------- */
/*  VARIÁVEIS GLOBAIS									 */
/* ----------------------------------------------------- */
Jogador *listaJogadores;
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

int criaCobra() {

}


/*	
	


*/

