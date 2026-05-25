#ifndef PLANTFACTORY_H
#define PLANTFACTORY_H

#include <QString>
#include <QStringList>
#include <map>
#include <functional>

class AbstractPlant;

/**
 * @brief 植物工厂类
 * @details 根据植物类型名称创建对应的植物对象实例
 */
class PlantFactory
{
public:
    static AbstractPlant* createPlant(const QString& typeName);
    static QStringList getAvailablePlantTypes();
    static void registerPlant(const QString& typeName, 
                              std::function<AbstractPlant*()> creator);

private:
    static std::map<QString, std::function<AbstractPlant*()>> s_creators;
    static bool s_registered;
    static void registerDefaults();
};

#endif // PLANTFACTORY_H