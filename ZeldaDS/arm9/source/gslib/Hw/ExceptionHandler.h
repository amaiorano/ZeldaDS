#ifndef GS_EXCEPTION_HANDLER_H
#define GS_EXCEPTION_HANDLER_H

namespace ExceptionHandler
{
	// Displays the "Guru Meditation Screen" with exception address, register and local stack dump
	void EnableDefaultHandler();
	
	// Displays nothing, just runs an infinite loop. Useful when you want to see
	// what was on screen when the exception occurs (i.e. printfs)
	void EnableQuietHandler();
}

#endif // GS_EXCEPTION_HANDLER_H
