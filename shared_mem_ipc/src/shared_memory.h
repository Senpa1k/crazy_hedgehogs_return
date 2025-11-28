#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <pthread.h>

enum class State {
    IDLE,
    REQUEST_READY,
    RESPONSE_READY,
    DONE
};

struct SharedMemory {
    char message[256];
    State client_state;
    State server_state;
    pthread_mutex_t mutex;
};

#endif
