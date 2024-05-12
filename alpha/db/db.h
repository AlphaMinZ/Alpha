#ifndef __ALPHA_DB_DB_H__
#define __ALPHA_DB_DB_H__

#include <memory>
#include <string>

namespace alphaMin {

/*
    ISQLData: 表示 SQL 查询结果的数据接口，提供了获取查询结果的各种方法，
    如获取错误信息、获取数据行数、获取列数、获取列类型以及获取各种数据类型的值等。
*/
class ISQLData {
public:
    typedef std::shared_ptr<ISQLData> ptr;
    virtual ~ISQLData() {}

    virtual int getDataCount() = 0;
    virtual int getColumnCount() = 0;
    virtual int getColumnBytes(int idx) = 0;
    virtual int getColumnType(int idx) = 0;
    virtual std::string getColumnName(int idx) = 0;

    virtual bool isNull(int idx) = 0;
    virtual int8_t getInt8(int idx) = 0;
    virtual uint8_t getUint8(int idx) = 0;
    virtual int16_t getInt16(int idx) = 0;
    virtual uint16_t getUint16(int idx) = 0;
    virtual int32_t getInt32(int idx) = 0;
    virtual uint32_t getUint32(int idx) = 0;
    virtual int64_t getInt64(int idx) = 0;
    virtual uint64_t getUint64(int idx) = 0;
    virtual float getFloat(int idx) = 0;
    virtual double getDouble(int idx) = 0;
    virtual std::string getString(int idx) = 0;
    virtual std::string getBlob(int idx) = 0;
    virtual time_t getTime(int idx) = 0;
    virtual bool next() = 0;
};

class ISQLUpdate {
public:
    virtual ~ISQLUpdate() {}
    virtual int execute(const char* format, ...) = 0;
    virtual int execute(const std::string& sql) = 0;
    virtual int64_t getLastInsertId() = 0;
};

class ISQLQuery {
public:
    virtual ~ISQLQuery() {}
    virtual ISQLData::ptr query(const char* format, ...) = 0;
    virtual ISQLData::ptr query(const std::string& sql) = 0;
};

class IStmt {
public:
    typedef std::shared_ptr<IStmt> ptr;
    virtual ~IStmt(){}
    virtual int bindInt8(int idx, const int8_t& value) = 0;
    virtual int bindUint8(int idx, const uint8_t& value) = 0;
    virtual int bindInt16(int idx, const int16_t& value) = 0;
    virtual int bindUint16(int idx, const uint16_t& value) = 0;
    virtual int bindInt32(int idx, const int32_t& value) = 0;
    virtual int bindUint32(int idx, const uint32_t& value) = 0;
    virtual int bindInt64(int idx, const int64_t& value) = 0;
    virtual int bindUint64(int idx, const uint64_t& value) = 0;
    virtual int bindFloat(int idx, const float& value) = 0;
    virtual int bindDouble(int idx, const double& value) = 0;
    virtual int bindString(int idx, const char* value) = 0;
    virtual int bindString(int idx, const std::string& value) = 0;
    virtual int bindBlob(int idx, const void* value, int64_t size) = 0;
    virtual int bindBlob(int idx, const std::string& value) = 0;
    virtual int bindTime(int idx, const time_t& value) = 0;
    virtual int bindNull(int idx) = 0;

    virtual int execute() = 0;
    virtual int64_t getLastInsertId() = 0;
    virtual ISQLData::ptr query() = 0;

    virtual int getErrno() = 0;
    virtual std::string getErrStr() = 0;
};

class ITransaction : public ISQLUpdate {
public:
    typedef std::shared_ptr<ITransaction> ptr;
    virtual ~ITransaction() {};
    virtual bool begin() = 0;
    virtual bool commit() = 0;
    virtual bool rollback() = 0;
};

class IDB : public ISQLUpdate
            ,public ISQLQuery {
public:
    typedef std::shared_ptr<IDB> ptr;
    virtual ~IDB() {}

    virtual IStmt::ptr prepare(const std::string& stmt) = 0;
    virtual int getErrno() = 0;
    virtual std::string getErrStr() = 0;
    virtual ITransaction::ptr openTransaction(bool auto_commit = false) = 0;
};

}

#endif
