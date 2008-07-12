/* vim: set sw=4 sts=4 et nofoldenable : */

/*
 * Copyright (c) 2007 Danny van Dyk <danny.dyk@uni-dortmund.de>
 * Copyright (c) 2007 Michael Abshoff <michael.abshoff@fsmath.mathematik.uni-dortmund.de>
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

#ifndef LIBLA_GUARD_DENSE_MATRIX_HH
#define LIBLA_GUARD_DENSE_MATRIX_HH 1

#include <honei/la/element_iterator.hh>
#include <honei/la/matrix.hh>
#include <honei/la/dense_vector.hh>
#include <honei/la/dense_vector_range.hh>
#include <honei/la/dense_vector_slice.hh>
#include <honei/la/matrix_error.hh>
#include <honei/util/shared_array-impl.hh>
#include <honei/util/stringify.hh>
#include <honei/util/type_traits.hh>

#include <algorithm>
#include <cstring>
#include <iterator>

namespace honei
{
    // Forward declarations
    template <typename DataType_> class DenseMatrixTile;
    template <typename DataType_> class SparseMatrix;

    /**
     * DenseMatrix is a matrix with O(column * row) non-zero elements which keeps its data
     * aligned and continuous.
     *
     * \ingroup grpmatrix
     */
    template <typename DataType_> class DenseMatrix :
        public RowAccessMatrix<DataType_>,
        public MutableMatrix<DataType_>
    {
        private:
            /// Pointer to our elements.
            SharedArray<DataType_> _elements;

            /// Our columns.
            unsigned long _columns;

            /// Our rows.
            unsigned long _rows;

            /// Our row-vectors.
            SharedArray<std::tr1::shared_ptr<DenseVectorRange<DataType_> > > _row_vectors;

            /// Our column-vectors.
            SharedArray<std::tr1::shared_ptr<DenseVectorSlice<DataType_> > > _column_vectors;

            /// Our implementation of ElementIteratorBase.
            class DenseElementIterator;

            typedef typename Matrix<DataType_>::MatrixElementIterator MatrixElementIterator;

        public:
            friend class DenseElementIterator;
            friend class DenseMatrixTile<DataType_>;

            /// Type of the const iterator over our elements.
            typedef typename Matrix<DataType_>::ConstElementIterator ConstElementIterator;

            /// Type of the iterator over our elements.
            typedef typename MutableMatrix<DataType_>::ElementIterator ElementIterator;

            /// Type of the const iterator over our vectors.
            typedef VectorIteratorWrapper<DataType_, const DataType_> ConstVectorIterator;

            /// Type of the iterator over our vectors.
            typedef VectorIteratorWrapper<DataType_, DataType_> VectorIterator;

            /// \name Constructors
            /// \{

            /**
             * Constructor.
             *
             * \param columns Number of columns of the new matrix.
             * \param rows Number of rows of the new matrix.
             */
            DenseMatrix(unsigned long rows, unsigned long columns) :
                _elements(rows * columns),
                _columns(columns),
                _column_vectors(columns),
                _rows(rows),
                _row_vectors(rows)
            {
                CONTEXT("When creating DenseMatrix:");
                ASSERT(rows > 0, "number of rows is zero!");
                ASSERT(columns > 0, "number of columns is zero!");
            }

            /**
             * Constructor.
             *
             * \param columns Number of columns of the new matrix.
             * \param rows Number of rows of the new matrix.
             * \param value Default value of each of the new matrice's elements.
             */
            DenseMatrix(unsigned long rows, unsigned long columns, DataType_ value) :
                _elements(rows * columns),
                _columns(columns),
                _column_vectors(columns),
                _rows(rows),
                _row_vectors(rows)
            {
                CONTEXT("When creating DenseMatrix:");
                ASSERT(rows > 0, "number of rows is zero!");
                ASSERT(columns > 0, "number of columns is zero!");

                TypeTraits<DataType_>::fill(_elements.get(), _rows * _columns, value);
            }

            /**
             * Constructor.
             *
             * \param other The SparseMatrix to densify.
             */
            DenseMatrix(const SparseMatrix<DataType_> & other) :
                _elements(other.rows() * other.columns()),
                _columns(other.columns()),
                _column_vectors(other.columns()),
                _rows(other.rows()),
                _row_vectors(other.rows())
            {
                CONTEXT("When creating DenseMatrix form SparseMatrix:");

                TypeTraits<DataType_>::fill(_elements.get(), _rows * _columns, DataType_(0));

                for (typename Matrix<DataType_>::ConstElementIterator i(other.begin_non_zero_elements()),
                        i_end(other.end_non_zero_elements()) ; i != i_end ; ++i)
                {
                    (*this)(i.row(), i.column()) = *i;
                }
            }

            /**
             * Constructor
             *
             * Create a submatrix from a given source matrix.
             * \param source The source matrix.
             * \param column_offset The source matrix column offset.
             * \param columns Number of columns of the new matrix.
             * \param row_offset The source matrix row offset.
             * \param rows Number of rows of the new matrix.
             */
            DenseMatrix(const DenseMatrix<DataType_> & source, unsigned long column_offset, unsigned long columns,
                    unsigned long row_offset, unsigned long rows) :
                    _elements(columns * rows),
                    _columns(columns),
                    _column_vectors(columns),
                    _rows(rows),
                    _row_vectors(rows)
            {
                ASSERT(rows > 0, "number of rows is zero!");
                ASSERT(columns > 0, "number of columns is zero!");

                if (column_offset + columns > source.columns())
                {
                    throw MatrixColumnsDoNotMatch(column_offset + columns, source.columns());
                }

                if (row_offset + rows > source.rows())
                {
                    throw MatrixRowsDoNotMatch(row_offset + rows, source.rows());
                }

                for (unsigned long i = 0 ; i < rows ; ++i)
                {
                    for (unsigned long j = 0; j < columns ; ++j)
                    {
                        _elements[j + columns * i] = source._elements[j + column_offset  +
                            ((i + row_offset) * source.columns())];
                    }
                }
            }
            /// \}

            /// Returns iterator pointing to the first element of the matrix.
            virtual ConstElementIterator begin_elements() const
            {
                return ConstElementIterator(new DenseElementIterator(*this, 0));
            }

            /// Returns iterator pointing to a given element of the matrix.
            virtual ConstElementIterator element_at(unsigned long index) const
            {
                 return ConstElementIterator(new DenseElementIterator(*this, index));
            }

            /// Returns iterator pointing behind the last element of the matrix.
            virtual ConstElementIterator end_elements() const
            {
                return ConstElementIterator(new DenseElementIterator(*this, _rows * _columns));
            }

            /// Returns iterator pointing to the first element of the matrix.
            virtual ElementIterator begin_elements()
            {
                return ElementIterator(new DenseElementIterator(*this, 0));
            }

            /// Returns iterator pointing to a given element of the matrix.
            virtual ElementIterator element_at(unsigned long index)
            {
                 return ElementIterator(new DenseElementIterator(*this, index));
            }

           /// Returns iterator pointing behind the last element of the matrix.
            virtual ElementIterator end_elements()
            {
                return ElementIterator(new DenseElementIterator(*this, _rows * _columns));
            }

            /// Returns the number of our columns.
            inline virtual unsigned long columns() const
            {
                return _columns;
            }

            /// Returns the number of our rows.
            inline virtual unsigned long rows() const
            {
                return _rows;
            }

            /// Returns the number of our elements.
            inline virtual unsigned long size() const
            {
                return _rows * _columns;
            }

            /// Retrieves row vector by index, zero-based, unassignable.
            virtual const DenseVectorRange<DataType_> & operator[] (unsigned long row) const
            {
                if (! _row_vectors[row])
                    _row_vectors[row].reset(new DenseVectorRange<DataType_>(_elements, _columns, row * _columns));

                return *_row_vectors[row];
            }

            /// Retrieves row vector by index, zero-based, assignable.
            virtual DenseVectorRange<DataType_> & operator[] (unsigned long row)
            {
                if (! _row_vectors[row])
                    _row_vectors[row].reset(new DenseVectorRange<DataType_>(_elements, _columns, row * _columns));

                return *_row_vectors[row];
            }

            /// Retrieves element at (row, column), unassignable.
            inline virtual const DataType_ & operator() (unsigned long row, unsigned long column) const
            {
                CONTEXT("When retrieving DenseMatrix element, unassignable:");
                ASSERT(row < _rows && row >= 0, "row index is out of bounds!");
                ASSERT(column < _columns && column >= 0, "column number is out of bounds!");
                return _elements.get()[column + row * _columns];
            }

            /// Retrieves element at (row, column), assignable.
            inline virtual DataType_ & operator() (unsigned long row, unsigned long column)
            {
                CONTEXT("When retrieving DenseMatrix element, assignable:");
                ASSERT(row < _rows && row >= 0, "row index is out of bounds!");
                ASSERT(column < _columns && column >= 0, "column number is out of bounds!");
                return _elements.get()[column + row * _columns];
            }

            /// Retrieves column vector by index, zero-based, unassignable.
            virtual const DenseVectorSlice<DataType_> & column(unsigned long column) const
            {
                if (! _column_vectors[column])
                    _column_vectors[column].reset(new DenseVectorSlice<DataType_>(_elements, _rows,  column, _columns));

                return *_column_vectors[column];
            }

            /// Retrieves column vector by index, zero-based, assignable.
            virtual DenseVectorSlice<DataType_> & column(unsigned long column)
            {
                if (! _column_vectors[column])
                    _column_vectors[column].reset(new DenseVectorSlice<DataType_>(_elements, _rows, column, _columns));

                return *_column_vectors[column];
            }

            /// Returns a pointer to our data array.
            inline DataType_ * elements() const
            {
                return _elements.get();
            }

            /// Return the address of our data
            inline void * address() const
            {
                return _elements.get();
            }

            /// Return our memory id
            inline unsigned long memid() const
            {
                return (unsigned long)_elements.get();
            }

            /// Request a memory access lock for our data.
            void * lock(LockMode mode, tags::TagValue memory = tags::CPU::memory_value) const
            {
                return MemoryArbiter::instance()->lock(mode, memory, this->memid(), this->address(), this->size() * sizeof(DataType_));
            }

            /// Release a memory access lock for our data.
            void unlock(LockMode mode) const
            {
                MemoryArbiter::instance()->unlock(mode, this->memid());
            }

            /// Returns a copy of the matrix.
            DenseMatrix copy() const
            {
                DenseMatrix result(_rows, _columns);
                this->lock(lm_read_only);

                TypeTraits<DataType_>::copy(_elements.get(), result._elements.get(), _columns * _rows);

                this->unlock(lm_read_only);
                return result;
            }
    };

    /**
     * \brief DenseMatrix::DenseElementIterator is a simple iterator implementation for dense matrices.
     *
     * \ingroup grpmatrix
     */
    template <> template <typename DataType_> class DenseMatrix<DataType_>::DenseElementIterator :
        public MatrixElementIterator
    {
        private:
            /// Our parent matrix.
            const DenseMatrix<DataType_> & _matrix;

            /// Our index.
            unsigned long _index;

        public:
            /// \name Constructors
            /// \{

            /**
             * Constructor.
             *
             * \param matrix The parent matrix that is referenced by the iterator.
             * \param index The index into the matrix.
             */
            DenseElementIterator(const DenseMatrix<DataType_> & matrix, unsigned long index) :
                _matrix(matrix),
                _index(index)
            {
            }

            /// Copy-constructor.
            DenseElementIterator(DenseElementIterator const & other) :
                _matrix(other._matrix),
                _index(other._index)
            {
            }

            /// \}

            /// \name Forward iterator interface
            /// \{

            /// Preincrement operator.
            virtual DenseElementIterator & operator++ ()
            {
                CONTEXT("When incrementing iterator by one:");

                ++_index;

                return *this;
            }

            /// In-place-add operator.
            virtual DenseElementIterator & operator+= (const unsigned long step)
            {
                CONTEXT("When incrementing iterator by '" + stringify(step) + "':");

                _index += step;

                return *this;
            }

            /// Dereference operator that returns assignable reference.
            virtual DataType_ & operator* ()
            {
                CONTEXT("When accessing assignable element at index '" + stringify(_index) + "':");

                return _matrix._elements[_index];
            }

            /// Dereference operator that returns umassignable reference.
            virtual const DataType_ & operator* () const
            {
                CONTEXT("When accessing unassignable element at index '" + stringify(_index) + "':");

                return _matrix._elements[_index];
            }

            /// Comparison operator for less-than.
            virtual bool operator< (const IteratorBase<DataType_, Matrix<DataType_> > & other) const
            {
                return _index < other.index();
            }

            /// Comparison operator for equality.
            virtual bool operator== (const IteratorBase<DataType_, Matrix<DataType_> > & other) const
            {
                return ((&_matrix == other.parent()) && (_index == other.index()));
            }

            /// Comparison operator for inequality.
            virtual bool operator!= (const IteratorBase<DataType_, Matrix<DataType_> > & other) const
            {
                return ((&_matrix != other.parent()) || (_index != other.index()));
            }

            /// \}

            /// \name IteratorTraits interface
            /// \{

            /// Returns our index.
            virtual unsigned long index() const
            {
                return _index;
            }

            /// Returns our column index.
            virtual unsigned long column() const
            {
                return _index % _matrix._columns;
            }

            /// Returns our row index.
            virtual unsigned long row() const
            {
                return _index / _matrix._columns;
            }

            /// Returns a pointer to our parent container.
            virtual const Matrix<DataType_> * parent() const
            {
                return &_matrix;
            }

            /// \}
    };
}

#endif
