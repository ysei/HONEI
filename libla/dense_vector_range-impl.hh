/* vim: set sw=4 sts=4 et nofoldenable : */

/*
 * Copyright (c) 2007 Volker Jung <volker.jung@uni-dortmund.de>
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

#ifndef LIBLA_GUARD_DENSE_VECTOR_RANGE_IMPL_HH
#define LIBLA_GUARD_DENSE_VECTOR_RANGE_IMPL_HH 1

#include <libla/dense_vector.hh>
#include <libla/dense_vector-impl.hh>
#include <libla/dense_vector_range.hh>
#include <libla/element_iterator.hh>
#include <libutil/assertion.hh>
#include <libutil/shared_array.hh>
#include <libutil/stringify.hh>

#include <algorithm>
#include <string>

/**
 * \file
 *
 * Implementation of DenseVectorRange and related classes.
 *
 * \ingroup grpvector
 **/
namespace honei
{
    /**
     * \brief DenseVectorRange::Implementation is the private implementation class for DenseVectorRange.
     *
     * \ingroup grpvector
     */
    template <typename DataType_> class DenseVectorRange<DataType_>::Implementation
    {
        private:
            /// Unwanted copy-constructor: Do not implement. See EffC++, Item 27.
            Implementation(const Implementation &);

            /// Unwanted assignment operator: Do not implement. See EffC++, Item 27.
            Implementation & operator= (const Implementation &);

        public:
            /// Our elements.
            SharedArray<DataType_> elements;

            /// Our size.
            const unsigned long size;

            /// Our offset.
            const unsigned long offset;

            /// Our stepsize.
            const unsigned long stepsize;

            /// Constructor.
            Implementation(const SharedArray<DataType_> & e, unsigned long s, unsigned long o) :
                elements(e),
                size(s),
                offset(o),
                stepsize(1)
            {
            }
    };

    template <typename DataType_>
    DenseVectorRange<DataType_>::DenseVectorRange(const DenseVector<DataType_> & source, const unsigned long size,
            const unsigned long offset) :
        _imp(new Implementation(source._imp->elements, size, offset))
    {
        CONTEXT("When creating DenseVectorRange:");
        ASSERT(size > 0, "size is zero!");
    }

    template <typename DataType_>
    DenseVectorRange<DataType_>::DenseVectorRange(const DenseVectorRange<DataType_> & other) :
        _imp(new Implementation(other._imp->elements, other._imp->size, other._imp->offset))
    {
    }

    template <typename DataType_>
    typename Vector<DataType_>::ConstElementIterator DenseVectorRange<DataType_>::begin_elements() const
    {
        return ConstElementIterator(new DenseElementIterator<DataType_>(*this, 0));
    }

    template <typename DataType_>
    typename Vector<DataType_>::ConstElementIterator DenseVectorRange<DataType_>::end_elements() const
    {
        return ConstElementIterator(new DenseElementIterator<DataType_>(*this, _imp->size));
    }

    template <typename DataType_>
    typename Vector<DataType_>::ConstElementIterator DenseVectorRange<DataType_>::element_at(unsigned long index) const
    {
        return ConstElementIterator(new DenseElementIterator<DataType_>(*this, index));
    }

    template <typename DataType_>
    typename Vector<DataType_>::ElementIterator DenseVectorRange<DataType_>::begin_elements()
    {
        return ElementIterator(new DenseElementIterator<DataType_>(*this, 0));
    }

    template <typename DataType_>
    typename Vector<DataType_>::ElementIterator DenseVectorRange<DataType_>::end_elements()
    {
        return ElementIterator(new DenseElementIterator<DataType_>(*this, _imp->size));
    }

    template <typename DataType_>
    typename Vector<DataType_>::ElementIterator DenseVectorRange<DataType_>::element_at(unsigned long index)
    {
        return ElementIterator(new DenseElementIterator<DataType_>(*this, index));
    }

    template <typename DataType_>
    unsigned long DenseVectorRange<DataType_>::size() const
    {
        return _imp->size;
    }

    template <typename DataType_>
    const DataType_ & DenseVectorRange<DataType_>::operator[] (unsigned long index) const
    {
        return _imp->elements[index + _imp->offset];
    }

    template <typename DataType_>
    DataType_ & DenseVectorRange<DataType_>::operator[] (unsigned long index)
    {
        return _imp->elements[index + _imp->offset];
    }

    template <typename DataType_>
    inline DataType_ * DenseVectorRange<DataType_>::elements() const
    {
        return _imp->elements.get() + _imp->offset;
    }

    template <typename DataType_>
    DenseVector<DataType_> DenseVectorRange<DataType_>::copy() const
    {
        DenseVector<DataType_> result(_imp->size);
        DataType_ * source(_imp->elements.get());
        DataType_ * target(result.elements());
        for (unsigned long i(0) ; i < _imp->size ; i++)
        {
            target[i] = source[_imp->stepsize * i + _imp->offset];
        }

        /// \todo: Use TypeTraits<DataType_>::copy()

        return result;
    }


    /**
     * \brief DenseVectorRange::DenseElementIterator is a simple iterator implementation for dense vector ranges.
     *
     * \ingroup grpvector
     **/
    template <> template <typename DataType_> class DenseVectorRange<DataType_>::DenseElementIterator<DataType_> :
        public VectorElementIterator
        {
            private:
                /// Our parent range.
                const DenseVectorRange<DataType_> & _vector;

                /// Our index.
                unsigned long _index;

            public:
                /// \name Constructors
                /// \{

                /**
                 * Constructor.
                 *
                 * \param vector The parent vector that is referenced by the iterator.
                 * \param index The index into the vector.
                 **/
                DenseElementIterator(const DenseVectorRange<DataType_> & vector, unsigned long index) :
                    _vector(vector),
                    _index(index)
                {
                }

                /// Copy-constructor.
                DenseElementIterator(DenseElementIterator<DataType_> const & other) :
                    _vector(other._vector),
                    _index(other._index)
                {
                }

                /// Destructor.
                virtual ~DenseElementIterator()
                {
                }

                /// \}

                /// \name Forward iterator interface
                /// \{

                /// Preincrement operator.
                virtual DenseElementIterator<DataType_> & operator++ ()
                {
                    CONTEXT("When incrementing iterator by one:");

                    ++_index;

                    return *this;
                }

                /// In-place-add operator.
                virtual DenseElementIterator<DataType_> & operator+= (const unsigned long step)
                {
                    CONTEXT("When incrementing iterator by '" + stringify(step) + "':");

                    _index += step;

                    return *this;
                }

                /// Dereference operator that returns an assignable reference.
                virtual DataType_ & operator* ()
                {
                    CONTEXT("When accessing assignable element at index '" + stringify(_index) + "':");

                    return _vector._imp->elements[_index + _vector._imp->offset];
                }

                /// Dereference operator that returns an unassignable reference.
                virtual const DataType_ & operator* () const
                {
                    CONTEXT("When accessing unassignable element at index '" + stringify(_index) + "':");

                    return _vector._imp->elements[_index + _vector._imp->offset];
                }

                /// Less-than operator.
                virtual bool operator< (const IteratorBase<DataType_, Vector<DataType_> > & other) const
                {
                    return _index < other.index();
                }

                /// Equality operator.
                virtual bool operator== (const IteratorBase<DataType_, Vector<DataType_> > & other) const
                {
                    return ((&_vector == other.parent()) && (_index == other.index()));
                }

                /// Inequality operator.
                virtual bool operator!= (const IteratorBase<DataType_, Vector<DataType_> > & other) const
                {
                    return ((&_vector != other.parent()) || (_index != other.index()));
                }

                /// \}

                /// \name IteratorTraits interface
                /// \{

                /// Returns our index.
                virtual unsigned long index() const
                {
                    return _index;
                }

                /// Returns a pointer to our parent container.
                virtual const Vector<DataType_> * parent() const
                {
                    return &_vector;
                }

                /// \}
        };


}

#endif
