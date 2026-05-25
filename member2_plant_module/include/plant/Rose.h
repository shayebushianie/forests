#ifndef ROSE_H
#define ROSE_H

#include "Flower.h"

class Rose : public Flower
{
public:
    Rose();
    virtual ~Rose();
    
    virtual QString getImagePath() const override;
    virtual QString getTypeName() const override;
    virtual QString getDisplayName() const override;
    virtual QString getStageDescription() const override;
};

#endif // ROSE_H