/*

#include "MongoDB.h"

// MongoData Implementation

MongoData::MongoData(mongocxx::cursor cursor)
    : _cursor(std::move(cursor)), _iterator(_cursor.begin()) {}

MongoData::~MongoData() {}

bool MongoData::next() {
    if (_iterator == _cursor.end()) return false;
    _current_doc = *_iterator;
    ++_iterator;
    return true;
}

bsoncxx::document::view MongoData::current() {
    if (_current_doc) {
        return _current_doc;
    }
    throw std::out_of_range("No current document");
}

// MongoUpdate Implementation

MongoUpdate::MongoUpdate(mongocxx::collection& collection)
    : _collection(collection) {}

MongoUpdate::~MongoUpdate() {}

int MongoUpdate::insertOne(const bsoncxx::document::view_or_value& doc) {
    try {
        auto result = _collection.insert_one(doc);
        return result ? 0 : -1;
    } catch (const mongocxx::exception& e) {
        // Handle exception
        return -1;
    }
}

int MongoUpdate::insertMany(const std::vector<bsoncxx::document::view_or_value>& docs) {
    try {
        auto result = _collection.insert_many(docs);
        return result ? 0 : -1;
    } catch (const mongocxx::exception& e) {
        // Handle exception
        return -1;
    }
}

int MongoUpdate::updateOne(const bsoncxx::document::view_or_value& filter, const bsoncxx::document::view_or_value& update) {
    try {
        auto result = _collection.update_one(filter, update);
        return result ? 0 : -1;
    } catch (const mongocxx::exception& e) {
        // Handle exception
        return -1;
    }
}

int MongoUpdate::updateMany(const bsoncxx::document::view_or_value& filter, const bsoncxx::document::view_or_value& update) {
    try {
        auto result = _collection.update_many(filter, update);
        return result ? 0 : -1;
    } catch (const mongocxx::exception& e) {
        // Handle exception
        return -1;
    }
}

int MongoUpdate::deleteOne(const bsoncxx::document::view_or_value& filter) {
    try {
        auto result = _collection.delete_one(filter);
        return result ? 0 : -1;
    } catch (const mongocxx::exception& e) {
        // Handle exception
        return -1;
    }
}

int MongoUpdate::deleteMany(const bsoncxx::document::view_or_value& filter) {
    try {
        auto result = _collection.delete_many(filter);
        return result ? 0 : -1;
    } catch (const mongocxx::exception& e) {
        // Handle exception
        return -1;
    }
}

// MongoQuery Implementation

MongoQuery::MongoQuery(mongocxx::collection& collection)
    : _collection(collection) {}

MongoQuery::~MongoQuery() {}

MongoData::ptr MongoQuery::find(const bsoncxx::document::view_or_value& filter) {
    mongocxx::cursor cursor = _collection.find(filter);
    return std::make_shared<MongoData>(std::move(cursor));
}

MongoData::ptr MongoQuery::findAll() {
    mongocxx::cursor cursor = _collection.find({});
    return std::make_shared<MongoData>(std::move(cursor));
}

// MongoStmt Implementation

MongoStmt::MongoStmt(mongocxx::collection& collection)
    : _collection(collection) {}

MongoStmt::~MongoStmt() {}

int MongoStmt::bind(const bsoncxx::document::view_or_value& doc) {
    _doc = doc;
    return 0;
}

int MongoStmt::execute() {
    try {
        auto result = _collection.insert_one(_doc);
        return result ? 0 : -1;
    } catch (const mongocxx::exception& e) {
        // Handle exception
        return -1;
    }
}

MongoData::ptr MongoStmt::query(const bsoncxx::document::view_or_value& filter) {
    mongocxx::cursor cursor = _collection.find(filter);
    return std::make_shared<MongoData>(std::move(cursor));
}

// MongoTransaction Implementation

MongoTransaction::MongoTransaction(mongocxx::client& client)
    : _session(client.start_session()) {}

MongoTransaction::~MongoTransaction() {}

bool MongoTransaction::begin() {
    try {
        _session.start_transaction();
        return true;
    } catch (const mongocxx::exception& e) {
        // Handle exception
        return false;
    }
}

bool MongoTransaction::commit() {
    try {
        _session.commit_transaction();
        return true;
    } catch (const mongocxx::exception& e) {
        // Handle exception
        return false;
    }
}

bool MongoTransaction::rollback() {
    try {
        _session.abort_transaction();
        return true;
    } catch (const mongocxx::exception& e) {
        // Handle exception
        return false;
    }
}

// MongoDB Implementation

MongoDB::MongoDB(const std::string& uri)
    : _client(mongocxx::uri(uri)) {}

MongoDB::~MongoDB() {}

MongoQuery::ptr MongoDB::getQuery(const std::string& db_name, const std::string& collection_name) {
    auto collection = _client[db_name][collection_name];
    return std::make_shared<MongoQuery>(collection);
}

MongoUpdate::ptr MongoDB::getUpdate(const std::string& db_name, const std::string& collection_name) {
    auto collection = _client[db_name][collection_name];
    return std::make_shared<MongoUpdate>(collection);
}

MongoStmt::ptr MongoDB::prepare(const std::string& db_name, const std::string& collection_name) {
    auto collection = _client[db_name][collection_name];
    return std::make_shared<MongoStmt>(collection);
}

MongoTransaction::ptr MongoDB::openTransaction() {
    return std::make_shared<MongoTransaction>(_client);
}

int MongoDB::getErrno() {
    // Implement error handling
    return 0;
}

std::string MongoDB::getErrStr() {
    // Implement error handling
    return "";
}

*/