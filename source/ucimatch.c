#include "debug.h"
#include "socket.h"
#include "thread.h"
#include "table.h"

#include <stdlib.h>

bool debug = false;

int rounds = 1;

char fen[256] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";


Table table;


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

/*
 * Keyboard interrupt - close the program (the threads)
 */
void sigint_handler(int signum)
{
  if(debug) info_print("Keyboard interrupt");
}

void sigint_handler_setup(void)
{
  struct sigaction sigAction;

  sigAction.sa_handler = sigint_handler;
  sigAction.sa_flags = 0;
  sigemptyset(&sigAction.sa_mask);

  sigaction(SIGINT, &sigAction, NULL);
}

void sigusr1_handler(int signum) {}

void sigusr1_handler_setup(void)
{
  struct sigaction sigAction;

  sigAction.sa_handler = sigusr1_handler;
  sigAction.sa_flags = 0;
  sigemptyset(&sigAction.sa_mask);

  sigaction(SIGUSR1, &sigAction, NULL);
}

void signals_handler_setup(void)
{
  signal(SIGPIPE, SIG_IGN); // Ignores SIGPIPE
  
  sigint_handler_setup();

  sigusr1_handler_setup();
}

int main(int argc, char* argv[])
{
  flags_parse(argc, argv);

  signals_handler_setup();

  table_load(&table);

  table_print(table);

  if(argc >= 2)
  {
    Engine engine;

    if(engine_parse(&engine, table, argv[1]) == 0)
    {
      info_print("sockfd: %d", engine.sockfd);

      socket_close(&engine.sockfd, true);
    }
    else
    {
      error_print("Failed to parse engine: (%s)", argv[1]);
    }
  }

  table_print(table);

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
