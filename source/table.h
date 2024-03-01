#ifndef TABLE_H
#define TABLE_H

#include "debug.h"

#include <string.h>
#include <stdlib.h>

// Engine
typedef struct
{
  char address[64];
  int port;
  char name[64];
  int sockfd;
} Engine;

// Table
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

extern int engine_parse(Engine* engine, Table table, const char* string);

extern void table_print(Table table);

#endif // TABLE_H
