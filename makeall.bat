@echo off
set path=%path%;C:\PalmDev\pilrc-2.8
set path=%path%;C:\PalmDev\PRC-Tools\H-i586-cygwin32\bin

echo _____________
echo *************
echo ****Nokia****
echo *************
pmake

echo _____________
echo *************
echo ****Palm*****
echo *************

cd palm
C:\cygnus\cygwin-b20\H-i586-cygwin32\bin\make
del *.bin
del *.o
del *.stm
del *.grc
cd ..
