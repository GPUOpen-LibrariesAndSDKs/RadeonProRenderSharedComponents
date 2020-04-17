#pragma once

#include "Matrix.h"

#include <algorithm>

namespace Ephere { namespace Geometry
{

//! Axis-aligned box
template <unsigned N, typename T>
class Box
{

	typedef	Matrix<N,1,T>	Vector;

public:

	//! No initialization
	Box()
	{
	}

	//! Construct with minimum and maximum coordinates
	Box( const Vector& pmin, const Vector& pmax ) :
	pmin_( pmin ),
	pmax_( pmax )
	{
	}


	/**
	Construct a box centered at a point with specified dimensions
	@param diameter If true dimensions are used as diamater, otherwise radius
	*/
	Box( const Vector& pcenter, const Vector& dim, bool diameter ) :
	pmin_( pcenter - ( diameter ? ( dim / T( 2 ) ) : ( dim ) ) ),
	pmax_( pcenter + ( diameter ? ( dim / T( 2 ) ) : ( dim ) ) )
	{
	}

	//! Return an empty box
	static Box Empty()
	{
		Box<N, T> result;
		result.setEmpty();
		return result;
	}

	//! Enlarge to include a given point
	void operator+=( const Vector& p )
	{
		for( unsigned i = 0; i < N; ++i )
		{
			pmin()[i] = std::min( pmin()[i], p[i] );
			pmax()[i] = std::max( pmax()[i], p[i] );
		}
	}

	bool isEmpty() const
	{
		for( unsigned i = 0; i < N; i++ )
		{
			if( pmin()[i] >= pmax()[i] )
			{
				return true;
			}
		}
		return false;
	}

	//! Enlarge to include a given box
	void operator+=( const Box& b )
	{
		if( b.isEmpty() )
		{
			// Box is already empty, nothing to add
			return;
		}
		
		*this += b.pmin();
		*this += b.pmax();
	}

	const Vector& pmin() const
	{
		return pmin_;
	}

	Vector& pmin()
	{
		return pmin_;
	}

	const Vector& pmax() const
	{
		return pmax_;
	}

	Vector& pmax()
	{
		return pmax_;
	}

	/**
	Add an 'offset' to each of the box's sides as specified by a vector 
	(ex. Vector4( left, top, right, bottom ) for N==2)
	*/
	void enlarge( const Matrix<N*2,1,T>& sides )
	{
		for( unsigned i=0; i<N; i++)
		{
			pmin()[i] -= sides;
			pmax()[i] += sides;
		}
	}

private:

	Vector pmin_;

	Vector pmax_;

};

typedef Box<3, float> Box3f;

//! Transform a box with a matrix
template <unsigned N, unsigned M, typename T>
Box<N,T> operator* ( const Matrix<N,M,T>&, const Box<N,T>& );

template <unsigned N, unsigned M, typename T>
Box<N,T> operator* ( const Box<N,T>&, const Matrix<N,M,T>& );


//! Box union
template <unsigned N, typename T>
Box<N,T> operator+ ( const Box<N,T>&, const Box<N,T>& );

//! Box intersection
template <unsigned N, typename T>
Box<N,T> operator* ( const Box<N,T>&, const Box<N,T>& );


} } // namespace Ephere::Geometry

