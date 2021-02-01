#include <stdlib.h>
#include <zconf.h>

#include "os_compat.h"
#ifdef THIS_IS_WINDOWS
#include <windows.h>
#else
#include <pthread.h>

#define TTETRIS_EVENT_INCOMPLETE 0
#define TTETRIS_EVENT_COMPLETE 1
#endif

#include "event.h"

struct ttetris_event {
#ifdef THIS_IS_WINDOWS
	HANDLE handle;
#else
	int event_state;
	pthread_mutex_t ready_mutex;
	pthread_cond_t ready_cond;
#endif
};

int event_id = 0;

TetrisEvent *ttetris_event_create() {
	struct ttetris_event *event = calloc(sizeof(struct ttetris_event), 1);
#ifdef THIS_IS_WINDOWS
	// create a unique name for the event
	char unique_name[7];
	unique_name[0] = (char)('A' + (event_id & 7));
	unique_name[1] = (char)('A' + (event_id >> 4 & 7));
	unique_name[2] = (char)('A' + (event_id >> 8 & 7));
	unique_name[3] = (char)('A' + (event_id >> 12 & 7));
	unique_name[4] = (char)('A' + (event_id >> 16 & 7));
	unique_name[5] = (char)('A' + (event_id >> 20 & 7));
	unique_name[6] = 0;
	event_id++;
	event->handle = CreateEvent(NULL,  // default security attributes
	                            TRUE,  // manual-reset event
	                            FALSE, // initial state is nonsignaled
	                            TEXT(unique_name) // object name
	);
#else
	event->event_state = TTETRIS_EVENT_INCOMPLETE;
#endif
	return event;
}

void ttetris_event_mark_complete(TetrisEvent *event) {
#ifdef THIS_IS_WINDOWS
	SetEvent(event->handle);
#else
	pthread_mutex_lock(&event->ready_mutex);
	event->event_state = TTETRIS_EVENT_COMPLETE;
	pthread_cond_broadcast(&event->ready_cond);
	pthread_mutex_unlock(&event->ready_mutex);
#endif
}

void ttetris_event_block_for_completion(TetrisEvent *event) {
#ifdef THIS_IS_WINDOWS
	WaitForSingleObject(event->handle, INFINITE);
#else
	pthread_mutex_lock(&event->ready_mutex);
	while (event->event_state == TTETRIS_EVENT_INCOMPLETE) {
		pthread_cond_wait(&event->ready_cond, &event->ready_mutex);
	}
	pthread_mutex_unlock(&event->ready_mutex);
#endif
}