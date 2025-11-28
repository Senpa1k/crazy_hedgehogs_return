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

    int fd = shm_open(NAME, O_RDWR, 0666);
    if (fd == -1) { 
        std::cerr << "shm_open failed " << std::endl; 
        exit(1); 
    }

    void* addr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) { 
        std::cerr << "mmap failed" << std::endl; 
        exit(1); 
    }

    SharedMemory* shm = static_cast<SharedMemory*>(addr);

    for (int i = 0; i < 5; ++i) {
        pthread_mutex_lock(&shm->mutex);
        if (shm->client_state == State::IDLE) {
            std::strcpy(shm->message, "ping");
            shm->client_state = State::REQUEST_READY;
        }
        pthread_mutex_unlock(&shm->mutex);

        bool got_response = false;
        while (!got_response) {
            pthread_mutex_lock(&shm->mutex);
            if (shm->server_state == State::RESPONSE_READY) {
                std::cout << "Client got " << shm->message << std::endl;
                shm->client_state = State::DONE;
                got_response = true;
            }
            pthread_mutex_unlock(&shm->mutex);
            usleep(10000);
        }

        usleep(500000);
    }

    munmap(addr, size);
    close(fd);

    return 0;
}
