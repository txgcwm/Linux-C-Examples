/******************************************************************
 *
 *
 *
 *
 *
 *
 ******************************************************************/

#include <getopt.h>
#include <string.h>
#include <stdio.h>

#include "getoptions.h"

void getoption(int argc, char **argv, pHttpServer phttp_server)
{
	int c, len;
	char *p = 0;
	int option_index = 0;
	static struct option long_options[] = {
		{"host", 1, 0, 0},
		{"port", 1, 0, 0},
		{"dir", 1, 0, 0},
		{"log", 1, 0, 0},
		{"daemon", 0, 0, 0},
		{0, 0, 0, 0}
	};

	opterr = 0;

	while (1) {
		c = getopt_long(argc, argv, "H:P:D:L", long_options, &option_index);
		if (c == -1 || c == '?')
			break;

		if (optarg)
			len = strlen(optarg);
		else
			len = 0;

		if ((!c && !(strcasecmp(long_options[option_index].name, "host")))
			|| c == 'H')
			p = phttp_server->host = (char *)malloc(len + 1);
		else if ((!c && !(strcasecmp(long_options[option_index].name, "port")))
				 || c == 'P')
			p = phttp_server->port = (char *)malloc(len + 1);
		else if ((!c && !(strcasecmp(long_options[option_index].name, "dir")))
				 || c == 'D')
			p = phttp_server->rootdir = (char *)malloc(len + 1);
		else if ((!c && !(strcasecmp(long_options[option_index].name, "log")))
				 || c == 'L')
			p = phttp_server->logdir = (char *)malloc(len + 1);
		else if ((!c
				  && !(strcasecmp(long_options[option_index].name, "daemon"))))
		{
			phttp_server->daemon = 1;
			continue;
		} else
			break;
		bzero(p, len + 1);
		memcpy(p, optarg, len);
	}

	return;
}
