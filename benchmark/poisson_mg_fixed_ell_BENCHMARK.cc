/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Markus Geveler <apryde@gmx.de>
 *
 * This file is part of the Math C++ library. LibMath is free software;
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

//#define SOLVER_VERBOSE 1
#include <honei/math/multigrid.hh>
#include <honei/math/matrix_io.hh>
#include <honei/math/vector_io.hh>
#include <honei/math/transposition.hh>
#include <benchmark/benchmark.hh>
#include <honei/util/stringify.hh>
#include <iostream>
#include <honei/math/endian_swap.hh>

//#include <cstdio>
//#include <cstdlib>

#include <fstream>
using namespace honei;
using namespace std;

template <typename Tag_, typename DT1_>
class PoissonBenchmarkMGELLFixed:
    public Benchmark
{
    private:
        unsigned long _size;
        int _count;
    public:
        PoissonBenchmarkMGELLFixed(const std::string & id, unsigned long size, int count) :
            Benchmark(id)
    {
            register_tag(Tag_::name);
            _size  = size;
            _count = count;
    }

        virtual void run()
        {
            unsigned long _root_n(_size);
            unsigned long n(_root_n * _root_n);
            MGInfo<DT1_, SparseMatrixELL<DT1_> > info;
            //configuration constants: /TODO: set/allocate!!!
            info.is_smoother = false;
            DenseVector<unsigned long> mask(8);

            info.macro_border_mask = new DenseVector<unsigned long>(8);
            for(int i(0); i < 8; ++i)
            {
                (*info.macro_border_mask)[i] = 2;
            }
            //set Neumann boundaries:
            //(*info.macro_border_mask)[5] =1;

            switch(n)
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
                case 66049:
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
                case 25:
                    {
                        info.max_level = 2;
                    }
                    break;
                case 9:
                    {
                        info.max_level = 1;
                    }
                    break;
                default:
                    throw InternalError("Uknown size!");
                    break;
            }

            info.min_level = 1;
            info.n_max_iter = 1000;
            info.initial_zero = true;
            info.tolerance = 1e-8;
            info.convergence_check = true;

            info.n_pre_smooth = 4;
            info.n_post_smooth = 4;
            info.n_max_iter_coarse = ((unsigned long)sqrt((DT1_)(pow((DT1_)2 , (DT1_)info.max_level) + 1)*(pow((DT1_)2 , (DT1_)info.max_level) + 1)));
            info.tolerance_coarse = std::numeric_limits<double>::epsilon();
            info.adapt_correction_factor = 1.;

            for (unsigned long i(0) ; i < info.min_level; ++i)
            {
                unsigned long size((unsigned long)(((unsigned long)pow((DT1_)2, (DT1_)i) + 1) * ((unsigned long)pow((DT1_)2, (DT1_)i) + 1)));
                if(i == 0)
                    size = 9;

                DenseVector<DT1_> dummy_band(size, DT1_(0));
                BandedMatrixQ1<DT1_> ac_a(size, dummy_band.copy(), dummy_band.copy(), dummy_band.copy(), dummy_band.copy(), dummy_band.copy(), dummy_band.copy(), dummy_band.copy(), dummy_band.copy(), dummy_band.copy());
                SparseMatrix<DT1_> sm(ac_a);
                SparseMatrixELL<DT1_> ac_s(sm);
                info.a.push_back(ac_s);
                info.prolmats.push_back(ac_s.copy());
                info.resmats.push_back(ac_s.copy());
                // iteration vectors
                DenseVector<DT1_> ac_c(size, DT1_(0));
                info.c.push_back(ac_c);
                DenseVector<DT1_> ac_d(size, DT1_(0));
                info.d.push_back(ac_d);
                DenseVector<DT1_> ac_rhs(size, DT1_(0));
                info.rhs.push_back(ac_rhs);
                DenseVector<DT1_> ac_x(size, DT1_(0));
                info.x.push_back(ac_x);

                info.diags_inverted.push_back(dummy_band.copy());
            }


            for (unsigned long i(info.min_level) ; i <= info.max_level; ++i)
            {
                unsigned long size = (unsigned long)(((unsigned long)pow((DT1_)2, (DT1_)i) + 1) * ((unsigned long)pow((DT1_)2, (DT1_)i) + 1));
                std::cout << size << std::endl;
                // iteration vectors
                DenseVector<DT1_> ac_c(size, DT1_(0));
                info.c.push_back(ac_c);
                DenseVector<DT1_> ac_d(size, DT1_(0));
                info.d.push_back(ac_d);
                DenseVector<DT1_> ac_x(size, DT1_(0));
                info.x.push_back(ac_x);

                DenseVector<DT1_> dummy_band(size, DT1_(0));
                //info.diags_inverted.push_back(dummy_band.copy());
            }

            //assemble all needed levels' matrices:
            for(unsigned long i(info.min_level); i <= info.max_level; ++i)
            {
                unsigned long N = (unsigned long)(((unsigned long)pow((DT1_)2, (DT1_)i) + 1) * ((unsigned long)pow((DT1_)2, (DT1_)i) + 1));
                DenseVector<DT1_> current_rhs(N);
                std::string A_file(HONEI_SOURCEDIR);
                A_file += "/honei/math/testdata/poisson/";
                A_file += "poisson_A_";
                A_file += stringify(i);
                A_file += ".ell";
                SparseMatrixELL<DT1_> smell(MatrixIO<io_formats::ELL>::read_matrix(A_file, DT1_(0)));

                std::string rhs_file(HONEI_SOURCEDIR);
                rhs_file += "/honei/math/testdata/poisson/";
                rhs_file += "poisson_rhs";
                if(i == info.max_level)
                    VectorIO<io_formats::EXP>::read_vector(rhs_file, current_rhs);

                info.rhs.push_back(current_rhs);
                info.a.push_back(smell);

                DenseVector<DT1_> scaled_diag_inverted(N);
                for(unsigned long j(0) ; j < N ; ++ j)
                    scaled_diag_inverted[j] = smell(j, j);

                ElementInverse<Tag_>::value(scaled_diag_inverted);
                Scale<Tag_>::value(scaled_diag_inverted, 0.7);

                info.diags_inverted.push_back(scaled_diag_inverted.copy());

                if(i >= info.min_level)
                {
                    if(i == 1)
                    {
                        SparseMatrix<DT1_> prol(1, 1);
                        SparseMatrixELL<DT1_> prolmat(prol);
                        info.prolmats.push_back(prolmat);
                        info.resmats.push_back(prolmat);
                    }
                    else
                    {
                        std::string prol_file(HONEI_SOURCEDIR);
                        prol_file += "/honei/math/testdata/poisson/";
                        prol_file += "poisson_prol_";
                        prol_file += stringify(i);
                        prol_file += ".ell";
                        SparseMatrixELL<DT1_> prolmat(MatrixIO<io_formats::ELL>::read_matrix(prol_file, DT1_(0)));
                        info.prolmats.push_back(prolmat);

                        SparseMatrix<DT1_> prol(prolmat);
                        SparseMatrix<DT1_> res(prol.columns(), prol.rows());
                        Transposition<Tag_>::value(prol, res);
                        SparseMatrixELL<DT1_> resmat(res);
                        info.resmats.push_back(resmat);
                    }
                }
            }
            //clear x data
            for(unsigned long i(0) ; i < info.max_level ; ++i)
            {
                unsigned long size((unsigned long)(((unsigned long)pow((DT1_)2, (DT1_)i) + 1) * ((unsigned long)pow((DT1_)2, (DT1_)i) + 1)));
                if(size==0)
                    size = 9;

                DenseVector<DT1_> null(size , DT1_(0));
                info.x[i] = null.copy();
            }

            /*for(unsigned long i(0) ; i < info.max_level ; ++i)
            {
                std::cout << "VECSIZE d " << info.d.at(i).size() << std::endl;
                std::cout << "VECSIZE rhs " << info.rhs.at(i).size() << std::endl;
                std::cout <<"SYSTEM"<<std::endl;
                std::cout <<"ROW: " << info.a.at(i).rows() << std::endl;
                std::cout <<"COLS: " << info.a.at(i).columns() << std::endl;
                std::cout <<"RESTRICTION"<<std::endl;
                std::cout <<"ROW: " << info.resmats.at(i).rows() << std::endl;
                std::cout <<"COLS: " << info.resmats.at(i).columns() << std::endl;
                std::cout <<"PROLONGATION"<<std::endl;
                std::cout <<"ROW: " << info.prolmats.at(i).rows() << std::endl;
                std::cout <<"COLS: " << info.prolmats.at(i).columns() << std::endl;

            }*/
            DenseVector<DT1_> rhs(info.rhs[info.max_level]);
            SparseMatrixELL<DT1_> system(info.a[info.max_level]);
            DenseVector<DT1_> ref_result(rhs.size());
            std::string sol_file(HONEI_SOURCEDIR);
            sol_file += "/honei/math/testdata/poisson/";
            sol_file += "poisson_sol";
            VectorIO<io_formats::EXP>::read_vector(sol_file, ref_result);


            DenseVector<DT1_> result(n, DT1_(0));
            for (int i(0) ; i < 1 ; ++i)
            {
                BENCHMARK(
                        for (unsigned long j(0) ; j < 1 ; ++j)
                        {
                        (Multigrid<Tag_, Tag_, methods::PROLMAT, JAC, CYCLE::V, FIXED >::value(info.a[info.max_level], info.rhs[info.max_level], result, (unsigned long)11, std::numeric_limits<DT1_>::epsilon(), info));
                        }
                        );
            }
            evaluate();
        }
};
/*#ifdef HONEI_SSE
PoissonBenchmarkMGELLFixed<tags::CPU::SSE, double> poisson_bench_mg_ell_d_sse("MG double SSE L6", 65, 1);
PoissonBenchmarkMGELLFixed<tags::CPU::MultiCore::SSE, double> poisson_bench_mg_ell_d_mcsse("MG double MCSSE L6", 65, 1);
#endif

#ifdef HONEI_CUDA
PoissonBenchmarkMGELLFixed<tags::GPU::CUDA, double> poisson_bench_mg_ell_float_cuda6("MG float CUDA L6", 65, 1);
#ifdef HONEI_CUDA_DOUBLE
PoissonBenchmarkMGELLFixed<tags::GPU::CUDA, double> poisson_bench_mg_ell_double_cuda6("MG double CUDA L6", 65, 1);
#endif
#endif*/

template <typename Tag_, typename DT1_>
class PoissonAdvancedBENCHMGSparseELLProlMat:
    public Benchmark
{
    private:
        unsigned long _size;
        std::string _res_f;
        unsigned long _sorting;

        static unsigned long _level_to_size(unsigned long level)
        {
            switch(level)
            {
                case 10:
                    {
                        return 2101248;
                    }
                case 9:
                    {
                        return 526336;
                    }
                case 8:
                    {
                        return 132096;
                    }
                case 7:
                    {
                        return 33280;
                    }
                case 6:
                    {
                        return 8448;
                    }
                case 5:
                    {
                        return 2176;
                    }
                case 4:
                    {
                        return 576;
                    }
                case 3:
                    {
                        return 160;
                    }
                case 2:
                    {
                        return 48;
                    }
                case 1:
                    {
                        return 16;
                    }
                default:
                    return 1;
            }
        }

    public:
        PoissonAdvancedBENCHMGSparseELLProlMat(const std::string & tag, unsigned long level, unsigned long sorting) :
            Benchmark(tag)
        {
            register_tag(Tag_::name);
            _size = level;
            _sorting = sorting;
        }

        virtual void run()
        {
            unsigned long n(_level_to_size(_size));
            MGInfo<DT1_, SparseMatrixELL<DT1_> > info;
            //configuration constants: /TODO: set/allocate!!!
            info.is_smoother = false;
            DenseVector<unsigned long> mask(8);

            info.macro_border_mask = new DenseVector<unsigned long>(8);
            for(int i(0); i < 8; ++i)
            {
                (*info.macro_border_mask)[i] = 2;
            }
            //set Neumann boundaries:
            //(*info.macro_border_mask)[5] =1;


            info.min_level = 1;
            info.max_level = _size;
            info.n_max_iter = 17;
            info.initial_zero = false;
            info.tolerance = 1e-5;
            info.convergence_check = true;

            info.n_pre_smooth = 4;
            info.n_post_smooth = 4;
            //info.n_max_iter_coarse = ((unsigned long)sqrt((DT1_)(pow((DT1_)2 , (DT1_)info.max_level) + 1)*(pow((DT1_)2 , (DT1_)info.max_level) + 1)));
            info.n_max_iter_coarse = 100;
            info.tolerance_coarse = 1e-2;
            info.adapt_correction_factor = 1.;

            for (unsigned long i(0) ; i < info.min_level; ++i)
            {
                unsigned long size(_level_to_size(i));
                if(i == 0)
                    size = 9;

                DenseVector<DT1_> dummy_band(size, DT1_(0));
                BandedMatrixQ1<DT1_> ac_a(size, dummy_band.copy(), dummy_band.copy(), dummy_band.copy(), dummy_band.copy(), dummy_band.copy(), dummy_band.copy(), dummy_band.copy(), dummy_band.copy(), dummy_band.copy());
                SparseMatrix<DT1_> sm(ac_a);
                SparseMatrixELL<DT1_> ac_s(sm);
                info.a.push_back(ac_s);
                info.prolmats.push_back(ac_s.copy());
                info.resmats.push_back(ac_s.copy());
                // iteration vectors
                DenseVector<DT1_> ac_c(size, DT1_(0));
                info.c.push_back(ac_c);
                DenseVector<DT1_> ac_d(size, DT1_(0));
                info.d.push_back(ac_d);
                DenseVector<DT1_> ac_rhs(size, DT1_(0));
                info.rhs.push_back(ac_rhs);
                DenseVector<DT1_> ac_x(size, DT1_(0));
                info.x.push_back(ac_x);

                info.diags_inverted.push_back(dummy_band.copy());
            }


            for (unsigned long i(info.min_level) ; i <= info.max_level; ++i)
            {
                unsigned long size(_level_to_size(i));
                std::cout << size << std::endl;
                // iteration vectors
                DenseVector<DT1_> ac_c(size, DT1_(0));
                info.c.push_back(ac_c);
                DenseVector<DT1_> ac_d(size, DT1_(0));
                info.d.push_back(ac_d);
                DenseVector<DT1_> ac_x(size, DT1_(0));
                info.x.push_back(ac_x);

                DenseVector<DT1_> dummy_band(size, DT1_(0));
                //info.diags_inverted.push_back(dummy_band.copy());
            }

            std::string file_base(HONEI_SOURCEDIR);
            file_base += "/honei/math/testdata/poisson_advanced/sort_" + stringify(_sorting) + "/";
            //assemble all needed levels' matrices:
            for(unsigned long i(info.min_level); i <= info.max_level; ++i)
            {
                unsigned long N(_level_to_size(i));
                DenseVector<DT1_> current_rhs(N);
                std::string A_file(file_base);
                A_file += "A_";
                A_file += stringify(i);
                A_file += ".ell";
                SparseMatrixELL<DT1_> smell(MatrixIO<io_formats::ELL>::read_matrix(A_file, DT1_(0)));

                std::string rhs_file(file_base);
                rhs_file += "rhs_" + stringify(_size);
                if(i == info.max_level)
                    VectorIO<io_formats::EXP>::read_vector(rhs_file, current_rhs);

                info.rhs.push_back(current_rhs);
                info.a.push_back(smell);

                DenseVector<DT1_> scaled_diag_inverted(N);
                for(unsigned long j(0) ; j < N ; ++ j)
                    scaled_diag_inverted[j] = smell(j, j);

                ElementInverse<Tag_>::value(scaled_diag_inverted);
                Scale<Tag_>::value(scaled_diag_inverted, 0.7);

                info.diags_inverted.push_back(scaled_diag_inverted.copy());

                if(i >= info.min_level)
                {
                    if(i == 1)
                    {
                        SparseMatrix<DT1_> prol(1, 1);
                        SparseMatrixELL<DT1_> prolmat(prol);
                        info.prolmats.push_back(prolmat);
                        info.resmats.push_back(prolmat);
                    }
                    else
                    {
                        std::string prol_file(file_base);
                        prol_file += "prol_";
                        prol_file += stringify(i);
                        prol_file += ".ell";
                        SparseMatrixELL<DT1_> prolmat(MatrixIO<io_formats::ELL>::read_matrix(prol_file, DT1_(0)));
                        info.prolmats.push_back(prolmat);

                        SparseMatrix<DT1_> prol(prolmat);
                        SparseMatrix<DT1_> res(prol.columns(), prol.rows());
                        Transposition<Tag_>::value(prol, res);
                        SparseMatrixELL<DT1_> resmat(res);
                        info.resmats.push_back(resmat);
                    }
                }
            }
            //clear x data
            for(unsigned long i(0) ; i < info.max_level ; ++i)
            {
                unsigned long size(_level_to_size(i));
                if(size==0)
                    size = 9;

                DenseVector<DT1_> null(size , DT1_(0));
                info.x[i] = null.copy();
            }

            /*for(unsigned long i(0) ; i < info.max_level ; ++i)
            {
                std::cout << "VECSIZE d " << info.d.at(i).size() << std::endl;
                std::cout << "VECSIZE rhs " << info.rhs.at(i).size() << std::endl;
                std::cout <<"SYSTEM"<<std::endl;
                std::cout <<"ROW: " << info.a.at(i).rows() << std::endl;
                std::cout <<"COLS: " << info.a.at(i).columns() << std::endl;
                std::cout <<"RESTRICTION"<<std::endl;
                std::cout <<"ROW: " << info.resmats.at(i).rows() << std::endl;
                std::cout <<"COLS: " << info.resmats.at(i).columns() << std::endl;
                std::cout <<"PROLONGATION"<<std::endl;
                std::cout <<"ROW: " << info.prolmats.at(i).rows() << std::endl;
                std::cout <<"COLS: " << info.prolmats.at(i).columns() << std::endl;

            }*/
            std::string init_file(file_base);
            init_file += "init_" + stringify(_size);
            DenseVector<DT1_> result(n, DT1_(0));
            VectorIO<io_formats::EXP>::read_vector(init_file, result);

            DenseVector<DT1_> rhs(info.rhs[info.max_level]);
            SparseMatrixELL<DT1_> system(info.a[info.max_level]);

            BENCHMARK(
                      (Multigrid<Tag_, Tag_, methods::PROLMAT, JAC, CYCLE::V, FIXED >::value(system, rhs, result, (unsigned long)11, std::numeric_limits<DT1_>::epsilon(), info));
                     );

            evaluate();
        }
};
#ifdef HONEI_SSE
  PoissonAdvancedBENCHMGSparseELLProlMat<tags::CPU::SSE, double> sse_poisson_advanced7_bench_mg_sparse_prolmat0_double("MGNEW double sse L7 sort 0", 7ul, 0ul);
  PoissonAdvancedBENCHMGSparseELLProlMat<tags::CPU::SSE, double> sse_poisson_advanced7_bench_mg_sparse_prolmat1_double("MGNEW double sse L7 sort 1", 7ul, 1ul);
  PoissonAdvancedBENCHMGSparseELLProlMat<tags::CPU::SSE, double> sse_poisson_advanced7_bench_mg_sparse_prolmat2_double("MGNEW double sse L7 sort 2", 7ul, 2ul);
  PoissonAdvancedBENCHMGSparseELLProlMat<tags::CPU::SSE, double> sse_poisson_advanced7_bench_mg_sparse_prolmat3_double("MGNEW double sse L7 sort 3", 7ul, 3ul);
  PoissonAdvancedBENCHMGSparseELLProlMat<tags::CPU::SSE, double> sse_poisson_advanced7_bench_mg_sparse_prolmat4_double("MGNEW double sse L7 sort 4", 7ul, 4ul);


  PoissonAdvancedBENCHMGSparseELLProlMat<tags::CPU::SSE, double> sse_poisson_advanced8_bench_mg_sparse_prolmat0_double("MGNEW double sse L8 sort 0", 8ul, 0ul);
  PoissonAdvancedBENCHMGSparseELLProlMat<tags::CPU::SSE, double> sse_poisson_advanced8_bench_mg_sparse_prolmat1_double("MGNEW double sse L8 sort 1", 8ul, 1ul);
  PoissonAdvancedBENCHMGSparseELLProlMat<tags::CPU::SSE, double> sse_poisson_advanced8_bench_mg_sparse_prolmat2_double("MGNEW double sse L8 sort 2", 8ul, 2ul);
  PoissonAdvancedBENCHMGSparseELLProlMat<tags::CPU::SSE, double> sse_poisson_advanced8_bench_mg_sparse_prolmat3_double("MGNEW double sse L8 sort 3", 8ul, 3ul);
  PoissonAdvancedBENCHMGSparseELLProlMat<tags::CPU::SSE, double> sse_poisson_advanced8_bench_mg_sparse_prolmat4_double("MGNEW double sse L8 sort 4", 8ul, 4ul);


  PoissonAdvancedBENCHMGSparseELLProlMat<tags::CPU::SSE, double> sse_poisson_advanced9_bench_mg_sparse_prolmat0_double("MGNEW double sse L9 sort 0", 9ul, 0ul);
  PoissonAdvancedBENCHMGSparseELLProlMat<tags::CPU::SSE, double> sse_poisson_advanced9_bench_mg_sparse_prolmat1_double("MGNEW double sse L9 sort 1", 9ul, 1ul);
  PoissonAdvancedBENCHMGSparseELLProlMat<tags::CPU::SSE, double> sse_poisson_advanced9_bench_mg_sparse_prolmat2_double("MGNEW double sse L9 sort 2", 9ul, 2ul);
  PoissonAdvancedBENCHMGSparseELLProlMat<tags::CPU::SSE, double> sse_poisson_advanced9_bench_mg_sparse_prolmat3_double("MGNEW double sse L9 sort 3", 9ul, 3ul);
  PoissonAdvancedBENCHMGSparseELLProlMat<tags::CPU::SSE, double> sse_poisson_advanced9_bench_mg_sparse_prolmat4_double("MGNEW double sse L9 sort 4", 9ul, 4ul);
  //PoissonAdvancedBENCHMGSparseELLProlMat<tags::CPU::MultiCore::SSE, double> mc_sse_poisson_advanced_bench_mg_sparse_prolmat_double("MGNEW double mcsse", 7ul, 0ul);
#endif
#ifdef HONEI_CUDA
#ifdef HONEI_CUDA_DOUBLE
PoissonAdvancedBENCHMGSparseELLProlMat<tags::GPU::CUDA, double> cuda_poisson_advanced7_bench_mg_sparse_prolmat0_double("MGNEW double cuda L7 sort 0", 7ul, 0ul);
PoissonAdvancedBENCHMGSparseELLProlMat<tags::GPU::CUDA, double> cuda_poisson_advanced7_bench_mg_sparse_prolmat1_double("MGNEW double cuda L7 sort 1", 7ul, 1ul);
PoissonAdvancedBENCHMGSparseELLProlMat<tags::GPU::CUDA, double> cuda_poisson_advanced7_bench_mg_sparse_prolmat2_double("MGNEW double cuda L7 sort 2", 7ul, 2ul);
PoissonAdvancedBENCHMGSparseELLProlMat<tags::GPU::CUDA, double> cuda_poisson_advanced7_bench_mg_sparse_prolmat3_double("MGNEW double cuda L7 sort 3", 7ul, 3ul);
PoissonAdvancedBENCHMGSparseELLProlMat<tags::GPU::CUDA, double> cuda_poisson_advanced7_bench_mg_sparse_prolmat4_double("MGNEW double cuda L7 sort 4", 7ul, 4ul);


PoissonAdvancedBENCHMGSparseELLProlMat<tags::GPU::CUDA, double> cuda_poisson_advanced8_bench_mg_sparse_prolmat0_double("MGNEW double cuda L8 sort 0", 8ul, 0ul);
PoissonAdvancedBENCHMGSparseELLProlMat<tags::GPU::CUDA, double> cuda_poisson_advanced8_bench_mg_sparse_prolmat1_double("MGNEW double cuda L8 sort 1", 8ul, 1ul);
PoissonAdvancedBENCHMGSparseELLProlMat<tags::GPU::CUDA, double> cuda_poisson_advanced8_bench_mg_sparse_prolmat2_double("MGNEW double cuda L8 sort 2", 8ul, 2ul);
PoissonAdvancedBENCHMGSparseELLProlMat<tags::GPU::CUDA, double> cuda_poisson_advanced8_bench_mg_sparse_prolmat3_double("MGNEW double cuda L8 sort 3", 8ul, 3ul);
PoissonAdvancedBENCHMGSparseELLProlMat<tags::GPU::CUDA, double> cuda_poisson_advanced8_bench_mg_sparse_prolmat4_double("MGNEW double cuda L8 sort 4", 8ul, 4ul);


PoissonAdvancedBENCHMGSparseELLProlMat<tags::GPU::CUDA, double> cuda_poisson_advanced9_bench_mg_sparse_prolmat0_double("MGNEW double cuda L9 sort 0", 9ul, 0ul);
PoissonAdvancedBENCHMGSparseELLProlMat<tags::GPU::CUDA, double> cuda_poisson_advanced9_bench_mg_sparse_prolmat1_double("MGNEW double cuda L9 sort 1", 9ul, 1ul);
PoissonAdvancedBENCHMGSparseELLProlMat<tags::GPU::CUDA, double> cuda_poisson_advanced9_bench_mg_sparse_prolmat2_double("MGNEW double cuda L9 sort 2", 9ul, 2ul);
PoissonAdvancedBENCHMGSparseELLProlMat<tags::GPU::CUDA, double> cuda_poisson_advanced9_bench_mg_sparse_prolmat3_double("MGNEW double cuda L9 sort 3", 9ul, 3ul);
PoissonAdvancedBENCHMGSparseELLProlMat<tags::GPU::CUDA, double> cuda_poisson_advanced9_bench_mg_sparse_prolmat4_double("MGNEW double cuda L9 sort 4", 9ul, 4ul);
#endif
#endif