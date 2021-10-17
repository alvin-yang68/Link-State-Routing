#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#include "link_state/context.hpp"
#include "io/network.hpp"
#include "io/log.hpp"

#define HEARTBEAT_INTERVAL_SEC 0
#define HEARTBEAT_INTERVAL_NSEC 500 * 1000 * 1000 // 500ms
#define CHECKUP_INTERVAL_SEC 1
#define CHECKUP_INTERVAL_NSEC 500 * 1000 * 1000 // 500ms
#define TIMEOUT_TOLERANCE_MS 1000

void announce_to_neighbors(Communicator *communicator);
void *announce_to_neighbors_t(void *arg);
void monitor_neighborhood(LinkStateContext *context, Communicator *communicator);
void *monitor_neighborhood_t(void *arg);

typedef struct
{
	LinkStateContext *context;
	Communicator *communicator;
} MonitorNeighborhoodParams;

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

	LinkStateContext context = LinkStateContext(self_id, cost_file_name);
	Log output_log = Log(log_file_name);
	Communicator communicator = Communicator(&context, &output_log);

	struct timespec heartbeat_interval;
	heartbeat_interval.tv_sec = HEARTBEAT_INTERVAL_SEC;
	heartbeat_interval.tv_nsec = HEARTBEAT_INTERVAL_NSEC;

	struct timespec checkup_interval;
	checkup_interval.tv_sec = CHECKUP_INTERVAL_SEC;
	checkup_interval.tv_nsec = CHECKUP_INTERVAL_NSEC;

	communicator.monitor_neighborhood(heartbeat_interval, checkup_interval);
}

void announce_to_neighbors(Communicator *communicator)
{
	pthread_t announcer_thread;
	pthread_create(&announcer_thread, NULL, announce_to_neighbors_t, (void *)communicator);
}

void *announce_to_neighbors_t(void *arg)
{
	Communicator *communicator = (Communicator *)arg;

	struct timespec sleepFor;
	sleepFor.tv_sec = HEARTBEAT_INTERVAL_SEC;
	sleepFor.tv_nsec = HEARTBEAT_INTERVAL_NSEC;
	while (1)
	{
		communicator->broadcast_heartbeats();
		nanosleep(&sleepFor, 0);
	}
}

void monitor_neighborhood(LinkStateContext *context, Communicator *communicator)
{
	MonitorNeighborhoodParams params;
	params.communicator = communicator;
	params.context = context;

	pthread_t monitor_thread;
	pthread_create(&monitor_thread, NULL, monitor_neighborhood_t, (void *)&params);
}

void *monitor_neighborhood_t(void *arg)
{
	MonitorNeighborhoodParams *params = (MonitorNeighborhoodParams *)arg;
	HeartbeatsTracker *hb_tracker = params->context->get_hb_tracker();
	Node *self_node = params->context->get_self_node();
	Communicator *communicator = params->communicator;

	struct timespec sleep_duration;
	sleep_duration.tv_sec = CHECKUP_INTERVAL_SEC;
	sleep_duration.tv_nsec = CHECKUP_INTERVAL_NSEC;

	while (1)
	{
		nanosleep(&sleep_duration, 0); // We sleep first because `globalLastHeartbeat` was initialized to the time when the program was started

		unordered_set<int> online_neighbors = hb_tracker->get_online_nodes(TIMEOUT_TOLERANCE_MS);
		if (online_neighbors != self_node->neighbors)
		{
			self_node->neighbors = online_neighbors;
			self_node->sequence_num += 1;

			LSA lsa = self_node->to_lsa();

			for (const int &id : online_neighbors)
			{
				communicator->send_lsa(id, lsa);
			}
		}
	}
}
