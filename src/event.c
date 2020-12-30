#include <pthread.h>
#include <stdlib.h>
#include <zconf.h>

#include "event.h"

#define TTETRIS_EVENT_INCOMPLETE 0
#define TTETRIS_EVENT_COMPLETE 1

struct ttetris_event {
	int event_state;
	pthread_mutex_t ready_mutex;
	pthread_cond_t ready_cond;
};

TetrisEvent *ttetris_event_create() {
	struct ttetris_event *event = calloc(sizeof(struct ttetris_event), 1);
	event->event_state = TTETRIS_EVENT_INCOMPLETE;
	return event;
}

void ttetris_event_mark_complete(TetrisEvent *event) {
	pthread_mutex_lock(&event->ready_mutex);
	event->event_state = TTETRIS_EVENT_COMPLETE;
	pthread_cond_broadcast(&event->ready_cond);
	pthread_mutex_unlock(&event->ready_mutex);
}

void ttetris_event_block_for_completion(TetrisEvent *event) {
	pthread_mutex_lock(&event->ready_mutex);
	while (event->event_state == TTETRIS_EVENT_INCOMPLETE) {
		pthread_cond_wait(&event->ready_cond, &event->ready_mutex);
	}
	pthread_mutex_unlock(&event->ready_mutex);
}