#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>
#include <cstring>
#include "shared_memory.h"

#define NAME "/my_shared_mem"

int main() {
    const size_t size = sizeof(SharedMemory);

    int fd = shm_open(NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) { 
        std::cerr << "shm_open failed " << std::endl; 
        exit(1); 
    }

    if (ftruncate(fd, size) == -1) { 
        std::cerr << "ftruncate failed " << std::endl; 
        exit(1); 
    }

    void* addr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) { 
        std::cerr << "mmap failed" << std::endl; 
        exit(1); 
    }

    SharedMemory* shm = static_cast<SharedMemory*>(addr);

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&shm->mutex, &attr);
    pthread_mutexattr_destroy(&attr);

    shm->client_state = State::IDLE;
    shm->server_state = State::IDLE;

    std::cout << "Server is listening" << std::endl;

    while (true) {
        pthread_mutex_lock(&shm->mutex);
        if (shm->client_state == State::REQUEST_READY) {
            std::cout << "Server got " << shm->message << std::endl;

            std::strcpy(shm->message, "pong");
            shm->server_state = State::RESPONSE_READY;
        }
        pthread_mutex_unlock(&shm->mutex);

        pthread_mutex_lock(&shm->mutex);
        if (shm->client_state == State::DONE && shm->server_state == State::RESPONSE_READY) {
            shm->server_state = State::IDLE;
            shm->client_state = State::IDLE;
        }
        pthread_mutex_unlock(&shm->mutex);

        usleep(10000);
    }

    munmap(addr, size);
    close(fd);
    shm_unlink(NAME);
    return 0;
}
