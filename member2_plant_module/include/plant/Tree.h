#ifndef TREE_H
#define TREE_H

#include "AbstractPlant.h"

/**
 * @brief 树类 - 第二层继承
 * @details 树的通用逻辑：生长周期较长，成熟需要更多时间
 */
class Tree : public AbstractPlant
{
public:
    Tree();
    virtual ~Tree();
    
    virtual void grow(int minutes) override;
    virtual void wither() override;
    virtual int getProgress() const override;
    
protected:
    void initTreeThresholds();  ///< 初始化树的生长阈值
};

#endif // TREE_H