#include "table.h"

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
  TableEngineServer* server = &engine->servers[++engine->amount];

  strcpy(server->address, address);
  server->port = port;

  return engine;
}

/*
 *
 */
Table* table_address_port_add(Table* table, const char* name, const char* address, int port)
{
  for(int index = 0; index < table->amount; index++)
  {
    TableEngine* engine = &table->engines[index];

    if(!strcmp(engine->name, name))
    {
      table_engine_address_port_add(engine, address, port);

      return table;
    }
  }
  // If no engine already exist, create a new one
  TableEngine* engine = &table->engines[++table->amount];

  strcpy(engine->name, name);

  table_engine_address_port_add(engine, address, port);

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

// Tip: use strtok to seperate address and port at ':'
int address_port_parse(char* address, int* port, const char* string, int length)
{
  if(address == NULL || port == NULL) return 1;

  char tempAddress[64];

  for(int index = 0; index < length - 1; index += 1)
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

    if(address_port_parse(server.address, &server.port, token, strlen(token)) == 0)
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
