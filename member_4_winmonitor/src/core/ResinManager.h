#ifndef RESINMANAGER_H
#define RESINMANAGER_H

#include <QString>

class ResinManager {
public:
    explicit ResinManager(const QString& filePath);
    ~ResinManager();

    bool load();
    bool save();

    int balance() const { return m_resin; }

    void add(int amount);
    bool spend(int amount);

private:
    QString m_filePath;
    int     m_resin = 0;
};

#endif
