#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <getopt.h>
#include <sys/types.h>

#ifdef DEBUG
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
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
    DEBUG_LOG("CLNT-INIT", "Starting client...");

    char *server_hostname;
    unsigned short port_number = 0;
    struct sockaddr_in server_address;
    short c;

    char *user_name;
    short user_info = 0;
    short user_home = 0;
    short user_list = 0;

    DEBUG_LOG("CLNT-INIT", "Processing options...");
    while ((c = getopt(argc, argv, "h:p:n:f:l::")) != -1)
    {
        switch (c)
        {
            case 'p':
                DEBUG_LOG("CLNT-ARG", "Port option registered:");

                char *garbage;
                port_number = strtoul(optarg, &garbage, 0);
                if (strcmp(garbage, "") != 0 || port_number <= 0)
                {
                    ERR("Port '%s' is not valid!\n", optarg);
                    exit(6);
                }

                DEBUG_PRINT("\tvalue: %d\n", port_number);
                break;
            case 'h':
                DEBUG_LOG("CLNT-ARG", "Host option registered:");

                server_hostname = optarg;

                DEBUG_PRINT("\tvalue: %s\n", optarg);
                break;
            case 'n':
                //  značí, že bude vráceno plné jméno uživatele včetně případných dalších informací pro uvedený login (User ID Info)
                DEBUG_LOG("CLNT-ARG", "-n option registered:");

                if (user_home || user_list)
                {
                    ERR("Can't use option -n with other type options (-f, -l).\n");
                    exit(6);
                }

                user_info = 1;
                user_name = optarg;
                DEBUG_PRINT("\tvalue: %s\n", optarg);

                break;
            case 'f':
                //  značí, že bude vrácena informace o domácím adresáři uživatele pro uvedený login (Home directory)
                DEBUG_LOG("CLNT-ARG", "-f option registered:");

                if (user_info || user_list)
                {
                    ERR("Can't use option -f with other type options (-n, -l).\n");
                    exit(6);
                }

                user_home = 1;
                user_name = optarg;

                DEBUG_PRINT("\tvalue: %s\n", optarg);
                break;
            case 'l':
                //  značí, že bude vrácen seznam všech uživatelů, tento bude vypsán tak, že každé uživatelské jméno bude
                //  na zvláštním řádku; v tomto případě je login nepovinný. Je-li však uveden bude použit jako prefix pro výběr uživatelů.
                DEBUG_LOG("CLNT-ARG", "-l option registered:");

                if (user_home || user_info)
                {
                    ERR("Can't use option -l with other type options (-f, -n).\n");
                    exit(6);
                }

                user_list = 1;
                if (optarg == NULL)
                {
                    if (optind < argc)
                    {
                        user_name = argv[optind];
                    }
                    else
                    {
                        user_name = "";
                    }
                }
                else
                    user_name = optarg;

                DEBUG_PRINT("\tvalue: %s\n", user_name);
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

    if (server_hostname == NULL)
    {
        ERR("Missing option -h: host or IP of remote server is not specified.\n");
        exit(7);
    }

    if (port_number == 0)
    {
        ERR("Missing option -p: port of remote server is not specified.\n");
        exit(7);
    }

    if (user_info == 0 && user_list == 0 && user_home == 0)
    {
        ERR("Missing option -n, -l, or -f: host or IP of remote server is not specified.\n");
        exit(7);
    }

    DEBUG_LOG("CLNT-INIT", "Searching for server...");
    DEBUG_PRINT("\thostname: %s\n", server_hostname);

    //  TODO: Check whether hostname is IP?
    struct hostent *server = gethostbyname(server_hostname);
    if (server == NULL)
    {
        server = gethostbyaddr(server_hostname, sizeof(server_hostname), AF_INET);
    }

    if (server == NULL)
    {
        DEBUG_ERR("CLNT-INIT", "Failed to find server!");

        ERR("Unknown server '%s'.\n", server_hostname);
        return 2;
    }
    DEBUG_LOG("CLNT-INIT", "Server found!");

    DEBUG_LOG("CLNT-INIT", "Creating socket...");
    //  Creating socket: IPv4, TCP
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (client <= 0)
    {
        DEBUG_ERR("CLNT-INIT", "Failed to create socket!");
        DEBUG_PRINT("result: %d\n", client);

        ERR("Socket failed to be created.\n");
        return 1;
    }
    DEBUG_LOG("CLNT-INIT", "Socket created.");

    //  Clean server_address contents
    memset(&server_address, 0, sizeof(server_address));

    //  IPv4
    server_address.sin_family = AF_INET;
    //  Application port
    server_address.sin_port = htons(port_number);
    //  Server IP
    //server_address.sin_addr.s_addr = server->h_addr_list[0];
    int addr_size = sizeof(server->h_addr_list[0]);
    memcpy(&server_address.sin_addr.s_addr, server->h_addr_list[0], addr_size);

    DEBUG_LOG("CLNT-INIT", "Connecting to server...");
    int connect_res = connect(client, (struct sockaddr *) &server_address, sizeof(server_address));
    if (connect_res < 0)
    {
        DEBUG_ERR("CLNT-INIT", "Failed to connect to server!");
        DEBUG_PRINT("result: %d\n", connect_res);

        ERR("Failed to establish connection to remote host.\n");
        return 3;
    }

    DEBUG_LOG("CLNT-COMM", "Preparing to send the message...");
    char buffer[4096];

    memset(&buffer, 0, sizeof(buffer) - 1);
    if (user_info)
    {
        strcat(buffer, "info");
    }
    else if (user_home)
    {
        strcat(buffer, "home");
    }
    else if (user_list)
    {
        strcat(buffer, "list");
    }
    strcat(buffer, ":");
    strcat(buffer, user_name);

    DEBUG_LOG("CLNT-COMM", "Sending the message to server!");
    DEBUG_PRINT("sending data: %s\n", buffer);
    int write_res = write(client, buffer, strlen(buffer) + 1);
    if (write_res < 0)
    {
        DEBUG_ERR("CLNT-COMM", "Error on write to socket!");
        DEBUG_PRINT("result: %d\n", write_res);
    }
	
	int buffer_pos = 0;
	int read_res = 0;
	char *data = "";
	
    while (1)
    {
		if (buffer_pos >= read_res)
		{
			//	V bufferu už nejsou žádné informace
			buffer_pos = 0;
			memset(buffer, 0, sizeof(buffer) - 1);

			DEBUG_LOG("CLNT-COMM", "Reading server response...");
			DEBUG_PRINT("\tclient: %d\n", client);
			read_res = read(client, buffer, sizeof(buffer) - 1);
			if (read_res <= 0)
			{
				DEBUG_ERR("CLNT-COMM", "Error on read from open socket!");
				DEBUG_PRINT("\tresult: %d\n", read_res);
				break;
			}
			DEBUG_PRINT("\tresult: %d\n", read_res);
			DEBUG_PRINT("\tresponse: '%s'\n", buffer);

			if(read_res < (int)sizeof(buffer))
			   buffer[read_res] = '\0';
			else
			   buffer[sizeof(buffer) - 1] = '\0';

			DEBUG_PRINT("\tresponse-post: '%s'\n", buffer);
		}
			
		data = (char *) &buffer[buffer_pos];

        if (strcmp(data, "!err") == 0)
        {
            ERR("There was an error on server side while processing the request.\n");
			close(client);
			break;
        }
        else if (strcmp(data, "!empty") == 0)
        {
            ERR("There were found no users matching your specifications.\n");
			close(client);
			break;
        }
        else if (strcmp(data, "!bye") == 0)
        {
            DEBUG_LOG("CLNT-COMM", "Server closing connection. Bye!");
			close(client);
			break;
        }
        else if (strcmp(data, "") != 0)
        {
			OUTPUT(data);
			OUTPUT("\n");
        }
        else
        {
            //ERR("There was a communication error.\n");
        }
		buffer_pos+= strlen(data) + 1;
    }

    return 0;
}
