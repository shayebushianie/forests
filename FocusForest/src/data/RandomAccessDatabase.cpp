#include "RandomAccessDatabase.h"
#include <QDebug>
#include <cstring>

RandomAccessDatabase::RandomAccessDatabase(const QString& filePath)
    : m_filePath(filePath)
    , m_recordCount(0) {}

RandomAccessDatabase::~RandomAccessDatabase() {
    close();
}

bool RandomAccessDatabase::open() {
    // 以二进制读写模式打开，若不存在则创建
    m_file.open(m_filePath.toStdString(),
                std::ios::binary | std::ios::in | std::ios::out | std::ios::ate);

    if (!m_file.is_open()) {
        // 文件不存在，尝试创建
        m_file.open(m_filePath.toStdString(),
                    std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
        if (!m_file.is_open()) {
            qWarning() << "[Database] 无法创建文件:" << m_filePath;
            return false;
        }
        // 初始化文件头：写入记录计数 0
        m_recordCount = 0;
        saveHeader();
    }

    loadHeader();
    qDebug() << "[Database] 已打开:" << m_filePath
             << " 记录数:" << m_recordCount;
    return true;
}

void RandomAccessDatabase::close() {
    if (m_file.is_open()) {
        saveHeader();
        m_file.close();
    }
}

bool RandomAccessDatabase::writeRecord(const FocusRecord& record) {
    if (!m_file.is_open()) return false;

    // 定位到文件末尾（追加写入）
    m_file.seekp(0, std::ios::end);
    m_file.write(reinterpret_cast<const char*>(&record), RECORD_SIZE);

    if (!m_file.good()) {
        qWarning() << "[Database] 写入失败";
        return false;
    }

    m_recordCount++;
    saveHeader();  // 更新文件头计数
    return true;
}

bool RandomAccessDatabase::readRecord(int index, FocusRecord& record) const {
    if (!m_file.is_open() || index < 0 || index >= m_recordCount) {
        return false;
    }

    // 计算偏移：HEADER_SIZE + index * RECORD_SIZE
    std::streampos offset = HEADER_SIZE + index * RECORD_SIZE;
    m_file.seekg(offset);

    if (!m_file.good()) return false;

    m_file.read(reinterpret_cast<char*>(&record), RECORD_SIZE);
    return m_file.good();
}

bool RandomAccessDatabase::updateRecord(int index, const FocusRecord& record) {
    if (!m_file.is_open() || index < 0 || index >= m_recordCount) {
        return false;
    }

    // 定位并覆盖写入
    std::streampos offset = HEADER_SIZE + index * RECORD_SIZE;
    m_file.seekp(offset);
    m_file.write(reinterpret_cast<const char*>(&record), RECORD_SIZE);

    return m_file.good();
}

bool RandomAccessDatabase::deleteRecord(int index) {
    FocusRecord record;
    if (!readRecord(index, record)) return false;

    // 逻辑删除：标记为失败
    record.isSuccess = 0;
    return updateRecord(index, record);
}

std::vector<FocusRecord> RandomAccessDatabase::readAll() const {
    std::vector<FocusRecord> records;
    records.reserve(m_recordCount);

    for (int i = 0; i < m_recordCount; ++i) {
        FocusRecord rec;
        if (readRecord(i, rec)) {
            records.push_back(rec);
        }
    }
    return records;
}

void RandomAccessDatabase::loadHeader() {
    if (!m_file.is_open()) return;

    m_file.seekg(0);
    m_file.read(reinterpret_cast<char*>(&m_recordCount), HEADER_SIZE);

    if (!m_file.good()) {
        m_recordCount = 0;
    }
}

void RandomAccessDatabase::saveHeader() {
    if (!m_file.is_open()) return;

    m_file.seekp(0);
    m_file.write(reinterpret_cast<const char*>(&m_recordCount), HEADER_SIZE);
    m_file.flush();
}
