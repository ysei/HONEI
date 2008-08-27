/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Markus Geveler <apryde@gmx.de>
 *
 * This file is part of the MATH C++ library. LibMath is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * LibMath is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MATH_GUARD_RESTRICT_HH
#define MATH_GUARD_RESTRICT_HH 1

#include<honei/la/dense_vector.hh>
#include<honei/math/methods.hh>
#include<honei/math/apply_dirichlet_boundaries.hh>
#include<cmath>

using namespace methods;
namespace honei
{
    template<typename Tag_>
        class Restriction
        {
            public:
                template <typename Prec_>
                    static DenseVector<Prec_> & value(DenseVector<Prec_>&  coarse, DenseVector<Prec_>& fine, DenseVector<unsigned long>& mask)
                    {
                        unsigned long n_fine(fine.size());
                        unsigned long n_x_fine((unsigned long)sqrt((Prec_)n_fine) - 1);
                        unsigned long n_x_coarse(n_x_fine / 2);
                        unsigned long n_y_fine(n_x_fine + 1);
                        unsigned long n_y_coarse(n_x_coarse + 1);
                        unsigned long i_step(2*n_y_fine);

                        // temps
                        int i_aux, i_aux_1;

                        // x___________x
                        // |     |     |
                        // |     |     |
                        // |_____|_____|
                        // |     |     |
                        // |     |     |
                        // x_____|_____x
                        for (unsigned long i(1) ; i <= n_y_coarse ; ++i)
                        {
                            for(unsigned long j(0), k(0) ; j < n_y_coarse ; ++j, k +=2)
                            {
                                coarse[1 + n_y_coarse * (i - 1) - 1 + j] = fine[1 + 2 * n_y_fine * (i - 1) + k -1];
                            }
                        }

                        for(unsigned i(1); i <= n_x_coarse; ++i)
                        {
                            i_aux = 1 + n_y_fine + 2 * n_y_fine * (i - 1);
                            i_aux_1 = 1 + n_y_coarse * i;

                            // x_______________________x
                            // | _         |         _ |
                            // ||\         |         /||
                            // | |         |         | |
                            // | |1/2      |     1/2 | |
                            // | |         |         | |
                            // | /         |         \ |
                            // o___________|___________o
                            // |           |           |
                            // | \         |         / |
                            // | |         |         | |
                            // | |1/2      |     1/2 | |
                            // | |         |         | |
                            // ||/_        |        _\||
                            // x___________|___________x

                            for(unsigned long j(0) ; j < n_y_coarse; ++j)
                            {
                                coarse[i_aux_1 - 1 + j] += 0.5 * fine[i_aux - 1 + j * 2];
                            }

                            i_aux_1 = i_aux_1 - n_y_coarse;

                            for(unsigned long j(0) ; j < n_y_coarse; ++j)
                            {
                                coarse[i_aux_1 - 1 + j] += 0.5 * fine[i_aux - 1 + j * 2];
                            }

                            // x___________o___________x
                            // | \_______/ | \_______/ |
                            // |    1/2    |    1/2    |
                            // |           |           |
                            // |           |           |
                            // |           |           |
                            // |           |           |
                            // |___________|___________|
                            // |           |           |
                            // |           |           |
                            // |           |           |
                            // |           |           |
                            // |  _______  |  _______  |
                            // | /  1/2  \ | /  1/2  \ |
                            // x___________o___________x

                            for(unsigned long j(0) ; j < n_y_coarse; ++j)
                            {
                                coarse[i - 1 + j * n_y_coarse] += 0.5 * fine[2 * i - 1 + j * i_step];
                            }

                            for(unsigned long j(0) ; j < n_y_coarse; ++j)
                            {
                                coarse[i + j * n_y_coarse] += 0.5 * fine[2 * i - 1 + j * i_step];
                            }

                            ++i_aux;

                            // _______________________
                            // |  _        |         _ |
                            // | |\        |         /||
                            // |   \ 1/4   |    1/4/   |
                            // |     \     |     /     |
                            // |       \   |   /       |
                            // |         \ | /         |
                            // |___________o___________|
                            // |         / | \         |
                            // |        /  |  \        |
                            // |      /1/4 |  1/4\     |
                            // |     /     |      \    |
                            // |   /       |        \  |
                            // | |/_       |        _\||
                            // x___________|___________x

                            for(unsigned long j(0) ; j < n_x_coarse; ++j)
                            {
                                coarse[i_aux_1 -1 + j] += 0.25 * fine[i_aux - 1 + j * 2];
                            }

                            for(unsigned long j(0) ; j < n_x_coarse; ++j)
                            {
                                coarse[i_aux_1 + j] += 0.25 * fine[i_aux - 1 + j * 2];
                            }

                            for(unsigned long j(0) ; j < n_x_coarse; ++j)
                            {
                                coarse[n_y_coarse * i + j] += 0.25 * fine[i_aux - 1 + j * 2];
                            }

                            for(unsigned long j(0) ; j < n_x_coarse; ++j)
                            {
                                coarse[(n_y_coarse + 1) * i + j] += 0.25 * fine[i_aux - 1 + j * 2];
                            }
                        }
                        ApplyDirichletBoundaries<Tag_>::value(fine, mask);
                    }
        };

    template <> struct Restriction<tags::GPU::CUDA>
    {
        static DenseVector<float> & value(DenseVector<float> & coarse,
                const DenseVector<float> & fine, const DenseVector<unsigned long> & mask);
    };
}
#endif
