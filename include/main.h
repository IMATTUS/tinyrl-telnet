#ifndef MAIN_H_
#define MAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>

#include <pthread.h>

#include <arpa/telnet.h>
#include <arpa/inet.h>

#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "cli_prompt.h"
#include "cli_telnet.h"

#include "tinyrl.h"
#include "tinyrl_complete.h"
#include "tinyrl_history.h"

/**
 * @brief The set of possible main app states.
 */
typedef enum
{
	NO_CHANGE_STATE = 0, START_APP, INIT_CLI, APP_IDDLE, QUIT_APP, DEINIT_CLI, DEINIT_APP, CLOSE_APP
} main_app_states;

void cli_set_call_state(int state);
int cli_get_call_state();

void _cli_set_machine_state(int state);

void cli_print_usage(char *prg);
void cli_sigterm(int signo);
void cli_quit_application(void);

char * cli_get_app_build_date();

#endif /* MAIN_H_ */
