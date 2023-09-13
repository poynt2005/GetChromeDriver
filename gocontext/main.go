package main

//#include <stdlib.h>
import "C"

import (
	"getchromedriver/utils"
	"unsafe"
)

var lastErrorString string = ""

//export FindUrlByVersionString
func FindUrlByVersionString(versionStr *C.char) *C.char {
	lastErrorString = ""

	if !utils.ValidateVersionString(C.GoString(versionStr)) {
		lastErrorString = "VersionStringValidateFalied"
		return nil
	}

	if utils.CheckIsHeigherThan115(C.GoString(versionStr)) {
		url, err := utils.FetchMeta(C.GoString(versionStr))

		if err != nil {
			lastErrorString = err.Error()
		}

		return C.CString(url)
	}

	url, err := utils.FetchMetaFromWebSite(C.GoString(versionStr))

	if err != nil {
		lastErrorString = err.Error()
	}

	return C.CString(url)
}

//export GetChromeVersion
func GetChromeVersion() *C.char {
	lastErrorString = ""

	versionStr, err := utils.GetRegistsry()

	if err != nil {
		lastErrorString = err.Error()
		return nil
	}

	return C.CString(versionStr)
}

//export GetLastErrorString
func GetLastErrorString() *C.char {
	return C.CString(lastErrorString)
}

//export UnzipDriver
func UnzipDriver(driverZipPath, dstExecutable *C.char) C.int {
	err := utils.UnzipFile(C.GoString(driverZipPath), C.GoString(dstExecutable))

	if err != nil {
		lastErrorString = err.Error()
		return C.int(0)
	}

	return C.int(1)
}

//export FreeGoBuffer
func FreeGoBuffer(buffer **C.char) {
	if buffer == nil || *buffer == nil {
		return
	}

	C.free(unsafe.Pointer(*buffer))
	*buffer = nil
}
func main() {
}
