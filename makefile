cc = gcc
cxx = g++
cflags = 
ldflags = 
libs = -lUrlMon

dll:
	cd gocontext && mingw32-make && powershell -Command "echo F | xcopy /Y .\\GoContext.dll ..\\GoContext.dll"
	$(cxx) -fPIC -std=c++17 $(cflags) -DBUILDGCDAPI -c GetChromeDriver.cc -o GetChromeDriver.o
	$(cxx) -shared $(ldflags) GetChromeDriver.o -Wl,--out-implib,GetChromeDriver.lib -o GetChromeDriver.dll $(libs)

main:
	$(cc) -c GetDriver.c -o GetDriver.o
	$(cc) -L"./" GetDriver.o -o GetDriver.exe -lGetChromeDriver