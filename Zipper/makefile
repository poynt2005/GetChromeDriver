cc = "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Tools\MSVC\14.29.30133\bin\Hostx64\x64\cl.exe"
cflags = /I"C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Tools\MSVC\14.29.30133\include" /I"C:\Program Files (x86)\Windows Kits\10\Include\10.0.17763.0\shared" /I"C:\Program Files (x86)\Windows Kits\10\Include\10.0.17763.0\ucrt" /I"C:\Program Files (x86)\Windows Kits\10\Include\10.0.17763.0\um" /I"C:\Program Files (x86)\Windows Kits\10\Include\10.0.17763.0\winrt"  /I"C:\Program Files (x86)\Windows Kits\NETFXSDK\4.8\Include\um"
ldflags =  /LIBPATH:"C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Tools\MSVC\14.29.30133\lib\x64" /LIBPATH:"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.17763.0\ucrt\x64" /LIBPATH:"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.17763.0\um\x64" /LIBPATH:"C:\Program Files (x86)\Windows Kits\NETFXSDK\4.8.1\Lib\um\x64"
Assemblies = /FU"C:\Windows\Microsoft.NET\Framework\v4.0.30319\System.IO.Compression.dll" /FU"C:\Windows\Microsoft.NET\Framework\v4.0.30319\System.IO.Compression.FileSystem.dll" /FU"C:\Windows\Microsoft.NET\Framework\v4.0.30319\System.IO.Compression.ZipFile.dll" 
libs = 

dll:
	$(cc) $(cflags) /std:c++17 /CLR /Ox /DEXPORTZIPPERAPI /c Zipper.cc $(Assemblies) /Fo:Zipper.obj 
	$(cc) /LD Zipper.obj /link $(ldflags) $(libs) /IMPLIB:Zipper.lib /OUT:Zipper.dll