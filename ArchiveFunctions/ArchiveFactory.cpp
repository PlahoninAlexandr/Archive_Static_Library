#include "ArchiveFactory.h"

IAbstractArchive* ArchiveFactory::CreateProductA() {
	return new ArchiveFunction();
}