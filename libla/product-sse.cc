
/* vim: set sw=4 sts=4 et nofoldenable : */

/*
 * Copyright (c) 2007 Dirk Ribbrock <dirk.ribbrock@uni-dortmund.de>
 * Copyright (c) 2007 Sven Mallach <sven.mallach@uni-dortmund.de>
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

#include <libla/product.hh>
#include <xmmintrin.h>
#include <emmintrin.h>

using namespace honei;

DenseVector<float> Product<tags::CPU::SSE>::value(const BandedMatrix<float> & a, const DenseVector<float> & b)
{
    CONTEXT("When multiplying BandedMatrix<float> with DenseVector<float> with SSE:");

    if (b.size() != a.columns())
    {
        throw VectorSizeDoesNotMatch(b.size(), a.columns());
    }

    DenseVector<float> result(a.rows(), float(0));

    __m128 m1, m2, m3, m4, m5, m6;

    unsigned long middle_index(a.rows() - 1);
    unsigned long quad_end, end, quad_start, start, op_offset;

    // If we are above or on the diagonal band, we start at Element 0 and go on until Element band_size-band_index.
    for (BandedMatrix<float>::ConstVectorIterator vi(a.band_at(middle_index)), vi_end(a.end_bands()) ;
            vi != vi_end ; ++vi)
    {
        if (! vi.exists())
            continue;
        op_offset = vi.index() - middle_index;
        end = vi->size() - op_offset; //Calculation of the element-index to stop in iteration!
        quad_end = end - (end % 8);
        if (end < 24) quad_end = 0;

        float * band_e = vi->elements();
        float * b_e = b.elements();
        float * r_e = result.elements();

        for (unsigned long index = 0 ; index < quad_end ; index += 8)
        {
            m2 = _mm_loadu_ps(b_e + index + op_offset);
            m5 = _mm_loadu_ps(b_e + index + op_offset + 4);
            m1 = _mm_load_ps(band_e + index);
            m4 = _mm_load_ps(band_e + index + 4);
            m3 = _mm_load_ps(r_e + index);
            m6 = _mm_load_ps(r_e + index + 4);

            m1 = _mm_mul_ps(m1, m2);
            m4 = _mm_mul_ps(m4, m5);
            m1 = _mm_add_ps(m1, m3);
            m4 = _mm_add_ps(m4, m6);

            _mm_store_ps(r_e + index, m1);
            _mm_store_ps(r_e + index + 4, m4);
        }

        for (unsigned long index = quad_end ; index < end ; index++) 
        {
            r_e[index] += band_e[index] * b_e[index + op_offset];
        }
    }

    // If we are below the diagonal band, we start at Element 'start' and go on until the last element.
    for (BandedMatrix<float>::ConstVectorIterator vi(a.begin_bands()), vi_end(a.band_at(middle_index)) ;
            vi != vi_end ; ++vi)
    {
        if (! vi.exists())
            continue;
        op_offset = middle_index - vi.index();
        start = op_offset; //Calculation of the element-index to start in iteration!
        quad_start = start + (8 - (start % 8));
        end = a.size();
        quad_end = end - (end % 8);
        if ( start + 24 > end)
        {
            quad_end = start;
            quad_start = start;
        }
        float * band_e = vi->elements();
        float * b_e = b.elements();
        float * r_e = result.elements();

        for (unsigned long index = start ; index < quad_start ; index++)
        {
            r_e[index] += band_e[index] * b_e[index - op_offset];
        }
        for (unsigned long index = quad_start ; index < quad_end ; index += 8)
        {
            m2 = _mm_loadu_ps(b_e + index - op_offset);
            m5 = _mm_loadu_ps(b_e + index - op_offset + 8);
            m1 = _mm_load_ps(band_e + index);
            m4 = _mm_load_ps(band_e + index + 8);
            m3 = _mm_load_ps(r_e + index);
            m6 = _mm_load_ps(r_e + index + 8);

            m1 = _mm_mul_ps(m1, m2);
            m4 = _mm_mul_ps(m4, m5);
            m1 = _mm_add_ps(m1, m3);
            m4 = _mm_add_ps(m4, m6);

            _mm_store_ps(r_e + index, m1);
            _mm_store_ps(r_e + index + 8, m4);
        }

        for (unsigned long index = quad_end ; index < end ; index++)
        {
            r_e[index] += band_e[index] * b_e[index - op_offset];
        }
    }
    return result;
}

DenseVector<double> Product<tags::CPU::SSE>::value(const BandedMatrix<double> & a, const DenseVector<double> & b)
{
    CONTEXT("When multiplying BandedMatrix<double> with DenseVector<double> with SSE:");

    if (b.size() != a.columns())
    {
        throw VectorSizeDoesNotMatch(b.size(), a.columns());
    }

    DenseVector<double> result(a.rows(), double(0));

    __m128d m1, m2, m3, m4, m5, m6;

    unsigned long middle_index(a.rows() - 1);
    unsigned long quad_end, end, quad_start, start, op_offset;

    // If we are above or on the diagonal band, we start at Element 0 and go on until Element band_size-band_index.
    for (BandedMatrix<double>::ConstVectorIterator vi(a.band_at(middle_index)), vi_end(a.end_bands()) ;
            vi != vi_end ; ++vi)
    {
        if (! vi.exists())
            continue;
        op_offset = vi.index() - middle_index;
        end = vi->size() - op_offset; //Calculation of the element-index to stop in iteration!
        quad_end = end - (end % 4);
        if (end < 12) quad_end = 0;

        double * band_e = vi->elements();
        double * b_e = b.elements();
        double * r_e = result.elements();

        for (unsigned long index = 0 ; index < quad_end ; index += 4)
        {
            m2 = _mm_loadu_pd(b_e + index + op_offset);
            m5 = _mm_loadu_pd(b_e + index + op_offset + 2);
            m1 = _mm_load_pd(band_e + index);
            m4 = _mm_load_pd(band_e + index + 2);
            m3 = _mm_load_pd(r_e + index);
            m6 = _mm_load_pd(r_e + index + 2);

            m1 = _mm_mul_pd(m1, m2);
            m4 = _mm_mul_pd(m4, m5);
            m1 = _mm_add_pd(m1, m3);
            m4 = _mm_add_pd(m4, m6);

            _mm_store_pd(r_e + index, m1);
            _mm_store_pd(r_e + index + 2, m4);
        }

        for (unsigned long index = quad_end ; index < end ; index++) 
        {
            r_e[index] += band_e[index] * b_e[index + op_offset];
        }
    }

    // If we are below the diagonal band, we start at Element 'start' and go on until the last element.
    for (BandedMatrix<double>::ConstVectorIterator vi(a.begin_bands()), vi_end(a.band_at(middle_index)) ;
            vi != vi_end ; ++vi)
    {
        if (! vi.exists())
            continue;
        op_offset = middle_index - vi.index();
        start = op_offset; //Calculation of the element-index to start in iteration!
        quad_start = start + (4 - (start % 4));
        end = a.size();
        quad_end = end - (end % 4);
        if ( start + 12 > end)
        {
            quad_end = start;
            quad_start = start;
        }
        double * band_e = vi->elements();
        double * b_e = b.elements();
        double * r_e = result.elements();

        for (unsigned long index = start ; index < quad_start ; index++)
        {
            r_e[index] += band_e[index] * b_e[index - op_offset];
        }
        for (unsigned long index = quad_start ; index < quad_end ; index += 4)
        {
            m2 = _mm_loadu_pd(b_e + index - op_offset);
            m5 = _mm_loadu_pd(b_e + index - op_offset + 2);
            m1 = _mm_load_pd(band_e + index);
            m4 = _mm_load_pd(band_e + index + 2);
            m3 = _mm_load_pd(r_e + index);
            m6 = _mm_load_pd(r_e + index + 2);

            m1 = _mm_mul_pd(m1, m2);
            m4 = _mm_mul_pd(m4, m5);
            m1 = _mm_add_pd(m1, m3);
            m4 = _mm_add_pd(m4, m6);

            _mm_store_pd(r_e + index, m1);
            _mm_store_pd(r_e + index + 2, m4);
        }

        for (unsigned long index = quad_end ; index < end ; index++)
        {
            r_e[index] += band_e[index] * b_e[index - op_offset];
        }
    }
    return result;
}

DenseVector<float> Product<tags::CPU::SSE>::value(const DenseMatrix<float> & a, const DenseVector<float> & b)
{
    CONTEXT("When multiplying DenseMatrix<float> with DenseVector<float> with SSE:");

    if (b.size() != a.columns())
    {
        throw VectorSizeDoesNotMatch(b.size(), a.columns());
    }

    DenseVector<float> result(a.rows());

    /// \todo Hardcode M*V product.
    for (Vector<float>::ElementIterator l(result.begin_elements()), l_end(result.end_elements()) ; l != l_end ; ++l)
    {
        *l = DotProduct<tags::CPU::SSE>::value(b, a[l.index()]);
    }

    return result;
}

DenseVector<double> Product<tags::CPU::SSE>::value(const DenseMatrix<double> & a, const DenseVector<double> & b)
{
    CONTEXT("When multiplying DenseMatrix<double> with DenseVector<double> with SSE:");

    if (b.size() != a.columns())
    {
        throw VectorSizeDoesNotMatch(b.size(), a.columns());
    }

    DenseVector<double> result(a.rows());

    /// \todo Hardcode M*V product.
    for (Vector<double>::ElementIterator l(result.begin_elements()), l_end(result.end_elements()) ; l != l_end ; ++l)
    {
        *l = DotProduct<tags::CPU::SSE>::value(b, a[l.index()]);
    }

    return result;
}
