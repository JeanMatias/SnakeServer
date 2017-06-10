#include "Servidor.h"

/* ----------------------------------------------------- */
/*  Função MAIN											 */
/* ----------------------------------------------------- */
int _tmain(int argc, LPTSTR argv[]) {
	
	Pedido aux;
	Resposta aux2;
	int indice;
	int resultado;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif
	

	srand((int)time(NULL));

	if (criaMemoriaPartilhada() == -1) {
		_tprintf(TEXT("ERRO"));
		exit(-1);
	}

	resetDadosJogo();

	while (1) {

		lePedidoDaFila(&aux);

		_tprintf(TEXT("Pid:%d CodigoMSG:%d\n"), aux.pid,aux.codigoPedido);
		switch (aux.codigoPedido)
		{
		case REGISTACLIENTELOCAL:indice = registaCliente(aux.pid, aux.tid, FALSE);
							if (indice != -1) {
								aux2.resposta = SUCESSO;
								notificaCliente(indice, aux2);
							}
			break;
		case REGISTACLIENTEREMTO:registaCliente(aux.pid, aux.tid, TRUE);
			break;
		case CRIARJOGO:indice = procuraCliente(aux.pid, aux.tid);
			resultado = Cria_Jogo(aux.config, aux.pid, aux.tid, aux.username);
			if (resultado == AGORANAO) {
				aux2.resposta = INSUCESSO;
				notificaCliente(indice, aux2);
			}
			else {
				aux2.resposta = SUCESSO;
				aux2.valor = resultado;//metemos no valor da resposta o valor que vai aparecer no mapa para esta cobra 
				notificaCliente(indice, aux2); //          para que o cliente saiba a cor com que a vai desenhar
			}
			break;
		case ASSOCIAR_JOGADOR1:indice = procuraCliente(aux.pid, aux.tid);
				resultado = AssociaJogo(aux.username, aux.pid, aux.tid, JOGADOR1);
				if (resultado == AGORANAO) {
					aux2.resposta = INSUCESSO;
					aux2.valor = AGORANAO;
					notificaCliente(indice, aux2);
				}
				else if (resultado == JOGOCHEIO) {
					aux2.resposta = INSUCESSO;
					aux2.valor = JOGOCHEIO;
					notificaCliente(indice, aux2);
				}
				else {
					aux2.resposta = SUCESSO;
					aux2.valor = resultado;//metemos no valor da resposta o valor que vai aparecer no mapa para esta cobra 
					notificaCliente(indice, aux2); //          para que o cliente saiba a cor com que a vai desenhar
				}
			break;
		case ASSOCIAR_JOGADOR2:indice = procuraCliente(aux.pid, aux.tid);
				resultado = AssociaJogo(aux.username, aux.pid, aux.tid, JOGADOR2);
				if (resultado == AGORANAO) {
					aux2.resposta = INSUCESSO;
					aux2.valor = AGORANAO;
					notificaCliente(indice, aux2);
				}
				else if (resultado == JOGOCHEIO) {
					aux2.resposta = INSUCESSO;
					aux2.valor = JOGOCHEIO;
					notificaCliente(indice, aux2);
				}
				else {
					aux2.resposta = SUCESSO;
					aux2.valor = resultado;//metemos no valor da resposta o valor que vai aparecer no mapa para esta cobra 
					notificaCliente(indice, aux2); //          para que o cliente saiba a cor com que a vai desenhar
				}
				break;
		case INICIARJOGO:indice = procuraCliente(aux.pid, aux.tid);
			resultado = IniciaJogo(aux.pid, aux.tid);
			if (resultado == AGORANAO) {
				aux2.resposta = INSUCESSO;
				aux2.valor = AGORANAO;
				notificaCliente(indice, aux2);
			}
			else if (resultado == CRIADORERRADO) {
				aux2.resposta = INSUCESSO;
				aux2.valor = CRIADORERRADO;
				notificaCliente(indice, aux2);
			}
			else {
				aux2.resposta = SUCESSO;
				notificaCliente(indice, aux2);
			}
			break;
		case CIMA:
		case BAIXO:
		case ESQUERDA:
		case DIREITA:
			mudaDirecaoJogador(aux.codigoPedido, aux.pid, aux.tid, aux.jogador);
			break;
		default:
			break;
		}
	}
	_gettch();


	fechaMemoriaPartilhadaGeral();
	
	return 0;
}

//Mete dados do jogo para ser possivel aceitar pedidos de criação de jogo.
void resetDadosJogo() {
	jogo.estadoJogo = CRIACAOJOGO;
	jogo.vagasJogadores = 0;
	jogo.cobrasVivas = 0;
}

void lePedidoDaFila(Pedido *param){

	WaitForSingleObject(hPodeLerPedidoServidor, INFINITE);

	param->pid = vistaPartilhaGeralServidor->fila.pedidos[vistaPartilhaGeralServidor->fila.frente].pid;
	param->tid = vistaPartilhaGeralServidor->fila.pedidos[vistaPartilhaGeralServidor->fila.frente].tid;
	param->jogador = vistaPartilhaGeralServidor->fila.pedidos[vistaPartilhaGeralServidor->fila.frente].jogador;
	param->codigoPedido = vistaPartilhaGeralServidor->fila.pedidos[vistaPartilhaGeralServidor->fila.frente].codigoPedido;
	param->config = vistaPartilhaGeralServidor->fila.pedidos[vistaPartilhaGeralServidor->fila.frente].config;
	_tcscpy_s(param->username , SIZE_USERNAME, vistaPartilhaGeralServidor->fila.pedidos[vistaPartilhaGeralServidor->fila.frente].username);
	for (int i = 0; i < NUMTIPOOBJECTOS; i++)
		param->objectosConfig[i] = vistaPartilhaGeralServidor->fila.pedidos[vistaPartilhaGeralServidor->fila.frente].objectosConfig[i];

	vistaPartilhaGeralServidor->fila.frente++;

	//chegou ao fim da fila temos de voltar a por desde o inicio da fila
	if (vistaPartilhaGeralServidor->fila.frente == MAX_PEDIDOS) {
		vistaPartilhaGeralServidor->fila.frente = 0;
	}

	ReleaseSemaphore(hPodeEscreverPedidoServidor, 1, NULL);
}

/*----------------------------------------------------------------- */
/*  THREAD - Função que movimenta uma cobra no mapa				 	*/
/* ---------------------------------------------------------------- */
DWORD WINAPI moveCobras(LPVOID param) {
	int posArrayAux, auxLinha, auxColuna;
	int idCobraNoMapaDefeito, idCobraNoMapa, posArrayCobra = (int)param;
	idCobraNoMapaDefeito = (posArrayCobra + 1) * 100;//calcular o id da cobra no mapa 
	while (jogo.jogadores[posArrayCobra].estadoJogador != MORTO) {
			//tratar os estados da cobra
			switch (jogo.jogadores[posArrayCobra].estadoJogador)
			{
			case TARTARUGA:Sleep(LENTIDAO_TARTARUGA);
				idCobraNoMapa = idCobraNoMapaDefeito + (TARTARUGA * 10);
				jogo.jogadores[posArrayCobra].duracaoEfeito--;
				if (jogo.jogadores[posArrayCobra].duracaoEfeito == 0)
					jogo.jogadores[posArrayCobra].estadoJogador = VIVO;
				break;
			case LEBRE:Sleep(LENTIDAO_LEBRE);
				idCobraNoMapa = idCobraNoMapaDefeito + (LEBRE * 10);
				jogo.jogadores[posArrayCobra].duracaoEfeito--;
				if (jogo.jogadores[posArrayCobra].duracaoEfeito == 0)
					jogo.jogadores[posArrayCobra].estadoJogador = VIVO;
				break;
			case BEBADO:Sleep(LENTIDAO_NORMAL);
				idCobraNoMapa = idCobraNoMapaDefeito + (BEBADO * 10);
				jogo.jogadores[posArrayCobra].duracaoEfeito--;
				if (jogo.jogadores[posArrayCobra].duracaoEfeito == 0)
					jogo.jogadores[posArrayCobra].estadoJogador = VIVO;
				break;
			default:Sleep(LENTIDAO_NORMAL);
				idCobraNoMapa = idCobraNoMapaDefeito;
				break;
			}
			for (int i = 0; i < MAXCLIENTES; i++) {
				WaitForSingleObject(hSemaforoMapaServidor, INFINITE);
			}
			switch (jogo.jogadores[posArrayCobra].direcao)
			{
			case CIMA://Coluna mantem-se, muda de linha(-1)
				if (jogo.jogadores[posArrayCobra].porAparecer != 0) {
					posArrayAux = jogo.jogadores[posArrayCobra].tamanho - jogo.jogadores[posArrayCobra].porAparecer;
					//Buscar a posição da cabeça
					auxLinha = jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux - 1][0];
					auxColuna = jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux - 1][1];
					jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux][0] = auxLinha-1;
					jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux][1] = auxColuna;
					jogo.jogadores[posArrayCobra].porAparecer--;
					vistaPartilhaGeralServidor->mapa[auxLinha][auxColuna] = idCobraNoMapa;
					vistaPartilhaGeralServidor->mapa[auxLinha - 1][auxColuna] = idCobraNoMapa + CIMA;
				}
				else {
					posArrayAux = jogo.jogadores[posArrayCobra].tamanho - 1;
					//Apagar a cauda do mapa 
					auxLinha = jogo.jogadores[posArrayCobra].posicoesCobra[0][0];
					auxColuna = jogo.jogadores[posArrayCobra].posicoesCobra[0][1];
					vistaPartilhaGeralServidor->mapa[auxLinha][auxColuna] = ESPACOVAZIO;
					if (jogo.jogadores[posArrayCobra].comeuGelo == TRUE) {//se comeu gelo na iteração anterior apaga duas posições
						auxLinha = jogo.jogadores[posArrayCobra].posicoesCobra[1][0];
						auxColuna = jogo.jogadores[posArrayCobra].posicoesCobra[1][1];
						vistaPartilhaGeralServidor->mapa[auxLinha][auxColuna] = ESPACOVAZIO;
						jogo.jogadores[posArrayCobra].tamanho--;
					}
					//Buscar a posição da cabeça
					auxLinha = jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux][0];
					auxColuna = jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux][1];
					//meter o id da cobra no lugar antido da cebeça sem a direção
					vistaPartilhaGeralServidor->mapa[auxLinha][auxColuna] = idCobraNoMapa;
					//Mover todas as posições da cobra no array sobrepondo as antigas
					for (int z = 0; z < posArrayAux; z++) {
						jogo.jogadores[posArrayCobra].posicoesCobra[z][0] = jogo.jogadores[posArrayCobra].posicoesCobra[z+1][0];
						jogo.jogadores[posArrayCobra].posicoesCobra[z][1] = jogo.jogadores[posArrayCobra].posicoesCobra[z+1][1];
					}
					if (jogo.jogadores[posArrayCobra].comeuGelo == TRUE) {//se comeu gelo temos de mover duas vezes todas as posições do array
						posArrayAux = jogo.jogadores[posArrayCobra].tamanho - 1;
						for (int z = 0; z < posArrayAux-1; z++) {
							jogo.jogadores[posArrayCobra].posicoesCobra[z][0] = jogo.jogadores[posArrayCobra].posicoesCobra[z + 1][0];
							jogo.jogadores[posArrayCobra].posicoesCobra[z][1] = jogo.jogadores[posArrayCobra].posicoesCobra[z + 1][1];
						}
						jogo.jogadores[posArrayCobra].comeuGelo = FALSE;
					}
					if (vistaPartilhaGeralServidor->mapa[auxLinha - 1][auxColuna] != ESPACOVAZIO) {
						if (trataColisao(auxLinha - 1, auxColuna, posArrayCobra)) {
							jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux][0] = auxLinha - 1;
							jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux][1] = auxColuna;
							vistaPartilhaGeralServidor->mapa[auxLinha - 1][auxColuna] = idCobraNoMapa + CIMA;
						}
						else {//cobra morreu, actualizar pontuação desta, detectar fim do jogo e apagar a cobra do mapa
							jogo.cobrasVivas--;
							if (acabouJogo()) {//não acabou o jogo, apagar esta cobra do mapa.
								for (int i = 0; i <= posArrayAux; i++) {
									auxLinha = jogo.jogadores[posArrayCobra].posicoesCobra[i][0];
									auxColuna = jogo.jogadores[posArrayCobra].posicoesCobra[i][1];
									vistaPartilhaGeralServidor->mapa[auxLinha][auxColuna] = ESPACOVAZIO;
								}
							}
							else {}//enviar pontuações aos clientes ligados 
						}
					}
					else {
						jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux][0] = auxLinha - 1;
						jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux][1] = auxColuna;
						vistaPartilhaGeralServidor->mapa[auxLinha - 1][auxColuna] = idCobraNoMapa + CIMA;
					}							
				}					
				break;
			case BAIXO://Coluna mantem-se, muda de linha(+1)
				if (jogo.jogadores[posArrayCobra].porAparecer != 0) {
					posArrayAux = jogo.jogadores[posArrayCobra].tamanho - jogo.jogadores[posArrayCobra].porAparecer;
					//Buscar a posição da cabeça
					auxLinha = jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux - 1][0];
					auxColuna = jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux - 1][1];
					jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux][0] = auxLinha + 1;
					jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux][1] = auxColuna;
					jogo.jogadores[posArrayCobra].porAparecer--;
					vistaPartilhaGeralServidor->mapa[auxLinha][auxColuna] = idCobraNoMapa;
					vistaPartilhaGeralServidor->mapa[auxLinha + 1][auxColuna] = idCobraNoMapa + BAIXO;
				}
				else {
					posArrayAux = jogo.jogadores[posArrayCobra].tamanho - 1;
					//Apagar a cauda do mapa 
					auxLinha = jogo.jogadores[posArrayCobra].posicoesCobra[0][0];
					auxColuna = jogo.jogadores[posArrayCobra].posicoesCobra[0][1];
					vistaPartilhaGeralServidor->mapa[auxLinha][auxColuna] = ESPACOVAZIO;
					if (jogo.jogadores[posArrayCobra].comeuGelo == TRUE) {//se comeu gelo na iteração anterior apaga duas posições
						auxLinha = jogo.jogadores[posArrayCobra].posicoesCobra[1][0];
						auxColuna = jogo.jogadores[posArrayCobra].posicoesCobra[1][1];
						vistaPartilhaGeralServidor->mapa[auxLinha][auxColuna] = ESPACOVAZIO;
						jogo.jogadores[posArrayCobra].tamanho--;
					}
					//Buscar a posição da cabeça
					auxLinha = jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux][0];
					auxColuna = jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux][1];
					//meter o id da cobra no lugar antido da cebeça sem a direção
					vistaPartilhaGeralServidor->mapa[auxLinha][auxColuna] = idCobraNoMapa;
					//Mover todas as posições da cobra no array sobrepondo as antigas
					for (int z = 0; z < posArrayAux; z++) {
						jogo.jogadores[posArrayCobra].posicoesCobra[z][0] = jogo.jogadores[posArrayCobra].posicoesCobra[z + 1][0];
						jogo.jogadores[posArrayCobra].posicoesCobra[z][1] = jogo.jogadores[posArrayCobra].posicoesCobra[z + 1][1];
					}
					if (jogo.jogadores[posArrayCobra].comeuGelo == TRUE) {//se comeu gelo temos de mover duas vezes todas as posições do array
						posArrayAux = jogo.jogadores[posArrayCobra].tamanho - 1;
						for (int z = 0; z < posArrayAux - 1; z++) {
							jogo.jogadores[posArrayCobra].posicoesCobra[z][0] = jogo.jogadores[posArrayCobra].posicoesCobra[z + 1][0];
							jogo.jogadores[posArrayCobra].posicoesCobra[z][1] = jogo.jogadores[posArrayCobra].posicoesCobra[z + 1][1];
						}
						jogo.jogadores[posArrayCobra].comeuGelo = FALSE;
					}
					if (vistaPartilhaGeralServidor->mapa[auxLinha + 1][auxColuna] != ESPACOVAZIO) {
						if (trataColisao(auxLinha + 1, auxColuna, posArrayCobra)) {
							jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux][0] = auxLinha + 1;
							jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux][1] = auxColuna;
							vistaPartilhaGeralServidor->mapa[auxLinha + 1][auxColuna] = idCobraNoMapa + BAIXO;
						}
						else {//cobra morreu, actualizar pontuação desta, detectar fim do jogo e apagar a cobra do mapa
							jogo.cobrasVivas--;
							if (acabouJogo()) {//não acabou o jogo, apagar esta cobra do mapa.
								for (int i = 0; i <= posArrayAux; i++) {
									auxLinha = jogo.jogadores[posArrayCobra].posicoesCobra[i][0];
									auxColuna = jogo.jogadores[posArrayCobra].posicoesCobra[i][1];
									vistaPartilhaGeralServidor->mapa[auxLinha][auxColuna] = ESPACOVAZIO;
								}
							}
							else {}//enviar pontuações aos clientes ligados 
						}
					}
					else {
						jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux][0] = auxLinha + 1;
						jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux][1] = auxColuna;
						vistaPartilhaGeralServidor->mapa[auxLinha + 1][auxColuna] = idCobraNoMapa + BAIXO;
					}
				}
				break;
			case ESQUERDA://Linha mantem-se, muda de coluna(-1)
				if (jogo.jogadores[posArrayCobra].porAparecer != 0) {
					posArrayAux = jogo.jogadores[posArrayCobra].tamanho - jogo.jogadores[posArrayCobra].porAparecer;
					//Buscar a posição da cabeça
					auxLinha = jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux - 1][0];
					auxColuna = jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux - 1][1];
					jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux][0] = auxLinha;
					jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux][1] = auxColuna - 1;
					jogo.jogadores[posArrayCobra].porAparecer--;
					vistaPartilhaGeralServidor->mapa[auxLinha][auxColuna] = idCobraNoMapa;
					vistaPartilhaGeralServidor->mapa[auxLinha][auxColuna - 1] = idCobraNoMapa + ESQUERDA;
				}
				else {
					posArrayAux = jogo.jogadores[posArrayCobra].tamanho - 1;
					//Apagar a cauda do mapa 
					auxLinha = jogo.jogadores[posArrayCobra].posicoesCobra[0][0];
					auxColuna = jogo.jogadores[posArrayCobra].posicoesCobra[0][1];
					vistaPartilhaGeralServidor->mapa[auxLinha][auxColuna] = ESPACOVAZIO;
					if (jogo.jogadores[posArrayCobra].comeuGelo == TRUE) {//se comeu gelo na iteração anterior apaga duas posições
						auxLinha = jogo.jogadores[posArrayCobra].posicoesCobra[1][0];
						auxColuna = jogo.jogadores[posArrayCobra].posicoesCobra[1][1];
						vistaPartilhaGeralServidor->mapa[auxLinha][auxColuna] = ESPACOVAZIO;
						jogo.jogadores[posArrayCobra].tamanho--;
					}
					//Buscar a posição da cabeça
					auxLinha = jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux][0];
					auxColuna = jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux][1];
					//meter o id da cobra no lugar antido da cebeça sem a direção
					vistaPartilhaGeralServidor->mapa[auxLinha][auxColuna] = idCobraNoMapa;
					//Mover todas as posições da cobra no array sobrepondo as antigas
					for (int z = 0; z < posArrayAux; z++) {
						jogo.jogadores[posArrayCobra].posicoesCobra[z][0] = jogo.jogadores[posArrayCobra].posicoesCobra[z + 1][0];
						jogo.jogadores[posArrayCobra].posicoesCobra[z][1] = jogo.jogadores[posArrayCobra].posicoesCobra[z + 1][1];
					}
					if (jogo.jogadores[posArrayCobra].comeuGelo == TRUE) {//se comeu gelo temos de mover duas vezes todas as posições do array
						posArrayAux = jogo.jogadores[posArrayCobra].tamanho - 1;
						for (int z = 0; z < posArrayAux - 1; z++) {
							jogo.jogadores[posArrayCobra].posicoesCobra[z][0] = jogo.jogadores[posArrayCobra].posicoesCobra[z + 1][0];
							jogo.jogadores[posArrayCobra].posicoesCobra[z][1] = jogo.jogadores[posArrayCobra].posicoesCobra[z + 1][1];
						}
						jogo.jogadores[posArrayCobra].comeuGelo = FALSE;
					}
					if (vistaPartilhaGeralServidor->mapa[auxLinha][auxColuna - 1] != ESPACOVAZIO) {
						if (trataColisao(auxLinha, auxColuna - 1, posArrayCobra)) {
							jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux][0] = auxLinha;
							jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux][1] = auxColuna - 1;
							vistaPartilhaGeralServidor->mapa[auxLinha][auxColuna - 1] = idCobraNoMapa + ESQUERDA;
						}
						else {//cobra morreu, actualizar pontuação desta, detectar fim do jogo e apagar a cobra do mapa
							jogo.cobrasVivas--;
							if (acabouJogo()) {//não acabou o jogo, apagar esta cobra do mapa.
								for (int i = 0; i <= posArrayAux; i++) {
									auxLinha = jogo.jogadores[posArrayCobra].posicoesCobra[i][0];
									auxColuna = jogo.jogadores[posArrayCobra].posicoesCobra[i][1];
									vistaPartilhaGeralServidor->mapa[auxLinha][auxColuna] = ESPACOVAZIO;
								}
							}
							else {}//enviar pontuações aos clientes ligados 
						}
					}
					else {
						jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux][0] = auxLinha;
						jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux][1] = auxColuna - 1;
						vistaPartilhaGeralServidor->mapa[auxLinha][auxColuna - 1] = idCobraNoMapa + ESQUERDA;
					}
				}
				break;
			case DIREITA://Linha mantem-se, muda de coluna(+1)
				if (jogo.jogadores[posArrayCobra].porAparecer != 0) {
					posArrayAux = jogo.jogadores[posArrayCobra].tamanho - jogo.jogadores[posArrayCobra].porAparecer;
					//Buscar a posição da cabeça
					auxLinha = jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux - 1][0];
					auxColuna = jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux - 1][1];
					jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux][0] = auxLinha;
					jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux][1] = auxColuna + 1;
					jogo.jogadores[posArrayCobra].porAparecer--;
					vistaPartilhaGeralServidor->mapa[auxLinha][auxColuna] = idCobraNoMapa;
					vistaPartilhaGeralServidor->mapa[auxLinha][auxColuna + 1] = idCobraNoMapa + DIREITA;
				}
				else {
					posArrayAux = jogo.jogadores[posArrayCobra].tamanho - 1;
					//Apagar a cauda do mapa 
					auxLinha = jogo.jogadores[posArrayCobra].posicoesCobra[0][0];
					auxColuna = jogo.jogadores[posArrayCobra].posicoesCobra[0][1];
					vistaPartilhaGeralServidor->mapa[auxLinha][auxColuna] = ESPACOVAZIO;
					if (jogo.jogadores[posArrayCobra].comeuGelo == TRUE) {//se comeu gelo na iteração anterior apaga duas posições
						auxLinha = jogo.jogadores[posArrayCobra].posicoesCobra[1][0];
						auxColuna = jogo.jogadores[posArrayCobra].posicoesCobra[1][1];
						vistaPartilhaGeralServidor->mapa[auxLinha][auxColuna] = ESPACOVAZIO;
						jogo.jogadores[posArrayCobra].tamanho--;
					}
					//Buscar a posição da cabeça
					auxLinha = jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux][0];
					auxColuna = jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux][1];
					//meter o id da cobra no lugar antido da cebeça sem a direção
					vistaPartilhaGeralServidor->mapa[auxLinha][auxColuna] = idCobraNoMapa;
					//Mover todas as posições da cobra no array sobrepondo as antigas
					for (int z = 0; z < posArrayAux; z++) {
						jogo.jogadores[posArrayCobra].posicoesCobra[z][0] = jogo.jogadores[posArrayCobra].posicoesCobra[z + 1][0];
						jogo.jogadores[posArrayCobra].posicoesCobra[z][1] = jogo.jogadores[posArrayCobra].posicoesCobra[z + 1][1];
					}
					if (jogo.jogadores[posArrayCobra].comeuGelo == TRUE) {//se comeu gelo temos de mover duas vezes todas as posições do array
						posArrayAux = jogo.jogadores[posArrayCobra].tamanho - 1;
						for (int z = 0; z < posArrayAux - 1; z++) {
							jogo.jogadores[posArrayCobra].posicoesCobra[z][0] = jogo.jogadores[posArrayCobra].posicoesCobra[z + 1][0];
							jogo.jogadores[posArrayCobra].posicoesCobra[z][1] = jogo.jogadores[posArrayCobra].posicoesCobra[z + 1][1];
						}
						jogo.jogadores[posArrayCobra].comeuGelo = FALSE;
					}
					if (vistaPartilhaGeralServidor->mapa[auxLinha][auxColuna + 1] != ESPACOVAZIO) {
						if (trataColisao(auxLinha , auxColuna + 1, posArrayCobra)) {
							jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux][0] = auxLinha;
							jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux][1] = auxColuna + 1;
							vistaPartilhaGeralServidor->mapa[auxLinha][auxColuna + 1] = idCobraNoMapa + DIREITA;
						}
						else {//cobra morreu, actualizar pontuação desta, detectar fim do jogo e apagar a cobra do mapa
							jogo.cobrasVivas--;
							if (acabouJogo()) {//não acabou o jogo, apagar esta cobra do mapa.
								for (int i = 0; i <= posArrayAux; i++) {
									auxLinha = jogo.jogadores[posArrayCobra].posicoesCobra[i][0];
									auxColuna = jogo.jogadores[posArrayCobra].posicoesCobra[i][1];
									vistaPartilhaGeralServidor->mapa[auxLinha][auxColuna] = ESPACOVAZIO;
								}
							}
							else {}//enviar pontuações aos clientes ligados 
						}
					}
					else {
						jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux][0] = auxLinha;
						jogo.jogadores[posArrayCobra].posicoesCobra[posArrayAux][1] = auxColuna + 1;
						vistaPartilhaGeralServidor->mapa[auxLinha][auxColuna + 1] = idCobraNoMapa + DIREITA;
					}
				}
				break;
			}
		_tprintf(TEXT("**********ITERAÇÂO DO MAPA**********\n"));
		SetEvent(hEventoMapaServidor);
		ResetEvent(hEventoMapaServidor);
		ReleaseSemaphore(hSemaforoMapaServidor, MAXCLIENTES, NULL);
	}

	return 1;
}

//Função que detecta fim de jogo
//devolve 1 se continua e 0 se acaba
int acabouJogo() {
	switch (jogo.tipoJogo) {
	case SINGLEPLAYER:jogo.estadoJogo = FINALJOGO;
		return 0;//acabou
		break;
	case MULTIPLAYER:if (jogo.cobrasVivas == 1) {
						jogo.estadoJogo = FINALJOGO;
						return 0;//acabou
					}
					else
						 return 1;//continua
		break;
	}
	return 1;//continua
}


//Função que trata colisões chamada quando a cobra vai entrar numa casa do mapa que não está vazia,
//devolve 1 se a cobra não morreu, devolve 0 se morreu e assim efectuar as devidas alterações no mapa e detectar fim de jogo
int trataColisao(int linha,int coluna, int indiceCobra) {
	int tipoGerado, indiceOutraCobra;
	switch (vistaPartilhaGeralServidor->mapa[linha][coluna]) {
	case PAREDE:jogo.jogadores[indiceCobra].estadoJogador = MORTO;
		return 0;
		break;
	case ALIMENTO:jogo.jogadores[indiceCobra].tamanho++;
		jogo.jogadores[indiceCobra].porAparecer++;
		jogo.jogadores[indiceCobra].pontuacao += 2;
		return 1;
		break;
	case GELO://jogo.jogadores[indiceCobra].tamanho--; tem de ser tratado no movecobra para se conseguir actualizar as posições da cobra no array...
		jogo.jogadores[indiceCobra].comeuGelo = TRUE;
		jogo.jogadores[indiceCobra].pontuacao -= 2;
		return 1;
		break;
	case GRANADA:jogo.jogadores[indiceCobra].estadoJogador = MORTO;
		return 0;
		break;
	case VODKA:jogo.jogadores[indiceCobra].estadoJogador = BEBADO;
		jogo.jogadores[indiceCobra].duracaoEfeito = CICLOS_VODKA;
		return 1;
		break;
	case OLEO:jogo.jogadores[indiceCobra].estadoJogador = LEBRE;
		jogo.jogadores[indiceCobra].duracaoEfeito = CICLOS_LEBRE;
		return 1;
		break;
	case COLA:jogo.jogadores[indiceCobra].estadoJogador = TARTARUGA;
		jogo.jogadores[indiceCobra].duracaoEfeito = CICLOS_TARTARUGA;
		return 1;
		break;
	case O_VODKA:for(int i=0;i<jogo.config.N;i++)
					if (i != indiceCobra) {
						jogo.jogadores[i].estadoJogador = BEBADO;
						jogo.jogadores[i].duracaoEfeito = CICLOS_VODKA;
					}
				 return 1;
		break;
	case O_OLEO:for (int i = 0; i<jogo.config.N; i++)
					if (i != indiceCobra) {
						jogo.jogadores[i].estadoJogador = LEBRE;
						jogo.jogadores[i].duracaoEfeito = CICLOS_LEBRE;
					}
				return 1;
		break;
	case O_COLA:for (int i = 0; i<jogo.config.N; i++)
					if (i != indiceCobra) {
						jogo.jogadores[i].estadoJogador = TARTARUGA;
						jogo.jogadores[i].duracaoEfeito = CICLOS_TARTARUGA;
					}
				return 1;
		break;
	case SURPRESA://Gera um objecto dentro do intervalo de probabilidades dos objectos excepto os objectos surpresa e granada;
		tipoGerado = rand() % PROB_O_COLA;
		if (tipoGerado < PROB_ALIMENTO) {
			jogo.jogadores[indiceCobra].tamanho++;
			jogo.jogadores[indiceCobra].porAparecer++;
			return 1;
		}
		else if (tipoGerado < PROB_GELO) {
			jogo.jogadores[indiceCobra].tamanho--;
			jogo.jogadores[indiceCobra].comeuGelo = TRUE;
			return 1;
		}
		else if (tipoGerado < PROB_OLEO) {
			jogo.jogadores[indiceCobra].estadoJogador = LEBRE;
			jogo.jogadores[indiceCobra].duracaoEfeito = CICLOS_LEBRE;
			return 1;
		}
		else if (tipoGerado < PROB_COLA) {
			jogo.jogadores[indiceCobra].estadoJogador = TARTARUGA;
			jogo.jogadores[indiceCobra].duracaoEfeito = CICLOS_TARTARUGA;
			return 1;
		}
		else if (tipoGerado < PROB_VODKA) {
			jogo.jogadores[indiceCobra].estadoJogador = BEBADO;
			jogo.jogadores[indiceCobra].duracaoEfeito = CICLOS_VODKA;
			return 1;
		}
		else if (tipoGerado < PROB_O_VODKA) {
			for (int i = 0; i<jogo.config.N; i++)
				if (i != indiceCobra) {
					jogo.jogadores[i].estadoJogador = BEBADO;
					jogo.jogadores[i].duracaoEfeito = CICLOS_VODKA;
					}
			return 1;
						
		}
		else if (tipoGerado < PROB_O_OLEO) {
			for (int i = 0; i<jogo.config.N; i++)
				if (i != indiceCobra) {
					jogo.jogadores[i].estadoJogador = LEBRE;
					jogo.jogadores[i].duracaoEfeito = CICLOS_LEBRE;
				}
			return 1;
		}
		else if (tipoGerado < PROB_O_COLA) {
			for (int i = 0; i<jogo.config.N; i++)
				if (i != indiceCobra) {
					jogo.jogadores[i].estadoJogador = TARTARUGA;
					jogo.jogadores[i].duracaoEfeito = CICLOS_TARTARUGA;
				}
			return 1;
		}
		break;
	default://colisão com outros jogadores
		indiceOutraCobra = vistaPartilhaGeralServidor->mapa[linha][coluna] / 100;
		jogo.jogadores[indiceCobra].estadoJogador = MORTO;
		jogo.jogadores[indiceOutraCobra].pontuacao += 10;
		return 0;
		break;
	}
	return 1;
}

//Gera as posições da cobra no mapa verificando se há colisões com paredes e fazendo a respectiva alteração á cobra
void criaCobra(TCHAR username[SIZE_USERNAME], int vaga, int pid, int tid, int jogador) {
	int posXGerada, posYGerada, dirGerada, idCobraMapa;
	
	while (1) {
		posXGerada = rand() % jogo.config.C;
		posYGerada = rand() % jogo.config.L;
		if (vistaPartilhaGeralServidor->mapa[posYGerada][posXGerada] == ESPACOVAZIO)
			break;
	}
	//posXGerada = posYGerada = 1;
	//Na posição 0 do array de posições ficam as Linhas e na 1 ficam as Colunas
	jogo.jogadores[vaga].posicoesCobra[0][0] = posYGerada;
	jogo.jogadores[vaga].posicoesCobra[0][1] = posXGerada;

	idCobraMapa = (vaga + 1) * 100;
	vistaPartilhaGeralServidor->mapa[posYGerada][posXGerada] = idCobraMapa;

	dirGerada = (rand() % 3) + 1;
	jogo.jogadores[vaga].direcao = dirGerada;
	jogo.jogadores[vaga].porAparecer = jogo.config.T - 1;
	jogo.jogadores[vaga].estadoJogador = VIVO;
	jogo.jogadores[vaga].pontuacao = 0;
	jogo.jogadores[vaga].jogador = jogador;
	jogo.jogadores[vaga].pid = pid;
	jogo.jogadores[vaga].tid = tid;
	jogo.jogadores[vaga].tamanho = jogo.config.T;
	_tcscpy_s(jogo.jogadores[vaga].username, SIZE_USERNAME, username);
	jogo.jogadores[vaga].comeuGelo = FALSE;
}



int AssociaJogo(TCHAR username[SIZE_USERNAME], int pid, int tid, int jogador) {
	
	//Se não existir jogo criado ou não existirem vagas
	if (jogo.estadoJogo != ASSOCIACAOJOGO) {
		_tprintf(TEXT("**********ERRO ASSOCIAR JOGO**********\n"));
		return AGORANAO;
	}
	else if ((jogo.config.N - jogo.vagasJogadores) == 0) {
		_tprintf(TEXT("**********ERRO ASSOCIAR JOGO**********\n"));
		return JOGOCHEIO;
	}
	
	criaCobra(username, jogo.vagasJogadores,pid,tid,jogador);
	jogo.vagasJogadores++;
		
	return jogo.vagasJogadores;
}

int IniciaJogo(int pid, int tid) {
	DWORD auxTid;
	HANDLE hThread; 
	
	if (jogo.estadoJogo != ASSOCIACAOJOGO){
		_tprintf(TEXT("**********ERRO INICIAR JOGO**********\n"));
		return AGORANAO;
	}
	//Se não for o computador que criou o jogo não pode dar inicio a este
	if (jogo.pidCriador != pid) {
		_tprintf(TEXT("**********ERRO INICIAR JOGO**********\n"));
		return CRIADORERRADO;
	}
	//Se for, criar as threads de mover as cobras, de gerir objectos e gerir cobras automaticas e notificar jogadores
	jogo.estadoJogo = DECORRERJOGO;
	for (int i = 0; i < jogo.vagasJogadores; i++) {
		hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)moveCobras, (LPVOID)i, 0, &auxTid);
	}

	//hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)gestorObjectos,NULL, 0, &tid);
	
	
	return 1;
}

int Cria_Jogo(ConfigInicial param, int pid, int tid, TCHAR username[SIZE_USERNAME]) {

	if ((jogo.estadoJogo != CRIACAOJOGO)) {
		return AGORANAO;
	}

	//preparar dados do jogo e mudar estado
	jogo.estadoJogo = ASSOCIACAOJOGO;
	jogo.config = param;
	jogo.pidCriador = pid;
	jogo.tidCriador = tid;
	jogo.vagasJogadores = 0;
	vistaPartilhaGeralServidor->colunas = param.C;
	vistaPartilhaGeralServidor->linhas = param.L;
	//preparar mapa
	preparaMapaJogo();

	criaCobra(username, jogo.vagasJogadores, pid, tid, JOGADOR1);
	jogo.vagasJogadores++;

	return jogo.vagasJogadores;
}

void preparaMapaJogo() {
	for (int z = 0; z < vistaPartilhaGeralServidor->colunas; z++) {
		vistaPartilhaGeralServidor->mapa[0][z] = PAREDE;
		vistaPartilhaGeralServidor->mapa[vistaPartilhaGeralServidor->linhas - 1][z] = PAREDE;
		for (int j = 1; j < vistaPartilhaGeralServidor->linhas - 1; j++) {
			if (z == 0 || z == vistaPartilhaGeralServidor->colunas - 1) {
				vistaPartilhaGeralServidor->mapa[j][z] = PAREDE;
			}
			else
				vistaPartilhaGeralServidor->mapa[j][z] = ESPACOVAZIO;
		}
	}
}

/*----------------------------------------------------------------- */
/*  THREAD - Função que gere os objectos no mapa				 	*/
/* ---------------------------------------------------------------- */
DWORD WINAPI gestorObjectos(LPVOID param) {
	criaObjectosMapaInicial();
	while (1) {
		Sleep(SEGUNDO);
		for (int i = 0; i < jogo.config.O; i++) {
			jogo.objectosMapa[i].segundosRestantes--;
			if (jogo.objectosMapa[i].segundosRestantes == 0) {
				for (int i = 0; i < MAXCLIENTES; i++) {
					WaitForSingleObject(hSemaforoMapaServidor, INFINITE);
				}
				vistaPartilhaGeralServidor->mapa[jogo.objectosMapa[i].linha][jogo.objectosMapa[i].coluna] = ESPACOVAZIO;
				geraObjecto(i);
				SetEvent(hEventoMapaServidor);
				ResetEvent(hEventoMapaServidor);
				ReleaseSemaphore(hSemaforoMapaServidor, MAXCLIENTES, NULL);
			}
		}
	}
}

void criaObjectosMapaInicial(void) {
	for (int i = 0; i < jogo.config.O; i++) {
		geraObjecto(i);
	}

}

int procuraObjecto(int linha, int coluna) {
	for (int i = 0; i < MAXOBJECTOS; i++) {
		if ((jogo.objectosMapa[i].coluna == coluna) && (jogo.objectosMapa[i].linha == linha))
			return i;
	}
	return -1;
}

void geraObjecto(int indice) {
	int posXGerada, posYGerada, tipoGerado;
	while (1) {
		posXGerada = rand() % jogo.config.C;
		posYGerada = rand() % jogo.config.L;
		if (vistaPartilhaGeralServidor->mapa[posYGerada][posXGerada] == ESPACOVAZIO)
			break;
	}
	//Gera um objecto dentro do intervalo de probabilidades dos objectos
	tipoGerado = rand() % PROB_GRANADA;

	//defenir o objecto no indice fornecido como sendo do tipo gerado
	if (tipoGerado < PROB_ALIMENTO)
		jogo.objectosMapa[indice].Tipo = ALIMENTO;
	else if (tipoGerado < PROB_GELO)
		jogo.objectosMapa[indice].Tipo = GELO;
	else if (tipoGerado < PROB_OLEO)
		jogo.objectosMapa[indice].Tipo = OLEO;
	else if (tipoGerado < PROB_COLA)
		jogo.objectosMapa[indice].Tipo = COLA;
	else if (tipoGerado < PROB_VODKA)
		jogo.objectosMapa[indice].Tipo = VODKA;
	else if (tipoGerado < PROB_O_VODKA)
		jogo.objectosMapa[indice].Tipo = O_VODKA;
	else if (tipoGerado < PROB_O_OLEO)
		jogo.objectosMapa[indice].Tipo = O_OLEO;
	else if (tipoGerado < PROB_O_COLA)
		jogo.objectosMapa[indice].Tipo = O_COLA;
	else if (tipoGerado < PROB_SURPRESA)
		jogo.objectosMapa[indice].Tipo = SURPRESA;
	else 
		jogo.objectosMapa[indice].Tipo = GRANADA;

	jogo.objectosMapa[indice].linha = posYGerada;
	jogo.objectosMapa[indice].coluna = posXGerada;
	jogo.objectosMapa[indice].segundosRestantes = jogo.configObjectos[jogo.objectosMapa[indice].Tipo - 1].S;
	vistaPartilhaGeralServidor->mapa[posYGerada][posXGerada] = jogo.objectosMapa[indice].Tipo;
}

int criaMemoriaPartilhada(void) {

	//hFicheiro = CreateFile(NOME_FILE_MAP, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	hMemoriaGeral = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, SIZE_MEM_GERAL, NOME_MEM_GERAL);

	vistaPartilhaGeralServidor = (MemGeral*)MapViewOfFile(hMemoriaGeral, FILE_MAP_ALL_ACCESS, 0, 0, SIZE_MEM_GERAL);

	hEventoMapaServidor = CreateEvent(NULL, TRUE, FALSE, NOME_EVNT_MAPA);
	hSemaforoMapaServidor = CreateSemaphore(NULL, MAXCLIENTES, MAXCLIENTES, NOME_SEM_MAPA);

	hPodeLerPedidoServidor = CreateSemaphore(NULL, 0, MAX_PEDIDOS, NOME_SEM_PODELER);
	hPodeEscreverPedidoServidor = CreateSemaphore(NULL, MAX_PEDIDOS, MAX_PEDIDOS, NOME_SEM_PODESCRVR);

	if (hMemoriaGeral == NULL || hEventoMapaServidor == NULL || hSemaforoMapaServidor == NULL || hPodeLerPedidoServidor == NULL || hPodeEscreverPedidoServidor == NULL) {
		_tprintf(TEXT("[Erro] Criação de objectos do Windows(%d)\n"), GetLastError());
		return -1;
	}
	else if (GetLastError() == ERROR_ALREADY_EXISTS) {
		_tprintf(TEXT("[Erro] Já existe servidor a correr\n"));
		exit(-1);
	}		

	vistaPartilhaGeralServidor->fila.frente = 0;
	vistaPartilhaGeralServidor->fila.tras = 0;
	return 1;
}

int criaMemoriaPartilhadaResposta(int pid, int tid, int indice) {
	TCHAR aux[TAM_BUFFER];
	TCHAR aux2[TAM_BUFFER];

	//concatenar pid com nome da memoria para ficar com um nome unico
	_stprintf_s(aux, TAM_BUFFER, NOME_MEM_RESPOSTA, pid, tid);

	clientes[indice].hMemResposta = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Resposta), aux);

	clientes[indice].vistaResposta = (Resposta*)MapViewOfFile(clientes[indice].hMemResposta, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Resposta));

	//concatenar pid com nome do evento para ficar com um nome unico
	_stprintf_s(aux2, TAM_BUFFER, NOME_EVNT_RESPOSTA, pid, tid);
	clientes[indice].hEventoResposta = CreateEvent(NULL, TRUE, FALSE, aux2);

	if (clientes[indice].hMemResposta == NULL || clientes[indice].hEventoResposta == NULL) {
		_tprintf(TEXT("[Erro] Criação de objectos do Windows(%d)\n"), GetLastError());
		return -1;
	}
	return 1;
}

int registaCliente(int pid, int tid, int remoto) {
	int indice;
	//procurar uma vaga no array de Clientes (pid=0)
	indice = procuraCliente(0,0);
	if (indice == -1)
		return -1;
	clientes[indice].pid = pid;
	clientes[indice].tid = tid;
	clientes[indice].remoto = remoto;
	if (criaMemoriaPartilhadaResposta(pid, tid, indice) == -1) {
		_tprintf(TEXT("****************ERRO*******************"));
	}
	return indice;
}

void notificaCliente(int indice, Resposta resp) {
	*clientes[indice].vistaResposta = resp;
	SetEvent(clientes[indice].hEventoResposta);
}

int procuraCliente(int pid, int tid) {
	for (int i = 0; i < MAXCLIENTES; i++) {
		if (clientes[i].pid == pid && clientes[i].tid == tid)
			return i;
	}
	return -1;
}

//altera a direcao do jogador 1 ou 2 de determinado pid, se a mudança de direção for inversa a actual ignora o movimento
void mudaDirecaoJogador(int direcao, int pid, int tid, int jogador) {
	int posicao;
	posicao = procuraJogador(pid, tid, jogador);
	//Se estiver sob o efeito da vodka deve inverter as direções enviadas
	if (jogo.jogadores[posicao].estadoJogador == BEBADO) {
		switch (direcao)
		{
		case CIMA:direcao = BAIXO;
			break;
		case BAIXO:direcao = CIMA;
			break;
		case ESQUERDA:direcao = DIREITA;
			break;
		case DIREITA:direcao = ESQUERDA;
			break;
		}
	}
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
int procuraJogador(int pid, int tid, int jogador) {
	for (int i = 0; i < jogo.config.N; i++) {
		if (jogo.jogadores[i].pid == pid && jogo.jogadores[i].tid == tid && jogo.jogadores[i].jogador == jogador) {
			_tprintf(TEXT("Posicao do jogador no array:%d\n"), i);
			return i;
		}
			
	}
	
	return -1;
}


