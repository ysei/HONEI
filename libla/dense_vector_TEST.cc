/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include <libla/dense_vector.hh>
#include <unittest/unittest.hh>

#include <string>

using namespace pg512;

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
            for (unsigned long size(10) ; size < (1 << 10) ; size <<= 1)
            {
                std::tr1::shared_ptr<DenseVector<DataType_> > dm(new DenseVector<DataType_>(size,
                            static_cast<DataType_>(0)));
                TEST_CHECK(true);
            }
        }
};

DenseVectorCreationTest<float> dense_vector_creation_test_float("float");
DenseVectorCreationTest<double> dense_vector_creation_test_double("double");

template <typename DataType_>
class DenseVectorEquationTest :
    public BaseTest
{
    public:
        DenseVectorEquationTest(const std::string & type) :
            BaseTest("dense_vector_creation_test<" + type + ">")
        {
        }

        virtual void run() const
        {
            for (unsigned long size(10) ; size < (1 << 10) ; size <<= 1)
            {
                std::tr1::shared_ptr<DenseVector<DataType_> > dv0(new DenseVector<DataType_>(size,
                    static_cast<DataType_>(4711)));
                std::tr1::shared_ptr<DenseVector<DataType_> > dv1(new DenseVector<DataType_>(size,
                    static_cast<DataType_>(4711)));
                    
                TEST_CHECK(dv0==dv1);
                TEST_CHECK_EQUAL(dv0->size(),size);
            }
        }
};

DenseVectorEquationTest<float> dense_vector_equation_test_float("float");
DenseVectorEquationTest<double> dense_vector_equation_test_double("double");



