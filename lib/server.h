#ifndef SERVER_H
#define SERVER_H

#include "default.h"

#define MAX_CLIENTS 5

struct player{
	int x, y;
	// int informações do game, hp, ataque, defesa, pontos...;
    int vida, ataque, defesa, turn;
    //ADICIONADO
    int DEX,WIS,MaxHP;
	int sockid, fight, whofight;
    char nome[NAME_SIZE];
    char ant;
};
/*
	fight
	= 0 se não está lutando
	= 1 se lutando contra monstro, e whofight = id do monstro
	= 2 se lutando contra outro player, e whofight = id do player, turn diz se é a vez de tal cleinte
*/

typedef struct player player;

int sock; // id do socket do server

fd_set active_fd_set, read_fd_set; // variáveis auxiliares do server
struct sockaddr_in clientname;

player clients[MAX_CLIENTS]; // vetor com clientes connectados e suas informações
int clients_connected; // quantidade de clientes connectados
int game_status; 
/* 
    game status 
    = 0 se ngm conectou
    = 1 se esperando confirmação do "criador do game" 
    = 2 se jogo rodando
*/

int pos_broad; // posição livre no vetor do broadcast declarado abaixo
upd_msg map_changes[BUFFER_SIZE]; // vetor com as alterações do mapa

void (*clientMoved)(int, mov_msg); 
void (*clientConnected)(int, clientInfo);
void (*clientDesconnected)(int);
void (*clientConfirmed)(void);


int makeSocket(uint16_t);
int readMovFromClient(int filedes, mov_msg *message);
int readClientInfo(int filedes, clientInfo *message);
void sendTxtToClient(int filedes, const char msg[]);
void sendUpdToClient(int filedes, upd_msg change);
void init();
void sleepServer();
void checkConn();
void wasClient();
void broadcast();
void broadcastTxt(const char msg[], int s);
void disconnectClient(int id);

#endif