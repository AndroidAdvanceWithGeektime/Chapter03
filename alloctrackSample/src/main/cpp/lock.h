#ifndef _Lock_H
#define _Lock_H

#include <pthread.h>

class ILock {
public:
    virtual ~ILock() {}

    virtual int Lock() const = 0;

    virtual int Unlock() const = 0;
};

class CMutex : public ILock {
public:
    CMutex();

    ~CMutex();

    virtual int Lock() const;

    virtual int Unlock() const;

    int Trylock(void);

private:
    mutable pthread_mutex_t m_mutex;
public:
    int ret;
};

class CMyLock {
public:
    CMyLock(const ILock &);

    int Unlock() const;

    ~CMyLock();

private:
    const ILock &m_lock;
};


#endif


