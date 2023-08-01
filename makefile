cc = gcc
cxx = g++
cflags = -I"C:\Users\SteveChung\AppData\Local\Programs\Python\Python310\include"
ldflags = -L".\Zipper2" -L"C:\Users\SteveChung\AppData\Local\Programs\Python\Python310\libs" 
libs = -lpython310 -lUrlmon -lZipper

dll:
	python compile.py
	windres Resource.rc -o Resource.o
	$(cxx) -fPIC -shared $(cflags) -std=c++17 -O3 -DBUILDGCDAPI -c GetChromeDriver.cc -o GetChromeDriver.o
	$(cxx) -fPIC -shared $(cflags) -c -O3 FindRuntimeFiles.cc -o FindRuntimeFiles.o
	$(cxx) -shared $(ldflags) GetChromeDriver.o FindRuntimeFiles.o Resource.o -Wl,--out-implib,GetChromeDriver.lib -o GetChromeDriver.dll $(libs)

test:
	$(cc) -c test.c -o test.o
	$(cc) -L"./" test.o -o test.exe -lGetChromeDriver