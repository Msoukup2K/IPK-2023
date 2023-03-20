#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>


#define BUFSIZE 1024


int main(int argc, char* argv[]) {
    if (argc != 7) {
        std::cerr << "Usage: " << argv[0] << " -h <host> -p <port> -m <mode>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string host;
    int port = 0;
    std::string mode;

    // Parse command line arguments
    for (int i = 1; i < argc; i += 2) {
        if (std::strcmp(argv[i], "-h") == 0) {
            host = argv[i + 1];
        } else if (std::strcmp(argv[i], "-p") == 0) {
            port = std::atoi(argv[i + 1]);
        } else if (std::strcmp(argv[i], "-m") == 0) {
            mode = argv[i + 1];
        }
        else
        {
            std::cerr << "Invalid argument";
            return EXIT_FAILURE;
        }
    }

    if( host.empty() )
    {
        std::cerr << "Host not specified";
        return EXIT_FAILURE;
    }

    if( port == 0 )
    {
        std::cerr << "Port not specified";
        return EXIT_FAILURE;
    }

    int client_sock;
    if( mode == "tcp" )
    {

         // Create socket
        client_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (client_sock == -1) {
            std::cerr << "Failed to create socket" << std::endl;
            return EXIT_FAILURE;
        }

    }
    else if( mode == "udp" )
    {
        client_sock = socket(AF_INET, SOCK_DGRAM, 0);
        if( client_sock == -1)
        {
            std::cerr << "Failed to create socket" << std::endl;
            return EXIT_FAILURE;
        }
    }
    else
    {
        std::cerr << "Can connect through TCP or UDP";
        return EXIT_FAILURE;
    }

    struct hostent* server = gethostbyname(host.c_str());
    if( server == NULL )
    {
        std::cerr << "Failed to get server address";
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_address;
    std::memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    std::memcpy(&server_address.sin_addr.s_addr, server->h_addr, server->h_length);
    server_address.sin_port = htons(port);

    //Connect
    if( mode == "tcp" )
    {
        if(connect(client_sock, (struct sockaddr*)&server_address, sizeof(server_address)) == -1)
        {
                std::cerr << "Failed to connect to server";
                return EXIT_FAILURE;
        }
    }

    std::string input;
    while (std::getline(std::cin, input)) {

        char buffer[BUFSIZE];

        std::memset(buffer,0,sizeof(buffer));

        if( mode == "tcp" )
        {
            std::strncpy(buffer, input.c_str(), sizeof(buffer));
            if( send(client_sock, buffer, sizeof(buffer),0) == -1)
            {
                std::cerr << "Sending failed";
                continue;
            }
        }
        else //udp
        {
            std::memcpy(buffer, input.c_str(), std::min(input.length(),sizeof(buffer)));
            if(sendto(client_sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&server_address, sizeof(server_address)) == -1)
            {
                std::cerr << "Sending failed";
                continue;
            }
        }

        std::memset(buffer, 0, sizeof( buffer ));

        if( mode == "tcp" )
        {
            if( recv(client_sock, buffer, sizeof(buffer), 0) == -1)
            {
                std::cerr << "Failed to get a response";
                continue;
            }

        }
        else //udp
        {
            socklen_t server_len = sizeof(server_address);

            if( recvfrom( client_sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&server_address, &server_len) == -1)
            {
                std::cerr << "Failed to get a response";
                continue;
            }
        }

        std::string response(buffer);

        if( response.substr(0,3) == "OK")
        {
            std::cout << response << "\n";

        }
        else if ( response.substr(0, 4) == "ERR:")
        {
            std::cerr << response << "\n";
        }
        else
        {
            std::cerr << "Invalid response";
        }

    }

    close(client_sock);

    return EXIT_SUCCESS;
}