/* This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */
#pragma once
#if !defined(_WIN32) && !defined(_MSC_VER)
#    include <pthread.h> /* pthread */
#endif
#if defined(_WIN32) || defined(_MSC_VER)
#   error("NO TEST") /// TODO for windows
#   include <windows.h> /* CRITICAL_SECTION */
#   define pthread_rwlock_t CRITICAL_SECTION
#endif
namespace uchat
{
/// Rwlock - a rwlock wrapper
struct Rwlock {
    Rwlock();
    virtual ~Rwlock();
    void rdlock();
    void wrlock();
    bool tryRdlock();
    bool tryWrlock();
    /*
#   if (!defined(_WIN32)) && (!defined(_MSC_VER))
    void unlock() {
        pthread_rwlock_unlock(&(this->rwlock));
    }
#   else
    void rdunlock() {
        ReleaseSRWLockShared(&(this->rwlock));
    }
    void wrunlock() {
        ReleaseSRWLockExclusive(&(this->rwlock));
        this->isWr = false;
    }
#   endif
    */
    void unlock();
    inline pthread_rwlock_t* getRwlock() noexcept;
    // do not try copy
    Rwlock(Rwlock const&) = delete;
    Rwlock& operator=(Rwlock const&) = delete;
protected:
    pthread_rwlock_t rwlock;
#   if defined(_WIN32) || defined(_MSC_VER)
    CRITICAL_SECTION cs;
    bool isWr;
    long long reader;
#   endif
};
inline pthread_rwlock_t* Rwlock::getRwlock() noexcept
{
    return &(this->rwlock);
}
}
#if defined(_WIN32) || defined(_MSC_VER)
#   undef pthread_rwlock_t
#endif
