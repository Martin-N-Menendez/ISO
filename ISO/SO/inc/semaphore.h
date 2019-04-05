#ifndef SEMAPHORE_H
#define SEMAPHORE_H

typedef struct {
	_Bool taken;
	uint32_t ticks;
} semaphore_t;


#endif
