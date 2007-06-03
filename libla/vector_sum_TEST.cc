/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include <libla/dense_vector.hh>
#include <libla/vector_sum.hh>
#include <libla/vector_norm.hh>
#include <unittest/unittest.hh>

#include <limits>
#include <tr1/memory>
#include <iostream>

using namespace pg512;

template <typename DataType_>
class DenseVectorAdditionTest :
    public BaseTest
{
    public:
        DenseVectorAdditionTest(const std::string & type) :
            BaseTest("dense_vector_addition_test<" + type + ">")
        {
        }

        virtual void run() const
        {
            for (unsigned long size(1) ; size < (1 << 14) ; size <<= 1)
            {
                std::tr1::shared_ptr<DenseVector<DataType_> > dv2(new DenseVector<DataType_>(size,
                    static_cast<DataType_>(1)));
                std::tr1::shared_ptr<DenseVector<DataType_> > dv1(new DenseVector<DataType_>(size,
                    static_cast<DataType_>(1)));

                DenseVector<DataType_> sum1(VectorAddition<DataType_>::value(*dv1,*dv2));
                DataType_ v1(VectorNorm<DataType_, vnt_l_one>::value(sum1));
                TEST_CHECK_EQUAL(v1,2*size);
                
            }
            
            std::tr1::shared_ptr<DenseVector<DataType_> > dv00(new DenseVector<DataType_>(1,
                    static_cast<DataType_>(1)));
            std::tr1::shared_ptr<DenseVector<DataType_> > dv01(new DenseVector<DataType_>(2,
                    static_cast<DataType_>(1)));
                    
            TEST_CHECK_THROWS(VectorAddition<DataType_>::value(*dv00,*dv01),VectorSizeDoesNotMatch);
        }
};

DenseVectorAdditionTest<float> dense_vector_addition_test_float("float");
DenseVectorAdditionTest<double> dense_scalar_product_double("double");                
