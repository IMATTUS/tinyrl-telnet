#ifndef CLI_H_
#define CLI_H_

#include <main.h>

int cli_prompt_init();
int cli_prompt_deinit();
void *cli_prompt_thread(void* arg);

#endif /* CLI_H_ */
