#pragma once

#include <Windows.h>
#include <string>
#include <vector>

class IAbstractArchive {
public:
	virtual ~IAbstractArchive() {};
	virtual void extract(const char* filename) = 0;
	virtual void addFileInArchive() = 0;
	virtual void close(HWND hWnd1, HWND hWnd2) = 0;
	virtual void writeArchiveSingle(HWND hWnd, std::wstring str, std::string path, std::vector<std::string> argv, std::string outname) = 0;
	virtual void writeArchiveDirectory(std::wstring str, std::string path, std::vector<std::string> argv, std::string outname) = 0;
	virtual void selectFile(HWND& hWnd, std::wstring& str) = 0;
	virtual void selectArchive(HWND& hWnd, std::wstring& str) = 0;
};