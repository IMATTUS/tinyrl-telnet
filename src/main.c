/**
 * @file cli.c
 * @author ibrahim
 * @date Jan 10, 2015
 * @brief Initiates CLIs over telnet, tcp/ip and tty
 */

#include "main.h"

/** @brief Actual main application state machine.*/
int main_app_state;

/** @brief Next machine state. The current state just change on pass IDLE state!*/
int main_app_state_next;

/**
 * @brief Application enter point
 * @param argc Number of arguments
 * @param argv String with all command line arguments
 * @return 0 Success
 */
int main(int argc, char **argv)
{
	int opt;
	while ((opt = getopt(argc, argv, "h?")) != -1)
	{
		switch (opt)
		{

		case 'h':
		case '?':
			cli_print_usage(basename(argv[0]));
			exit(EXIT_SUCCESS);
			break;

		default:
			fprintf(stderr, "Unknown option %c\n", opt);
			exit(EXIT_FAILURE);
			break;
		}
	}

	// Initial state after init app.
	main_app_state = START_APP;
	main_app_state_next = NO_CHANGE_STATE;

	for (;;)
	{
		switch (main_app_state)
		{
		case START_APP:
			fprintf(stdout, "Starting application.");
			fflush(stdout);
			main_app_state = INIT_CLI;
			break;

		case INIT_CLI:
			fprintf(stdout, "Initializing CLI.");
			fflush(stdout);
			cli_prompt_init();
			cli_telnet_init();

			main_app_state = APP_IDDLE;
			break;

		case APP_IDDLE:
			main_app_state = APP_IDDLE;

			// If there is a new state, switch to it
			if (main_app_state_next != NO_CHANGE_STATE)
			{
				main_app_state = main_app_state_next;
				main_app_state_next = NO_CHANGE_STATE;
			}
			else
			{
				usleep(100 * 1000);
			}
			break;

			// Finish Main loop
		case QUIT_APP:
			// finalize all apps before start closing all threads
			main_app_state = DEINIT_CLI;
			break;

		case DEINIT_CLI:
			fprintf(stdout, "Deinitializing CLI.");
			main_app_state = DEINIT_APP;
			cli_telnet_deinit();
			cli_prompt_deinit();

			break;

		case DEINIT_APP:
			fprintf(stdout, "Deinitializing app.");
			main_app_state = CLOSE_APP;
			break;

		case CLOSE_APP:
			fprintf(stdout, "Closing app.\n\r");

			exit(EXIT_SUCCESS);
			break;

		default:
			fprintf(stdout, "Invalid state machine.");
			main_app_state = APP_IDDLE;
			break;
		}
	}

	return EXIT_FAILURE;
}

/**
 * @brief Set a new state machine to be executed on next IDDLE event
 * @param state New state to be set
 */
void _cli_set_machine_state(int state)
{
	main_app_state_next = state;
}

/**
 * @brief Print usage options of program to use on command line
 * @param prg Program name
 */
void cli_print_usage(char *prg)
{
	fprintf(stderr, "\nUsage: %s\n", prg);
	fprintf(stderr, "\ncli, compile date: %s - %s\n", __DATE__, __TIME__);
}

/**
 * @brief Function that process all SIG signals. To avoid that Ctrl+C, Ctrl+Z... close the app
 * @param signo Signal number received
 */
void cli_sigterm(int signo)
{
	fprintf(stderr, "Received kill signal, code: %u.", signo);
}

/**
 * @brief Function that change state machine to quit application
 */
void cli_quit_application(void)
{
	_cli_set_machine_state(QUIT_APP);
}
