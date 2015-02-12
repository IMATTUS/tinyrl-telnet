/*
 * cli_telnet.h
 *
 *  Created on: Dec 3, 2014
 *      Author: ibrahim
 */

#ifndef CLI_TELNET_H_
#define CLI_TELNET_H_

#include <main.h>

int cli_telnet_init();
int cli_telnet_deinit();
void *cli_telnet_thread(void* arg);

#endif /* CLI_TELNET_H_ */
