/**
  \ingroup tinyrl
  \defgroup tinyrl_history history
  @{

  \brief This class handles the maintenance of a historical list of command lines.

*/
#ifndef _tinyrl_history_h
#define _tinyrl_history_h

#include <stdbool.h>
#include "tinyrl.h"
/**************************************
 * tinyrl_history class interface
 ************************************** */

//typedef struct _tinyrl tinyrl_t;

struct tinyrl_history *tinyrl_history_new(tinyrl_t *tinyrl, unsigned limit);

extern void tinyrl_history_delete(struct tinyrl_history *history);
extern void tinyrl_history_add(struct tinyrl_history *history, const char *line);

/*
   HISTORY LIST MANAGEMENT 
   */
extern void tinyrl_history_remove(struct tinyrl_history *history, unsigned offset);
extern void tinyrl_history_clear(struct tinyrl_history *history);
extern void tinyrl_history_limit(struct tinyrl_history *history, unsigned limit);

extern const char *tinyrl_history_get(const struct tinyrl_history *history,
				      unsigned offset);
extern size_t tinyrl_history_length(const struct tinyrl_history *history);

extern bool tinyrl_history_key_up(void *context, int key);
extern bool tinyrl_history_key_down(void *context, int key);

#endif				/* _tinyrl_history_h */
/** @} tinyrl_history */
