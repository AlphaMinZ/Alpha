#ifndef __ALPHA_DB_MONGO_H__
#define __ALPHA_DB_MONGO_H__

/*

#include <memory>
#include <string>
#include <vector>
#include <bsoncxx/json.hpp>
#include <bsoncxx/types.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/exception/exception.hpp>
#include <mongocxx/collection.hpp>
#include <mongocxx/transaction.hpp>
#include <mongocxx/stdx.hpp>

class MongoData {
public:
    using ptr = std::shared_ptr<MongoData>;
    MongoData(mongocxx::cursor cursor);
    virtual ~MongoData();

    bool next();
    bsoncxx::document::view current();

private:
    mongocxx::cursor _cursor;
    mongocxx::cursor::iterator _iterator;
    bsoncxx::document::view _current_doc;
};

class MongoUpdate {
public:
    MongoUpdate(mongocxx::collection& collection);
    virtual ~MongoUpdate();

    int insertOne(const bsoncxx::document::view_or_value& doc);
    int insertMany(const std::vector<bsoncxx::document::view_or_value>& docs);
    int updateOne(const bsoncxx::document::view_or_value& filter, const bsoncxx::document::view_or_value& update);
    int updateMany(const bsoncxx::document::view_or_value& filter, const bsoncxx::document::view_or_value& update);
    int deleteOne(const bsoncxx::document::view_or_value& filter);
    int deleteMany(const bsoncxx::document::view_or_value& filter);

private:
    mongocxx::collection& _collection;
};

class MongoQuery {
public:
    MongoQuery(mongocxx::collection& collection);
    virtual ~MongoQuery();

    MongoData::ptr find(const bsoncxx::document::view_or_value& filter);
    MongoData::ptr findAll();

private:
    mongocxx::collection& _collection;
};

class MongoStmt {
public:
    using ptr = std::shared_ptr<MongoStmt>;
    MongoStmt(mongocxx::collection& collection);
    virtual ~MongoStmt();

    int bind(const bsoncxx::document::view_or_value& doc);
    int execute();
    MongoData::ptr query(const bsoncxx::document::view_or_value& filter);

private:
    mongocxx::collection& _collection;
    bsoncxx::document::view_or_value _doc;
};

class MongoTransaction {
public:
    using ptr = std::shared_ptr<MongoTransaction>;
    MongoTransaction(mongocxx::client& client);
    virtual ~MongoTransaction();

    bool begin();
    bool commit();
    bool rollback();

private:
    mongocxx::client_session _session;
};

class MongoDB {
public:
    using ptr = std::shared_ptr<MongoDB>;
    MongoDB(const std::string& uri);
    virtual ~MongoDB();

    MongoQuery::ptr getQuery(const std::string& db_name, const std::string& collection_name);
    MongoUpdate::ptr getUpdate(const std::string& db_name, const std::string& collection_name);
    MongoStmt::ptr prepare(const std::string& db_name, const std::string& collection_name);
    MongoTransaction::ptr openTransaction();

    int getErrno();
    std::string getErrStr();

private:
    mongocxx::client _client;
};

*/

#endif