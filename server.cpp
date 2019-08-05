

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdlib>

#include<iostream>

/*
 * created by Arya Haji Taheri
 * 1518105
 * COSC 3360 - AS2
 * Please use g++ server.cpp -o * - to compile
 * ./*  portno <input - to run
 * used https://www.cs.dartmouth.edu/~campbell/cs50/socketprogramming.html as a reference for socket help
 */

#define LISTENQ 8 /*maximum number of client connections */

int main(int argc, char **argv)
{
	//we need port no to continue.
	if (argc < 2) {
		std::cerr << "ERROR, no port provided\n";
		return -1;
	}
	// network initialization
	int portno = atoi(argv[1]);
	int listenfd, connfd, n;
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;
	//input variable initialization
	int state = -2, number_of_child = 0, state_position = 0;
	const int MAX_SIZE = 2000;
	int *state_array = new int[MAX_SIZE];

	//read inputfile 
	while (std::cin >> state)
	{
		state_array[state_position++] = state;
		//std::cout << state << " \n";
	}
	//creation of the socket
	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	//preparation of the socket address
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(portno);

	bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

	listen(listenfd, LISTENQ);

	std::cout << "Server is  running ... Waiting for a client to connect\n";

	int number_to_send = -2, converted_number = -2, received_int = -2;
	//set up indefinite connection
	for (; ; ) {

		clilen = sizeof(cliaddr);
		connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);

		state_position = 0;

		received_int = 0;

		//listen to request and send value.
		while ((n = recv(connfd, &received_int, sizeof(received_int), 0)) > 0) {

			std::cout << "\n \n Read Access requested ... sending information to client\n";
			//converting int to bytestream
			number_to_send = state_array[state_position++];
			converted_number = htonl(number_to_send);

			// Write the number to the opened socket
			send(connfd, &converted_number, sizeof(converted_number), 0);

			//exit server after sending -1 state
			if (number_to_send == -1) {
				std::cout << "\n\n Client terminated connection. Server is now exiting. \n";
				delete[] state_array;
				close(listenfd);
				return 0;
			}
		}

		//invalid request
		if (n < 0) {
			std::cerr << "read error";
			return -1;
		}
		close(connfd);


	}
	delete[] state_array;
	//close listening socket
	close(listenfd);
	return -1;
}