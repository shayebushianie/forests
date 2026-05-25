#ifndef OAKTREE_H
#define OAKTREE_H

#include "Tree.h"

/**
 * @brief 橡树 - 第三层具体植物
 */
class OakTree : public Tree
{
public:
    OakTree();
    virtual ~OakTree();
    
    virtual QString getImagePath() const override;
    virtual QString getTypeName() const override;
    virtual QString getDisplayName() const override;
    virtual QString getStageDescription() const override;
};

#endif // OAKTREE_H