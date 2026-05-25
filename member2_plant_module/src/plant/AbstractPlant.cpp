#include "plant/AbstractPlant.h"

AbstractPlant::AbstractPlant()
    : m_totalMinutes(0)
    , m_isWithered(false)
{
    m_growThresholds[0] = 30;
    m_growThresholds[1] = 120;
    m_growThresholds[2] = 240;
}

AbstractPlant::~AbstractPlant() {}

QByteArray AbstractPlant::serialize() const
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << m_totalMinutes << m_isWithered << getTypeName();
    return data;
}

void AbstractPlant::deserialize(const QByteArray& data)
{
    QDataStream stream(data);
    QString typeName;
    stream >> m_totalMinutes >> m_isWithered >> typeName;
    // typeName 用于校验，实际植物类型由工厂决定
}

void AbstractPlant::reset()
{
    m_totalMinutes = 0;
    m_isWithered = false;
}