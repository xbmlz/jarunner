package main

import "C"
import (
	"archive/zip"
	"fmt"
	"os"
	"path"
	"strconv"
	"strings"

	"github.com/go-ini/ini"
)

//export GetJarMajorVersion
func GetJarMajorVersion(jarPath *C.char) C.int {
	jarFile, err := os.Open(C.GoString(jarPath))
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
					return C.int(majorVersion - 44)
				}
			}
			return -6
		}
	}
	return -7
}

//export GetLocalJavaHome
func GetLocalJavaHome(jarVersion C.int, javaHomeRet **C.char) C.int {
	javaHome, isExist := os.LookupEnv("JAVA_HOME")
	if isExist {
		releaseFile, err := ini.Load(path.Join(javaHome, "release"))
		if err != nil {
			return -1
		}
		localVersion := releaseFile.Section("").Key("JAVA_VERSION").String()
		arrVersion := strings.Split(localVersion, ".")
		var localMajorVersion = 0
		if arrVersion[0] == "1" {
			// convert 1.8.0_181 to 8 (int)
			localMajorVersion, _ = strconv.Atoi(arrVersion[1])
		} else {
			// convert 11.0.1 to 11 (int)
			localMajorVersion, _ = strconv.Atoi(arrVersion[0])
		}
		if localMajorVersion >= int(jarVersion) {
			*javaHomeRet = C.CString(javaHome)
			return C.int(localMajorVersion)
		}
	}
	return -1
}

//export GetIniValue
func GetIniValue(iniPath *C.char, section *C.char, key *C.char, valueRet **C.char) C.int {
	iniFile, err := ini.Load(C.GoString(iniPath))
	if err != nil {
		return -1
	}
	*valueRet = C.CString(iniFile.Section(C.GoString(section)).Key(C.GoString(key)).String())
	return 0
}

func main() {
	// v := GetLocalJavaHome(8)
	// fmt.Println(v)
	iniFile, _ := ini.Load("../build/jarunner.ini")
	val := iniFile.Section("").Key("jarPath").String()
	fmt.Println(val)
}
