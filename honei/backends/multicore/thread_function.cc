/* vim: set sw=4 sts=4 et foldmethod=syntax : */
/*
 * Copyright (c) 2009, 2010 Sven Mallach <mallach@honei.org>
 *
 * This file is part of the HONEI C++ library. HONEI is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * HONEI is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <honei/backends/multicore/thread_function.hh>
#include <honei/util/attributes.hh>
#include <honei/util/exception.hh>
#include <honei/util/lock.hh>
#include <honei/util/private_implementation_pattern-impl.hh>
#include <honei/util/stringify.hh>

#include <errno.h>
#include <utility>
#include <sys/syscall.h>

namespace honei
{
    template <> struct Implementation<mc::ThreadFunction>
    {
        /// Our internal ID
        const unsigned pool_id;

        /// Our Thread ID (given by the operating system)
        unsigned thread_id;

        /* The logical processor this thread is bound to, in fact a
         * scheduler id which is only used with affinity enabled. */
        const unsigned sched_lpu;

        /// The thread pool's mutex (for grabbing work).
        Mutex * const pool_mutex;

        /// The task list administrated by the thread pool.
        std::list<mc::ThreadTask *> * const tasklist;

        /// ConditionVariable for work-status.
        ConditionVariable * const global_barrier;

        /// Flag if the Thread shall stop.
        bool terminate;

        /// The ThreadTask to be currently executed by the thread.
        mc::ThreadTask * task;

        /// A comparison object for mc::ThreadTask objects.
        mc::TaskComp * const comp;

        /// Helper function to pick work out of the pool's task list
        inline void pick_work() HONEI_INLINE
        {
            Lock l(*pool_mutex);

            task = 0;

            for (std::list<mc::ThreadTask *>::iterator i(tasklist->begin()) , i_end(tasklist->end()) ; i != i_end ; ++i)
            {
                if ((*comp)(*i))
                {
                    task = *i;
                    unsigned & sched_id = task->ticket->sid();
                    sched_id = sched_lpu;
                    tasklist->remove(*i);
                    break;
                }
            }
        }

        Implementation(Mutex * const mutex, ConditionVariable * const barrier,
                std::list<mc::ThreadTask *> * const list, unsigned pid, unsigned sched) :
            pool_id(pid),
            thread_id(0),
            sched_lpu(sched),
            pool_mutex(mutex),
            tasklist(list),
            global_barrier(barrier),
            terminate(false),
            task(0),
            comp(new mc::TaskComp(sched))
            {
            }

            void stop()
            {
                Lock l(*pool_mutex);
                terminate = true;
                global_barrier->broadcast();
            }

            ~Implementation()
            {
            }
    };
}

using namespace honei::mc;

ThreadFunction::ThreadFunction(Mutex * const mutex, ConditionVariable * const barrier, std::list<ThreadTask *> * const list, unsigned pool_id, unsigned sched_id) :
    PrivateImplementationPattern<ThreadFunction, Shared>(new Implementation<ThreadFunction>(mutex, barrier, list, pool_id, sched_id))
{
}

ThreadFunction::~ThreadFunction()
{
}

void ThreadFunction::stop()
{
    _imp->stop();
}

void ThreadFunction::operator() ()
{
    _imp->thread_id = syscall(__NR_gettid);

    do
    {
        // Check work-status and condition again under same mutex protection to avoid race conditions
        // between pick / if and wait
        {
            Lock l(*_imp->pool_mutex);
            _imp->pick_work();
            if (_imp->task == 0 && ! _imp->terminate)
            {
                _imp->global_barrier->wait(*_imp->pool_mutex);
                _imp->pick_work();
            }
        }

        while (_imp->task != 0)
        {
            (*_imp->task->functor)();
            _imp->task->ticket->mark();
            delete _imp->task;
            _imp->pick_work();
        }
    }
    while (! _imp->terminate);

    /* Make sure that this thread does not terminate and
     * there are still tasks dedicated to it in the task list */

    _imp->pick_work();

    while (_imp->task != 0)
    {
        (*_imp->task->functor)();
        _imp->task->ticket->mark();
        delete _imp->task;
        _imp->task = 0;
        _imp->pick_work();
    }
}

unsigned ThreadFunction::pool_id() const
{
    return _imp->pool_id;
}

unsigned ThreadFunction::tid() const
{
    return _imp->thread_id;
}
