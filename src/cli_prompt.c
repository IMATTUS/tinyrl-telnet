/**
 * @file cli.c
 * @author zampar
 * @date Jun 18, 2014
 * @brief User CLI (Command line interface).
 * @brief Also know as user prompt
 *
 * @see http://www.writesys.com.br
 */

#include "cli_prompt.h"

/*** @brief pThread pointer */
pthread_t xCli_Thread_id;

/** @brief Prototype transport call function */
typedef void cmd_function_t(char *, tinyrl_t * this);

/** @brief Readline command available table */
typedef struct
{
	char *name; /** User printable name */
	cmd_function_t *func; /** Function to call */
	char *doc; /** Documentation  */
} command_t;

/** @brief Used to save/restore terminal settings */
static struct termios cli_terminal_settings;

/* Private functions to cli */
static char *cli_trim_space_char(char *string);
static void cli_execute_command(char *line, tinyrl_t * this);
static command_t *cli_find_command(char *name);

/* Private functions for each command to be executed */
static void cli_command_help(char *, tinyrl_t * this);
static void cli_command_quit(char *arg, tinyrl_t * this);

static void cli_command_1(char *arg, tinyrl_t * this);
static void cli_command_2(char *arg, tinyrl_t * this);

/** @brief Structure with all commands. The table must be in alphabetical order */
static command_t commands[] =
{
{ "command_1", cli_command_1, "" },
{ "command_2", cli_command_2, "" },

{ "help", cli_command_help, "" },
{ "quit", cli_command_quit, "" },
{ "?", cli_command_help, "" },

{ (char *) NULL, (cmd_function_t *) NULL, (char *) NULL } };

/**
 * @brief  Check if current command exists in commands table
 * @param  name Command string to be checked
 * @return Command pointer if success or NULL if command not found
 **/
static command_t *cli_find_command(char *name)
{
	register int i;
	size_t namelen;

	if ((name == NULL) || (*name == '\0'))
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
 *         If success execute the right command else return an error message
 * @param  line Command line to be executed
 * @return void
 **/
static void cli_execute_command(char *line, tinyrl_t * this)
{

	register int line_index;
	command_t *command;
	char *word;
	/* Isolate the command word. */
	line_index = 0;

	while (line[line_index] && (((line[line_index]) == ' ') || ((line[line_index]) == '\t')))
		line_index++;
	word = line + line_index;

	while (line[line_index] && !(((line[line_index]) == ' ') || ((line[line_index]) == '\t')))
		line_index++;

	if (line[line_index])
		line[line_index++] = '\0';
	command = cli_find_command(word);
	if (!command)
	{
		tinyrl_printf(this, "\r%s: No such command.  There is `help\'.\n\r", word);
		return;
	}

	/* Get argument to command, if any. */
	while ((((line[line_index]) == ' ') || ((line[line_index]) == '\t')))
		line_index++;

	word = line + line_index;

	/* invoke the command function. */
	(*command->func)(word, this);
}

/**
 * @brief Strip whitespace from the start and end of string.
 * @param string Text to be checked and if necessary remove space chars
 * @return Text with space trim
 **/
static char *cli_trim_space_char(char *string)
{
	register char *s, *t;

	s = string;
	while ((((*s) == ' ') || ((*s) == '\t')))
		s++;

	if (*s == '\0')
		return s;

	t = s + strlen(s) - 1;
	while (t > s && (((*t) == ' ') || ((*t) == '\t')))
		t--;

	*++t = '\0';

	return s;
}

/**
 * @brief  Initialize cli functions. Create all necessary threads
 * @return 0 Success
 */
int cli_prompt_init()
{

	int r;
	/* Create CLI thread */
	r = pthread_create(&xCli_Thread_id, NULL, &cli_prompt_thread, NULL);
	if (r != 0)
	{
		fprintf(stdout, "Fail creating thread. ERR=%u.", r);
	}

	/* Save terminal settings */
	tcgetattr(0, &cli_terminal_settings);

	return r;
}

/**
 * @brief  Deinitialize cli functions. Kill all threads
 * @return 0 Success
 */
int cli_prompt_deinit()
{
	int r;

	/* Cancel CLI Thread */
	r = pthread_cancel(xCli_Thread_id);
	if (r != 0)
	{
		fprintf(stdout, "Fail canceling thread. ERR=%u.", r);
	}

	/* Restore terminal settings */
	tcsetattr(0, TCSANOW, &cli_terminal_settings);

	fprintf(stdout, "Function deinitialized.");

	return (EXIT_SUCCESS);
}

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

static bool tab_key(void *context, int key)
{
	tinyrl_t *t = context;

	if (complete(t, false, false))
		return tinyrl_insert_text(t, " ");
	return false;
}

static bool space_key(void *context, int key)
{
	tinyrl_t *t = context;

	if (complete(t, true, false))
		return tinyrl_insert_text(t, " ");
	return false;
}

static bool enter_key(void *context, int key)
{
	tinyrl_t *t = context;

	if (complete(t, true, true))
	{
		tinyrl_crlf(t);
		tinyrl_done(t);
		return true;
	}
	else
	{
//		tinyrl_crlf(t);
		tinyrl_done(t);
	}
	return false;
}
/**
 * @brief  Main cli loop thread
 * @return void *
 */
void* cli_prompt_thread(void* arg)
{

//	struct thread_data *td;
//	td = (struct thread_data *) arg;
	tinyrl_t *t;

	t = tinyrl_new(stdin, stdout);
	tinyrl_bind_key(t, '\t', tab_key, t);
	tinyrl_bind_key(t, '\r', enter_key, t);
	tinyrl_bind_key(t, ' ', space_key, t);
//
	t->history = tinyrl_history_new(t, 0);
	tinyrl_crlf(t);
	while (1)
	{
		char *line, *cmd;

		line = tinyrl_readline(t, "CLI> ");
		if (!line)
			break;

		/* Remove leading and trailing whitespace from the line. */
		cmd = cli_trim_space_char(line);
		/* If anything left, add to history and execute it. */
		if (*cmd)
		{
			tinyrl_history_add(t->history, line);
			cli_execute_command(cmd, t);
		}

		free(line);
	}
	tinyrl_history_delete(t->history);
	tinyrl_delete(t);
	return 0;
}

/**
 * @brief Show user command available to be used
 * @param arg String with passed arguments
 */
static void cli_command_help(char *arg, tinyrl_t * this)
{
	register int i;
	command_t *cmd;

	if (!*arg)
	{
		/* print help for all commands */
		for (i = 0; commands[i].name; i++)
		{
			tinyrl_printf(this, "%s\t\t%s.\n", commands[i].name, commands[i].doc);
		}

	}
	else if ((cmd = cli_find_command(arg)))
	{
		tinyrl_printf(this, "%s\t\t%s.\n", cmd->name, cmd->doc);

	}
	else
	{
		int printed = 0;

		tinyrl_printf(this, "No `%s' command.  Valid command names are:\n", arg);

		for (i = 0; commands[i].name; i++)
		{
			/* Print in six columns. */
			if (printed == 6)
			{
				printed = 0;
				tinyrl_printf(this, "\n");
			}

			tinyrl_printf(this, "%s\t", commands[i].name);
			printed++;
		}

		tinyrl_printf(this, "\n\nTry `help [command]\' for more information.\n");
	}
}

/**
 * @brief Quit application
 * @param arg Not used
 */
static void cli_command_quit(char *arg, tinyrl_t * this)
{
	cli_quit_application();
}

static void cli_command_1(char *arg, tinyrl_t * this)
{

	return;
}
static void cli_command_2(char *arg, tinyrl_t * this)
{

	return;
}
