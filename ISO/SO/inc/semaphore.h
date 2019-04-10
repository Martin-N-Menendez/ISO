#ifndef SEMAPHORE_H
#define SEMAPHORE_H

typedef struct {
	_Bool taken;
	uint32_t ticks;
} semaphore_t;


void semaphore_create(semaphore_t* arg);
void semaphore_give(semaphore_t* sem);
void semaphore_take(semaphore_t* sem);



#endif
