package utils

import (
	"archive/zip"
	"errors"
	"io"
	"os"
	"path"
	"path/filepath"
	"strconv"
	"strings"
	"time"
)

func moveFile(source, destination string) (err error) {
	src, err := os.Open(source)
	if err != nil {
		return err
	}
	defer src.Close()
	fi, err := src.Stat()
	if err != nil {
		return err
	}
	flag := os.O_WRONLY | os.O_CREATE | os.O_TRUNC
	perm := fi.Mode() & os.ModePerm
	dst, err := os.OpenFile(destination, flag, perm)
	if err != nil {
		return err
	}
	defer dst.Close()
	_, err = io.Copy(dst, src)
	if err != nil {
		dst.Close()
		os.Remove(destination)
		return err
	}
	err = dst.Close()
	if err != nil {
		return err
	}
	err = src.Close()
	if err != nil {
		return err
	}
	err = os.Remove(source)
	if err != nil {
		return err
	}
	return nil
}

func UnzipFile(srcZipFile, dstExeFilePath string) error {
	absSrcZipFile, err := filepath.Abs(srcZipFile)
	if err != nil {
		return errors.New("SorceZipAbsPathParseFailed")
	}

	absDstExecutableFile, err := filepath.Abs(dstExeFilePath)
	if err != nil {
		return errors.New("DstExecatableAbsPathParseFailed")
	}

	srcZipInfo, err := os.Stat(absSrcZipFile)

	if err != nil {
		return errors.New("SorceZipNotExists")
	}

	if srcZipInfo.IsDir() || strings.ToLower(path.Ext(absSrcZipFile)) != ".zip" {
		return errors.New("SorceZipFormatNotValid")
	}

	tempDir := path.Join(os.TempDir(), "GetChromeDriverTemp")

	info, err := os.Stat(tempDir)

	if err != nil || !info.IsDir() {
		os.MkdirAll(tempDir, os.ModePerm)
	}

	randomStr := strconv.FormatInt(time.Now().Unix(), 10)

	targetZipBallPath := path.Join(tempDir, "driver_"+randomStr+".zip")

	if err := os.MkdirAll(path.Join(tempDir, "driver_"+randomStr), os.ModePerm); err != nil {
		return errors.New("CreateDstTempFolderFailed")
	}

	if err := moveFile(absSrcZipFile, targetZipBallPath); err != nil {
		return errors.New("MoveZipFileFailed")
	}

	archive, err := zip.OpenReader(targetZipBallPath)

	if err != nil {
		return errors.New("ReadZipballFailed")
	}

	var unZipExecutablePath string
	var isUnzipSuccess bool = false

	var unzipErrors error = nil

	for _, f := range archive.File {
		contentFileName := filepath.Base(f.Name)

		if strings.ToLower(filepath.Ext(contentFileName)) == ".exe" {
			unZipExecutablePath = path.Join(tempDir, "driver_"+randomStr, contentFileName)

			dstExecutableFile, err := os.OpenFile(unZipExecutablePath, os.O_WRONLY|os.O_CREATE|os.O_TRUNC, f.Mode())

			if err != nil {
				dstExecutableFile.Close()
				unzipErrors = errors.New("OpenDstExecutableFailed")
				break
			}

			srcExecutableFile, err := f.Open()

			if err != nil {
				dstExecutableFile.Close()
				srcExecutableFile.Close()
				unzipErrors = errors.New("ReadSrcExecutableFailed")
				break
			}

			if _, err := io.Copy(dstExecutableFile, srcExecutableFile); err != nil {
				dstExecutableFile.Close()
				srcExecutableFile.Close()
				unzipErrors = errors.New("UnzipExecutableFailed")
				break
			}

			dstExecutableFile.Close()
			srcExecutableFile.Close()

			isUnzipSuccess = true

			break
		}
	}

	archive.Close()

	if unzipErrors != nil {
		return unzipErrors
	}

	if len(unZipExecutablePath) == 0 {
		return errors.New("CannotGetUnzipPath")
	}

	if !isUnzipSuccess {
		return errors.New("UnzipExecatableNotSucceed")
	}

	if err := moveFile(unZipExecutablePath, absDstExecutableFile); err != nil {
		return errors.New("CannotMoveUnzipedExecutable")
	}

	os.RemoveAll(tempDir)

	return nil
}
