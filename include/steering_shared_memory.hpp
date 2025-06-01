#pragma once

#include <cstdint>
#include <cstring>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

struct SteeringData {
    int64_t timestamp;
    float predicted;
    float actual;
    int32_t has_more_data;
};

class SteeringSharedMemory{
public:
    SteeringSharedMemory(key_t shm_key, size_t shm_size, key_t sem_key);
    ~SteeringSharedMemory();

    bool write(const SteeringData &data);
private:
    key_t m_shm_key;
    size_t m_shm_size;
    key_t m_sem_key;

    int m_semid;
    int m_shmid;
    bool m_is_initialized;
    void* m_shmaddr;
};
