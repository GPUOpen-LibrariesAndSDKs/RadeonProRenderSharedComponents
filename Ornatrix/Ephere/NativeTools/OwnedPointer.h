// THIS FILE MUST COMPILE WITH C AND C++98 (Visual Studio 2010)

/*!
Tools for passing object ownership across module boundaries
*/

#pragma once

typedef struct Ephere_OwnerContainer Ephere_OwnerContainer;

typedef void( *Ephere_DeleteFunctionType )( void* pointer, Ephere_OwnerContainer* );

typedef struct Ephere_OwnerContainer
{
	Ephere_DeleteFunctionType Deleter;
} Ephere_OwnerContainer;

// These macros can be useful in C code, enable them if needed or just copy/paste the code
#if 0

#define EPHERE_DECLARE_OWNED_POINTER( Type ) \
	typedef struct { \
		Type* Pointer; \
		Ephere_OwnerContainer* Owner; \
	} Type##_OwnedPointer;


#define EPHERE_DELETE_OWNED_POINTER( owned ) \
	if( owned.Pointer != NULL && owned.Owner != NULL ) {\
		owned.Owner->Deleter( owned.Pointer, owned.Owner ); }

#endif


#ifdef __cplusplus

#include <memory>

namespace Ephere
{
	template <typename T>
	struct DeleteOwner : Ephere_OwnerContainer
	{
		DeleteOwner()
		{
			Deleter = Delete;
		}

	private:

		static void Delete( void* pointer, Ephere_OwnerContainer* )
		{
			delete static_cast<T*>( pointer );
		}
	};

#if !defined( _MSC_VER ) || _MSC_VER >= 1600
	template <typename T>
	struct SharedOwner : Ephere_OwnerContainer
	{
		SharedOwner( const std::shared_ptr<T>& shared )
			: shared_( shared )
		{
			Deleter = Delete;
		}

	private:

		static void Delete( void*, Ephere_OwnerContainer* owner )
		{
			delete static_cast<SharedOwner*>( owner );
		}

		std::shared_ptr<T> shared_;
	};
#endif

	template <typename T>
	class OwnedPointer
	{
	public:

		OwnedPointer()
			: pointer_( nullptr ),
			owner_( nullptr )
		{
		}

		OwnedPointer( T* pointer, Ephere_OwnerContainer* owner )
			: pointer_( pointer ),
			owner_( owner )
		{
		}

		//! Takes ownership over a raw pointer
		template <class TOther>
		static OwnedPointer AcquireRaw( TOther* pointer )
		{
			static DeleteOwner<TOther> deleteOwner;
			return pointer != nullptr ? OwnedPointer( pointer, &deleteOwner ) : OwnedPointer();
		}

		//! Wraps a raw pointer without owning it
		template <class TOther>
		static OwnedPointer NotOwned( TOther* pointer )
		{
			return OwnedPointer( pointer, nullptr );
		}

#if !defined( _MSC_VER ) || _MSC_VER >= 1600
		static OwnedPointer FromShared( const std::shared_ptr<T>& shared )
		{
			return shared != nullptr ? OwnedPointer( shared.get(), new SharedOwner<T>( shared ) ) : OwnedPointer();
		}
#endif

		T* Get() const
		{
			return pointer_;
		}

		//Ephere_OwnerContainer* GetOwner() const
		//{
		//	return owner_;
		//}

		T* operator ->() const
		{
			return Get();
		}

		// We can make OwnedPointer actually own the pointer only with VC 2010 or later (needs move semantics). Our non-VC compilers do have it.
#if !defined( _MSC_VER ) || _MSC_VER >= 1600
		template <typename U>
		OwnedPointer( OwnedPointer<U>&& other )
			: pointer_( other.Get() ),
			owner_( other.ReleaseOwnership() )
		{
		}

		~OwnedPointer()
		{
			if( pointer_ != nullptr && owner_ != nullptr )
			{
				owner_->Deleter( pointer_, owner_ );
			}
		}

		typename std::add_lvalue_reference<T>::type operator*() const
		{
			// TODO: Assert
			return *pointer_;
		}

		bool operator==( std::nullptr_t ) const
		{
			return pointer_ == nullptr;
		}

		bool operator!=( std::nullptr_t ) const
		{
			return pointer_ != nullptr;
		}

#if !defined( _MSC_VER ) || _MSC_VER >= 1800
		explicit
#endif
		operator bool() const
		{
			return pointer_ != nullptr;
		}

		Ephere_OwnerContainer* ReleaseOwnership()
		{
			auto result = owner_;
			owner_ = nullptr;
			return result;
		}

		// Converts this pointer to a shared_ptr, releasing the ownership but retaining the pointer
		std::shared_ptr<T> ToShared()
		{
			if( pointer_ == nullptr )
			{
				return std::shared_ptr<T>();
			}

			std::shared_ptr<T> result;

			auto owner = ReleaseOwnership();
			if( owner == nullptr )
			{
				result.reset( pointer_, []( T* ) {} );
			}
			else
			{
				result.reset( pointer_, [owner]( T* pointer )
				{
					owner->Deleter( pointer, owner );
				} );
			}

			return result;
		}

	private:

		// Don't use =delete to support VC 2010
		OwnedPointer( const OwnedPointer& );
		OwnedPointer& operator=( const OwnedPointer& );
#endif

	private:

		T* pointer_;
		Ephere_OwnerContainer* owner_;
	};
}

#endif
