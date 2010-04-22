ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules

export AUDIOFILES	:=	$(notdir $(wildcard *.wav))
export AUDIOFILES	+=	$(notdir $(wildcard *.it))

soundbank.bin : $(AUDIOFILES)
	@echo creating sound bank...
	@mmutil $^ -d -osoundbank.bin -hsoundbank.h
	###@mmutil $^ -d -osoundbank.bin

clean:
	@echo clean ...
	@rm -fr soundbank.*
