#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#include "link_state/link_state.hpp"

#define HEARTBEAT_INTERVAL_MS 1000
#define CHECKUP_INTERVAL_MS 2000
#define TIMEOUT_TOLERANCE_MS 2000

int main(int argc, char **argv)
{
	if (argc != 4)
	{
		fprintf(stderr, "Usage: %s mynodeid initialcostsfile logfile\n\n", argv[0]);
		exit(1);
	}

	int self_id = atoi(argv[1]);
	char *cost_file_name = argv[2];
	char *log_file_name = argv[3];

	LinkState ls = LinkState(self_id, cost_file_name, log_file_name);

	ls.monitor_neighborhood(HEARTBEAT_INTERVAL_MS, CHECKUP_INTERVAL_MS, TIMEOUT_TOLERANCE_MS);
}