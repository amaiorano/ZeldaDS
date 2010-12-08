#include <nds.h>
//#include <dswifi7.h>
#include <maxmod7.h>
#include "gslib/Core/Config.h"

void VcountHandler()
{
	inputGetAndSend();
}

void VblankHandler(void)
{
	//Wifi_Update();
}

int main()
{
	irqInit();
	fifoInit();

	// read User Settings from firmware
	readUserSettings();

	// Start the RTC tracking IRQ
	initClockIRQ();

	SetYtrigger(80);

	//installWifiFIFO();

#if AUDIO_ENABLED
	installSoundFIFO();
	mmInstall(FIFO_MAXMOD);
#endif

	installSystemFIFO();
	
	irqSet(IRQ_VCOUNT, VcountHandler);
	//irqSet(IRQ_VBLANK, VblankHandler);

	irqEnable( /*IRQ_VBLANK |*/ IRQ_VCOUNT /*| IRQ_NETWORK*/);   

	// Keep the ARM7 mostly idle
	while (true)
	{
		swiWaitForVBlank();
	}
}


