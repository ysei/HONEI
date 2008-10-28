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

#ifndef MPI_GUARD_OPERATIONS_HH
#define MPI_GUARD_OPERATIONS_HH 1

namespace honei
{
    namespace mpi
    {
        void mpi_init(int * argc, char*** argv);
        void mpi_finalize();
        void mpi_comm_size(int * size);
        void mpi_comm_rank(int * id);
        template <typename DT_> void mpi_bcast(DT_ * data, unsigned long size, int sender);
        template <typename DT_> void mpi_send(DT_ * data, unsigned long size, int target, int tag);
        template <typename DT_> void mpi_recv(DT_ * data, unsigned long size, int sender, int tag);
    }
}

#endif
