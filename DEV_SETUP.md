# ZeldaDS Developer Setup

These steps are for a Windows installation. I suspect it should be easy enough to get this building on Mac or Linux.

## Prerequisites

* Windows 7
* Visual Studio 2008

## ZeldaDS (Game)

* Install devKitPro toolchain for ARM processors
    * Install the latest version using the windows installer, making sure to select "devkitArm" (PPC and PSP not required)
        Download link: http://sourceforge.net/projects/devkitpro/
        Community site: http://www.devkitpro.org/
    * Make sure the environment variables DEVKITARM and DEVKITPRO have been added, and set properly. For example:
		DEVKITARM=/c/devkitPro/devkitARM
		DEVKITPRO=/c/devkitPro
    * Make sure PATH includes the path to msys\bin, for example: c:\devkitPro\msys\bin

* Add an environment variable named NDSGAMEROOT set to the root folder containing ZeldaDS
    * For example: NDSGAMEROOT=C:\Code\ZeldaDS

* Open %NDSGAMEROOT%\Game\ZeldaDS.sln in Visual Studio, open project properties, select "All Configurations", then go to Configuration Properties -> Debugging, and set:
    * Command: $(NDSEMUPATH)
    * Command Arguments: $(NDSEMUARGS)

* Compile the game by building the solution (build Debug configuration)

* Run by pressing F5, which should launch the game through desmume
    * If you get the "Failed to init file system..." error, in desmume, go to Emulation -> GBA Slot, then Browse and select ZeldaDS_d.nds (you'll have to put a '*' in the File name field and press Enter to show all files, otherwise you can only select *.gba files). This only has to be done once. Note that a future release of desmume will allow me to pass this in by commandline, and you won't have to worry about this step anymore.

* NOTE: If compiling to run on actual hardware (i.e. flashcart), make sure that in gslib/Core/Config.h, PRINT_TO_EMULATOR_CONSOLE is set to 0, otherwise the game will crash on the first call to printf().


## Zelous (Editor)

* Before starting with this setup, make sure you've got Zelda DS up and running (section above)

* Open %NDSGAMEROOT%\Editor\Zelous\Zelous.csproj in Visual Studio, open project properties...
    * Debug tab, set Working Directory to "%NDSGAMEROOT%\Editor\Zelous\out\Zelous" by browsing to this folder. Unfortunately, this field cannot contain environment variables, so click Browse... and enter the string, and Windows will resolve it to your local path.

* You should now be able to build the project and debug it (F5).

* NOTE: When building Zelous, there is a post-build step that copies the required data to temporary bin/<config> folder. This allows the editor to be debugged in its default output location correctly. For proper packaging, you can create a Zelous package by running package/make_package.bat


## git subtree

* ZeldaDS makes use of git subtree to bring in git modules:

git remote add hsm git@github.com:amaiorano/hsm.git
git subtree pull --prefix=Game/ZeldaDS/arm9/source/external/hsm hsm master --squash
git subtree push --prefix=Game/ZeldaDS/arm9/source/external/hsm hsm master
