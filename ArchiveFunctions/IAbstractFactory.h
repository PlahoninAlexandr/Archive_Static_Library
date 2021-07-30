#pragma once
#include "IAbstractArchive.h"

class AbstractFactory {
public:
	virtual IAbstractArchive* CreateProductA() = 0;
};