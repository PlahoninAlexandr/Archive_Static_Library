#pragma once

#include <vector>
#include <string>
#include <windows.h>

class IAbstractArchive {
public:
	virtual ~IAbstractArchive() {};
	virtual void extractArchive() = 0;
	virtual void addFileInArchive() = 0;
	virtual void writeArchiveSingle() = 0;
	virtual void writeArchiveDirectory() = 0;
	virtual void selectFile() = 0;
	virtual void selectArchive() = 0;
	virtual void Draw() = 0;
	virtual void DoArchiveParam(std::vector<float>& size_, std::vector<std::string>& name_, std::vector<int>& height_) = 0;
	virtual void DoFileParam(std::vector<float>& size_, std::vector<std::string>& name_, std::vector<int>& height_) = 0;
};