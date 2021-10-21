#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#include "link_state/link_state.hpp"

#define HEARTBEAT_INTERVAL_SEC 0
#define HEARTBEAT_INTERVAL_NSEC 500 * 1000 * 1000 // 500ms
#define CHECKUP_INTERVAL_SEC 1
#define CHECKUP_INTERVAL_NSEC 500 * 1000 * 1000 // 500ms
#define TIMEOUT_TOLERANCE_SEC 1

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

	ls.send_lsa_to_neighbors();

	struct timespec heartbeat_interval;
	heartbeat_interval.tv_sec = HEARTBEAT_INTERVAL_SEC;
	heartbeat_interval.tv_nsec = HEARTBEAT_INTERVAL_NSEC;

	struct timespec checkup_interval;
	checkup_interval.tv_sec = CHECKUP_INTERVAL_SEC;
	checkup_interval.tv_nsec = CHECKUP_INTERVAL_NSEC;

	ls.monitor_neighborhood(heartbeat_interval, checkup_interval, TIMEOUT_TOLERANCE_SEC);
}