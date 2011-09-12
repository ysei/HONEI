/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Dirk Ribbrock <dirk.ribbrock@uni-dortmund.de>
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

#include <honei/la/dense_matrix.hh>
#include <honei/woolb3/grid3.hh>
#include <honei/woolb3/packed_grid3.hh>
#include <honei/util/unittest.hh>

#include <iostream>

using namespace honei;
using namespace tests;
using namespace std;

template <typename Tag_, typename DataType_>
class PackedGrid3Test :
    public QuickTaggedTest<Tag_>
{
    public:
        PackedGrid3Test(const std::string & type) :
            QuickTaggedTest<Tag_>("packed grid_quick_test<" + type + ">")
        {
        }

        virtual void run() const
        {
            DenseMatrix<bool> geometry(4, 4, false);
            DenseMatrix<DataType_> h(4, 4, 1);
            DenseMatrix<DataType_> b(4, 4, 1);
            DenseMatrix<DataType_> u(4, 4, 1);
            DenseMatrix<DataType_> v(4, 4, 1);
            geometry(1,1) = true;
            geometry(2,3) = true;
            Grid3<DataType_, 9>::print_numbering(geometry, "z-curve");
            std::cout<<geometry;
            Grid3<DataType_, 9> grid(geometry, h, b, u, v);

            for (unsigned long i(0) ; i < grid.size() ; ++i)
            {
                std::cout<<grid.get_cell(i)->get_id()<<"("<<grid.get_cell(i)->get_y()<<"/"<<grid.get_cell(i)->get_x()<<")"<<":"<<std::endl;
                for (unsigned long j(0) ; j < 9 ; ++j)
                {
                    std::cout<<j<<": ";
                    if (grid.get_cell(i)->get_neighbours(j).size() > 0)
                        std::cout << grid.get_cell(i)->get_neighbours(j).front()->get_id()<<
                            "("<<grid.get_cell(i)->get_neighbours(j).front()->get_y()<<"/"<<grid.get_cell(i)->get_neighbours(j).front()->get_x()<<")";
                    std::cout<<" | ";
                }
                std::cout<<endl;
            }
            std::cout<<endl;

            PackedGrid3<DataType_, 9> pgrid(grid);

            std::cout<<"raw dir vectors: "<<std::endl;
            for (unsigned long i(0) ; i < 9 ; ++i)
                std::cout<<i<<": "<<*(pgrid.neighbours[i]);

            std::cout<<std::endl<<"dir and dir index vectors:"<<std::endl;
            for (unsigned long i(0) ; i < 9 ; ++i)
            {
                std::cout<<i<<": "<<std::endl;
                std::cout<<*(pgrid.dir[i]);
                std::cout<<*(pgrid.dir_index[i]);
            }
        }
};
PackedGrid3Test<tags::CPU, double> packed_grid_test("double");