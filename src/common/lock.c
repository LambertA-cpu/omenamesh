#include "lock.h"
#include "debug.h"
#include "types.h"

/*NOTE have a global manager for this to work so before main, initialize the
 * lock, disclamer IDK if that will work*/
__CJLF_GENERICS lock_init(LockManager *manager) {
	for (int i = 0; i < MAX_LOCKS; i++) {
		atomic_store(&manager->locks[i].locked, false);
	}
	manager->request_queue.front = NULL;
	manager->request_queue.rear = NULL;
	atomic_store(&manager->request_queue.size, 0);
}

/* Acquire the lock with a callback and argument for queued requests */
__CJLF_GENERICS acquire_lock(LockManager *manager,
			     __CJLF_GENERICS (*callback)(__CJLF_GENERICS *arg),
			     __CJLF_GENERICS *arg) {
	OMENA_MESH_TODO(
	    "keep count of the locks, in the lock manager have lock_count, "
	    "this can help us assert before we try to get the lock and just "
	    "queue it");

	bool acquired = false; /*atomic?*/

	/*spin & try to acquire a lock*/
	for (int i = 0; i < MAX_LOCKS; i++) {
		bool expected = false;
		if (!atomic_load(&manager->locks[i].locked) &&
		    atomic_compare_exchange_strong(&manager->locks[i].locked,
						   &expected, true)) {
			acquired = true;
			OMENA_MESH_LOG(1, "Lock %d acquired.\n", i);
			return;
		}
	}

	/*If no lock is available, queue the request */
	if (!acquired) {
		queue_lock_request(manager, callback, arg);
		OMENA_MESH_LOG(1, "No locks available, request queued.\n");
	}
}

/* Release a specific lock and process any pending requests in the queue */
__CJLF_GENERICS release_lock(LockManager *manager, int lock_id) {
	if (lock_id >= 0 && lock_id < MAX_LOCKS) {
		atomic_store(&manager->locks[lock_id].locked, false);
		process_lock_queue(manager);
	}
}

/* Check if a lock is currently held */
bool is_locked(LockManager *manager, int lock_id) {
	if (lock_id >= 0 && lock_id < MAX_LOCKS) {
		return atomic_load(&manager->locks[lock_id].locked);
	}
	return false;
}

__CJLF_GENERICS queue_lock_request(
    LockManager *manager,
    __CJLF_GENERICS (*callback)(__CJLF_GENERICS *arg),
    __CJLF_GENERICS *arg) {
	LockRequestNode *new_request =
	    (LockRequestNode *)malloc(sizeof(LockRequestNode));
	new_request->callback = callback;
	new_request->arg = arg;
	new_request->next = NULL;

	// Add the new request to the end of the queue
	if (atomic_load(&manager->request_queue.size) == 0) {
		manager->request_queue.front = new_request;
	} else {
		manager->request_queue.rear->next = new_request;
	}
	manager->request_queue.rear = new_request;
	atomic_fetch_add(&manager->request_queue.size, 1);
}

/*Dequeue a lock request from the queue */

LockRequestNode *dequeue_lock_request(LockManager *manager) {
	if (atomic_load(&manager->request_queue.size) == 0) {
		return NULL;
	}

	LockRequestNode *front_request = manager->request_queue.front;
	manager->request_queue.front = front_request->next;
	atomic_fetch_sub(&manager->request_queue.size, 1);

	return front_request;
}

__CJLF_GENERICS process_lock_queue(LockManager *manager) {
	if (atomic_load(&manager->request_queue.size) > 0) {
		LockRequestNode *request = dequeue_lock_request(manager);
		if (request != NULL) {
			request->callback(request->arg);
			free(request);
		}
	}
}
