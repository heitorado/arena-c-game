#include "server.h"

void init(){
	clients_connected = 0;
	pos_broad = 0;
	game_status = 0;
	sock = makeSocket (PORT);
	if (listen (sock, 100) < 0){
	    perror ("listen");
	    exit (EXIT_FAILURE);
	}

	FD_ZERO (&active_fd_set);
	FD_SET (sock, &active_fd_set);
	memset(clients, 0, sizeof clients);
}

int makeSocket (uint16_t port){
  	int sock;
  	struct sockaddr_in name;

  	sock = socket (AF_INET, SOCK_STREAM, 0);
  	if (sock < 0){
     	perror ("socket");
    	exit (EXIT_FAILURE);
  	}
  	name.sin_family = AF_INET;
  	name.sin_port = htons (port);
  	name.sin_addr.s_addr = htonl (INADDR_ANY);
  	if (bind (sock, (struct sockaddr *) &name, sizeof (name)) < 0){
    	perror ("bind");
      	exit (EXIT_FAILURE);
    }

  	return sock;
}

int readMovFromClient (int filedes, mov_msg *message){
  	int nbytes = read (filedes, message, sizeof(mov_msg));
 	if (nbytes <= 0)
		return -1;
    return 0;
}

int readClientInfo (int filedes, clientInfo *message){
    int nbytes = read (filedes, message, sizeof(clientInfo));
    if (nbytes <= 0)
        return -1;
    return 0;
}

void sendTxtToClient(int filedes, const char msg[]){
	write (filedes, msg, BUFFER_SIZE);
}

void sendUpdToClient(int filedes, upd_msg change) {
	write (filedes, &change, sizeof(upd_msg));
}

void sleepServer() {
	read_fd_set = active_fd_set;
    if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0){
        perror ("select");
        exit (EXIT_FAILURE);
   	}
}

void checkConn() {
    int i, sd;
	if(FD_ISSET(sock, &read_fd_set)){//connect
   		int new;
        new = accept (sock, NULL, NULL);
        if (new < 0){
            perror ("accept");
            exit (EXIT_FAILURE);
        }

        clientInfo startInfo;

        char msg[BUFFER_SIZE];
        if(readClientInfo(new, &startInfo) < 0)
            return;

        if(clients_connected == MAX_CLIENTS || game_status == 2){
        	sendTxtToClient(new, "- O jogo já começou ou já temos muitos clientes conectados");
        	close(new);
        }
        else{
            FD_SET (new, &active_fd_set);
            
            for(i = 0; i < MAX_CLIENTS; ++i){
                if(clients[i].sockid == 0)
                {
                    clients[i].sockid = new;
                    clientConnected(i, startInfo);
                    ++clients_connected;
                    break;
                }
            }
        }
   	}
}

void wasClient() {
    int i, sd;
	for (i = 0; i < MAX_CLIENTS; ++i)
    {
    	sd = clients[i].sockid;
        if (FD_ISSET (sd, &read_fd_set))
        {
            mov_msg new_mov;
            
            if (readMovFromClient (sd, &new_mov) < 0)
            { //o usuário se desconectou.
                close (sd);
                FD_CLR (sd, &active_fd_set);
                clients[i].sockid = 0;
                --clients_connected;
                if(clientDesconnected != NULL)
                {
                    clientDesconnected(i);
                }
                if(clients_connected == 0)
                    game_status = 0;
            }

            else
            {
            	if(game_status == 1 && i == 0)
                {
                    clientConfirmed();
                }
                else if(game_status == 2)
                {
                    clientMoved(i, new_mov);
                }
            }
        }
    }
}

void broadcast() {
	assert(game_status == 2);
    int i, sd, j = 0;
	for (i = 0; i < MAX_CLIENTS; ++i)
    {
    	sd = clients[i].sockid;
        if (sd > 0)
        {
            for (j = 0; j < pos_broad; ++j) 
            {
                sendUpdToClient(sd, map_changes[j]);
            }
        }
    }

    pos_broad = 0;
}

void broadcastTxt(const char msg[], int s){
    int i, sd;
    for (i = 0; i < MAX_CLIENTS; ++i){
        sd = clients[i].sockid;
        if (sd > 0 && i != s){
            sendTxtToClient(sd, msg);
        }
    }
}


void disconnectClient(int id)
{
    shutdown(clients[id].sockid, SHUT_RD);
    clients[id].sockid = 0;
    clients_connected--;
}