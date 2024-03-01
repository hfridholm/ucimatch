#include "debug.h"
#include "socket.h"
#include "thread.h"

#include <stdlib.h>

bool debug = false;

int rounds = 1;

char fen[256] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

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

int main(int argc, char* argv[])
{
  flags_parse(argc, argv);

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
