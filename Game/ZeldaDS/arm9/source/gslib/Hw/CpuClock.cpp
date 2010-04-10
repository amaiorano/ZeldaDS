#include "CpuClock.h"
#include <nds/timers.h>

namespace CpuClock
{
	namespace
	{
		CpuTickType gLastFrameElapsedTicks = 0;
		const uint32 gTimerChannel = 0;
	}

	void Update()
	{
		gLastFrameElapsedTicks = cpuEndTiming();
		cpuStartTiming(gTimerChannel); // Restart timer for this frame
	}

	CpuTickType GetLastFrameElapsedTicks()
	{
		return gLastFrameElapsedTicks;
	}

	CpuTickType GetCurrFrameElapsedTicks()
	{
		//@TODO: Add cpuGetElapsed() that does the following...
		return ( (TIMER_DATA(gTimerChannel) | (TIMER_DATA(gTimerChannel+1)<<16) ));
	}
}
