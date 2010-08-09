#ifndef GENERIC_H
#define GENERIC_H

// Lots of this stuff is similar to boost's metaprogramming library (MPL) and type_traits

namespace type_traits
{
	template <typename T>
	struct remove_pointer
	{
		typedef T type;
	};

	template <typename T>
	struct remove_pointer<T*>
	{
		typedef T type;
	};

	template <typename T>
	struct is_pointer
	{
		enum { value = false };
	};

	template <typename T>
	struct is_pointer<T*>
	{
		enum { value = true };
	};

	template <typename T, typename U>
	struct is_same
	{
		enum { value = false };
	};

	template <typename T>
	struct is_same<T, T>
	{
		enum  { value = true };
	};
}

// Metaprogramming Library
namespace mpl
{
	// Power metafunction
	template <int base, int exp>
	struct pow
	{
		static const int value = base * pow<base, exp-1>::value;
	};
	template <int base> struct pow<base, 1> { static const int value = base; };
	template <int base> struct pow<base, 0> { static const int value = base; };
	
	// Specialize for powers of two
	template <int exp> struct pow<2, exp> { static const int value = (1<<exp); };
}


#endif // GENERIC_H
