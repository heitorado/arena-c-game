#include "default.h"

int network_socket;
struct sockaddr_in server_address;

void connectToServer(const char *server_IP);

int sendMovToServer(mov_msg message);

int readTxtFromServer(char* message);

int getch();

int sendInfoToServer(clientInfo info);

int readUpdFromServer(upd_msg* message);