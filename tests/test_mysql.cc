#include <iostream>
#include "../alpha/db/mysql.h"
#include "../alpha/iomanager.h"

void run() {
    do {
        std::map<std::string, std::string> params;
        params["host"] = "127.0.0.1";
        params["user"] = "root";
        params["passwd"] = "883721";
        params["dbname"] = "alpha";

        alphaMin::MySQL::ptr mysql(new alphaMin::MySQL(params));
        if(!mysql->connect()) {
            std::cout << "connect fail" << std::endl;
            return;
        }

        //auto stmt = mysql_stmt_init(mysql->getRaw());
        //std::string sql = "select * from sylar where status >= ?";
        //mysql_stmt_prepare(stmt, sql.c_str(), sql.size());
        //MYSQL_BIND b;
        //int a = 0;
        //b.buffer_type = MYSQL_TYPE_LONG;
        //b.buffer = &a;
        //mysql_stmt_bind_param(m_

        // alphaMin::MySQLStmt::ptr stmt = alphaMin::MySQLStmt::Create(mysql, "update user set update_time = ? where id = 1");
        // stmt->bindString(1, "2018-01-01 10:10:10");
        // int rt = stmt->execute();
        // std::cout << "rt=" << rt << std::endl;

        //MYSQL_TIME mt;
        //sylar::time_t_to_mysql_time(time(0), mt);

        int a = 0;
        auto res = mysql->query("select * from tb_user");
        std::cout << res->getDataCount() << std::endl;
        std::cout << res->getColumnCount() << std::endl;
        while(res->next()) {
            int count = res->getColumnCount();
            for(int i = 0; i < count; ++i) {
                std::cout << res->getString(i) << "---";
            }
            std::cout << std::endl;
        } 

        // auto stmt = mysql->prepare("select * from tb_user");
        // stmt->bind(0, a);
        // auto res = std::dynamic_pointer_cast<alphaMin::MySQLStmtRes>(stmt->query());

        //auto res = std::dynamic_pointer_cast<sylar::MySQLStmtRes>
        //    //(mysql->queryStmt("select * from sylar"));
        //    (mysql->queryStmt("select *, 'hello' as xx from user where status >= ? and status <= ?"
        //                      , a, a));
        //    //(mysql->queryStmt("select id,name, keyword, creator as aa, last_update_time from sylar "
        //    //                  " where last_update_time > ?", (time_t)0));
        ////auto res = std::dynamic_pointer_cast<sylar::MySQLRes>
        ////    (mysql->query("select * from search_brand"));
        //if(!res) {
        //    std::cout << "invalid" << std::endl;
        //    return;
        //}
        //if(res->getErrno()) {
        //    std::cout << "errno=" << res->getErrno()
        //        << " errstr=" << res->getErrStr() << std::endl;
        //    return;
        //}

        //int i = 0;
        //while(res->next()) {
        //    ++i;
        //    std::cout << res->getInt64(0)
        //        << " - " << res->getInt64(1) << std::endl;
        //}
        //std::cout << "===" << i << std::endl;
    } while(false);
    std::cout << "over" << std::endl;
}

int main(int argc, char** argv) {
    alphaMin::IOManager iom(1);
    iom.schedule(run);
    // iom.addTimer(1000, run, true);
    return 0;
}