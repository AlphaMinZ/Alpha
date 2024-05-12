#ifndef __ALPHA_DB_MYSQL_H__
#define __ALPHA_DB_MYSQL_H__

#include <mysql/mysql.h>
#include <memory>
#include <functional>
#include <map>
#include <vector>
#include "../mutex.h"
#include "db.h"
#include "../singleton.h"

namespace alphaMin {

class MySQL;
class MySQLStmt;

struct MySQLTime {
    MySQLTime(time_t t)
        :ts(t) {}
    time_t ts;
};

bool mysql_time_to_time_t(const MYSQL_TIME& mt, time_t& ts);
bool time_t_to_mysql_time(const time_t& ts, MYSQL_TIME& mt);

// 查询结果
class MySQLRes : public ISQLData {
public:
    typedef std::shared_ptr<MySQLRes> ptr;
    typedef std::function<bool(MYSQL_ROW row
                ,int field_count, int row_no)> data_cb;
    MySQLRes(MYSQL_RES* res, int eno, const char* estr);

    MYSQL_RES* get() const { return m_data.get();}

    int getErrno() const { return m_errno;}
    const std::string& getErrStr() const { return m_errstr;}

    // 对查询结果集中的每一行调用回调函数 cb，并返回是否成功。
    bool foreach(data_cb cb);
    // 返回行数
    int getDataCount() override;
    // 一行数据的成员个数
    int getColumnCount() override;
    int getColumnBytes(int idx) override;
    int getColumnType(int idx) override;
    std::string getColumnName(int idx) override;

    bool isNull(int idx) override;
    int8_t getInt8(int idx) override;
    uint8_t getUint8(int idx) override;
    int16_t getInt16(int idx) override;
    uint16_t getUint16(int idx) override;
    int32_t getInt32(int idx) override;
    uint32_t getUint32(int idx) override;
    int64_t getInt64(int idx) override;
    uint64_t getUint64(int idx) override;
    float getFloat(int idx) override;
    double getDouble(int idx) override;
    std::string getString(int idx) override;
    std::string getBlob(int idx) override;
    time_t getTime(int idx) override;
    bool next() override;
private:
    int m_errno;
    std::string m_errstr;
    MYSQL_ROW m_cur;    // 表示当前查询结果集中的一行数据。
    unsigned long* m_curLength;
    std::shared_ptr<MYSQL_RES> m_data;  // 存储 MySQL 查询结果集对象的智能指针。
};

// 表示 MySQL 预处理语句的执行结果
class MySQLStmtRes : public ISQLData {
friend class MySQLStmt;
public:
    typedef std::shared_ptr<MySQLStmtRes> ptr;
    static MySQLStmtRes::ptr Create(std::shared_ptr<MySQLStmt> stmt);
    ~MySQLStmtRes();

    int getErrno() const { return m_errno;}
    const std::string& getErrStr() const { return m_errstr;}

    int getDataCount() override;
    int getColumnCount() override;
    int getColumnBytes(int idx) override;
    int getColumnType(int idx) override;
    std::string getColumnName(int idx) override;

    bool isNull(int idx) override;
    int8_t getInt8(int idx) override;
    uint8_t getUint8(int idx) override;
    int16_t getInt16(int idx) override;
    uint16_t getUint16(int idx) override;
    int32_t getInt32(int idx) override;
    uint32_t getUint32(int idx) override;
    int64_t getInt64(int idx) override;
    uint64_t getUint64(int idx) override;
    float getFloat(int idx) override;
    double getDouble(int idx) override;
    std::string getString(int idx) override;
    std::string getBlob(int idx) override;
    time_t getTime(int idx) override;
    bool next() override;
private:
    MySQLStmtRes(std::shared_ptr<MySQLStmt> stmt, int eno, const std::string& estr);
    struct Data {
        Data();
        ~Data();

        void alloc(size_t size);

#ifdef  my_bool
        my_bool is_null;
        my_bool error;
#else
        bool is_null;
        bool error;
#endif
        enum_field_types type;
        unsigned long length;
        int32_t data_length;
        char* data;
    };
private:
    int m_errno;
    std::string m_errstr;
    std::shared_ptr<MySQLStmt> m_stmt;
    std::vector<MYSQL_BIND> m_binds;
    std::vector<Data> m_datas;
};

class MySQLManager;

// MySQL 数据库连接
class MySQL : public IDB
              ,public std::enable_shared_from_this<MySQL> {
friend class MySQLManager;
public:
    typedef std::shared_ptr<MySQL> ptr;

    MySQL(const std::map<std::string, std::string>& args);

    bool connect();
    bool ping();

    virtual int execute(const char* format, ...) override;
    int execute(const char* format, va_list ap);
    virtual int execute(const std::string& sql) override;
    int64_t getLastInsertId() override;
    std::shared_ptr<MySQL> getMySQL();
    std::shared_ptr<MYSQL> getRaw();

    // 获取受影响的行数
    uint64_t getAffectedRows();

    ISQLData::ptr query(const char* format, ...) override;
    ISQLData::ptr query(const char* format, va_list ap); 
    ISQLData::ptr query(const std::string& sql) override;

    // 开启一个事务
    ITransaction::ptr openTransaction(bool auto_commit) override;
    // 准备SQL语句
    alphaMin::IStmt::ptr prepare(const std::string& sql) override;

    template<typename... Args>
    // 执行带有参数的 SQL 语句
    int execStmt(const char* stmt, Args&&... args);

    template<class... Args>
    // 执行带有参数的 SQL 查询
    ISQLData::ptr queryStmt(const char* stmt, Args&&... args);

    // 获取 MySQL 连接的命令字符串
    const char* cmd();

    // 切换当前数据库
    bool use(const std::string& dbname);
    int getErrno() override;
    std::string getErrStr() override;
    // 获取最后插入的 ID
    uint64_t getInsertId();
private:
    bool isNeedCheck();
private:
    // 连接参数
    std::map<std::string, std::string> m_params;
    // 连接对象
    std::shared_ptr<MYSQL> m_mysql;

    // 命令字符串
    std::string m_cmd;
    // 数据库名称
    std::string m_dbname;

    // 上次使用时间
    uint64_t m_lastUsedTime;
    bool m_hasError;
    // 连接池大小
    int32_t m_poolSize;
};

// MySQL 数据库的事务
class MySQLTransaction : public ITransaction {
public:
    typedef std::shared_ptr<MySQLTransaction> ptr;

    static MySQLTransaction::ptr Create(MySQL::ptr mysql, bool auto_commit);
    ~MySQLTransaction();

    bool begin() override;
    bool commit() override;
    bool rollback() override;

    virtual int execute(const char* format, ...) override;
    int execute(const char* format, va_list ap);
    virtual int execute(const std::string& sql) override;
    // 获取最后插入的 ID
    int64_t getLastInsertId() override;
    std::shared_ptr<MySQL> getMySQL();

    bool isAutoCommit() const { return m_autoCommit;}
    bool isFinished() const { return m_isFinished;}
    bool isError() const { return m_hasError;}
private:
    MySQLTransaction(MySQL::ptr mysql, bool auto_commit);
private:
    MySQL::ptr m_mysql;
    bool m_autoCommit;
    bool m_isFinished;
    bool m_hasError;
};

// MySQL 预处理语句
class MySQLStmt : public IStmt
                  ,public std::enable_shared_from_this<MySQLStmt> {
public:
    typedef std::shared_ptr<MySQLStmt> ptr;
    static MySQLStmt::ptr Create(MySQL::ptr db, const std::string& stmt);

    ~MySQLStmt();
    int bind(int idx, const int8_t& value);
    int bind(int idx, const uint8_t& value);
    int bind(int idx, const int16_t& value);
    int bind(int idx, const uint16_t& value);
    int bind(int idx, const int32_t& value);
    int bind(int idx, const uint32_t& value);
    int bind(int idx, const int64_t& value);
    int bind(int idx, const uint64_t& value);
    int bind(int idx, const float& value);
    int bind(int idx, const double& value);
    int bind(int idx, const std::string& value);
    int bind(int idx, const char* value);
    int bind(int idx, const void* value, int len);
    //int bind(int idx, const MYSQL_TIME& value, int type = MYSQL_TYPE_TIMESTAMP);
    //for null type
    int bind(int idx);

    int bindInt8(int idx, const int8_t& value) override;
    int bindUint8(int idx, const uint8_t& value) override;
    int bindInt16(int idx, const int16_t& value) override;
    int bindUint16(int idx, const uint16_t& value) override;
    int bindInt32(int idx, const int32_t& value) override;
    int bindUint32(int idx, const uint32_t& value) override;
    int bindInt64(int idx, const int64_t& value) override;
    int bindUint64(int idx, const uint64_t& value) override;
    int bindFloat(int idx, const float& value) override;
    int bindDouble(int idx, const double& value) override;
    int bindString(int idx, const char* value) override;
    int bindString(int idx, const std::string& value) override;
    int bindBlob(int idx, const void* value, int64_t size) override;
    int bindBlob(int idx, const std::string& value) override;
    //int bindTime(int idx, const MYSQL_TIME& value, int type = MYSQL_TYPE_TIMESTAMP);
    int bindTime(int idx, const time_t& value) override;
    int bindNull(int idx) override;

    int getErrno() override;
    std::string getErrStr() override;

    int execute() override;
    int64_t getLastInsertId() override;
    ISQLData::ptr query() override;

    MYSQL_STMT* getRaw() const { return m_stmt;}
private:
    MySQLStmt(MySQL::ptr db, MYSQL_STMT* stmt);
private:
    MySQL::ptr m_mysql;
    MYSQL_STMT* m_stmt;
    std::vector<MYSQL_BIND> m_binds;
};

// 管理 MySQL 数据库连接
class MySQLManager {
public:
    typedef alphaMin::Mutex MutexType;

    MySQLManager();
    ~MySQLManager();

    MySQL::ptr get(const std::string& name);
    void registerMySQL(const std::string& name, const std::map<std::string, std::string>& params);

    void checkConnection(int sec = 30);

    // 获取最大连接数
    uint32_t getMaxConn() const { return m_maxConn;}
    void setMaxConn(uint32_t v) { m_maxConn = v;}

    int execute(const std::string& name, const char* format, ...);
    int execute(const std::string& name, const char* format, va_list ap);
    int execute(const std::string& name, const std::string& sql);

    ISQLData::ptr query(const std::string& name, const char* format, ...);
    ISQLData::ptr query(const std::string& name, const char* format, va_list ap); 
    ISQLData::ptr query(const std::string& name, const std::string& sql);

    // 开启一个事务。需要传入连接名称和是否自动提交事务的标志。
    MySQLTransaction::ptr openTransaction(const std::string& name, bool auto_commit);
private:
    // 释放指定名称的 MySQL 连接对象
    void freeMySQL(const std::string& name, MySQL* m);
private:
    uint32_t m_maxConn;
    MutexType m_mutex;
    std::map<std::string, std::list<MySQL*> > m_conns;
    std::map<std::string, std::map<std::string, std::string> > m_dbDefines;
};

// MySQLUtil 工具类
class MySQLUtil {
public:
    static ISQLData::ptr Query(const std::string& name, const char* format, ...);
    static ISQLData::ptr Query(const std::string& name, const char* format,va_list ap); 
    static ISQLData::ptr Query(const std::string& name, const std::string& sql);

    static ISQLData::ptr TryQuery(const std::string& name, uint32_t count, const char* format, ...);
    static ISQLData::ptr TryQuery(const std::string& name, uint32_t count, const std::string& sql);

    static int Execute(const std::string& name, const char* format, ...);
    static int Execute(const std::string& name, const char* format, va_list ap); 
    static int Execute(const std::string& name, const std::string& sql);

    static int TryExecute(const std::string& name, uint32_t count, const char* format, ...);
    static int TryExecute(const std::string& name, uint32_t count, const char* format, va_list ap); 
    static int TryExecute(const std::string& name, uint32_t count, const std::string& sql);
};

typedef alphaMin::Singleton<MySQLManager> MySQLMgr;

namespace {

template<size_t N, typename... Args>
// 绑定参数到 MySQL 预处理语句
struct MySQLBinder {
    static int Bind(std::shared_ptr<MySQLStmt> stmt) { return 0; }
};

template<typename... Args>
int bindX(MySQLStmt::ptr stmt, Args&... args) {
    return MySQLBinder<1, Args...>::Bind(stmt, args...);
}
}

template<typename... Args>
// 执行 SQL 语句
int MySQL::execStmt(const char* stmt, Args&&... args) {
    auto st = MySQLStmt::Create(shared_from_this(), stmt);
    if(!st) {
        return -1;
    }
    int rt = bindX(st, args...);
    if(rt != 0) {
        return rt;
    }
    return st->execute();
}

template<class... Args>
// 执行查询语句
ISQLData::ptr MySQL::queryStmt(const char* stmt, Args&&... args) {
    auto st = MySQLStmt::Create(shared_from_this(), stmt);
    if(!st) {
        return nullptr;
    }
    int rt = bindX(st, args...);
    if(rt != 0) {
        return nullptr;
    }
    return st->query();
}

namespace {

template<size_t N, typename Head, typename... Tail>
struct MySQLBinder<N, Head, Tail...> {
    static int Bind(MySQLStmt::ptr stmt
                    ,const Head&, Tail&...) {
        //static_assert(false, "invalid type");
        static_assert(sizeof...(Tail) < 0, "invalid type");
        return 0;
    }
};

#define XX(type, type2) \
template<size_t N, typename... Tail> \
struct MySQLBinder<N, type, Tail...> { \
    static int Bind(MySQLStmt::ptr stmt \
                    , type2 value \
                    , Tail&... tail) { \
        int rt = stmt->bind(N, value); \
        if(rt != 0) { \
            return rt; \
        } \
        return MySQLBinder<N + 1, Tail...>::Bind(stmt, tail...); \
    } \
};

//template<size_t N, typename... Tail>
//struct MySQLBinder<N, const char(&)[], Tail...> {
//    static int Bind(MySQLStmt::ptr stmt
//                    , const char value[]
//                    , const Tail&... tail) {
//        int rt = stmt->bind(N, (const char*)value);
//        if(rt != 0) {
//            return rt;
//        }
//        return MySQLBinder<N + 1, Tail...>::Bind(stmt, tail...);
//    }
//};

XX(char*, char*);
XX(const char*, char*);
XX(std::string, std::string&);
XX(int8_t, int8_t&);
XX(uint8_t, uint8_t&);
XX(int16_t, int16_t&);
XX(uint16_t, uint16_t&);
XX(int32_t, int32_t&);
XX(uint32_t, uint32_t&);
XX(int64_t, int64_t&);
XX(uint64_t, uint64_t&);
XX(float, float&);
XX(double, double&);
//XX(MYSQL_TIME, MYSQL_TIME&);
#undef XX

}

}

#endif