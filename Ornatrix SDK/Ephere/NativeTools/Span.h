// Must compile with VC 2012 / GCC 4.8 (partial C++11)

#pragma once

#include "Ephere/NativeTools/Asserts.h"

#include <array>

namespace Ephere
{
	/*! A much simpler variant of GSL's dynamic span<T> class. To be used mostly as a function parameter passed by value */
	template <typename T>
	struct Span
	{
		typedef T element_type;
		typedef typename std::remove_cv<T>::type value_type;
		typedef int size_type;
		typedef size_type index_type;
		typedef T* pointer;
		typedef T& reference;

		typedef T* iterator;
		typedef T* const_iterator;

		Span() :
			data_( nullptr ),
			count_( 0 )
		{
		}

		Span( T* data, size_type count )
			: data_( data ),
			count_( count )
		{
			ASSERT_RANGE( data == nullptr ? size() == 0 : size() >= 0 );
		}

		Span( T* begin, T* end )
			: data_( begin )
		{
			auto distance = end - begin;
			ASSERT_RANGE( distance >= 0 );
			count_ = size_type( distance );
		}

		template <std::size_t N>
		Span( element_type( &array )[N] )
			: data_( std::addressof( array[0] ) ),
			count_( size_type( N ) )
		{
		}

#if !defined(_MSC_VER) || _MSC_VER >= 1900
		template <std::size_t N, class ArrayElementType = typename std::remove_const<element_type>::type>
		Span( std::array<ArrayElementType, N>& array )
			: data_( array.data() ),
			count_( size_type( N ) )
		{
		}

		template <class Container,
				class = typename std::enable_if<
				std::is_convertible<typename Container::pointer, pointer>::value &&
				std::is_convertible<typename Container::pointer,
				decltype( std::declval<Container>().data() )>::value>::type>
		Span( Container& cont )
			: Span( cont.data(), size_type( cont.size() ) )
		{
		}

		template <class Container,
				class = typename std::enable_if<
				std::is_convertible<typename Container::pointer, pointer>::value &&
				std::is_convertible<typename Container::pointer,
				decltype( std::declval<Container>().data() )>::value>::type>
		Span( const Container& cont )
			: Span( cont.data(), size_type( cont.size() ) )
		{
			static_assert( std::is_const<element_type>::value, "Span element type must be const" );
		}
#else
		template <std::size_t N, class ArrayElementType>
		Span( std::array<ArrayElementType, N>& array )
			: data_( array.data() ),
			count_( size_type( N ) )
		{
		}

		template <class Container>
		Span( Container& cont )
			: data_( cont.data() ),
			count_( size_type( cont.size() ) )
		{
		}

		template <class Container>
		Span( const Container& cont )
			: data_( cont.data() ),
			count_( size_type( cont.size() ) )
		{
			static_assert( std::is_const<element_type>::value, "Span element type must be const" );
		}
#endif

		size_type size() const
		{
			return count_;
		}

		bool empty() const
		{
			return count_ == 0;
		}

		T* begin() const
		{
			return data_;
		}

		T* end() const
		{
			return data_ + count_;
		}

		T* data() const
		{
			return data_;
		}

		T& operator[]( size_type index ) const
		{
			ASSERT_RANGE( 0 <= index && index < size() );
			return data_[index];
		}

		Span<element_type> subspan( size_type offset, size_type count = -1 ) const
		{
			ASSERT_RANGE( offset >= 0 && size() - offset >= 0 );

			if( count < 0 )
			{
				return Span<element_type>( data() + offset, size() - offset );
			}

			ASSERT_RANGE( size() - offset >= count );
			return Span<element_type>( data() + offset, count );
		}

	private:

		T* data_;
		size_type count_;
	};
}
