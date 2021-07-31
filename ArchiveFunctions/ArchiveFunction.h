#pragma once

#include <archive.h>
#include <archive_entry.h>

#include <io.h>
#include <tchar.h>
#include <fcntl.h>
#include <conio.h>
#include <stdio.h>
#include <windows.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include <vector>
#include <string>
#include <map>

#include <memory>
#include <filesystem>
#include <functional>

#include "IAbstractArchive.h"
#include "IAbstractFactory.h"
#include "ArchiveFactory.h"

#pragma warning(disable : 4996)

using namespace std;
namespace fs = std::filesystem;

#ifndef UNICODE  
typedef string String;
#else
typedef wstring String;
#endif

class ArchiveFunction : public IAbstractArchive {
	int Flag;
	OPENFILENAME ofn = { 0 };
	string glob_path, glob_arch, tmp_path, woutname;
	wstring wstr;
	wchar_t file[1024] = { 0 };

	string lastWordFile(string word);
	string lastWordDirectory(string word);
	static int copy_data(struct archive* ar, struct archive* aw);
	void readArchive(const string path);
	void write_archive(const string outname_tmp, const vector<string> filename);
	void chekBrokenPath(vector<string>& vec);
	void openFile(HWND hWnd);
	void openArchive(HWND hWnd);
	void openDirectory();
	void saveFile();
	void extract(string archive_path, string save_path);
public:
	void extractArchive(HWND hWnd, wstring str, string& path, string& outname) override;
	void addFileInArchive() override;
	void close(HWND hWnd1, HWND hWnd2) override;
	void writeArchiveSingle(HWND hWnd, wstring str, string path, vector<string> argv, string outname) override;
	void writeArchiveDirectory(wstring str, string path, vector<string> argv, string outname) override;
	void selectFile(HWND& hWnd, wstring& str) override;
	void selectArchive(HWND& hWnd, wstring& str) override;
};