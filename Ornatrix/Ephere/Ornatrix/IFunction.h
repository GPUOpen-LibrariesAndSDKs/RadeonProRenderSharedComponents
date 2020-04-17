// Must compile with VC 2012 / GCC 4.8

#pragma once

#include "Ephere/Ornatrix/Types.h"

namespace Ephere { namespace Ornatrix
{

//! A generic interface representing a function
template<class TInput, class TOutput>
class IFunction
{
public:

	/** Evaluate function at a specified position
		@param position Value in range {0,1}, 0 being beginning of curve and 1 being the end
		@return Evaluated value at specified position
	*/
	virtual TOutput Evaluate( TInput position ) const = 0;

protected:

	~IFunction() {}
	IFunction& operator=( const IFunction& ) { return *this; }
};

typedef IFunction<Real, Real> IFunction1;

} } // Ephere::Ornatrix
