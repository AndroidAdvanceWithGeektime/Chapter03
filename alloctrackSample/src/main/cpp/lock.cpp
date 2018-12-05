#include "lock.h"

CMutex::CMutex() {
    pthread_mutex_init(&m_mutex, nullptr);
}

CMutex::~CMutex() {
    pthread_mutex_destroy(&m_mutex);
}

int CMutex::Lock() const {
    return pthread_mutex_lock(&m_mutex);
}

int CMutex::Trylock(void) {
    this->ret = pthread_mutex_trylock(&m_mutex);
    return this->ret;
}

int CMutex::Unlock() const {
    return pthread_mutex_unlock(&m_mutex);
}

CMyLock::CMyLock(const ILock &m) : m_lock(m) {
    m_lock.Lock();
}

int CMyLock::Unlock() const {
    return m_lock.Unlock();
}

CMyLock::~CMyLock() {
    m_lock.Unlock();
}

