/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Markus Geveler <apryde@gmx.de>
 * Copyright (c) 2008 Dirk Ribbrock <dirk.ribbrock@uni-dortmund.de>
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

#include <honei/math/restriction.hh>
#include <unittest/unittest.hh>
#include <honei/util/stringify.hh>
#include <iostream>
#include <honei/la/dense_matrix.hh>
using namespace honei;
using namespace tests;
using namespace std;

template <typename Tag_, typename DT1_>
class RestrictionTest:
    public BaseTest
{
    public:
        RestrictionTest(const std::string & tag) :
            BaseTest("Restriction test <" + tag + ">")
        {
            register_tag(Tag_::name);
        }

        virtual void run() const
        {
            for (float level(0) ; level < 10 ; ++level)
            {
                unsigned long N_fine((unsigned long)pow((pow(2, level + 1) + 1), 2));
                //unsigned long width_fine = (unsigned long)sqrt((double)N_fine);
                unsigned long N_coarse((unsigned long)pow((pow(2, level) + 1), 2));
                //unsigned long width_coarse = (unsigned long)sqrt((double)N_coarse);

                DenseVector<DT1_> fine(N_fine, DT1_(1));
                DenseVector<DT1_> coarse(N_coarse, DT1_(4711));
                DenseVector<DT1_> coarse_ref(N_coarse, DT1_(4711));
                /*for (unsigned long i(0) ; i < fine.size() ; ++i)
                {
                    fine[i] = DT1_(i % 1000);
                }*/
                DenseVector<unsigned long> mask(8);
                for(unsigned long i(0) ; i < 8 ; ++i)
                {
                    mask[i] = 2;
                }

                Restriction<Tag_>::value(coarse, fine, mask);
                Restriction<tags::CPU>::value(coarse_ref, fine, mask);
                coarse.lock(lm_read_only);
                coarse_ref.lock(lm_read_only);
                std::cout << "At level: " << level + 1 << std::endl;
                for(unsigned long i(0) ; i < coarse.size() ; ++i)
                {
                    if (coarse[i] != coarse_ref[i])
                        std::cout << "Not matching: " << i << std::endl;
                }
                TEST_CHECK_EQUAL(coarse, coarse_ref);

                coarse.unlock(lm_read_only);
                coarse_ref.unlock(lm_read_only);
            }
        }
};
RestrictionTest<tags::CPU, float> restriction_test("float");
#ifdef HONEI_CUDA
RestrictionTest<tags::GPU::CUDA, float> cuda_restriction_test("float");
#endif
