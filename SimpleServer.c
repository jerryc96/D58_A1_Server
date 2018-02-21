#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void clean_exit(int rc, int fd, char *message){
	if (rc == -1 || fd == -1){
		if (fd != -1){
			close(fd);
		}
		perror(message);
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char *argv[]){
	int server_fd, client_fd, client_addr_len, opt;
	struct sockaddr_in server_addr, client_addr;
	char client_msg[1024], *server_msg = "Server message, don't connect";
	// require port to listen to
	if (argc != 2){
		fprintf(stderr, "[Usage]: %s PORT \n", argv[0]);
		exit(EXIT_FAILURE);
	}
	// create server address struct
	memset(&server_addr, 0, sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[1]));
	server_addr.sin_addr.s_addr = INADDR_ANY; // bind to availible IP on machine running server code;
	// open socket, check for error
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	clean_exit(server_fd, server_fd, "[Server socket Err]: ");

	// allow reusable port after disconnect or termination with server
	rc = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, (socklen_t)sizeof(int));
	clean_exit(rc, server_fd, "[Server setsockopt Err]: ");

	// listen on socket for up to 5 connections and check error
	rc = listen(server_fd, 5);
	clean_exit(rc, server_fd, "[Server listen Err]: " );

	// accept connections to server
	fprintf("SERVER AT %s:%s LISTENING FOR CONNECTIONS", 
		inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
	while(1){
		client_fd = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_len);
		clean_exit(client_fd, server_fd, "[Server accept Err]: ");
		// process requests:

		fprintf(stdout, "RECEIVED CONNECTION FROM %s:%d\n", 
			inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

		read(client_fd, client_msg, 1024);
		fprintf(stdout, "%s said: %s", inet_ntoa(client_addr.sin_addr), client_msg);
		write(client_fd, server_msg, strlen(server_msg));
		// close client
		close(client_fd);
	}
}