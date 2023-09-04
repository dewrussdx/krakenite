#include <iomanip>
#include <iostream>
#include "test.h"

#include <iostream>
#include <winsock2.h>

#pragma comment(lib,"ws2_32.lib") 
#pragma warning(disable:4996) 

#define SERVER "127.0.0.1"  // or "localhost" - ip address of UDP server
#define BUFLEN 512  // max length of answer
#define PORT 8888  // the port on which to listen for incoming data

int client()
{
    // initialise winsock
    WSADATA ws;
    if (WSAStartup(MAKEWORD(2, 2), &ws) != 0)
    {
        printf("Failed. Error Code: %d", WSAGetLastError());
        return 1;
    }
 
    // create socket
    sockaddr_in server;
    SOCKET client_socket;
    if ((client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR) // <<< UDP socket
    {
        printf("socket() failed with error code: %d", WSAGetLastError());
        return 2;
    }

    // setup address structure
    memset((char*)&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.S_un.S_addr = inet_addr(SERVER);

    // start communication
    while (true)
    {
        char message[BUFLEN];
        printf("Enter message: ");
        std::cin.getline(message, BUFLEN);

        // send the message
        if (sendto(client_socket, message, static_cast<int>(strlen(message)), 
            0, (sockaddr*)&server, sizeof(sockaddr_in)) == SOCKET_ERROR)
        {
            printf("sendto() failed with error code: %d", WSAGetLastError());
            return 3;
        }

        // receive a reply and print it
        // clear the answer by filling null, it might have previously received data
        char answer[BUFLEN] = {};

        // try to receive some data, this is a blocking call
        int slen = sizeof(sockaddr_in);
        int answer_length;
        if (answer_length = recvfrom(client_socket, answer, BUFLEN, 0, (sockaddr*)&server, &slen) == SOCKET_ERROR)
        {
            printf("recvfrom() failed with error code: %d", WSAGetLastError());
            exit(0);
        }

        std::cout << answer << "\n";
    }

    closesocket(client_socket);
    WSACleanup();
}

// Entry point
int main(int argc, const char** argv)
{
    (void)argc;
    (void)argv;

	// set the default precision to two decimal points
	std::cout << std::fixed << std::setprecision(2) << std::setfill(' ');
	//auto result = UnitTests("input_trimmed.csv").run();
	//assert(result);
}