package utils

import (
	"errors"

	"golang.org/x/sys/windows/registry"
)

func GetRegistsry() (string, error) {
	key, err := registry.OpenKey(registry.CURRENT_USER, `Software\Google\Chrome\BLBeacon`, registry.QUERY_VALUE)

	if err != nil {
		return "", errors.New("CannotOpenTargeteRegistry")
	}
	defer key.Close()

	keyValue, _, err := key.GetStringValue("Version")

	if err != nil {
		return "", errors.New("CannotOpenGetVerionsFromRegistry")
	}

	return keyValue, nil
}
