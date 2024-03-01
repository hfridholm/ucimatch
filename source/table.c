#include "table.h"
#include "socket.h"

void table_print(Table table)
{
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

  info_print("engine_write(%s)", string);

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

  strncpy(string, buffer, strlen(buffer) - 1);

  info_print("engine_read(%s)", string);

  return status;
}

/*
 * Check uci compatibility and save the engine's name
 *
 * RETURN
 * - 0 | Success!
 * - 1 | Failed to write to engine via socket
 * - 2 | Failed to read from engine via socket
 */
static int engine_greet(Engine* engine, int sockfd)
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
  while(strcmp(buffer, "uciok") != 0);

  return 0;
}

/*
 * RETURN
 * - 0 | Success!
 * - 1 | Failed to create client socket
 * - 2 | Failed to greet engine with uci
 */
static int address_port_engine_create(Engine* engine, const char* address, int port)
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
 * - int amount | The amount of deleted address-port pairs
 */
int table_engine_address_port_delete(TableEngine* engine, const char* address, int port)
{
  // When deleting array elements, you alloc the elements with an offset
  int offset = 0;

  for(int index = 0; index < engine->amount; index++)
  {
    TableEngineServer server = engine->servers[index];

    // If the address and the port matches, delete it
    if(!strcmp(server.address, address) && server.port == port)
    {
      // Increase the offset, to account for another deleted element
      offset++;

      continue;
    }
    // Allocate the not deleted element with an accounted offset
    engine->servers[index - offset] = server;
  }
  // Adjust size of server array (amount), based on deleted elements
  engine->amount -= offset;

  return offset;
}

/*
 * RETURN
 * - int amount | The amount of deleted address-port pairs
 */
int table_address_port_delete(Table* table, const char* address, int port)
{
  int amount = 0;
  int offset = 0;

  for(int index = 0; index < table->amount; index++)
  {
    TableEngine engine = table->engines[index];

    amount += table_engine_address_port_delete(&engine, address, port);

    // If there are no servers associated with an engine, delete the engine
    if(engine.amount == 0)
    {
      // Increase the offset, to account for another deleted element
      offset++;

      continue;
    }
    // Allocate the not deleted element with an accounted offset
    table->engines[index - offset] = engine;
  }
  // Adjust size of engine array (amount), based on deleted elements
  table->amount -= offset;

  return amount;
}

/*
 *
 */
TableEngine* table_engine_address_port_add(TableEngine* engine, const char* address, int port)
{
  TableEngineServer server;

  strcpy(server.address, address);
  server.port = port;

  engine->servers[engine->amount] = server;
  
  engine->amount++;
  
  return engine;
}

/*
 *
 */
Table* table_address_port_add(Table* table, const char* name, const char* address, int port)
{
  info_print("table amount: %d", table->amount);

  for(int index = 0; index < table->amount; index++)
  {
    TableEngine* engine = &table->engines[index];

    info_print("engine amount: %d", engine->amount);

    if(!strcmp(engine->name, name))
    {
      table_engine_address_port_add(engine, address, port);

      return table;
    }
  }
  // If no engine already exist, create a new one
  // Maybe have to initialize and then allocate?
  TableEngine engine;

  strcpy(engine.name, name);

  table_engine_address_port_add(&engine, address, port);
  
  table->engines[++table->amount] = engine;

  return table;
}

/*
 * Save the engine lookup table to a CSV file
 */
int table_save(Table table)
{
  /*
  FILE*  sh
  fprintf("%s", name);

  fprintf(",%s:%d", address, port);

  fprintf("\n");
  */
  return 0;
}

/*
 * Create a chess engine out of a table engine
 *
 * RETURN
 * - 0 | Success!
 * - 1 | Failed to create engine out of table engine
 */
int table_engine_lookup_engine_create(Engine* engine, TableEngine tableEngine)
{
  for(int index = 0; index < tableEngine.amount; index++)
  {
    TableEngineServer server = tableEngine.servers[index];

    if(address_port_engine_create(engine, server.address, server.port) == 0) return 0;
  }
  return 1;
}

/*
 * Create a chess engine by looking up the name of an engine in the table
 *
 * RETURN
 * - 0 | Success!
 * - 1 | Failed to lookup and create a chess engine
 */
int table_lookup_engine_create(Engine* engine, Table table, const char* name)
{
  for(int index = 0; index < table.amount; index++)
  {
    TableEngine tableEngine = table.engines[index];

    if(strcmp(tableEngine.name, name) != 0) continue;

    if(table_engine_lookup_engine_create(engine, tableEngine) == 0) return 0;
  }
  return 1;
}

// Tip: use strtok to seperate address and port at ':'
int address_port_parse(char* address, int* port, const char* string)
{
  if(address == NULL || port == NULL) return 1;

  char tempAddress[64];

  for(int index = 0; index < strlen(string) - 1; index += 1)
  {
    if(string[index] == ':')
    {
      // Parse the rest of the string as the port
      *port = atoi(string + index + 1);

      // Terminate the address string
      tempAddress[index] = '\0';

      strcpy(address, tempAddress);

      return 0; // Success!
    }
    else tempAddress[index] = string[index];
  }
  // No address:port seperator was encountered
  return 2;
}

/*
 * Parse a line in table CSV with engine name and servers
 *
 * RETURN
 * - int amount | The amount of parsed servers
 */
int table_line_parse(TableEngine* engine, char* line)
{
  // name,address:port,address:port
  char* token = strtok(line, ",");

  strcpy(engine->name, token);
  engine->amount = 0;

  while((token = strtok(NULL, ",")) != NULL)
  {
    TableEngineServer server;

    if(address_port_parse(server.address, &server.port, token) == 0)
    {
      engine->servers[engine->amount++] = server;
    }
  }
  return engine->amount;
}

/*
 * Read the lines of the inputted stream
 *
 * RETURN
 * - int amount | The amount of read lines
 */
static int stream_lines_read(char (*lines)[256], FILE* stream)
{
  char buffer[256];
  int index = 0;

  for(index = 0; !feof(stream); index++)
  {
    if(fgets(buffer, sizeof(buffer), stream) == NULL) break;

    memset(lines[index], '\0', sizeof(lines[index]));

    sscanf(buffer, "%[^\n]%*c", lines[index]);
  }
  return index;
}

/*
 * Read the lines of the inputted filepath
 *
 * RETURN
 * - SUCCESS | The amount of read lines
 * - ERROR   | -1
 */
static int filepath_lines_read(char (*lines)[256], const char* filepath)
{
  if(lines == NULL  || filepath == NULL) return -1;

  FILE* stream = fopen(filepath, "r");

  if(stream == NULL) return -1;

  int amount = stream_lines_read(lines, stream);

  fclose(stream);

  return amount;
}

/*
 * RETURN
 * - SUCCESS | The amount of loaded engines
 * - ERROR   | -1
 */
int table_load(Table* table)
{
  char lines[256][256];

  int amount = filepath_lines_read(lines, "../assets/engines.csv");

  if(amount < 0) return -1;

  table->amount = 0;

  for(int index = 0; index < amount; index++)
  {
    TableEngine engine;

    // If the engine has saved servers, add the engine
    if(table_line_parse(&engine, lines[index]) > 0)
    {
      table->engines[table->amount++] = engine;
    }
  }
  return table->amount;
}

/*
 * Create engine from inputted string
 *
 * RETURN
 * - 0 | Error
 * - 1 | Created engine by parsing string as address and port
 * - 2 | Created engine by looking up name in engine table
 */
static int engine_create(Engine* engine, Table table, const char* string)
{
  char address[64];
  int port;

  if(address_port_parse(address, &port, string) == 0)
  {
    int status = address_port_engine_create(engine, address, port);

    return (status != 0) ? 0 : 1;
  }
  else // If the string could not be parsed into address and port,
      // try looking up the name in the engine table
  {
    int status = table_lookup_engine_create(engine, table, string);

    return (status != 0) ? 0 : 2;
  }
}

void engine_print(Engine engine)
{
  printf("Name\t: %s\n", engine.name);
  printf("Address\t: %s\n", engine.address);
  printf("Port\t: %d\n", engine.port);
}

/*
 * Parse engine from inputted string, and update table content
 *
 * RETURN
 * - 0 | Success!
 * - 1 | Failed to parse engine from string
 */
int engine_parse(Engine* engine, Table table, const char* string)
{
  // 1. Create engine, either a new engine or lookup existing engine
  if(engine_create(engine, table, string) == 0) return 1;

  engine_print(*engine);

  // 3. Remove every instance of address and port in lookup table
  table_address_port_delete(&table, engine->address, engine->port);

  // 4. Add new instance of address and port in lookup table
  table_address_port_add(&table, engine->name, engine->address, engine->port);

  return 0;
}
