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
	string glob_path, glob_arch, tmp_path, woutname, path, outname;
	vector<string> argv;
	wstring wstr, str;
	wchar_t file[1024] = { 0 };

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
public:
	void extractArchive() override;
	void addFileInArchive() override;
	void writeArchiveSingle() override;
	void writeArchiveDirectory() override;
	void selectFile() override;
	void selectArchive() override;
};