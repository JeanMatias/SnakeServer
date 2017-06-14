//SnakeDLL.h
// O bloco ifdef seguinte � o modo standard de criar macros que tornam a exporta��o de
// fun��es e vari�veis mais simples. Todos os ficheiros neste projeto DLL s�o
// compilados com o s�mbolo DLL_IMP_EXPORTS definido. Este s�mbolo n�o deve ser definido
// em nenhum projeto que use a DLL. Desta forma, qualquer outro projeto que inclua este
// este ficheiro ir� ver as fun��es e vari�veis DLL_IMP_API como sendo importadas de uma
// DLL.

#include <windows.h>
#include <tchar.h>
#include "TiposConstantes.h"

//Definir uma constante para facilitar a leitura do prot�tipo da fun��o
//Este .h deve ser inclu�do no projeto que o vai usar (modo impl�cito)

//Esta macro � definida pelo sistema caso estejamos na DLL (<DLL_IMP>_EXPORTS definida)
//ou na app (<DLL_IMP>_EXPORTS n�o definida) onde DLL_IMP � o nome deste projeto
//#ifdef DLL_EXPORTS

//**********************************************************************************
#ifdef SNAKEDLL_EXPORTS   // VEIO DE -> PROP PROJ -> C++ -> LINHA COMANDO   -->  Est� l� o nome do proj + o EXPORT, copiar para aqui
//**********************************************************************************

#define DLL_IMP_API __declspec(dllexport)   // Deve ser acrescentado antes de cada declara��o de fun��o ****************
#else										// ou var global a usar fora da DLL, � para exportar		****************
#define DLL_IMP_API __declspec(dllimport)   // Se estiver na DLL faz o de sima, se n�o estiver faz este
#endif

//Vari�vel global da DLL
extern DLL_IMP_API HANDLE hMemoria;
extern DLL_IMP_API HANDLE hSemaforoMapa;
extern DLL_IMP_API HANDLE hPodeLerPedido;
extern DLL_IMP_API HANDLE hPodeEscreverPedido;
extern DLL_IMP_API HANDLE hEventoMapa;
extern DLL_IMP_API HANDLE hFicheiro;
extern DLL_IMP_API HANDLE hMemResposta;
extern DLL_IMP_API HANDLE hEventoResposta;
extern DLL_IMP_API MemGeral *vistaPartilhaGeral;
extern DLL_IMP_API Resposta *vistaResposta;

//Fun��es a serem exportadas/importadas
DLL_IMP_API int preparaMemoriaPartilhada(void);
DLL_IMP_API int preparaMemoriaPartilhadaResposta(int pid, int tid);
DLL_IMP_API int pede_CriaJogo(ConfigInicial param, int pid, int tid, TCHAR username[SIZE_USERNAME],ConfigObjecto objectosConfig[NUMTIPOOBJECTOS]);
DLL_IMP_API int pede_Sair(int pid, int tid);
DLL_IMP_API int pede_IniciaJogo(int pid, int tid);
DLL_IMP_API int pede_RegistarClienteRemoto(int pid, int tid);
DLL_IMP_API int pede_RegistarClienteLocal(int pid, int tid);
DLL_IMP_API int pede_AssociaJogo(int pid, int tid, TCHAR username[SIZE_USERNAME], int codigoPedido);
DLL_IMP_API void esperaPorActualizacaoMapa(void);
DLL_IMP_API void mudaDirecao(int direcao, int pid, int tid, int jogador);
DLL_IMP_API void fechaMemoriaPartilhadaGeral(void);
DLL_IMP_API void fechaMemoriaPartilhadaResposta(void);
DLL_IMP_API void getMapa(int mapa[MAX_LINHAS][MAX_COLUNAS]);
DLL_IMP_API void getLimitesMapa(int *linhas, int *colunas);

