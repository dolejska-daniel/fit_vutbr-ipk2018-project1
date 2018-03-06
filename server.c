#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include <sys/types.h>

#ifdef DEBUG
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#endif // DEBUG

#ifdef DEBUG_PRINT_ENABLED
#define DEBUG_PRINT(...) do{ fprintf( stderr, __VA_ARGS__ ); } while( 0 )
#else
#define DEBUG_PRINT(...) do{ } while ( 0 )
#endif // DEBUG_PRINT_ENABLED

#ifdef DEBUG_LOG_ENABLED
#define DEBUG_LOG(...) do{ fprintf( stderr, "[%s]     %s\n", __VA_ARGS__ ); } while( 0 )
#else
#define DEBUG_LOG(...) do{ } while ( 0 )
#endif // DEBUG_LOG_ENABLED

#ifdef DEBUG_ERR_ENABLED
#define DEBUG_ERR(...) do{ fprintf( stderr, "[%s] ERR %s\n", __VA_ARGS__ ); } while( 0 )
#else
#define DEBUG_ERR(...) do{ } while ( 0 )
#endif // DEBUG_ERR_ENABLED

#define ERR(...) do{ fprintf( stderr, __VA_ARGS__ ); } while( 0 )
#define OUTPUT(...) do{ fprintf( stdout, __VA_ARGS__ ); } while( 0 )

int main(int argc, char **argv)
{
    DEBUG_LOG("SRV-INIT", "Starting server...");

	FILE *src;
    struct sockaddr_in server_address;
    unsigned short port_number = 0;
    short c;
	
	src = fopen("/etc/passwd", "r");

    DEBUG_LOG("SRV-INIT", "Processing options...");
    while ((c = getopt(argc, argv, "p:")) != -1)
    {
        switch (c)
        {
            case 'p':
                DEBUG_LOG("SRV-ARG", "Port option registered:");

                char *garbage;
                port_number = strtoul(optarg, &garbage, 0);
                if (strcmp(garbage, "") != 0 || port_number <= 0)
                {
                    ERR("Port '%s' is not valid!\n", optarg);
                    exit(6);
                }

                DEBUG_PRINT("\tvalue: %d\n", port_number);
                break;
            case '?':
                if (isprint(optopt))
                    ERR("Unknown option '-%c' or missing it's argument.\n", optopt);
                else
                    ERR("Unknown option character `\\x%x'.\n", optopt);

                exit(5);
            default:
                ERR("Unknown option character %c.\n", c);
                exit(5);
        }
    }

    if (port_number == 0)
    {
        ERR("Missing option -p: port of remote server is not specified.\n");
        exit(7);
    }

    //  Creating socket: IPv4, TCP
    int server = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (server <= 0)
    {
        DEBUG_ERR("SRV-INIT", "Failed to create socket!");
        DEBUG_PRINT("result: %d\n", server);

        ERR("Socket failed to be created.\n");
        return 1;
    }
    DEBUG_LOG("SRV-INIT", "Socket created.");
	

    int optval = 1;
    int dc_timeout = 10;
    //  Setting something XD
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, (const void*) &optval, sizeof(int));
	setsockopt(server, SOL_SOCKET, SO_LINGER, (const void*) &dc_timeout, sizeof(int));

    //  Clean server_address contents
    memset(&server_address, 0, sizeof(server_address));
    //  IPv4
    server_address.sin_family = AF_INET;
    //  Accepts from any IP
    server_address.sin_addr.s_addr = INADDR_ANY;
    //  Application port
    server_address.sin_port = htons(port_number);

    //  Bind to address and port
    int bind_res = bind(server, (struct sockaddr *) &server_address, sizeof(server_address));
    if (bind_res < 0)
    {
        DEBUG_ERR("SRV-INIT", "Failed to bind!");
        DEBUG_PRINT("result: %d\n", bind_res);

        ERR("Failed to bind the server to port '%d' and any IP address.\n", port_number);
        return 2;
    }
    DEBUG_LOG("SRV-INIT", "Socket bind successful.");

    int listen_res = listen(server, 5);
    if (listen_res < 0)
    {
        DEBUG_ERR("SRV-INIT", "Failed to listen!");
        DEBUG_PRINT("result: %d\n", listen_res);

        ERR("Failed to start listening.\n");
        return 3;
    }
    DEBUG_LOG("SRV-INIT", "Socket listen successful.");

    struct sockaddr_in client;
    unsigned int client_size = sizeof(client);
    char buffer[256];

    while (1)
    {
		rewind(src);
		
        DEBUG_LOG("SRV-CONN", "Waiting for incomming transmission...");
        int socket = accept(server, (struct sockaddr *) &client, &client_size);

        DEBUG_LOG("SRV-CONN", "Connection with client established!");
        if (socket < 0)
        {
            DEBUG_ERR("SRV-CONN", "Error on accept!");
            DEBUG_PRINT("result: %d\n", socket);
        }
        else
        {
			DEBUG_LOG("SRV-CONN", "Preparing to read incomming data...");
			while (1)
			{
				memset(&buffer, 0, sizeof(buffer));
				DEBUG_LOG("SRV-COMMS", "Reading data...");
				int read_res = read(socket, buffer, sizeof(buffer));
				if (read_res < 0)
				{
					DEBUG_ERR("SRV-COMMS", "Error on read from received socket!");
					DEBUG_PRINT("result: %d\n", read_res);
                    close(socket);
					break;
				}
				else
				{
				    int write_res;

					DEBUG_PRINT("received: %s\n", buffer);
					DEBUG_ERR("SRV-COMMS", "Parsing command and search info.");

					char *command;
					char *search;

					command = strtok(buffer, ":");
					search = strtok(NULL, ":");

					DEBUG_PRINT("\tcmd: %s\n", command);
					DEBUG_PRINT("\tsearch: %s\n", search);

					unsigned short cmd_target = 0;
					if (command == NULL || (strcmp(command, "list") != 0 && search == NULL))
                    {
                        ERR("Received invalid command from client.\n");
						write(socket, "!err", 5);
						write(socket, "Received empty command.", strlen("Received empty command.") + 1);
						
                        DEBUG_LOG("SRV-COMMS", "Terminating connection...");
                        close(socket);
                        break;
                    }
					else if (strcmp(command, "info") == 0)
                    {
                        //  -n
                        cmd_target = 5;
                    }
                    else if (strcmp(command, "home") == 0)
                    {
                        //  -f
                        cmd_target = 6;
                    }
                    else if (strcmp(command, "list") == 0)
                    {
                        //  -l
                        cmd_target = 1;

						if (search == NULL)
							search = "";
                    }
                    else
                    {
                        ERR("Received invalid command from client.\n");
						write(socket, "!err", 5);
						write(socket, "Received unknown command.", strlen("Received unknown command.") + 1);
						
                        DEBUG_LOG("SRV-COMMS", "Terminating connection...");
                        close(socket);
                        break;
                    }

                    DEBUG_LOG("SRV-COMMS", "Making system command call...");
					unsigned c_idx = 0;
					unsigned r_idx = 0;
					char results[4096][256];
					char *result;
					char c;
					
					int matches = 1;
					
					while ((c = fgetc(src)) && c != EOF)
					{
						if (c == '\n' || matches == 0)
						{
							if (c == '\n')
							{
								if (matches == 1)
								{
									r_idx++;
								}
								
								matches = 1;
								c_idx = 0;
							}
							
							continue;
						}
						
						results[r_idx][c_idx] = c;
						//results[r_idx][c_idx + 1] = '\0';
						
						if (c_idx < strlen(search))
						{
							if (c != search[c_idx])
							{
								matches = 0;
								continue;
							}
						}
						
						c_idx++;
					}

					DEBUG_LOG("SRV-COMMS", "Search completed!");

					if (r_idx == 0)
					{
						DEBUG_LOG("SRV-COMMS", "Search empty, sending special empty response!");
						write_res = write(socket, "!empty", 7);
						if (write_res < 0)
						{
							DEBUG_ERR("WHILE", "Error on write to socket!");
							DEBUG_PRINT("\tresult: %d\n", write_res);
							break;
						}
					}
					else
					{
						for (unsigned int i = 0; i < r_idx; i++)
						{
							DEBUG_LOG("SRV-COMMS", "Sending response...");
							
							DEBUG_PRINT("\tfull: '%s'\n", results[i]);
							result = strtok(results[i], ":");
							for (unsigned int x = 1; x < cmd_target; x++)
								result = strtok(NULL, ":");
							
							DEBUG_PRINT("\tdata: '%s'\n", result);
							write_res = write(socket, result, strlen(result) + 1);
							if (write_res < 0)
							{
								DEBUG_ERR("WHILE", "Error on write to socket!");
								DEBUG_PRINT("\tresult: %d\n", write_res);
								break;
							}
						}
					}

					DEBUG_LOG("SRV-COMMS", "Result emptied. Sending bye...");
					
					write_res = write(socket, "!bye", 5);

                    DEBUG_LOG("SRV-COMMS", "Terminating connection...");
                    close(socket);
                    break;
				}
			}
        }
    }

    return 0;
}
