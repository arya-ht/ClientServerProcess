
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdlib>

#include <iostream>
#define LISTENQ 8 /*maximum number of client connections */

/*
 * created by Arya Haji Taheri
 * 1518105
 * COSC 3360 - AS2
 *  used https://www.cs.dartmouth.edu/~campbell/cs50/socketprogramming.html as a reference for socket help
 * Please use g++ client.cpp -o * - to compile
 * ./* ipaddress portno  - to run
 */

int main(int argc, char **argv)
{
	//we need port no to continue.

	if (argc < 3) {
		std::cerr << "usage ip-address port\n";
		exit(1);
	}
	// set port number


		int sockfd;
		struct sockaddr_in servaddr;
		if (argc < 3) {
			std::cerr << "usage hostname port\n";
			return 1;
		}
		int portno = atoi(argv[2]);
		//basic check of the arguments
		//additional checks can be inserted


		//Create a socket for the client
		if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			std::cerr << "Problem in creating the socket";
			return 2;
		}

		//Creation of the socket
		memset(&servaddr, 0, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr = inet_addr(argv[1]);
		servaddr.sin_port = htons(portno); //convert to big-endian order

		//Connection of the client to the socket
		if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
			std::cerr << "Problem in connecting to the server";
			return 3;
		}
		int i = 0;

		//Send request to obtain information about the children number
		int send_request = htonl(1);
		send(sockfd, &send_request, sizeof(send_request), 0);

		int children_number = 0;
		//Recieve children number
		if (recv(sockfd, &children_number, sizeof(children_number), 0) > 0) {
			std::cout << "\nNumber of ring processes are: " << ntohl(children_number) << "\n\n";
		}
		//convert children number to int
		children_number = ntohl(children_number);
		if (children_number  == 0) { std::cerr << "Ring number cannot be 0\n";
		return -1;
		}
		//initialize ring variables
		int pipefd[--children_number + 1][2];
		int pid;
		int token;
		int state = 0;

		//create pipes
		for (i = 0; i <= children_number; i++)
		{
			if (pipe(pipefd[i]) < 0)
			{
				std::cerr << "Pipe Creation failed\n";
				return 1;
			}
		}

		//this loop handles child processes' interaction with the server.
		for (i = 0; i < children_number; i++)
		{
			if ((pid = fork()) == 0)
			{
				while (1)
				{
					read(pipefd[i][0], &token, sizeof(token));
					//get process state
					send_request = htonl(1);
					send(sockfd, &send_request, sizeof(send_request), 0);

					state = 0;

					if (recv(sockfd, &state, sizeof(state), 0) > 0) {

					}
					state = ntohl(state);

					//if the process has token and state is true, print that it's using the network.
					if (token >= 0 && state == 1)
						std::cout << "Process "<< i + 1 <<" is using the network\n" ;
					//if the state is -1, end the process.
					if (token == -1 || state == -1)
					{
						token = -1;
						write(pipefd[i + 1][1], &token, sizeof(token));
						_exit(0);
					}
					//pass token
					token = token + 1;
					write(pipefd[i + 1][1], &token, sizeof(token));
				}

			}
			//These ends are no longer needed in parent.
			close(pipefd[i][0]);
			close(pipefd[i + 1][1]);
		}
		// handle the parent process interaction with the server.
		while (1)
		{
			//send request to get state number.
			send_request = htonl(1);
			send(sockfd, &send_request, sizeof(send_request), 0);
			state = 0;
			//obtain state number
			recv(sockfd, &state, sizeof(state), 0);

			state = ntohl(state);
			//state is valid, therefore print
			if (state == 1)
				std::cout<<"Process 0  is using the network\n";
			token = token + 1;
			write(pipefd[0][1], &token, sizeof(int));
			//Read from the pipe, which is attached to the last child created
			read(pipefd[children_number][0], &token, sizeof(int));
			// exit process if state is -1.
			if (token == -1 || state == -1)
			{
				token = -1;
				write(pipefd[0][1], &token, sizeof(int));
				break;
			}
		}
		std::cout << "\nExecution finished. Ring is closed \n\n";

		for (i = 0; i < children_number; i++) {
			wait(NULL);
		}
		return 0;
	}