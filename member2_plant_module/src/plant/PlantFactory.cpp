#include "plant/PlantFactory.h"
#include "plant/OakTree.h"
#include "plant/PineTree.h"
#include "plant/Rose.h"
#include "plant/Sunflower.h"

std::map<QString, std::function<AbstractPlant*()>> PlantFactory::s_creators;
bool PlantFactory::s_registered = false;

AbstractPlant* PlantFactory::createPlant(const QString& typeName)
{
    if (!s_registered) {
        registerDefaults();
    }
    
    auto it = s_creators.find(typeName);
    if (it != s_creators.end()) {
        return it->second();
    }
    return nullptr;
}

QStringList PlantFactory::getAvailablePlantTypes()
{
    if (!s_registered) {
        registerDefaults();
    }
    
    QStringList types;
    for (const auto& pair : s_creators) {
        types.append(pair.first);
    }
    return types;
}

void PlantFactory::registerPlant(const QString& typeName, 
                                  std::function<AbstractPlant*()> creator)
{
    s_creators[typeName] = creator;
}

void PlantFactory::registerDefaults()
{
    registerPlant("OakTree", []() { return new OakTree(); });
    registerPlant("PineTree", []() { return new PineTree(); });
    registerPlant("Rose", []() { return new Rose(); });
    registerPlant("Sunflower", []() { return new Sunflower(); });
    s_registered = true;
}