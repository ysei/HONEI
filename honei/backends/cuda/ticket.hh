/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Dirk Ribbrock <dirk.ribbrock@tu-dortmund.de>
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

#pragma once
#ifndef CUDA_GUARD_TICKET_HH
#define CUDA_GUARD_TICKET_HH 1

#include <honei/util/ticket.hh>

namespace honei
{
        template <> class Ticket<tags::GPU::MultiCore> :
            public TicketBase,
            public PrivateImplementationPattern<Ticket<tags::GPU::MultiCore>, Shared>
        {
            private:
                /// \name Private members
                /// \{
                /// \}

            public:
                /// \name Basic Operations
                /// \{

                /// Constructor
                Ticket();

                /// \}

                /// Mark ticket as completed.
                virtual void mark();

                /// Wait for ticket completion.
                virtual void wait() const;

                /// Retrieve unique ticket ID
                unsigned uid() const;
       };
}
#endif
