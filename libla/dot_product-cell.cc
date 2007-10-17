/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Danny van Dyk <danny.dyk@uni-dortmund.de>
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

#include <libla/dot_product.hh>
#include <libutil/cell.hh>
#include <libutil/memory_backend_cell.hh>
#include <libutil/spe_instruction.hh>
#include <libutil/spe_manager.hh>

namespace honei
{
    float
    DotProduct<tags::Cell>::value(const DenseVector<float> & a, const DenseVector<float> & b)
    {
        CONTEXT("When calculating DenseVector<float>-DenseVector<float> dot product (Cell):");

        if (b.size() != a.size())
            throw VectorSizeDoesNotMatch(b.size(), a.size());

        float result;

        Operand oa = { a.elements() };
        Operand ob = { b.elements() };
        Operand oc = { &result };
        SPEInstruction instruction(oc_dense_dense_float_dot_product, a.size(), oa, ob, oc);

        SPEManager::instance()->dispatch(instruction);

        instruction.wait();

        return result;
    }
}
