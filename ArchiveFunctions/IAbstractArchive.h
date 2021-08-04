#pragma once

#include <Windows.h>
#include <string>
#include <vector>

class IAbstractArchive {
public:
	virtual ~IAbstractArchive() {};
	virtual void extractArchive() = 0;
	virtual void addFileInArchive() = 0;
	virtual void writeArchiveSingle() = 0;
	virtual void writeArchiveDirectory() = 0;
	virtual void selectFile() = 0;
	virtual void selectArchive() = 0;
};