rem This script builds the audio data needed for ZeldaDS. After running it, from
rem the output directory, copy the 'bin files to Game/ZeldaDS/nitrofiles/Audio/
rem and the 'h' files to Game/ZeldaDS/arm9/source/data/

@echo off
setlocal
make clean
make
