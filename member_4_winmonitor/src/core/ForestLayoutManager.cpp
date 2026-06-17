#include "ForestLayoutManager.h"

#include <QFile>
#include <QDataStream>
#include <QDebug>

ForestLayoutManager::ForestLayoutManager(const QString& filePath)
    : m_filePath(filePath) {}

bool ForestLayoutManager::load() {
    QFile file(m_filePath);
    if (!file.exists()) return true;

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "[ForestLayout] 无法打开文件" << m_filePath;
        return false;
    }

    QDataStream in(&file);
    quint32 count;
    in >> count;

    m_placements.clear();
    m_placements.reserve(count);

    for (quint32 i = 0; i < count; ++i) {
        ForestPlacement p;
        in >> p.recordId >> p.posX >> p.posY;
        m_placements.append(p);
    }

    file.close();
    qDebug() << "[ForestLayout] 加载" << count << "个摆放位置";
    return true;
}

bool ForestLayoutManager::save() {
    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "[ForestLayout] 无法保存" << m_filePath;
        return false;
    }

    QDataStream out(&file);
    out << (quint32)m_placements.size();
    for (const auto& p : m_placements) {
        out << p.recordId << p.posX << p.posY;
    }

    file.close();
    return true;
}

void ForestLayoutManager::place(int32_t recordId, int32_t posX, int32_t posY) {
    int idx = indexOf(recordId);
    if (idx >= 0) {
        m_placements[idx].posX = posX;
        m_placements[idx].posY = posY;
    } else {
        m_placements.append({ recordId, posX, posY });
    }
}

void ForestLayoutManager::movePlacement(int32_t recordId, int32_t posX, int32_t posY) {
    int idx = indexOf(recordId);
    if (idx >= 0) {
        m_placements[idx].posX = posX;
        m_placements[idx].posY = posY;
    }
}

void ForestLayoutManager::remove(int32_t recordId) {
    int idx = indexOf(recordId);
    if (idx >= 0) {
        m_placements.removeAt(idx);
    }
}

void ForestLayoutManager::clear() {
    m_placements.clear();
}

bool ForestLayoutManager::hasPlacement(int32_t recordId) const {
    return indexOf(recordId) >= 0;
}

const ForestPlacement* ForestLayoutManager::findPlacement(int32_t recordId) const {
    for (const auto& p : m_placements) {
        if (p.recordId == recordId) return &p;
    }
    return nullptr;
}

int ForestLayoutManager::indexOf(int32_t recordId) const {
    for (int i = 0; i < m_placements.size(); ++i) {
        if (m_placements[i].recordId == recordId) return i;
    }
    return -1;
}
