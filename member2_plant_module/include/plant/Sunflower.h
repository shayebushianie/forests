#ifndef SUNFLOWER_H
#define SUNFLOWER_H

#include "Flower.h"

class Sunflower : public Flower
{
public:
    Sunflower();
    virtual ~Sunflower();
    
    virtual QString getImagePath() const override;
    virtual QString getTypeName() const override;
    virtual QString getDisplayName() const override;
    virtual QString getStageDescription() const override;
};

#endif // SUNFLOWER_H