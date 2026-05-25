#ifndef PINETREE_H
#define PINETREE_H

#include "Tree.h"

class PineTree : public Tree
{
public:
    PineTree();
    virtual ~PineTree();
    
    virtual QString getImagePath() const override;
    virtual QString getTypeName() const override;
    virtual QString getDisplayName() const override;
    virtual QString getStageDescription() const override;
};

#endif // PINETREE_H