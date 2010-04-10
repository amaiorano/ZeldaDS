#ifndef CPU_CLOCK_H
#define CPU_CLOCK_H

#include "gslib/Core/Types.h"
#include <stdio.h>

typedef uint32 CpuTickType;
typedef float CpuTimeType;

const CpuTickType HwClockTicksPerSec = 33611400; // 33.6 MHz
const CpuTickType HwClockTicksPerMs = HwClockTicksPerSec / 1000; // 33.6 MHz

inline CpuTimeType TicksToSecs(CpuTickType ticks)	{ return static_cast<CpuTimeType>(ticks) / HwClockTicksPerMs; }
inline CpuTimeType TicksToMs(CpuTickType ticks)		{ return static_cast<CpuTimeType>(ticks) / HwClockTicksPerMs; }

// The one CPU clock
namespace CpuClock
{
	// Call at start of each frame
	void Update();

	// Returns time between last two calls to Update() (the frame delta time)
	CpuTickType GetLastFrameElapsedTicks();

	// Returns time since last call to Update() (the frame elapsed time)
	CpuTickType GetCurrFrameElapsedTicks();
}

// Utility class for timing sections within a single frame
// (does not work across frame boundaries)
class CpuTimer
{
public:
	void Start() { mStartTime = CpuClock::GetCurrFrameElapsedTicks(); mElapsedTime = 0; }
	void Stop() { mElapsedTime = CpuClock::GetCurrFrameElapsedTicks() - mStartTime; }
	const CpuTickType& GetElapsedTicks() const { return mElapsedTime; }

private:
	CpuTickType mStartTime;
	CpuTickType mElapsedTime;
};

// Utility class that times a scope and prints the result on destruction
class ScopedPrintCpuTimer : protected CpuTimer
{
public:
	ScopedPrintCpuTimer(const char* scopeName) : mScopeName(scopeName) { Start(); }
	~ScopedPrintCpuTimer()
	{
		Stop();
		printf("(CpuTimer) %s: %f ms\n", mScopeName, TicksToMs(GetElapsedTicks()));
	}

private:
	const char* mScopeName;
};

// Utility container for CPU ticks with conversion helpers
class CpuTime
{
public:
	// Non-explicit so you can assign ticks directly to a CpuTime instance
	CpuTime(CpuTickType ticks = 0) : mTicks(ticks) { }

	CpuTickType Ticks() const	{ return mTicks; }
	CpuTimeType Secs() const	{ return TicksToSecs(mTicks); }
	CpuTimeType Ms() const		{ return TicksToMs(mTicks); }

	void operator+=(const CpuTime& rhs) { mTicks += rhs.mTicks; }
	void operator+=(CpuTickType ticks) { mTicks += ticks; }

private:
	CpuTickType mTicks;
};

#endif // CPU_CLOCK_H
