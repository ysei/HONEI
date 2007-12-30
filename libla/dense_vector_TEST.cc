/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Danny van Dyk <danny.dyk@uni-dortmund.de>
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

#include <libla/dense_vector.hh>
#include <libla/sparse_vector.hh>
#include <libla/algorithm.hh>
#include <unittest/unittest.hh>

#include <string>


using namespace honei;
using namespace tests;

template <typename DataType_>
class DenseVectorCreationTest :
    public BaseTest
{
    public:
        DenseVectorCreationTest(const std::string & type) :
            BaseTest("dense_vector_creation_test<" + type + ">")
        {
        }

        virtual void run() const
        {
            for (unsigned long size(10) ; size < (1 << 8) ; size <<= 1)
            {
                DenseVector<DataType_> dv(size, DataType_(0));
                TEST_CHECK(true);
            }
        }
};
DenseVectorCreationTest<float> dense_vector_creation_test_float("float");
DenseVectorCreationTest<double> dense_vector_creation_test_double("double");

template <typename DataType_>
class DenseVectorCopyTest :
    public BaseTest
{
    public:
        DenseVectorCopyTest(const std::string & type) :
            BaseTest("dense_vector_copy_test<" + type + ">")
        {
        }

        virtual void run() const
        {
            for (unsigned long size(10) ; size < (1 << 8) ; size <<= 1)
            {
                DenseVector<DataType_> dv1(size, static_cast<DataType_>(0)), dv2(size, static_cast<DataType_>(1));
                std::tr1::shared_ptr<DenseVector<DataType_> > c(new DenseVector<DataType_>(dv1.copy()));

                for (typename Vector<DataType_>::ElementIterator i(c->begin_elements()), i_end(c->end_elements()) ;
                        i != i_end ; ++i)
                {
                    TEST_CHECK_EQUAL_WITHIN_EPS(*i, 0, std::numeric_limits<DataType_>::epsilon());
                    *i = 1;
                }

                for (typename Vector<DataType_>::ConstElementIterator i(dv1.begin_elements()),
                        i_end(dv1.end_elements()) ; i != i_end ; ++i)
                {
                    TEST_CHECK_EQUAL_WITHIN_EPS(*i, 0, std::numeric_limits<DataType_>::epsilon());
                }

                for (typename Vector<DataType_>::ConstElementIterator i(dv2.begin_elements()),
                        i_end(dv2.end_elements()) ; i != i_end ; ++i)
                {
                    TEST_CHECK_EQUAL_WITHIN_EPS(*i, 1, std::numeric_limits<DataType_>::epsilon());
                }
            }
        }
};
DenseVectorCopyTest<float> dense_vector_copy_test_float("float");
DenseVectorCopyTest<double> dense_vector_copy_test_double("double");

template <typename DataType_>
class DenseVectorDensifyQuickTest :
    public QuickTest
{
    public:
        DenseVectorDensifyQuickTest(const std::string & type) :
            QuickTest("dense_vector_densify_quick_test<" + type + ">")
        {
        }

        virtual void run() const
        {
                unsigned long size(4711);
                DenseVector<DataType_> dv0(size, DataType_(0));
                SparseVector<DataType_> sv0(size, size / 8 + 1);

                for (typename Vector<DataType_>::ElementIterator i(dv0.begin_elements()), j(sv0.begin_elements()),
                    i_end(dv0.end_elements()) ; i != i_end ; ++i , ++j)
                {
                    if (i.index() % 7 == 0)
                    {
                        *i = i.index();
                        *j = i.index();
                    }
                }
                DenseVector<DataType_> dv1(sv0);

                TEST_CHECK_EQUAL(dv1, dv0);

            }
};
DenseVectorDensifyQuickTest<float> dense_vector_densify_quick_test_float("float");
DenseVectorDensifyQuickTest<double> dense_vector_densify_quick_test_double("double");

template <typename DataType_>
class DenseVectorEqualityTest :
    public BaseTest
{
    public:
        DenseVectorEqualityTest(const std::string & type) :
            BaseTest("dense_vector_equality_test<" + type + ">")
        {
        }

        virtual void run() const
        {
            for (unsigned long size(10) ; size < (1 << 8) ; size <<= 1)
            {
                DenseVector<DataType_> dv0(size, DataType_(1.23456));
                DenseVector<DataType_> dv1(size, DataType_(1.23456));

                for (typename Vector<DataType_>::ElementIterator i(dv0.begin_elements()), j(dv1.begin_elements()),
                    i_end(dv0.end_elements()) ; i != i_end ; ++i , ++j)
                {
                    TEST_CHECK_EQUAL_WITHIN_EPS(*i, *j, std::numeric_limits<DataType_>::epsilon());
                }

                TEST_CHECK_EQUAL(dv0, dv1);

                DenseVector<DataType_> dv2(size + 1);
                TEST_CHECK_THROWS(dv0 == dv2, VectorSizeDoesNotMatch);
            }
        }
};
DenseVectorEqualityTest<float> dense_vector_equality_test_float("float");
DenseVectorEqualityTest<double> dense_vector_equality_test_double("double");

template <typename DataType_>
class DenseVectorFunctionsTest :
    public BaseTest
{
    public:
        DenseVectorFunctionsTest(const std::string & type) :
            BaseTest("dense_vector_functions_test<" + type + ">")
        {
        }

        virtual void run() const
        {
            for (unsigned long size(10) ; size < (1 << 8) ; size <<= 1)
            {
                DenseVector<DataType_> dv(size);
                for (typename Vector<DataType_>::ElementIterator i(dv.begin_elements()), i_end(dv.end_elements()) ;
                        i != i_end ; ++i)
                {
                    *i = static_cast<DataType_>((i.index() + 1) / 1.23456789);
                }
                TEST_CHECK_EQUAL(dv.size(), size);

                for (int i=0 ; i<size ; ++i)
                {
                    DataType_ s((i+1)/1.23456789);
                    TEST_CHECK_EQUAL_WITHIN_EPS((dv)[i], s,
                        std::numeric_limits<DataType_>::epsilon());
                }
                for (int i=0 ; i<size ; ++i)
                {
                    DataType_ s((i+5)/1.23456789);
                    (dv)[i] = s;
                    TEST_CHECK_EQUAL_WITHIN_EPS((dv)[i], s,
                        std::numeric_limits<DataType_>::epsilon());
                }

            }
        }
};

DenseVectorFunctionsTest<float> dense_vector_functions_test_float("float");
DenseVectorFunctionsTest<double> dense_vector_functions_test_double("double");

template <typename DataType_>
class DenseVectorQuickTest :
    public QuickTest
{
    public:
        DenseVectorQuickTest(const std::string & type) :
            QuickTest("dense_vector_quick_test<" + type + ">")
        {
        }

        virtual void run() const
        {
            DenseVector<DataType_> dv(4711, DataType_(123.987));
            TEST_CHECK_EQUAL(dv.size(), 4711);
            TEST_CHECK_EQUAL(dv, dv);
            TEST_CHECK_EQUAL_WITHIN_EPS((dv)[4710] , 123.987, sqrt(std::numeric_limits<DataType_>::epsilon()));
            DataType_ s = DataType_(1.2345);
            (dv)[333] = s;
            TEST_CHECK_EQUAL_WITHIN_EPS((dv)[333] , s, sqrt(std::numeric_limits<DataType_>::epsilon()));

        #if 0
            DenseVector<DataType_> dv2(10);
            for (int i = 0 ; i < 10 ; ++i)
            {
                dv2[i] = DataType_(i);
            }
            DenseVector<DataType_> dv3(5);
            for (int i = 0 ; i < 5 ; ++i)
            {
                dv3[i] = DataType_(i + 3);
            }
            DenseVector<DataType_> dv4(dv2, 3, 5);
            TEST_CHECK_EQUAL(dv3, dv4);
        #endif
        }
};
DenseVectorQuickTest<float>  dense_vector_quick_test_float("float");
DenseVectorQuickTest<double> dense_vector_quick_test_double("double");

class DenseVectorConvertTest :
    public BaseTest
{
    public:
        DenseVectorConvertTest() :
            BaseTest("dense_vector_convert_test")
        {
        }

        virtual void run() const
        {
            for (unsigned long size(10) ; size < (1 << 8) ; size <<= 1)
            {
                DenseVector<float> dvf1(size), dvf2(size), dvfr(size);
                DenseVector<double> dvd1(size), dvd2(size), dvdr(size);
                Vector<float>::ElementIterator fr(dvfr.begin_elements());
                Vector<double>::ElementIterator dr(dvdr.begin_elements());
                Vector<float>::ElementIterator f1(dvf1.begin_elements());
                Vector<double>::ElementIterator d1(dvd1.begin_elements());
                for (Vector<float>::ElementIterator i_end(dvf1.end_elements()) ; f1 != i_end ; ++fr, ++dr, ++f1, ++d1)
                {
                    *fr = float(fr.index()) / 1.1234;
                    *dr = double(fr.index()) / 1.1234;
                    *f1 = float(fr.index()) / 1.1234;
                    *d1 = double(fr.index()) / 1.1234;
                }
                convert(dvf2, dvd1);
                convert(dvd2, dvf1);
                TEST_CHECK_EQUAL(dvf2, dvfr);
                for (Vector<double>::ConstElementIterator i(dvd2.begin_elements()), r(dvdr.begin_elements()), i_end(dvd2.end_elements())
                        ; i != i_end ; ++i, ++r)
                {
                    TEST_CHECK_EQUAL_WITHIN_EPS(*i, *r, sqrt(std::numeric_limits<float>::epsilon()));
                }
            }
            DenseVector<double> dvf01(3), dvf02(3);
            DenseVector<double> dvd01(2), dvd02(4);
            TEST_CHECK_THROWS(convert(dvf01, dvd01), VectorSizeDoesNotMatch);
            TEST_CHECK_THROWS(convert(dvd02, dvf02), VectorSizeDoesNotMatch);
        }
} dense_vector_convert_test;

class DenseVectorConvertQuickTest :
    public QuickTest
{
    public:
        DenseVectorConvertQuickTest() :
            QuickTest("dense_vector_convert_quick_test")
        {
        }

        virtual void run() const
        {
            unsigned long size(4711);
            DenseVector<float> dvf1(size), dvf2(size), dvfr(size);
            DenseVector<double> dvd1(size), dvd2(size), dvdr(size);
            Vector<float>::ElementIterator fr(dvfr.begin_elements());
            Vector<double>::ElementIterator dr(dvdr.begin_elements());
            Vector<float>::ElementIterator f1(dvf1.begin_elements());
            Vector<double>::ElementIterator d1(dvd1.begin_elements());
            for (Vector<float>::ElementIterator i_end(dvf1.end_elements()) ; f1 != i_end ; ++fr, ++dr, ++f1, ++d1)
            {
                *fr = float(fr.index()) / 1.1234;
                *dr = double(fr.index()) / 1.1234;
                *f1 = float(fr.index()) / 1.1234;
                *d1 = double(fr.index()) / 1.1234;
            }
            convert(dvf2, dvd1);
            convert(dvd2, dvf1);
            TEST_CHECK_EQUAL(dvf2, dvfr);
            for (Vector<double>::ConstElementIterator i(dvd2.begin_elements()), r(dvdr.begin_elements()), i_end(dvd2.end_elements())
                    ; i != i_end ; ++i, ++r)
            {
                TEST_CHECK_EQUAL_WITHIN_EPS(*i, *r, sqrt(std::numeric_limits<float>::epsilon()));
            }
            DenseVector<double> dvf01(3), dvf02(3);
            DenseVector<double> dvd01(2), dvd02(4);
            TEST_CHECK_THROWS(convert(dvf01, dvd01), VectorSizeDoesNotMatch);
            TEST_CHECK_THROWS(convert(dvd02, dvf02), VectorSizeDoesNotMatch);
        }
} dense_vector_convert_quick_test;
