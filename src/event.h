/**
 * simple cross-platform library for describing events and waiting for events
 */

#ifndef TTETRIS_EVENT_H
#define TTETRIS_EVENT_H

typedef struct ttetris_event TetrisEvent;

/**
 * Allocates an event
 * @return event
 */
TetrisEvent *ttetris_event_create();

/**
 * Mark an event as completed. All threads blocking on the event will resume.
 * @param event
 */
void ttetris_event_mark_complete(TetrisEvent *event);

/**
 * Wait for an event to complete
 * @param event
 */
void ttetris_event_block_for_completion(TetrisEvent *event);

#endif // TTETRIS_EVENT_H
