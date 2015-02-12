/**
 * @file cli_telnet.c
 * @author ibrahim
 * @date Dec 03, 2014
 * @brief User CLI (Command line interface) over telnet
 * @brief Also know as user telnet prompt
 *
 * @see http://www.writesys.com.br
 */

#include "cli_telnet.h"

/***@brief CLI Telnet pThread pointer */
static pthread_t xCli_Telnet_Thread_id;

int sockfd;
int newsocket_fd;

/** @brief Prototype transport call function */
typedef void cmd_function_t(tinyrl_t *, char *);
/** @brief Readline command available table */
typedef struct
{
	char *name; /**@brief Function displayed name*/
	cmd_function_t *func; /**@brief Function to call */
	char *doc; /**@brief Command Documentation  */
} command_t;

/*@brief Private functions to cli */
static char *cli_telnet_trim_space_char(char *string);
static void cli_telnet_execute_command(char *line, tinyrl_t * this);
static command_t *cli_telnet_find_command(char *name);

/*@brief Private functions for each command to be executed */
static void cli_telnet_command_help(tinyrl_t * this, char *arg);
static void cli_telnet_command_quit(tinyrl_t * this, char *arg);

static void cli_command_1(tinyrl_t * this, char *arg);
static void cli_command_2(tinyrl_t * this, char *arg);

/** @brief Structure with all commands. The table must be in alphabetical order */
static command_t commands[] =
{
{ "command_1", cli_command_1, "" },
{ "command_2", cli_command_2, "" },

{ "help", cli_telnet_command_help, "" },
{ "quit", cli_telnet_command_quit, "" },
{ "?", cli_telnet_command_help, "" },

{ (char *) NULL, (cmd_function_t *) NULL, (char *) NULL } };

/**
 * @brief  Check if current command exists in commands table
 * @param  Command name (string) to be checked
 * @return Command pointer if success or NULL if command not found
 **/
static command_t *cli_telnet_find_command(char *name)
{
	register int i;
	size_t namelen;
	if ((name == NULL) || (name == '\0'))
		return ((command_t *) NULL);
	namelen = strlen(name);
	for (i = 0; commands[i].name; i++)
	{
		if (strncmp(name, commands[i].name, namelen) == 0)
		{
			/* make sure the match is unique */
			if ((commands[i + 1].name) && (strncmp(name, commands[i + 1].name, namelen) == 0))
				return ((command_t *) NULL);
			else
				return (&commands[i]);
		}
	}
	return ((command_t *) NULL);
}

/**
 * @brief  Each ENTER key this function will be executed.
 * @brief  If success execute the right command else return an error message
 * @param  line Command line to be executed
 * @return void
 **/
static void cli_telnet_execute_command(char *line, tinyrl_t * this)
{
	int line_index;
	line_index = 0;
	command_t *command;
	char *word;
	/* Isolate the command word. */
	for (line_index = 0; line_index < strlen(line); line_index++)
	{
		if (!(line[line_index] && (((line[line_index]) == ' ') || ((line[line_index]) == '\t'))))
		{
			break;
		}
	}

	word = line + line_index;

	while (line[line_index] && !(((line[line_index]) == ' ') || ((line[line_index]) == '\t')))
		line_index++;

	if (line[line_index])
		line[line_index++] = '\0';
	command = cli_telnet_find_command(word);
	if (!command)
	{
		tinyrl_printf(this, "\n%s: No such command.  There is `help\'.\n\r", word);
		return;
	}

	/* Get argument to command, if any. */
	while ((((line[line_index]) == ' ') || ((line[line_index]) == '\t')))
		line_index++;

	word = line + line_index;

	/* invoke the command function. */
	(*command->func)(this, word);
}

/**
 * @brief Function generator for command completion.
 * @param t: pointer to the tinyrl structure used
 * @param allow_prefix: bool indicating if prefix will be allowed
 * @param allow_empty: bool indicating if empty strings will be allowed
 * @return Possible next word to be used
 **/
static bool complete(tinyrl_t *t, bool allow_prefix, bool allow_empty)
{
	const char *text;
	unsigned start;
	unsigned end;
	char **matches;
	bool ret = false;

	/* find the start of the current word */
	text = tinyrl__get_line(t);
	start = end = tinyrl__get_point(t);
	while (start && !isspace(text[start - 1]))
		start--;
	if (start == end && allow_empty)
		return true;

	/* build a list of possible completions */
	matches = NULL;
	int count = 0;
	while ((commands[count].name))
	{
		matches = tinyrl_add_match(t, start, matches, commands[count].name);
		count++;
	}

	if (!matches)
		return false;

	/* select the longest completion */
	ret = tinyrl_complete(t, start, matches, allow_prefix);

	tinyrl_delete_matches(matches);

	return ret;
}
/**
 * @brief Strip whitespace from the start and end of string.
 * @param string Text to be checked and if necessary remove space chars
 * @return Text with space trim
 **/
static char *cli_telnet_trim_space_char(char *string)
{
	register char *s;
	s = string;
	char *end;

	// Trim leading space
	while (isspace(*s))
		s++;

	if (*s == 0)  // All spaces?
		return s;

	// Trim trailing space
	end = s + strlen(s) - 1;
	while (end > s && isspace(*end))
		end--;

	// Write new null terminator
	*(end + 1) = 0;

	return s;
}

/**
 * @brief function binded to TAB key
 * @param context: data structure (tinyrl_t) used on the CLI
 */
static bool tab_key(void *context, int key)
{
	tinyrl_t *t = context;

	if (complete(t, false, false))
		return tinyrl_insert_text(t, " ");
	return false;
}

/**
 * @brief function binded to SPACE key
 * @param context: data structure (tinyrl_t) used on the CLI
 */
static bool space_key(void *context, int key)
{
	tinyrl_t *t = context;

	if (complete(t, true, false))
		return tinyrl_insert_text(t, " ");
	return false;
}

/**
 * @brief function binded to ENTER key
 * @param context: data structure (tinyrl_t) used on the CLI
 */
static bool enter_key(void *context, int key)
{
	tinyrl_t *t = context;

	if (complete(t, true, true))
	{
		tinyrl_crlf(t);
		tinyrl_done(t);
		return true;
	}
	return false;
}

/**
 * @brief Calls tinyrl read and interpret the input
 * @return Void
 **/
static void * new_socket_thread(void* arg)
{
	int newsocket_fd;
	newsocket_fd = (int) arg;
	pthread_detach(pthread_self());

	/**
	 * Send telnet command characters to make it character mode
	 * Declaration of the array of command characters
	 */
	const static unsigned char send_telnet[] =
	{
	IAC,
	WILL,
	TELOPT_SGA,
	IAC,
	WILL,
	TELOPT_ECHO };
	/*
	 * Sends (writes) the command array into the socket
	 */
	write(newsocket_fd, send_telnet, sizeof(send_telnet));
	fprintf(stdout, "Setting telnet session.");

	FILE * fdstream;
	fdstream = (FILE*) fdopen(newsocket_fd, "w+");

	tinyrl_t * t;
	t = tinyrl_new(fdstream, fdstream);
	tinyrl_bind_key(t, '\t', tab_key, t);
	tinyrl_bind_key(t, '\r', enter_key, t);
	tinyrl_bind_key(t, ' ', space_key, t);

	t->history = tinyrl_history_new(t, 5);
	t->thread_id = pthread_self();
	t->sock_fd = newsocket_fd;

	char *line, *cmd;
	line = malloc(64);
	cmd = malloc(64);

	while (1)
	{
		line = tinyrl_readline(t, "CLI> ");
		if (!line)
			break;
		/* Remove leading and trailing whitespace from the line. */
		cmd = cli_telnet_trim_space_char(line);

		/* If anything left, add to history and execute it. */
		if (*cmd)
		{
			tinyrl_history_add(t->history, line);
			cli_telnet_execute_command(cmd, t);
		}
	}
	return NULL;
}

/**
 * @brief  Main cli telnet loop thread
 * @return void *
 */
void* cli_telnet_thread(void * arg)
{
	xCli_Telnet_Thread_id = pthread_self();
	socklen_t client_socket_len;
	/** @brief file descriptors for socket and new sockets */
	int portno;
	int r;
	static struct sockaddr_in serv_addr, client_socket_addr;
	pthread_t thread_id;
	while (1)
	{
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0)
		{
			fprintf(stdout, "ERROR opening socket.\n\rWill retry.\n\rERR=%u.\n\r", sockfd);
			fflush(stdout);
			sleep(1);
		}
		else
		{
			portno = 2023;
			serv_addr.sin_family = AF_INET;
			serv_addr.sin_addr.s_addr = INADDR_ANY;
			serv_addr.sin_port = htons(portno);

			if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
			{
				fprintf(stdout, "ERROR binding socket. Will retry. ERR=%u.\n\r", errno);
				fflush(stdout);
				sleep(1);
			}
			else
			{
				fprintf(stdout, "Socket successfully binded.");
				while (1)
				{
					if (listen(sockfd, 1) < 0)
					{
						fprintf(stdout, "ERROR listening socket. Will retry. ERR=%u.\n\r", errno);
						fflush(stdout);
						sleep(1);
					}
					else
					{
						if ((newsocket_fd = accept(sockfd, (struct sockaddr *) &client_socket_addr, &client_socket_len)) < 0)
						{
							fprintf(stdout, "ERROR accepting connection from socket. Will retry. ERR=%u.\n\r", errno);
							fflush(stdout);
							sleep(1);
						}
						else
						{
							r = pthread_create(&thread_id, NULL, &new_socket_thread, newsocket_fd);
							if (r != 0)
							{
								fprintf(stdout, "ERROR creating thread. Will retry. ERR=%u.\n\r", errno);
								fflush(stdout);
								sleep(1);
							}
						}
					}
				}
				close(sockfd);
				return 0;
			}
		}
		sleep(1);
	}
	return 0;
}

/**
 * @brief Initialize cli telnet functions. Create the main telnet thread loop
 * @return 0 Success
 */
int cli_telnet_init()
{

	int r;

	/* Create CLI Telnet thread */
	r = pthread_create(&xCli_Telnet_Thread_id, NULL, &cli_telnet_thread, NULL);
	if (r != 0)
	{
		fprintf(stdout, "Fail creating thread. ERR=%u.", r);
	}

	return r;
}

/**
 * @brief  Deinitialize cli telnet functions.
 * @return 0 Success
 */
int cli_telnet_deinit()
{
	int r;

	r=close(sockfd);

	if (r != 0)
		{
			fprintf(stdout, "Fail closing telnet socket. ERR=%u.\n\r", r);
		}
	/* Cancel CLI Telnet Thread */
	r = pthread_cancel(xCli_Telnet_Thread_id);
	if (r != 0)
	{
		fprintf(stdout, "Fail canceling thread. ERR=%u.", r);
	}
	else
	{
		fprintf(stdout, "Cli Telnet deinitialized.");
	}
	return 0;
}

/**
 * @brief Show commands available
 * @param this: data structure for a specific tinyrl instance
 * @param arg:  String with passed arguments
 */
static void cli_telnet_command_help(tinyrl_t * this, char *arg)
{
	register int i;
	command_t *cmd;
	if (!*arg)
	{
		/* print help for all commands */
		for (i = 0; commands[i].name; i++)
		{
			tinyrl_printf(this, "%s\t\t%s.\n\r", commands[i].name, commands[i].doc);

		}
	}
	else if ((cmd = cli_telnet_find_command(arg)))
	{
		tinyrl_printf(this, "%s\t\t%s.\n\r", cmd->name, cmd->doc);
	}
	else
	{
		int printed = 0;

		tinyrl_printf(this, "No `%s' command.  Valid command names are:\n\r", arg);

		for (i = 0; commands[i].name; i++)
		{
			/* Print in six columns. */
			if (printed == 6)
			{
				printed = 0;
				tinyrl_printf(this, "\n\r");
			}
			tinyrl_printf(this, "%s\t\n\r", commands[i].name);
			printed++;
		}
		tinyrl_printf(this, "\n\n\rTry `help [command]\' for more information.\n\r");
	}
}

/**
 * @brief Quit application
 * @param this: data structure for a specific tinyrl instance
 * @param arg Not used
 */
static void cli_telnet_command_quit(tinyrl_t * this, char *arg)
{
	int r;


	r = close(this->sock_fd);
	if (r != 0)
	{
		fprintf(stdout, "Fail closing telnet socket. ERR=%u.\n\r", r);
	}
	this->sock_fd = 0;

	fclose(this->istream);
	fflush(stdout);
	tinyrl_history_delete(this->history);
	free(this);
	this = NULL;
	pthread_exit(NULL);
	return;
}

static void cli_command_1(tinyrl_t * this, char *arg)
{

	return;
}
static void cli_command_2(tinyrl_t * this, char *arg)
{

	return;
}
