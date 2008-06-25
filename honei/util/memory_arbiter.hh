/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Dirk Ribbrock <dirk.ribbrock@uni-dortmund.de>
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

#ifndef MEMORY_GUARD_MEMORY_CONTROLLER_HH
#define MEMORY_GUARD_MEMORY_CONTROLLER_HH 1

#include <honei/util/instantiation_policy.hh>
#include <honei/util/private_implementation_pattern.hh>
#include <honei/util/tags.hh>
#include <honei/util/exception.hh>
#include <honei/util/assertion.hh>
#include <honei/util/lock.hh>
#include <honei/util/log.hh>
#include <honei/util/stringify.hh>
#include <honei/util/mutex.hh>
#include <honei/util/condition_variable.hh>
#include <honei/util/memory_backend_base.hh>

#include <set>
#include <map>

namespace honei
{
    /// Logical representation of a used chunk of memory.
    struct MemoryBlock
    {
        public:
            MemoryBlock(tags::TagValue w) :
                writer(w)
        {
            read_count = 0;
            write_count = 0;
        }

            unsigned read_count;
            unsigned write_count;
            tags::TagValue writer;
            std::set<tags::TagValue> readers;
    };

    /**
     * MemoryArbiter handles read/write locks for all used memory blocks and
     * distributes necessary memory transfers jobs.
     *
     * \ingroup grpmemorymanager
     */
    class MemoryArbiter :
        public PrivateImplementationPattern<MemoryArbiter, Shared> ,
        public InstantiationPolicy<MemoryArbiter, Singleton>
    {
        public:
            friend class InstantiationPolicy<MemoryArbiter, Singleton>;
            friend class MemoryBackendRegistrator;

            /**
             * Constructor.
             */
            MemoryArbiter();

            /**
             * Destructor.
             */
            ~MemoryArbiter();

            /**
             * Add a chunk of memory to the memory block map.
             *
             * \param memid A unique key identifying the added chunk.
             */
            void add_memblock(unsigned long memid);

            /**
             * Remove a chunk of memory from the memory block map.
             *
             * \param memid A unique key identifying the removed chunk.
             */
            void remove_memblock(unsigned long memid);

            /**
             * Request a read lock for a specific memory block.
             *
             * \param memid A unique key identifying the requested chunk.
             * \param address The address where our reading will begin.
             * \param bytes The amount of bytes we want to read.
             */
            void * read(tags::TagValue memory, unsigned long memid, void * address, unsigned long bytes);

            /**
             * Request a write lock for a specific memory block.
             *
             * \param memid A unique key identifying the requested chunk.
             * \param address The address where our writing will begin.
             * \param bytes The amount of bytes we want to write.
             */
            void * write(tags::TagValue memory, unsigned long memid, void * address, unsigned long bytes);

            /**
             * Release a read lock for a specific memory block.
             *
             * \param memid A unique key identifying the released chunk.
             */
            void release_read(unsigned long memid);

            /**
             * Release a write lock for a specific memory block.
             *
             * \param memid A unique key identifying the released chunk.
             */
            void release_write(unsigned long memid);

        private:
            void insert_backend(std::pair<tags::TagValue, MemoryBackendBase *> backend);

    };

    /**
     * MemoryBackendRegistrator registers a descendant of MemoryBackendBase with MemoryArbiter.
     *
     * \ingroup grpmemorymanager
     */
    struct MemoryBackendRegistrator
    {
        /**
         * Constructor.
         *
         * \param v Memory tag value that the backend is associated with.
         * \param f Singleton-instance pointer to the backend.
         */
        MemoryBackendRegistrator(const tags::TagValue v, MemoryBackendBase * m)
        {
            MemoryArbiter::instance()->insert_backend(std::make_pair(v, m));
        }
    };
}
#endif
