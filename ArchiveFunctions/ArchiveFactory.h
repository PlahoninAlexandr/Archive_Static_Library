#pragma once
#include "IAbstractFactory.h"
#include "ArchiveFunction.h"

class ArchiveFactory : public AbstractFactory {
public:
    IAbstractArchive* CreateProductA() override;
};
