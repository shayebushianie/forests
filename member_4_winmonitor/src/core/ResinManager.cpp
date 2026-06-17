#include "ResinManager.h"

#include <QFile>
#include <QDataStream>
#include <QDebug>

ResinManager::ResinManager(const QString& filePath)
    : m_filePath(filePath) { load(); }

ResinManager::~ResinManager() { save(); }

bool ResinManager::load() {
    QFile file(m_filePath);
    if (!file.exists()) { m_resin = 0; return true; }
    if (!file.open(QIODevice::ReadOnly)) return false;
    QDataStream in(&file);
    in >> m_resin;
    file.close();
    return true;
}

bool ResinManager::save() {
    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly)) return false;
    QDataStream out(&file);
    out << m_resin;
    file.close();
    return true;
}

void ResinManager::add(int amount) {
    if (amount <= 0) return;
    m_resin += amount;
    save();
}

bool ResinManager::spend(int amount) {
    if (amount <= 0 || amount > m_resin) return false;
    m_resin -= amount;
    save();
    return true;
}
