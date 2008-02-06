/* vim: set number sw=4 sts=4 et nofoldenable : */

/*
 * Copyright (c) 2008 Markus Geveler <apryde@gmx.de>
 *
 * This file is part of the SWE C++ library. LiSWE is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * LibSWE is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef ALLBENCH
#include <benchmark/benchmark.cc>
#include <tr1/memory>
#include <string>
#endif

#include <honei/libmath/conjugate_gradients.hh>
#include <endian_swap.hh>

using namespace std;
using namespace honei;

template <typename Tag_, typename DataType_>

class PoissonPCGBench :
    public Benchmark
{
    public:
        PoissonPCGBench(const std::string & id) :
            Benchmark(id)
        {
            register_tag(Tag_::name);
        }

        virtual void run()
        {
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
            double* ana_sol;
            double* ref_sol;

            file = fopen("ehq.1.1.1.1.bin", "rb");
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
            ana_sol = new double[n];
            ref_sol = new double[n];

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
            fread(ana_sol, sizeof(double), n, file);
            fread(ref_sol, sizeof(double), n, file);
            fclose(file);

#ifdef HONEI_CELL
            for(unsigned long i(0); i < n; ++i)
            {
                dd[i] = DoubleSwap(dd[i]);
                ll[i] = DoubleSwap(ll[i]);
                ld[i] = DoubleSwap(ld[i]);
                lu[i] = DoubleSwap(lu[i]);
                dl[i] = DoubleSwap(dl[i]);
                du[i] = DoubleSwap(du[i]);
                ul[i] = DoubleSwap(ul[i]);
                ud[i] = DoubleSwap(ud[i]);
                uu[i] = DoubleSwap(uu[i]);
                b[i] = DoubleSwap(b[i]);
                ana_sol[i] = DoubleSwap(ana_sol[i]);
                ref_sol[i] = DoubleSwap(ref_sol[i]);

            }
#endif
            DenseVector<float> dd_v(n, float(0));
            DenseVector<float> ll_v(n, float(0));
            DenseVector<float> ld_v(n, float(0));
            DenseVector<float> lu_v(n, float(0));
            DenseVector<float> dl_v(n, float(0));
            DenseVector<float> du_v(n, float(0));
            DenseVector<float> ul_v(n, float(0));
            DenseVector<float> ud_v(n, float(0));
            DenseVector<float> uu_v(n, float(0));
            DenseVector<float> b_v(n, float(0));
            DenseVector<float> ana_sol_v(n, float(0));
            DenseVector<float> ref_sol_v(n, float(0));
            for(unsigned long i = 0; i < n; ++i)
            {
                dd_v[i] = (float)dd[i];
                ll_v[i] = (float)ll[i];
                ld_v[i] = (float)ld[i];
                lu_v[i] = (float)lu[i];
                dl_v[i] = (float)dl[i];
                du_v[i] = (float)du[i];
                ul_v[i] = (float)ul[i];
                ud_v[i] = (float)ud[i];
                uu_v[i] = (float)uu[i];
                b_v[i] = (float)b[i];
                ana_sol_v[i] = (float)ana_sol[i];
                ref_sol_v[i] = (float)ref_sol[i];
            }
            //std::cout<<dd[4]<<endl;
            //std::cout<<dd_v<<endl;


            long root_n = (long)sqrt(n);
            BandedMatrix<float> A(n,dd_v.copy());
            //std::cout<<A.band(0)<<endl;
            //A->insert_band(0, dd_v.copy());
            A.insert_band(1, du_v);
            A.insert_band(-1, dl_v);
            A.insert_band(root_n, ud_v);
            A.insert_band(root_n+1, uu_v);
            A.insert_band(root_n-1, ul_v);
            A.insert_band(-root_n, ld_v);
            A.insert_band(-root_n-1, ll_v );
            A.insert_band(-root_n+1, lu_v);

            //std::cout<<A.band(0)[0] * double(1) << endl;

            //std::cout<< n << " " << A << " "<< root_n<<endl;
            DenseVector<float> result(n, float(0));
            BENCHMARK((ConjugateGradients<Tag_, JAC>::value(A, b_v, std::numeric_limits<float>::epsilon())));
            evaluate();
        }
};
PoissonPCGBench<tags::CPU, float> poisson_pcg_bench_float("Poisson PCG benchmark float CPU");
#ifdef HONEI_SSE
PoissonPCGBench<tags::CPU::SSE, float> poisson_pcg_bench_float_sse("Poisson PCG benchmark float SSE");
#endif
#ifdef HONEI_CELL
PoissonPCGBench<tags::Cell, float> poisson_pcg_bench_float_cell("Poisson PCG benchmark float Cell");
#endif
