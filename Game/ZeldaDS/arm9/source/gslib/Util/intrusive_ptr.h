#ifndef GS_INTRUSIVE_PTR_H
#define GS_INTRUSIVE_PTR_H

// Similar to boost::intrusive_ptr, but I don't want to bring in all of boost.
// One difference is that I provide default implementations for intrusive_ptr_add_ref()
// and intrusive_ptr_release() and an optional mixin class intrusive_ptr_client.

template <typename T>
class intrusive_ptr
{
public:
	typedef intrusive_ptr<T> this_type;

	intrusive_ptr(T* pObject = 0) : mpObject(pObject)
	{
		safe_add_ref();
	}

	~intrusive_ptr()
	{
		safe_remove_ref();
	}

	intrusive_ptr(const this_type& rhs) : mpObject(rhs.get())
	{
		safe_add_ref();
	}

	template <typename U>
	intrusive_ptr(const intrusive_ptr<U>& rhs) : mpObject(rhs.get())
	{
		safe_add_ref();
	}

	this_type& operator=(const this_type& rhs)
	{
		// This neat "swap" trick uses the constructor/destructor to do the work.
		this_type(rhs).swap(*this);
		return *this;
	}

	template <typename U>
	this_type& operator=(const intrusive_ptr<U>& rhs)
	{
		this_type(rhs).swap(*this);
		return *this;
	}

	// Since the object holds its own ref count, we can safely assign an object
	// directly to an intrusive_ptr, even if another intrusive_ptr already points
	// to it. Note that this is not possible with shared_ptr, so this assignment
	// operator is not made available.
	this_type& operator=(T* pObject)
	{
		reset(pObject);
		return *this;
	}

	void reset(T* pObject = 0)
	{
		this_type(pObject).swap(*this);
	}

	T* get() const
	{
		return mpObject;
	}

	T* operator->()
	{
		return mpObject;
	}
	
	T& operator*()
	{
		return *mpObject;
	}

	void swap(this_type& rhs)
	{
		T* pLhsObject = mpObject;
		mpObject = rhs.mpObject;
		rhs.mpObject = pLhsObject;
	}

private:
	void safe_add_ref()
	{
		if (mpObject)
		{
			intrusive_ptr_add_ref(mpObject);
		}
	}

	void safe_remove_ref()
	{
		if (mpObject)
		{
			intrusive_ptr_release(mpObject);
			mpObject = 0;
		}
	}

	T* mpObject;
};


// Default implementations: expect functions to exist on pObject. To override,
// simply overload this function for your specific type T.
template <typename T>
void intrusive_ptr_add_ref(T* pObject)
{
	pObject->intrusive_ptr_add_ref(); // Must increase ref count
}

template <typename T>
void intrusive_ptr_release(T* pObject)
{
	pObject->intrusive_ptr_release(); // Must delete if ref count drops to 0
}

// Optional class that T can derive from for a default intrusive_add_ref/intrusive_remove_ref implementation
class intrusive_ptr_client
{
public:
	intrusive_ptr_client() : mRefCount(0)
	{
	}

	virtual ~intrusive_ptr_client()
	{
	}

	void intrusive_ptr_add_ref() const
	{
		++mRefCount;
		//printf("0x%x AddRef (count = %d)\n", this, mRefCount);
	}

	void intrusive_ptr_release() const
	{
		--mRefCount;
		//printf("0x%x Release (count = %d)\n", this, mRefCount);
		
		//@TODO: ASSERT(mRefCount >= 0)
		if (mRefCount == 0)
		{
			delete this;
		}
	}

private:
	mutable int mRefCount;
};

#endif // GS_INTRUSIVE_PTR_H
