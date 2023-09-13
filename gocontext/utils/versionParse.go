package utils

import (
	"regexp"
	"strconv"
	"strings"
)

func GetVersionMatched(srcVersion string, dstVersion string) bool {
	srcV := strings.Split(srcVersion, ".")
	dstN := strings.Split(dstVersion, ".")
	return (strings.Trim(srcV[0], " ") == strings.Trim(dstN[0], " ")) && (strings.Trim(srcV[2], " ") == strings.Trim(dstN[2], " "))
}

func CheckIsHeigherThan115(srcVersion string) bool {
	srcV := strings.Split(srcVersion, ".")

	if v, _ := strconv.Atoi(srcV[0]); v >= 115 {
		return true
	}

	return false
}

func ValidateVersionString(srcVersion string) bool {
	pattern, _ := regexp.Compile("^[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+$")
	return pattern.MatchString(srcVersion)
}
