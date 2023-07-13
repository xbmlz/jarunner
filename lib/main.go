package main

import "C"
import (
	"archive/zip"
	"os"
	"path"
)

//export GetJarMajorVersion
func GetJarMajorVersion(jarPath string) int {
	jarFile, err := os.Open(jarPath)
	if err != nil {
		return -1
	}
	jarStat, err := jarFile.Stat()
	if err != nil {
		return -2
	}
	r, err := zip.NewReader(jarFile, jarStat.Size())
	if err != nil {
		return -3
	}
	for _, f := range r.File {
		// read first class file
		if !f.FileInfo().IsDir() && path.Ext(f.Name) == ".class" {
			classFile, err := f.Open()
			if err != nil {
				return -4
			}
			defer classFile.Close()
			// read file header
			header := make([]byte, 8)
			_, err = classFile.Read(header)
			if err != nil {
				return -5
			}
			// check class file identifier
			if string(header[:4]) == "\xCA\xFE\xBA\xBE" {
				majorVersion := int(header[6])<<8 + int(header[7])
				if majorVersion >= 45 {
					return majorVersion - 44
				}
			}
			return -6
		}
	}
	return -7
}

func main() {}
