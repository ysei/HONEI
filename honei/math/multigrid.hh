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

#ifndef LIBMATH_GUARD_MULTIGRID_HH
#define LIBMATH_GUARD_MULTIGRID_HH 1

#include<honei/la/dense_vector.hh>
#include<honei/la/banded_matrix_q1.hh>
#include<honei/math/methods.hh>
#include<honei/math/conjugate_gradients.hh>
#include<honei/math/jacobi.hh>
#include<honei/la/norm.hh>
#include<honei/la/sum.hh>
#include<honei/la/difference.hh>
#include<honei/la/scaled_sum.hh>
#include<honei/la/scale.hh>
#include<honei/la/product.hh>
#include<honei/math/restriction.hh>
#include<honei/math/prolongation.hh>
#include<vector>
#include<string>
#include<fstream>
using namespace methods;
namespace honei
{

    template <typename Prec_>
    struct MGInfo
    {
        public:
            //configuration constants:
            bool is_smoother;
            DenseVector<unsigned long>* macro_border_mask;
            unsigned long min_level;
            unsigned long max_level;

            unsigned long  n_max_iter;
            bool initial_zero;
            Prec_ tolerance;
            bool convergence_check;

            unsigned long  n_pre_smooth;
            unsigned long  n_post_smooth;
            unsigned long  n_max_iter_coarse;
            Prec_ tolerance_coarse;
            Prec_ adapt_correction_factor;
            unsigned long n_truncate_d_o_f;
            unsigned long n_threshold_d_o_f;

            std::vector<DenseVector<Prec_> > c;
            std::vector<DenseVector<Prec_> > d;
            std::vector<DenseVector<Prec_> > rhs;
            std::vector<DenseVector<Prec_> > x;
            std::vector<DenseVector<Prec_> > temp;

            std::vector<BandedMatrixQ1<Prec_> > a;

    };

    template<typename Tag_, typename SmootherType_, typename CycleType_, typename Mode_>
        struct Multigrid
        {
        };

    template<typename Tag_>
        class Multigrid<Tag_, JAC, CYCLE::V, FIXED>
        {
            private:
                    template <typename Prec_>
                    static DenseVector<Prec_> _multigrid_kernel(BandedMatrixQ1<Prec_>&  system, DenseVector<Prec_>& right_hand_side, unsigned long max_levels, Prec_ * cappa, MGInfo<Prec_> info)
                    {
                        // compute initial defect
                        // when start vector is zero: set d = rhs (we can save one matvec here)
                        // when start vector is not zero: d = rhs - A*x
                        if(info.initial_zero)
                        {
                            info.d[info.max_level] = info.rhs[info.max_level].copy();
                        }
                        else
                        {
                            DenseVector<Prec_> rhs_c(info.rhs[info.max_level].copy());
                            info.d[info.max_level] = (Difference<Tag_>::value(rhs_c, Product<Tag_>::value(info.a[info.max_level], info.x[info.max_level].copy())));
                        }

                        Prec_ defect, initial_defect;
                        // compute norm of initial defect, also used for relative convergence control
                        if(!info.is_smoother)
                        {
                            defect = Norm<vnt_l_two, false, Tag_>::value(info.d[info.max_level]);
                            initial_defect = defect;
                        }
                        else
                        {
                            defect = Prec_(1e8);
                        }

                        // check if nothing needs to be done
                        if(info.convergence_check && defect <= info.tolerance)
                        {
                            //do nothing
                        }
                        else
                        {
                            //start cycles
                            unsigned long iter, current_level = info.max_level;
                            unsigned long local_cycle[max_levels];
                            bool restriction_started;
                            for(iter = 1 ; iter <= info.n_max_iter ; ++iter)
                            {
                                // set current level to the maximal level
                                current_level = info.max_level;

                                // Initialise the array providing the local cycle information
                                //V cycle!
                                for(unsigned long i(info.min_level + 1); i < info.max_level; ++i)
                                {
                                    local_cycle[i] = 1; //Using 1 to code local V cycle
                                }
                                //Restriction loop:
                                while(true)
                                {
                                    // set a flag that the restriction has just started
                                    restriction_started = true;
                                    while(true)
                                    {
                                        // for the case that we have actually only one level, do no presmoothing
                                        // and jump to the coarse grid solver directly.
                                        if (info.min_level == info.max_level)
                                            goto endRestrictionLoop;

                                        // -------------
                                        // presmoothing
                                        // -------------

                                        if (restriction_started)
                                        {
                                            // When the restriction loop just started
                                            (info.d[current_level]) = (Jacobi<Tag_>::value(info.a[current_level], info.c[current_level], info.n_pre_smooth));
                                            Sum<Tag_>::value(info.x[current_level], info.c[current_level]);
                                        }
                                        else
                                        {
                                            // otherwise the solution process can be started directly with
                                            // the cleared solution vector (as the solution vector itself represents
                                            // the defect correction here)
                                            info.x[current_level] = (Jacobi<Tag_>::value((info.a[current_level]), (info.d[current_level]), info.n_pre_smooth));
                                        }
                                        DenseVector<Prec_> rhs_c_2(info.rhs[current_level]->copy());
                                        info.d[current_level] = (Difference<Tag_>::value(rhs_c_2, Product<Tag_>::value(info.a[current_level], info.x[current_level].copy())));
                                        //----------------------------------
                                        //restriction ("go down one level")
                                        //----------------------------------

                                        --current_level;

                                        // restrict defect from level icurrentLevel+1 to icurrentLevel,
                                        // set homogeneous Dirichlet boundary conditions in the restricted defect vector
                                        // depending on Dirichlet mask (see routine for details), and store a copy in RHS

                                        Restriction<Tag_>::value((info.d[current_level]), (info.d[current_level + 1]), *info.macro_border_mask);
                                        info.rhs[current_level] =(info.d[current_level].copy());

                                        // if we reached the coarsest level exit the restricition loop
                                        if (current_level == info.min_level)
                                            goto endRestrictionLoop;

                                        restriction_started = false;
                                    }
endRestrictionLoop:
                                    // ----------------------
                                    // coarse grid correction
                                    // ----------------------
                                    if (info.min_level == info.max_level)
                                    {
                                        // For the case we actually have only one MG level, only
                                        // the following coarse grid correction (and no smoothing) is done.

                                        (info.d[current_level]) =(ConjugateGradients<Tag_, NONE>::value((info.a[current_level]), (info.x[current_level]), info.tolerance));

                                        DenseVector<Prec_> rhs_c_4(info.rhs[current_level].copy());
                                        (info.d[current_level]) = (Difference<Tag_>::value(rhs_c_4, Product<Tag_>::value((info.a[current_level]), info.x[current_level].copy())));
                                        //
                                        // the MG cycle can be exited immediately
                                        //
                                        goto endCycleLoop;
                                    }
                                    else
                                    {
                                        // Otherwise this is a "real" coarse grid correction, which is
                                        // started with a zero start vector
                                        //manager->log(OL_TRACE2, "MG(GPU): coarse grid correction for neqs", data->neqs[icurrentLevel]);

                                        (info.d[current_level]) =(ConjugateGradients<Tag_, NONE>::value((info.a[current_level]), (info.x[current_level]), info.tolerance));
                                    }

                                    //-------------
                                    //prolongation
                                    //-------------
                                    while (true)
                                    {

                                        //------------------------------
                                        //prolongation ("go up one level")
                                        //------------------------------

                                        ++current_level;

                                        //
                                        // prolongate solution vector from level icurrentLevel-1 to icurrentLevel,
                                        // set homogeneous Dirichlet boundary conditions in the prolongated correction vector
                                        // depending on Dirichlet mask passed in from FEAST (see code for details)
                                        //
                                        Prolongation<Tag_>::value((info.c[current_level]), (info.x[current_level]), *info.macro_border_mask);

                                        //
                                        // perform adaptive coarse grid correction if required
                                        //
                                        Prec_ alpha;
                                        if (info.adapt_correction_factor == 0.0)
                                        {
                                            //Compute dalpha = (d,c)/(Ac,c) where d is the current residual
                                            //(data->d[icurrentLevel]) and c the prolongated correction (c[icurrentLevel])
                                            Prec_ d1,d2;
                                            d1 = DotProduct<Tag_>::value((info.d[current_level]), (info.c[current_level]));
                                            //delete info.temp[current_level];
                                            //info.temp[current_level] = 0;
                                            (info.temp[current_level]) = (Product<Tag_>::value((info.a[current_level]), (info.c[current_level])));
                                            d2 = DotProduct<Tag_>::value((info.temp[current_level]), (info.c[current_level]));

                                            alpha = Prec_(d1 / d2);
                                        }
                                        else
                                        {
                                            alpha = info.adapt_correction_factor;
                                        }

                                        //
                                        // add the prolongated correction to the iteration vector
                                        // on the current level
                                        //
                                        ScaledSum<Tag_>::value((info.x[current_level]), (info.c[current_level]), alpha);

                                        //--------------
                                        //postsmoothing
                                        //--------------
                                        //
                                        // smooth A*x = rhs based on the RHS for that level we stored during restriction
                                        //

                                        (info.x[current_level]) =(Jacobi<Tag_>::value((info.a[current_level]), (info.rhs[current_level]), info.n_post_smooth));

                                        //
                                        // update defect
                                        //

                                        //delete info.d[current_level];
                                        //info.d[current_level] = 0;
                                        DenseVector<Prec_> rhs_c_5(info.rhs[current_level].copy());
                                        (info.d[current_level]) = (Difference<Tag_>::value(rhs_c_5, Product<Tag_>::value((info.a[current_level]), info.x[current_level].copy())));

                                        // if the maximal level is reached then the MG cycle is finished,
                                        // so exit the MG cycle loop
                                        if (current_level == info.max_level)
                                            goto endCycleLoop;
                                    }
endProlongationLoop:;
                                }

endCycleLoop:
                                if(info.min_level == info.max_level)
                                    break;
                                if(!info.is_smoother)
                                {
                                    defect = Norm<vnt_l_two, false, Tag_>::value((info.d[info.max_level]));

                                    *cappa = pow(defect / initial_defect, 1.0/((Prec_)iter));

                                    if (defect <= info.tolerance)
                                        break;


                                }
                                else
                                {
                                    *cappa = Prec_(-1);
                                }
                            }
                        }
                        return (info.x[info.max_level]);

                    }

            public:
                template<typename Prec_>
                    static DenseVector<Prec_> value(BandedMatrixQ1<Prec_>&  system, DenseVector<Prec_>& right_hand_side, unsigned long max_levels, Prec_ conv_rad)
                    {
                        Prec_ cappa;

                        DenseVector<Prec_> result(right_hand_side.size(), Prec_(0)); //final result
                        MGInfo<Prec_> info;
                        info.macro_border_mask = new DenseVector<unsigned long>(8);
                        for(unsigned long i(0); i < 8; ++i)
                        {
                            (*info.macro_border_mask)[i] = 2;
                        }

                        // current and initial defect
                        Prec_ defect, initial_defect;

                        // cycle control
                        unsigned long restriction_started, iter;
                        unsigned long local_cycle[max_levels]; //former MAXLEVELS
                        //configuration constants: /TODO: set/allocate!!!
                        info.is_smoother = true;
                        DenseVector<unsigned long> mask(8);
                        info.macro_border_mask = &mask;
                        for(unsigned long i(0); i < 8 ; ++i)
                        {
                            (*info.macro_border_mask)[i] = (unsigned long)(2);
                        }

                        info.min_level = 3;
                        switch(right_hand_side.size())
                        {
                            case 1050625:
                                {
                                    info.max_level = 10;
                                }
                                break;
                            case 263169:
                                {
                                    info.max_level = 9;
                                }
                                break;
                            case 663169:
                                {
                                    info.max_level = 8;
                                }
                                break;
                            case 16641:
                                {
                                    info.max_level = 7;
                                }
                                break;
                            case 4225:
                                {
                                    info.max_level = 6;
                                }
                                break;
                            case 1089:
                                {
                                    info.max_level = 5;
                                }
                                break;
                            case 289:
                                {
                                    info.max_level = 4;
                                }
                                break;
                            case 81:
                                {
                                    info.max_level = 3;
                                }
                                break;
                            case 27:
                                {
                                    info.max_level = 2;
                                }
                                break;
                            case 9:
                                {
                                    info.max_level = 1;
                                }
                                break;
                        }

                        info.n_max_iter = 2;
                        info.initial_zero = true;
                        info.tolerance = 1e-2;
                        info.convergence_check = true;

                        info.n_pre_smooth = 2;
                        info.n_post_smooth = 2;
                        info.n_max_iter_coarse = ((unsigned long)sqrt((double)81.));
                        info.tolerance_coarse = 1e-2;
                        info.adapt_correction_factor = 1.;
                        info.n_truncate_d_o_f = 1;
                        info.n_threshold_d_o_f = 1;

                        //push back dummy matrices/vectors in order not to disturb std::vectors index range:
                        for (unsigned long i(0) ; i < info.min_level; ++i)
                        {
                            unsigned long size((unsigned long)(((unsigned long)pow(2, i) + 1) * ((unsigned long)pow(2, i) + 1)));
                            if(i == 0)
                                size = 9;

                            DenseVector<Prec_> dummy_band(size, Prec_(0));
                            BandedMatrixQ1<Prec_> ac_a(size, dummy_band.copy(), dummy_band.copy(), dummy_band.copy(), dummy_band.copy(), dummy_band.copy(), dummy_band.copy(), dummy_band.copy(), dummy_band.copy(), dummy_band.copy());
                            info.a.push_back(ac_a);
                            // iteration vectors
                            DenseVector<Prec_> ac_c(size, Prec_(0));
                            info.c.push_back(ac_c);
                            DenseVector<Prec_> ac_d(size, Prec_(0));
                            info.d.push_back(ac_d);
                            DenseVector<Prec_> ac_rhs(size, Prec_(0));
                            info.rhs.push_back(ac_rhs);
                            DenseVector<Prec_> ac_x(size, Prec_(0));
                            info.x.push_back(ac_x);
                            DenseVector<Prec_> ac_temp(size, Prec_(0));
                            info.temp.push_back(ac_temp);
                        }
                        for (unsigned long i(info.min_level) ; i <= info.max_level; ++i)
                        {
                            unsigned long size = (unsigned long)(((unsigned long)pow(2, i) + 1) * ((unsigned long)pow(2, i) + 1));
                            // iteration vectors
                            DenseVector<Prec_> ac_c(size, Prec_(0));
                            info.c.push_back(ac_c);
                            DenseVector<Prec_> ac_d(size, Prec_(0));
                            info.d.push_back(ac_d);
                            DenseVector<Prec_> ac_x(size, Prec_(0));
                            info.x.push_back(ac_x);
                            DenseVector<Prec_> ac_temp(size, Prec_(0));
                            info.temp.push_back(ac_temp);
                        }

                        //assemble all needed levels' matrices:
                        for(unsigned long i(info.min_level); i <= info.max_level; ++i)
                        {
                            unsigned long N = (unsigned long)(((unsigned long)pow(2, i) + 1) * ((unsigned long)pow(2, i) + 1));
                            DenseVector<Prec_> LL_v(N);
                            DenseVector<Prec_> LD_v(N);
                            DenseVector<Prec_> LU_v(N);
                            DenseVector<Prec_> DL_v(N);
                            DenseVector<Prec_> DD_v(N);
                            DenseVector<Prec_> DU_v(N);
                            DenseVector<Prec_> UL_v(N);
                            DenseVector<Prec_> UD_v(N);
                            DenseVector<Prec_> UU_v(N);
                            BandedMatrixQ1<Prec_> current_matrix(N,LL_v, LD_v, LU_v, DL_v, DD_v, DU_v, UL_v, UD_v, UU_v);

                            DenseVector<Prec_> current_rhs(N);
                            int n;

                            FILE* file;

                            double* dd;

                            double* ll;
                            double* ld;
                            double* lu;
                            double* dl;
                            double* du;
                            double* ul;
                            double* ud;
                            double* uu;
                            double* b;
                            std::string file_path("testdata/" + stringify(DD_v.size()) +"/ehq.1.1.1.1.bin");
                            file = fopen(file_path.c_str(), "rb");
                            fread(&n, sizeof(int), 1, file);

#ifdef HONEI_CELL
                            unsigned char b1, b2, b3, b4;
                            b1 = n & 255;
                            b2 = ( n >> 8 ) & 255;
                            b3 = ( n>>16 ) & 255;
                            b4 = ( n>>24 ) & 255;
                            n = ((int)b1 << 24) + ((int)b2 << 16) + ((int)b3 << 8) + b4;
#endif
                            dd = new double[n];
                            ll = new double[n];
                            ld = new double[n];
                            lu = new double[n];
                            dl = new double[n];
                            du = new double[n];
                            ul = new double[n];
                            ud = new double[n];
                            uu = new double[n];

                            b = new double[n];
                            fread(dd, sizeof(double), n, file);
                            fread(ll, sizeof(double), n, file);
                            fread(ld, sizeof(double), n, file);
                            fread(lu, sizeof(double), n, file);
                            fread(dl, sizeof(double), n, file);
                            fread(du, sizeof(double), n, file);
                            fread(ul, sizeof(double), n, file);
                            fread(ud, sizeof(double), n, file);
                            fread(uu, sizeof(double), n, file);
                            fread(b,  sizeof(double), n, file);
                            fclose(file);

#ifdef HONEI_CELL
                            for(unsigned long j(0); j < n; ++j)
                            {
                                dd[j] = DoubleSwap(dd[j]);
                                ll[j] = DoubleSwap(ll[j]);
                                ld[j] = DoubleSwap(ld[j]);
                                lu[j] = DoubleSwap(lu[j]);
                                dl[j] = DoubleSwap(dl[j]);
                                du[j] = DoubleSwap(du[j]);
                                ul[j] = DoubleSwap(ul[j]);
                                ud[j] = DoubleSwap(ud[j]);
                                uu[j] = DoubleSwap(uu[j]);

                                b[j] = DoubleSwap(b[j]);
                            }
#endif
                            for(unsigned long j(0); j < DD_v.size(); ++j)
                            {
                                LL_v[j] = (Prec_)ll[j];
                                LD_v[j] = (Prec_)ld[j];
                                LU_v[j] = (Prec_)lu[j];
                                DL_v[j] = (Prec_)dl[j];
                                DD_v[j] = (Prec_)dd[j];
                                DU_v[j] = (Prec_)du[j];
                                UL_v[j] = (Prec_)ul[j];
                                UD_v[j] = (Prec_)ud[j];
                                UU_v[j] = (Prec_)uu[j];
                                current_rhs[j] = (Prec_)b[j];
                            }
                            info.rhs.push_back(current_rhs);
                            info.a.push_back(current_matrix);

                        }
                        DenseVector<Prec_> initial_guess(right_hand_side.size(), Prec_(0)); //x_0
                        DenseVector<Prec_> outer_defect(right_hand_side.size(), Prec_(0));
                        DenseVector<Prec_> temp_vector(right_hand_side.size(), Prec_(0)); //delete if unneeded


                        // apply Dirichlet BCs for boundary nodes (semi-implicit approach)
                        // note that we cleared the solution vector previously
                        unsigned long sqrt_N((unsigned long)sqrt((double)right_hand_side.size()));
                        for (unsigned long i(0) ; i < sqrt_N; ++i)
                        {
                            initial_guess[i] = right_hand_side[i];
                        }
                        for (unsigned long i(right_hand_side.size() - sqrt_N); i < right_hand_side.size(); ++i)
                        {
                            initial_guess[i] = right_hand_side[i];
                        }
                        for (unsigned long i(0); i < right_hand_side.size(); i += sqrt_N)
                        {
                            initial_guess[i] = right_hand_side[i];
                        }

                        for (unsigned long i(sqrt_N - 1); i < right_hand_side.size(); i += sqrt_N)
                        {
                            initial_guess[i] = right_hand_side[i];
                        }

                        unsigned long timing_loop(1);
                        for(unsigned long timing(0); timing < timing_loop; ++timing)
                        {
                            unsigned long inner_iterations(0);
                            unsigned long outer_iterations(1);
                            Prec_ scale_factor(1.0);

                            // calc initial defect
                            // D = B - Ax, d0 = ||D||
                            DenseVector<Prec_> product(Product<Tag_>::value(system, initial_guess));
                            DenseVector<Prec_> rhs_c(right_hand_side.copy());
                            outer_defect = Difference<Tag_>::value(rhs_c, product);
                            initial_defect = Norm<vnt_l_two, false, Tag_>::value(outer_defect);

                            Prec_ def_norm;
                            Prec_ inv;
                            unsigned long step_iterations;
                            while(inner_iterations < 16)
                            {
                                // set defect as RHS to inner solver
                                for (unsigned long i(0); i < right_hand_side.size(); ++i)
                                {
                                    temp_vector[i] = (Prec_)outer_defect[i];
                                }
                                (info.rhs)[info.max_level] = (temp_vector.copy());
                                // run inner solver as long as neccessary
                                _multigrid_kernel<Prec_>(system, right_hand_side, max_levels, &cappa, info);
                                inner_iterations += 1; //Markus: for now, this is ok, later: add kernel iterations

                                // get "solution" and update outer solution
                                temp_vector = info.x[info.max_level].copy();
                                for (unsigned long i(0); i < right_hand_side.size(); ++i)
                                {
                                    result[i] += scale_factor * (Prec_)temp_vector[i];
                                }

                                // calculate defect
                                DenseVector<Prec_> rhs_c_1(right_hand_side.copy());
                                outer_defect = Difference<Tag_>::value(rhs_c_1, Product<Tag_>::value(system, result));
                                // calc norm of defect
                                def_norm = Norm<vnt_l_two, false, Tag_>::value(outer_defect);

                                // check for convergence
                                Prec_ outer_eps(1e-8);
                                if (def_norm  < outer_eps * initial_defect)
                                {
                                    break;
                                }
                                // scale defect
                                scale_factor = def_norm;

                                Prec_ inv(1.0 / scale_factor);
                                Scale<Tag_>::value(outer_defect, inv);
                                outer_iterations++;
                            }
                        }
                        return result;
                    }
        };
}
#endif