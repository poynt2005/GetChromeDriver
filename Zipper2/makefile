cc = g++
cflags = 
ldflags = 
libs = 

dll:
	windres Resource.rc -o Resource.o
	$(cc) -fPIC -shared -DEXPORTZIPPERAPI $(cflags) -std=c++17 -O3 -c Zipper.cc -o Zipper.o
	$(cc) -shared $(ldflags) Resource.o Zipper.o -Wl,--out-implib,Zipper.lib -o Zipper.dll

test:
	$(cc) test.cc -c test.o
	$(cc) -L./ test.o -o test.exe -lZipper
	