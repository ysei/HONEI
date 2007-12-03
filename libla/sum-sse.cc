/* vim: set sw=4 sts=4 et nofoldenable : */

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

#include <libla/sum.hh>

#include <xmmintrin.h>
#include <emmintrin.h>

using namespace honei;

DenseVector<float> & Sum<tags::CPU::SSE>::value(DenseVector<float> & a, const DenseVector<float> & b)
{
    CONTEXT("When adding DenseVector<float> to DenseVector<float> with SSE:");


    if (a.size() != b.size())
        throw VectorSizeDoesNotMatch(b.size(), a.size());

    __m128 m1, m2, m3, m4, m5, m6, m7, m8;

    unsigned long quad_end(a.size() - (a.size() % 16));
    if (a.size() < 16) quad_end = 0;

    for (unsigned long index = 0 ; index < quad_end ; index += 16)
    {
        m1 = _mm_load_ps(a.elements() + index);
        m3 = _mm_load_ps(a.elements() + index + 4);
        m5 = _mm_load_ps(a.elements() + index + 8);
        m7 = _mm_load_ps(a.elements() + index + 12);
        m2 = _mm_load_ps(b.elements() + index);
        m4 = _mm_load_ps(b.elements() + index + 4);
        m6 = _mm_load_ps(b.elements() + index + 8);
        m8 = _mm_load_ps(b.elements() + index + 12);

        m1 = _mm_add_ps(m1, m2);
        m3 = _mm_add_ps(m3, m4);
        m5 = _mm_add_ps(m5, m6);
        m7 = _mm_add_ps(m7, m8);

        _mm_stream_ps(a.elements() + index, m1);
        _mm_stream_ps(a.elements() + index + 4, m3);
        _mm_stream_ps(a.elements() + index + 8, m5);
        _mm_stream_ps(a.elements() + index + 12, m7);
    }
    for (unsigned long index(quad_end) ; index < a.size() ; ++index)
    {
        a.elements()[index] += b.elements()[index];
    }
    return a;
}

DenseVector<double> & Sum<tags::CPU::SSE>::value(DenseVector<double> & a, const DenseVector<double> & b)
{
    CONTEXT("When adding DenseVector<double> to DenseVector<double> with SSE:");


    if (a.size() != b.size())
        throw VectorSizeDoesNotMatch(b.size(), a.size());

    __m128d m1, m2, m3, m4, m5, m6, m7, m8;

    unsigned long quad_end(a.size() - (a.size() % 8));
    if (a.size() < 8) quad_end = 0;

    for (unsigned long index = 0 ; index < quad_end ; index += 8)
    {
        m1 = _mm_load_pd(a.elements() + index);
        m3 = _mm_load_pd(a.elements() + index + 2);
        m5 = _mm_load_pd(a.elements() + index + 4);
        m7 = _mm_load_pd(a.elements() + index + 6);
        m2 = _mm_load_pd(b.elements() + index);
        m4 = _mm_load_pd(b.elements() + index + 2);
        m6 = _mm_load_pd(b.elements() + index + 4);
        m8 = _mm_load_pd(b.elements() + index + 6);

        m1 = _mm_add_pd(m1, m2);
        m3 = _mm_add_pd(m3, m4);
        m5 = _mm_add_pd(m5, m6);
        m7 = _mm_add_pd(m7, m8);

        _mm_stream_pd(a.elements() + index, m1);
        _mm_stream_pd(a.elements() + index + 2, m3);
        _mm_stream_pd(a.elements() + index + 4, m5);
        _mm_stream_pd(a.elements() + index + 6, m7);
    }
    for (unsigned long index(quad_end) ; index < a.size() ; ++index)
    {
        a.elements()[index] += b.elements()[index];
    }
    return a;
}

DenseVectorContinuousBase<float> & Sum<tags::CPU::SSE>::value(DenseVectorContinuousBase<float> & a, const DenseVectorContinuousBase<float> & b)
{
    CONTEXT("When adding DenseVectorContinuousBase<float> to DenseVectorContinuousBase<float> with SSE:");
    if (a.size() != b.size())
        throw VectorSizeDoesNotMatch(b.size(), a.size());

    __m128 m1, m2, m3, m4, m5, m6, m7, m8;

    unsigned long a_address = (unsigned long)a.elements();
    unsigned long a_offset = a_address % 16;
    unsigned long b_address = (unsigned long)b.elements();
    unsigned long b_offset = b_address % 16;

    unsigned long x_offset(a_offset / 4);
    x_offset = (4 - x_offset) % 4;

    unsigned long quad_start = x_offset;
    unsigned long quad_end(a.size() - ((a.size()-quad_start) % 16));
    if (a.size() < 24)
    {
        quad_end = 0;
        quad_start = 0;
    }
    if (a_offset == b_offset)
    {
        for (unsigned long index = quad_start ; index < quad_end ; index += 16)
        {
            m1 = _mm_load_ps(a.elements() + index);
            m3 = _mm_load_ps(a.elements() + index + 4);
            m5 = _mm_load_ps(a.elements() + index + 8);
            m7 = _mm_load_ps(a.elements() + index + 12);
            m2 = _mm_load_ps(b.elements() + index);
            m4 = _mm_load_ps(b.elements() + index + 4);
            m6 = _mm_load_ps(b.elements() + index + 8);
            m8 = _mm_load_ps(b.elements() + index + 12);

            m1 = _mm_add_ps(m1, m2);
            m3 = _mm_add_ps(m3, m4);
            m5 = _mm_add_ps(m5, m6);
            m7 = _mm_add_ps(m7, m8);

            _mm_stream_ps(a.elements() + index, m1);
            _mm_stream_ps(a.elements() + index + 4, m3);
            _mm_stream_ps(a.elements() + index + 8, m5);
            _mm_stream_ps(a.elements() + index + 12, m7);
        }
    }
    else
    {
        for (unsigned long index = quad_start ; index < quad_end ; index += 16)
        {
            m1 = _mm_load_ps(a.elements() + index);
            m3 = _mm_load_ps(a.elements() + index + 4);
            m5 = _mm_load_ps(a.elements() + index + 8);
            m7 = _mm_load_ps(a.elements() + index + 12);
            m2 = _mm_loadu_ps(b.elements() + index);
            m4 = _mm_loadu_ps(b.elements() + index + 4);
            m6 = _mm_loadu_ps(b.elements() + index + 8);
            m8 = _mm_loadu_ps(b.elements() + index + 12);

            m1 = _mm_add_ps(m1, m2);
            m3 = _mm_add_ps(m3, m4);
            m5 = _mm_add_ps(m5, m6);
            m7 = _mm_add_ps(m7, m8);

            _mm_stream_ps(a.elements() + index, m1);
            _mm_stream_ps(a.elements() + index + 4, m3);
            _mm_stream_ps(a.elements() + index + 8, m5);
            _mm_stream_ps(a.elements() + index + 12, m7);
        }
    }
    for (unsigned long index = 0 ; index < quad_start ; index++)
    {
        a.elements()[index] += b.elements()[index];
    }
    for (unsigned long index = quad_end ; index < a.size() ; index++)
    {
        a.elements()[index] += b.elements()[index];
    }
    return a;
}

DenseVectorContinuousBase<double> & Sum<tags::CPU::SSE>::value(DenseVectorContinuousBase<double> & a, const DenseVectorContinuousBase<double> & b)
{
    CONTEXT("When adding DenseVectorContinuousBase<double> to DenseVectorContinuousBase<double> with SSE:");

    if (a.size() != b.size())
        throw VectorSizeDoesNotMatch(b.size(), a.size());

    __m128d m1, m2, m3, m4, m5, m6, m7, m8;

    unsigned long a_address = (unsigned long)a.elements();
    unsigned long a_offset = a_address % 16;
    unsigned long b_address = (unsigned long)b.elements();
    unsigned long b_offset = b_address % 16;

    unsigned long x_offset(a_offset / 8);

    unsigned long quad_start = x_offset;
    unsigned long quad_end(a.size() - ((a.size() - quad_start) % 8));
    if (a.size() < 16)
    {
        quad_end = 0;
        quad_start = 0;
    }

    if (a_offset == b_offset)
    {
        for (unsigned long index = quad_start ; index < quad_end ; index += 8)
        {
            m1 = _mm_load_pd(a.elements() + index);
            m3 = _mm_load_pd(a.elements() + index + 2);
            m5 = _mm_load_pd(a.elements() + index + 4);
            m7 = _mm_load_pd(a.elements() + index + 6);
            m2 = _mm_load_pd(b.elements() + index);
            m4 = _mm_load_pd(b.elements() + index + 2);
            m6 = _mm_load_pd(b.elements() + index + 4);
            m8 = _mm_load_pd(b.elements() + index + 6);

            m1 = _mm_add_pd(m1, m2);
            m3 = _mm_add_pd(m3, m4);
            m5 = _mm_add_pd(m5, m6);
            m7 = _mm_add_pd(m7, m8);

            _mm_stream_pd(a.elements() + index, m1);
            _mm_stream_pd(a.elements() + index + 2, m3);
            _mm_stream_pd(a.elements() + index + 4, m5);
            _mm_stream_pd(a.elements() + index + 6, m7);
        }
    }
    else
    {
        for (unsigned long index = quad_start ; index < quad_end ; index += 8)
        {
            m1 = _mm_load_pd(a.elements() + index);
            m3 = _mm_load_pd(a.elements() + index + 2);
            m5 = _mm_load_pd(a.elements() + index + 4);
            m7 = _mm_load_pd(a.elements() + index + 6);
            m2 = _mm_loadu_pd(b.elements() + index);
            m4 = _mm_loadu_pd(b.elements() + index + 2);
            m6 = _mm_loadu_pd(b.elements() + index + 4);
            m8 = _mm_loadu_pd(b.elements() + index + 6);

            m1 = _mm_add_pd(m1, m2);
            m3 = _mm_add_pd(m3, m4);
            m5 = _mm_add_pd(m5, m6);
            m7 = _mm_add_pd(m7, m8);

            _mm_stream_pd(a.elements() + index, m1);
            _mm_stream_pd(a.elements() + index + 2, m3);
            _mm_stream_pd(a.elements() + index + 4, m5);
            _mm_stream_pd(a.elements() + index + 6, m7);
        }
    }

    for (unsigned long index = 0 ; index < quad_start ; index++)
    {
        a.elements()[index] += b.elements()[index];
    }
    for (unsigned long index = quad_end ; index < a.size() ; index++)
    {
        a.elements()[index] += b.elements()[index];
    }
    return a;
}
