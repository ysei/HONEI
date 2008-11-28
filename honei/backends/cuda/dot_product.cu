/* vim: set sw=4 sts=4 et nofoldenable : */

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

#include <honei/backends/cuda/cuda_util.hh>

namespace honei
{
    namespace cuda
    {
        __global__ void dot_product_gpu(float * x, float * y, float * tmp, unsigned long size, unsigned long blocksize)
        {
            // calculate how many elements each thread needs to calculate
            const unsigned long iter = size / (blockDim.x * gridDim.x);
            unsigned long pos = blockIdx.x* blocksize + threadIdx.x;

            // clear the output
            tmp[blockIdx.x * blocksize + threadIdx.x] = 0;

            for (unsigned long i = 0 ; i < iter ; ++i)
            {
                tmp[blockIdx.x * blocksize + threadIdx.x] += x[pos] * y[pos];
                pos += blockDim.x * gridDim.x;
            }

            // for the last iteration, check if the elements are still available
            if (pos < size)
            {
                tmp[blockIdx.x * blocksize + threadIdx.x] += x[pos] * y[pos];
            }
        }
    }
}

extern "C" float cuda_dot_product_two_float(const void * x, const void * y, unsigned long size, unsigned long blocksize,
        unsigned long gridsize)
{
    float result(0.);

    dim3 grid(gridsize);
    dim3 block(blocksize);
    float * x_gpu((float* )x);
    float * y_gpu((float *)y);
    float * tmp_cpu(0);
    float * tmp_gpu(0);

    cudaMalloc((void**)&tmp_gpu, gridsize * blocksize * sizeof(float));
    cudaMallocHost((void**)&tmp_cpu, gridsize * blocksize * sizeof(float));

    honei::cuda::dot_product_gpu<<<grid, block>>>(x_gpu, y_gpu, tmp_gpu, size, blocksize);

    cudaMemcpy(tmp_cpu, tmp_gpu, blocksize * gridsize * sizeof(float), cudaMemcpyDeviceToHost);
    for (unsigned long i(0) ; i < blocksize * gridsize ; ++i)
    {
        result += tmp_cpu[i];
    }

    cudaFree(tmp_gpu);
    cudaFreeHost(tmp_cpu);

    CUDA_ERROR();
    return result;
}
