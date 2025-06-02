#include "steering_shared_memory.hpp"
#include <cstring>
#include "logger.hpp"
#include "cluon-complete.hpp"

SteeringSharedMemory::SteeringSharedMemory(key_t shm_key, size_t shm_size, key_t sem_key)
    : m_shm_key(shm_key), m_shm_size(shm_size), m_sem_key(sem_key),
      m_semid(-1), m_shmid(-1), m_is_initialized(false), m_shmaddr(nullptr) {

    auto logger = Logger::get_instance().get_logger();
    
    m_semid = semget(m_sem_key, 1, IPC_CREAT | 0666);
    if (m_semid == -1) {
        throw std::runtime_error("Failed to create semaphore: " + std::string(strerror(errno)));
    }
    logger->info("Steering Shared Memory semaphore created");

    semun arg;
    arg.val = 1;
    if (semctl(m_semid, 0, SETVAL, arg) == -1) {
        throw std::runtime_error("Failed to initialize semaphore: " + std::string(strerror(errno)));
    }
    logger->info("Steering Shared Memory semaphore initialized");

    m_shmid = shmget(m_shm_key, m_shm_size, IPC_CREAT | 0666);
    if (m_shmid < 0) {
        throw std::runtime_error("Failed to get shared memory segment: " + std::string(strerror(errno)));
    }
    logger->info("Steering Shared Memory shared memory received");

    m_shmaddr = shmat(m_shmid, nullptr, 0);
    if (m_shmaddr == (void *)-1) {
        throw std::runtime_error("Failed to attach shared memory: " + std::string(strerror(errno)));
    }
    logger->info("Steering Shared Memory shared memory attached");

    m_is_initialized = true;
}

SteeringSharedMemory::~SteeringSharedMemory() { auto logger = Logger::get_instance().get_logger();

    // detach from shared memory
    if (m_shmaddr != nullptr && m_shmaddr != (void *)-1) {
        if (shmdt(m_shmaddr) == -1) {
            logger->error("Failed to detach shared memory: {}", std::string(strerror(errno)));
        } else {
            logger->info("Detached from shared memory");
        }
        m_shmaddr = nullptr;
    }

    // remove shared memory
    if (m_shmid != -1) {
        if (shmctl(m_shmid, IPC_RMID, nullptr) == -1) {
            if (errno != EINVAL) {
                logger->error("Failed to remove shared memory: {}", std::string(strerror(errno)));
            }
        } else {
            logger->info("Removed shared memory segment with key '{}'", std::to_string(m_shm_key));
        }
        m_shmid = -1;
    }

    // remove semaphore
    if (m_semid != -1) {
        if (semctl(m_semid, 0, IPC_RMID) == -1) {
            if (errno != EINVAL) {
                logger->error("Failed to remove semaphore: {}", std::string(strerror(errno)));
            }
        } else {
            logger->info("Removed semaphore with key '{}'", std::to_string(m_sem_key));
        }
        m_semid = -1;
    }
}

bool SteeringSharedMemory::write(const SteeringData &data) {
    auto logger = Logger::get_instance().get_logger();

    if (!m_is_initialized) {
        logger->warn("Shared memory is not initialized");
        return false;
    }

    struct sembuf lock_op = {0, -1, 0};
    struct sembuf unlock_op = {0, 1, 0};
    if (semop(m_semid, &lock_op, 1) == -1) {
        logger->error("Failed to lock semaphore: {}", strerror(errno));
        return false;
    }

    std::memcpy(m_shmaddr, &data, sizeof(SteeringData));

    if (semop(m_semid, &unlock_op, 1) == -1) {
        logger->error("Failed to unlock semaphore: {}", strerror(errno));
        return false;
    }

    return true;
}

