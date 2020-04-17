// THIS FILE MUST COMPILE WITH C++98 (Visual Studio 2010)

#pragma once

#include "Ephere/NativeTools/OwnedPointer.h"

#include <memory>

namespace Ephere
{

/*! Interface identifier.
Recommended scheme: 0xCCCIIIVV, CCC is company code, III - interface number, VV - version */
typedef unsigned InterfaceId;

const InterfaceId InterfaceIdNone = 0;

const InterfaceId InterfaceId_Company_Private = 0;
const InterfaceId InterfaceId_Company_Ephere = 0x100000;

/*!
Returns requested interfaces, if supported. Similar to COM IUnknown.
Retrieving an interface through IInterfaceProvider::GetInterface has the following advantages over C++ dynamic_cast from a common base class:

- The target interface may be renamed inside the provider DLL. The client will continue to work using the old name because the interface is identified by a number,
not by its name. dynamic_cast won't work if the class name doesn't match in both the provider and the client DLLs.

- Returning an owning pointer allows the provider to build a new object that implements the interface and pass its ownership to the client.
Also, if the implementation is held by a shared pointer inside the provider DLL, the client DLL can share the ownership.
*/
struct IInterfaceProvider
{
	static const unsigned CurrentVersion = 1;


	virtual unsigned Version() const
	{
		return CurrentVersion;
	}

	virtual OwnedPointer<void> GetInterface( const char* sourceName, InterfaceId interfaceId ) = 0;


	template <class T>
	std::shared_ptr<T> GetInterface( const char* sourceName = nullptr )
	{
		std::shared_ptr<T> result;
		if( auto ownedPointer = GetInterface( sourceName, T::IID ) )
		{
			return std::static_pointer_cast<T>( ownedPointer.ToShared() );
		}

		return result;
	}

protected:

	~IInterfaceProvider()
	{
	}
};

}
