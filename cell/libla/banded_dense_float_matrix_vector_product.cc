/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Dirk Ribbrock <dirk.ribbrock@uni-dortmund.de>
 *
 * This file is part of the LA C++ library. LibLa is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * LibLa is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <cell/cell.hh>
#include <cell/libutil/allocator.hh>
#include <cell/libutil/transfer.hh>

#include <spu_intrinsics.h>
#include <spu_mfcio.h>
#include <stdio.h>

using namespace honei;

void banded_dense_float_matrix_vector_product(const Instruction & inst)
{
    allocator::Allocation * block_a(allocator::acquire_block());
    allocator::Allocation * block_b(allocator::acquire_block());
    allocator::Allocation * block_r(allocator::acquire_block());

    Pointer<float> a = { block_a->address };
    Pointer<float> b = { block_b->address };
    Pointer<float> r = { block_r->address };

    unsigned start(inst.d.u);
    unsigned end(inst.e.u);
    signed op_offset(inst.f.s);
    unsigned x_offset(inst.g.u);
    unsigned y_offset((4 - x_offset) % 4);

    mfc_get(a.untyped, inst.a.ea, multiple_of_sixteen((end - start) * sizeof(float)), 1, 0, 0);
    mfc_get(b.untyped, inst.b.ea, multiple_of_sixteen(inst.size * sizeof(float)), 2, 0, 0);
    mfc_get(r.untyped, inst.c.ea, multiple_of_sixteen((end - start) * sizeof(float)), 3, 0, 0);
    mfc_write_tag_mask(1 << 3 | 1 << 2 | 1 << 1);
    mfc_read_tag_status_all();

    for (unsigned i(0) ; i < end - start ; i+=4)
    {
        vector float temp = b.vectorised[(i + start + op_offset - x_offset)/4]; // temp version needed?
        extract(temp, b.vectorised[(i + start + op_offset + y_offset)/4], x_offset);
        r.vectorised[i/4] = spu_madd(a.vectorised[i/4], temp, r.vectorised[i/4]);
    }

    mfc_put(r.untyped, inst.c.ea, multiple_of_sixteen((end - start) * sizeof(float)), 3, 0, 0);
    mfc_write_tag_mask(1 << 3);
    mfc_read_tag_status_all();

    allocator::release_block(*block_a);
    allocator::release_block(*block_b);
    allocator::release_block(*block_r);
}
