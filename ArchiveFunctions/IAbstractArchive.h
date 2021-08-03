#pragma once

#include <Windows.h>
#include <string>
#include <vector>

class IAbstractArchive {
public:
	virtual ~IAbstractArchive() {};
	virtual void extractArchive(std::wstring str, std::string& path, std::string& outname) = 0;
	virtual void addFileInArchive() = 0;
	virtual void writeArchiveSingle(std::wstring str, std::string path, std::vector<std::string> argv, std::string outname) = 0;
	virtual void writeArchiveDirectory(std::wstring str, std::string path, std::vector<std::string> argv, std::string outname) = 0;
	virtual void selectFile(std::wstring& str) = 0;
	virtual void selectArchive(std::wstring& str) = 0;
};