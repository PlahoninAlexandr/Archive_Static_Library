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
	int Flag, number, lft = 70, rht = 50;
	OPENFILENAME ofn = { 0 };
	string glob_path, global_archive, tmp_path, woutname, path, outname;
	vector<pair<float, string>> amount_size_files;
	vector<string> argv;
	vector<float> size;
	vector<string> name;
	vector<int> height;
	float max_size, size_archive;
	wstring wstr, str;
	wchar_t file[1024] = { 0 };
	PAINTSTRUCT ps;
	HFONT hFont = CreateFont(15, 7, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH, L"Times New Roman");
	RECT r;
	HDC hdc;
	HWND hWnd1;

	string lastWordFile(string word);
	string lastWordDirectory(string word);
	static int copy_data(struct archive* ar, struct archive* aw);
	void readArchive(const string path);
	void write_archive(const string outname_tmp, const vector<string> filename);
	void chekBrokenPath(vector<string>& vec);
	void openFile();
	void openArchive();
	void openDirectory();
	void saveFile();
	void extract(string archive_path, string save_path);
	vector<pair<float, string>> sizeFiles(const string path, int& count);
	void ratioCpp(float max, vector<float> vec, vector<int>& hg);
	void ratioCs(float max, vector<float> vec, vector<int>& hg);
public:
	void extractArchive() override;
	void addFileInArchive() override;
	void writeArchiveSingle() override;
	void writeArchiveDirectory() override;
	void selectFile() override;
	void selectArchive() override;
	void Draw(HWND& hWnd) override;
	void DoArchiveParam(std::vector<float>& size_, std::vector<std::string>& name_, std::vector<int>& height_) override;
	void DoFileParam(std::vector<float>& size_, std::vector<std::string>& name_, std::vector<int>& height_) override;
};