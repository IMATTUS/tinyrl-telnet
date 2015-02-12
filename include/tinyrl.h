/**
  \ingroup tinyrl
  \defgroup tinyrl_class tinyrl
  @{

  \brief This class provides instances which are capable of handling user input
  from a CLI in a "readline" like fashion.

*/
#ifndef _tinyrl_tinyrl_h
#define _tinyrl_tinyrl_h

#include <stdbool.h>
#include <stdio.h>
#include <regex.h> // nao está no original
#include <termios.h> // nao está no original
#include <sys/socket.h> // nao está no original

/* Data passed to the threads we create */
//struct thread_data {
//    pthread_t thread_id;		/* Terminal thread (for pthread_kill()) */
//    FILE * fdstream;
//    int new_fd;
//};

/* define the class member data and virtual methods */
struct _tinyrl {
	FILE *istream;
	FILE *ostream;
	const char *line;
	unsigned max_line_length;
	const char *prompt;
	char *buffer;
	size_t buffer_size;
	bool done;
	unsigned point;
	unsigned end;
	char *kill_string;
	struct tinyrl_keymap *keymap;
	struct tinyrl_history *history; // todo remover -- inserido para usar o historico, retirar e usar como no original // fixme
	char echo_char;
	bool echo_enabled;
	struct termios default_termios;
	bool isatty;
	char *last_buffer;	/* hold record of the previous
				   buffer for redisplay purposes */
	unsigned last_point;	/* hold record of the previous
				   cursor position for redisplay purposes */
	pthread_t thread_id;
	int sock_fd;
};
////////////////////////////////

typedef struct _tinyrl tinyrl_t;

enum tinyrl_key {
	TINYRL_KEY_UP,
	TINYRL_KEY_DOWN,
	TINYRL_KEY_LEFT,
	TINYRL_KEY_RIGHT,
};

/* virtual methods */
typedef int tinyrl_hook_func_t(tinyrl_t * instance);

/**
 * \return
 * - true if the action associated with the key has
 *   been performed successfully
 * - false if the action was not successful
 */
typedef bool tinyrl_key_func_t(void * context, int key);

/* exported functions */
extern tinyrl_t *tinyrl_new(FILE * instream,
			    FILE * outstream);

/*lint -esym(534,tinyrl_printf)  Ignoring return value of function */
extern int tinyrl_printf(const tinyrl_t * instance, const char *fmt, ...);

extern void tinyrl_delete(tinyrl_t * instance);

extern const char *tinyrl__get_prompt(const tinyrl_t * instance);

extern void tinyrl_done(tinyrl_t * instance);

/**
 * This operation returns the current line in use by the tinyrl instance
 * NB. the pointer will become invalid after any further operation on the 
 * instance.
 */
extern const char *tinyrl__get_line(const tinyrl_t * instance);

extern unsigned tinyrl__get_point(const tinyrl_t * instance);

extern unsigned tinyrl__get_end(const tinyrl_t * instance);

extern unsigned tinyrl__get_width(const tinyrl_t * instance);

extern void tinyrl__set_istream(tinyrl_t * instance, FILE * istream);

extern bool tinyrl__get_isatty(const tinyrl_t * instance);

extern FILE *tinyrl__get_istream(const tinyrl_t * instance);

extern FILE *tinyrl__get_ostream(const tinyrl_t * instance);

char * tinyrl_readline(tinyrl_t * instance, const char *prompt);

void tinyrl_bind_key(tinyrl_t * instance, unsigned char key,
		     tinyrl_key_func_t *handler, void *context);
void tinyrl_bind_special(tinyrl_t * instance, enum tinyrl_key key,
			 tinyrl_key_func_t *handler, void *context);

extern void tinyrl_crlf(const tinyrl_t * instance);
extern void tinyrl_ding(const tinyrl_t * instance);

extern void tinyrl_reset_line_state(tinyrl_t * instance);

extern bool tinyrl_insert_text(
	tinyrl_t * instance, const char *text);
extern bool tinyrl_insert_text_len(
	tinyrl_t * instance, const char *text, unsigned len);
extern void tinyrl_delete_text(
	tinyrl_t * instance, unsigned start, unsigned end);

extern void tinyrl_redisplay(tinyrl_t * instance);

/* text must be persistent */
extern void tinyrl_set_line(tinyrl_t * instance, const char *text);

extern void
tinyrl_replace_line(tinyrl_t * instance, const char *text, int clear_undo);

/**
 * Disable echoing of input characters when a line in input.
 * 
 */
extern void tinyrl_disable_echo(
	/** 
	 * The instance on which to operate
	 */
	tinyrl_t * instance,
	/**
	 * The character to display instead of a key press.
	 *
	 * If this has the special value '/0' then the insertion point will not 
	 * be moved when keys are pressed.
	 */
	char echo_char);
/**
 * Enable key echoing for this instance. (This is the default behavior)
 */
extern void tinyrl_enable_echo(
	/** 
	 * The instance on which to operate
	 */
	tinyrl_t * instance);
/**
 * Limit maximum line length
 */
extern void tinyrl_limit_line_length(
	/** 
	 * The instance on which to operate
	 */
	tinyrl_t * instance,
	/** 
	 * The length to limit to (0) is unlimited
	 */
	unsigned length);

/* External functions (from tinyrl_history) to provide navigation through the history of commands */
extern bool tinyrl_history_key_up(void *context, int key);
extern bool tinyrl_history_key_down(void *context, int key);

#endif				/* _tinyrl_tinyrl_h */
/** @} tinyrl_tinyrl */
