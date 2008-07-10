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

#ifndef UTIL_GUARD_MEMORY_BACKEND_BASE_HH
#define UTIL_GUARD_MEMORY_BACKEND_BASE_HH 1


namespace honei
{
    /**
     * MemoryBackendBase is the abstract base class for all memory backends.
     *
     * \ingroup grpmemorymanager
     */
    class MemoryBackendBase
    {
        public:
            virtual void * upload(unsigned long memid, void * address, unsigned long bytes) = 0;

            virtual void download(unsigned long memid, void * address, unsigned long bytes) = 0;

            virtual void * alloc(unsigned long memid, void * address, unsigned long bytes) = 0;

            virtual void free(unsigned long memid) = 0;
    };
}
#endif