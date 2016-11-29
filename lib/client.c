#include "client.h"

void connectToServer(const char *server_IP){
	network_socket = socket(AF_INET, SOCK_STREAM, 0);
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);

	if(server_IP == NULL)
		server_address.sin_addr.s_addr = INADDR_ANY;
	else
		inet_aton(server_IP, &server_address.sin_addr); // lembra de tirar o .s_addr

	printf("Connecting...\n");
	int connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));
	if(connection_status==0){
		printf("Connected\n");
	}else if(connection_status==-1){
		printf("ERROR! Connection was not succefull\n");
		exit(1);
	}

	int flags = fcntl(network_socket, F_GETFL, 0); //<<< o read não vai parar o programa mais...
	fcntl(network_socket, F_SETFL, flags | O_NONBLOCK);
}

int sendMovToServer(mov_msg message){
	return write(network_socket, &message, sizeof(mov_msg));
}


int readTxtFromServer(char* message){
	usleep(1000); // dar um tempinho, já que n para e fica esperando
	return read(network_socket, message, BUFFER_SIZE);
}

int getch() {
    struct termios oldt, newt;
    int ch;
    struct pollfd mypoll = { STDIN_FILENO, POLLIN|POLLPRI };
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);

    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    if ( poll(&mypoll, 1, 100)){
	    ch = getchar();
	    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	}
	else{
	    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
		return -1;
	}
    return ch;
}

int sendInfoToServer(clientInfo info){
	return write(network_socket, &info, sizeof(clientInfo));
}

int readUpdFromServer(upd_msg* message){
	usleep(100); // dar um tempinho, já que n para e fica esperando
	return read(network_socket, message, sizeof(upd_msg));
}
