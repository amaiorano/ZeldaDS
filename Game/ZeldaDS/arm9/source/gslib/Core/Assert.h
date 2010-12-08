#ifndef GS_ASSERT_H
#define GS_ASSERT_H

#include "Config.h"

#if ASSERTS_ENABLED

	namespace AssertImpl
	{
		struct AssertFunctor
		{
			AssertFunctor(const char* file, int line, const char* condition)
				: mAlwaysSkip(false), mFile(file), mLine(line), mCondition(condition)
			{
			}

			bool AlwaysSkip() const { return mAlwaysSkip; }
			void Printf(const char* format, ...);

		private:
			bool mAlwaysSkip;
			const char* mFile;
			int mLine;
			const char* mCondition;
		};
	}

	#define _DoAssertFunctor(exp, format, file, line) \
		do { \
			if (!(exp)) { \
				static AssertImpl::AssertFunctor assertFunctor##file##line(file, line, #exp); \
				if (!assertFunctor##file##line.AlwaysSkip()) assertFunctor##file##line.Printf format; \
			} \
		} while (false)

	//@TODO: Both assert msg and formatted should use the above macro, and pass in FILE and LINE so that it's the file/line of the actual assert.
	#define _AssertMsg(exp, msg)			_DoAssertFunctor(exp, (msg), __FILE__, __LINE__)
	#define _AssertFormatted(exp, format)	_DoAssertFunctor(exp, format, __FILE__, __LINE__)

#else // !ASSERTS_ENABLED

	#define _AssertMsg(e, msg)  	((void)0)
	#define _AssertFormatted(e, f)	((void)0)

#endif // ASSERTS_ENABLED


// Macros that should be used in code

#define ASSERT(exp)						_AssertMsg(exp, "")
#define ASSERT_MSG(exp, msg)			_AssertMsg(exp, msg)
#define ASSERT_FORMATTED(exp, format)	_AssertFormatted(exp, format)

#define FAIL()							_AssertMsg(false, "FAIL")
#define FAIL_MSG(msg)					_AssertMsg(false, "FAIL: " msg)
#define FAIL_FORMATTED(format)			_AssertFormatted(false, format)

#if ASSERTS_ENABLED
	#define VERIFY(exp) ASSERT(exp)
#else
	#define VERIFY(exp)	exp
#endif

// Useful for wrapping debug-only statements; i.e. declaring variables used in assertions
#if ASSERTS_ENABLED
	#define DBG_STATEMENT(s) s
#else
	#define DBG_STATEMENT(s)
#endif

// Compile-time assertion, always available
#define CT_ASSERT(expr)				struct __UNIQUE_NAME { unsigned int no_message : expr; }
#define CT_ASSERT_MSG(expr, msg)	struct __UNIQUE_NAME { unsigned int msg : expr; }
#define __UNIQUE_NAME				__MAKE_NAME(__LINE__)
#define __MAKE_NAME(line)			__MAKE_NAME2(line)
#define __MAKE_NAME2(line)			ct_assert_failure_ ## line

#endif // GS_ASSERT_H
