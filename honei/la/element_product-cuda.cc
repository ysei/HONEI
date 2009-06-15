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

#include <honei/la/element_product.hh>
#include <honei/backends/cuda/operations.hh>
#include <honei/util/configuration.hh>


using namespace honei;

DenseVectorContinuousBase<float> & ElementProduct<tags::GPU::CUDA>::value(DenseVectorContinuousBase<float> & a,
        const DenseVectorContinuousBase<float> & b)
{
    CONTEXT("When multiplying DenseVectorContinuousBase<float> and DenseVectorContinuousBase<float> elementwise "
            "(CUDA):");

    if (a.size() != b.size())
        throw VectorSizeDoesNotMatch(b.size(), a.size());

    unsigned long blocksize(Configuration::instance()->get_value("cuda::element_inverse_one_float", 128ul));

    void * a_gpu(a.lock(lm_read_and_write, tags::GPU::CUDA::memory_value));
    void * b_gpu(b.lock(lm_read_only, tags::GPU::CUDA::memory_value));
    cuda_element_product_three_float(a_gpu, a_gpu, b_gpu, a.size(), blocksize);
    b.unlock(lm_read_only);
    a.unlock(lm_read_and_write);

    return a;
}

#ifdef HONEI_CUDA_DOUBLE
DenseVectorContinuousBase<double> & ElementProduct<tags::GPU::CUDA>::value(DenseVectorContinuousBase<double> & a,
        const DenseVectorContinuousBase<double> & b)
{
    CONTEXT("When multiplying DenseVectorContinuousBase<double> and DenseVectorContinuousBase<double> elementwise "
            "(CUDA):");

    if (a.size() != b.size())
        throw VectorSizeDoesNotMatch(b.size(), a.size());

    unsigned long blocksize(Configuration::instance()->get_value("cuda::element_inverse_one_double", 128ul));

    void * a_gpu(a.lock(lm_read_and_write, tags::GPU::CUDA::memory_value));
    void * b_gpu(b.lock(lm_read_only, tags::GPU::CUDA::memory_value));
    cuda_element_product_three_double(a_gpu, a_gpu, b_gpu, a.size(), blocksize);
    b.unlock(lm_read_only);
    a.unlock(lm_read_and_write);

    return a;
}
#endif

DenseVectorContinuousBase<float> & ElementProduct<tags::GPU::CUDA>::value(DenseVectorContinuousBase<float> & result, DenseVectorContinuousBase<float> & a,
        const DenseVectorContinuousBase<float> & b)
{
    CONTEXT("When multiplying DenseVectorContinuousBase<float> and DenseVectorContinuousBase<float> elementwise "
            "(CUDA):");

    if (a.size() != b.size())
        throw VectorSizeDoesNotMatch(b.size(), a.size());

    unsigned long blocksize(Configuration::instance()->get_value("cuda::element_inverse_one_float", 128ul));

    void * result_gpu(result.lock(lm_write_only, tags::GPU::CUDA::memory_value));
    void * a_gpu(a.lock(lm_read_only, tags::GPU::CUDA::memory_value));
    void * b_gpu(b.lock(lm_read_only, tags::GPU::CUDA::memory_value));
    cuda_element_product_three_float(result_gpu, a_gpu, b_gpu, a.size(), blocksize);
    result.unlock(lm_write_only);
    b.unlock(lm_read_only);
    a.unlock(lm_read_only);

    return result;
}

#ifdef HONEI_CUDA_DOUBLE
DenseVectorContinuousBase<double> & ElementProduct<tags::GPU::CUDA>::value(DenseVectorContinuousBase<double> & result, DenseVectorContinuousBase<double> & a,
        const DenseVectorContinuousBase<double> & b)
{
    CONTEXT("When multiplying DenseVectorContinuousBase<double> and DenseVectorContinuousBase<double> elementwise "
            "(CUDA):");

    if (a.size() != b.size())
        throw VectorSizeDoesNotMatch(b.size(), a.size());

    unsigned long blocksize(Configuration::instance()->get_value("cuda::element_inverse_one_double", 128ul));

    void * result_gpu(result.lock(lm_write_only, tags::GPU::CUDA::memory_value));
    void * a_gpu(a.lock(lm_read_only, tags::GPU::CUDA::memory_value));
    void * b_gpu(b.lock(lm_read_only, tags::GPU::CUDA::memory_value));
    cuda_element_product_three_double(result_gpu, a_gpu, b_gpu, a.size(), blocksize);
    result.unlock(lm_write_only);
    b.unlock(lm_read_only);
    a.unlock(lm_read_only);

    return result;
}
#endif
DenseMatrix<float> & ElementProduct<tags::GPU::CUDA>::value(DenseMatrix<float> & a, const DenseMatrix<float> & b)
{
    CONTEXT("When multiplying DenseMatrix<float> and DenseMatrix<float> elementwise (CUDA):");

    if (a.columns() != b.columns())
    {
        throw MatrixColumnsDoNotMatch(b.columns(), a.columns());
    }

    if (a.rows() != b.rows())
    {
        throw MatrixRowsDoNotMatch(b.rows(), a.rows());
    }

    unsigned long blocksize(Configuration::instance()->get_value("cuda::element_inverse_one_float", 128ul));

    void * a_gpu(a.lock(lm_read_and_write, tags::GPU::CUDA::memory_value));
    void * b_gpu(b.lock(lm_read_only, tags::GPU::CUDA::memory_value));
    cuda_element_product_three_float(a_gpu, a_gpu, b_gpu, a.size(), blocksize);
    b.unlock(lm_read_only);
    a.unlock(lm_read_and_write);

    return a;
}

