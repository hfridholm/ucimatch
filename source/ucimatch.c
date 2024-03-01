#include "debug.h"
#include "socket.h"
#include "thread.h"
#include "table.h"

#include <stdlib.h>

bool debug = false;

int rounds = 1;

char fen[256] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";


Table table;


typedef struct
{
  char address[64];
  int port;
  char name[64];
  int sockfd;
} Engine;

/*
 * Parse the current passed flag
 *
 * FLAGS
 * --debug           | Output debug messages
 * --rounds=<amount> | The amount of played rounds
 */
void flag_parse(char flag[])
{
  if(!strcmp(flag, "--debug"))
  {
    debug = true;
  }
  else if(!strncmp(flag, "--rounds=", 9))
  {
    rounds = atoi(flag + 9);
  }
  else if(!strncmp(flag, "--position=", 11))
  {
    strcpy(fen, flag + 11);
  }
}

/*
 * Parse every passed flag
 */
void flags_parse(int argc, char* argv[])
{
  for(int index = 1; index < argc; index += 1)
  {
    flag_parse(argv[index]);
  }
}

int address_port_lookup(char* address, int* port, const char* name)
{
  return 1;
}

/*
 * socket_write, with new-line character
 *
 * RETURN (same as socket_write)
 * - SUCCESS | The number of written characters
 * - ERROR   | -1
 */
int engine_write(int sockfd, const char* string, size_t size)
{
  char buffer[size + 1];

  sprintf(buffer, "%s\n", string);

  return socket_write(sockfd, buffer, strlen(buffer));
}

/*
 * socket_read, without new-line character
 *
 * RETURN (same as socket_read)
 * - SUCCESS | The number of read characters
 * - ERROR   | -1
 */
int engine_read(int sockfd, char* string, size_t size)
{
  char buffer[size + 1];
  memset(buffer, '\0', sizeof(buffer));

  int status = socket_read(sockfd, buffer, size + 1);

  if(status == -1) return -1;

  strncpy(string, buffer, size);

  return status;
}

/*
 * Check uci compatibility and save the engine's name
 */
int engine_greet(Engine* engine, int sockfd)
{
  engine->sockfd = sockfd;

  if(engine_write(sockfd, "uci", 3) != 3) return 1;

  char buffer[1024];
  do 
  {
    memset(buffer, '\0', sizeof(buffer));

    if(engine_read(sockfd, buffer, sizeof(buffer)) == -1) return 2;

    // Save the engine's name
    if(!strncmp(buffer, "id name ", 8))
    {
      strcpy(engine->name, buffer + 8);
    }
  }
  while(strcmp(buffer, "uciok"));

  return 0;
}

int engine_create(Engine* engine, const char* address, int port)
{
  int sockfd = client_socket_create(address, port, true);

  if(sockfd == -1) return 1;

  strcpy(engine->address, address);
  engine->port = port;

  if(engine_greet(engine, sockfd) != 0)
  {
    socket_close(&sockfd, true);
    
    return 2;
  }
  return 0; // Success!
}

/*
 * RETURN
 * - 0 | ERROR
 * - 1 | Successfully parsed address and port from string
 * - 2 | Successfully looked up address and port from name (string)
 */
int address_port_get(char* address, int* port, const char* string, int length)
{
  if(address_port_parse(address, port, string, length) == 0) return 1;

  if(address_port_lookup(address, port, string) == 0) return 2;

  return 0; // ERROR
}

int engine_parse(Engine* engine, const char* string, int length)
{
  char address[64];
  int port;

  // 1. Get address and port of chess engine
  if(address_port_get(address, &port, string, length) == 0) return 1;

  // 2. Create engine struct and connect to the engine socket
  if(engine_create(engine, address, port) != 0) return 2;

  // 3. Remove every instance of address and port in lookup table
  table_address_port_delete(&table, address, port);

  // 4. Add new instance of address and port in lookup table

  return 0;
}

int main(int argc, char* argv[])
{
  flags_parse(argc, argv);

  if(argc >= 2)
  {
    printf("String: (%s)\n", argv[1]);

    char address[64];
    int port;

    if(address_port_parse(address, &port, argv[1], strlen(argv[1])) == 0)
    {
      printf("Address: (%s) Port: (%d)\n", address, port);
    }
    else printf("Failed to parse address and port\n");
  }

  table_load(&table);

  for(int i = 0; i < table.amount; i++)
  {
    TableEngine engine = table.engines[i];

    printf("%s", engine.name);

    for(int j = 0; j < engine.amount; j++)
    {
      TableEngineServer server = engine.servers[j];

      printf(",%s:%d", server.address, server.port);
    }
    printf("\n");
  }

  /*
  if(argc < 3) // Not enough passed arguments to execute
  {
    return 1;
  }
  */

  /*
   * 1. Extract address and port for both player1 and player2
   * 2. Connect to both player1 and player2 ucinodes (socket server)
   * 3. Start with player1:
   *    - Setup position and ask for best move
   * 4. Continue with next player:
   *    - Setup new position and ask for best move
   *    - Switch player and execute step 4 again
   * 5. Register match result and as long as there are rounds left, go to step 3
   * 6. Summerize all matches and display them to the user
   *
   */

  return 0;
}
