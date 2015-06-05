@echo off

rmdir /S /Q Debug
rmdir /S /Q Release

cd AdVisuo
rmdir /S /Q Debug
rmdir /S /Q Release
cd ..

cd adv
rmdir /S /Q Debug
rmdir /S /Q Release
cd ..

cd advtest
rmdir /S /Q Debug
rmdir /S /Q Release
cd ..

cd ifctest
rmdir /S /Q Debug
rmdir /S /Q Release
cd ..

cd advsrv
rmdir /S /Q Debug
rmdir /S /Q Release
cd ..

cd "AdVisuo Setup"
rmdir /S /Q Debug
rmdir /S /Q Release
cd ..

cd advideo
rmdir /S /Q Debug
rmdir /S /Q Release
cd ..

cd advideotest
rmdir /S /Q Debug
rmdir /S /Q Release
cd ..

cd advidgen
rmdir /S /Q Debug
rmdir /S /Q Release
cd ..

cd advidmon
rmdir /S /Q Debug
rmdir /S /Q Release
cd ..

echo Done...
