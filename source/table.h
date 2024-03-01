#ifndef TABLE_H
#define TABLE_H

#include "debug.h"

#include <string.h>
#include <stdlib.h>

typedef struct
{
  char address[64];
  int port;
} TableEngineServer;

typedef struct
{
  char name[64];
  int amount;
  TableEngineServer servers[16];
} TableEngine;

typedef struct
{
  int amount;
  TableEngine engines[32];
} Table;

extern int table_load(Table* table);

extern int address_port_parse(char* address, int* port, const char* string, int length);

extern int table_address_port_delete(Table* table, const char* address, int port);

#endif // TABLE_H
