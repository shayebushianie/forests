#ifndef FLOWER_H
#define FLOWER_H

#include "AbstractPlant.h"

/**
 * @brief 花类 - 第二层继承
 * @details 花的通用逻辑：生长周期较短，开花快
 */
class Flower : public AbstractPlant
{
public:
    Flower();
    virtual ~Flower();
    
    virtual void grow(int minutes) override;
    virtual void wither() override;
    virtual int getProgress() const override;
    
protected:
    void initFlowerThresholds();  ///< 初始化花的生长阈值
};

#endif // FLOWER_H