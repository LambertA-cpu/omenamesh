

#include "common/lock.h"

void *worker(void *arg) {
	acquire_lock(&global_lock_manager, 0, 0);
	printf("Thread %ld acquired lock\n", pthread_self());
	sleep(2);
	release_lock(&global_lock_manager, 0);
	printf("Thread %ld released lock\n", pthread_self());
	return NULL;
}

int main() {
	pthread_t t1, t2;
	pthread_create(&t1, NULL, worker, NULL);
	sleep(1);
	pthread_create(&t2, NULL, worker, NULL);

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	return 0;
}
