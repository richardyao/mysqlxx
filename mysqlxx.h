// Copyright (c) 2014. All rights reserved.

#ifndef MYSQL_MYSQLXX_H
#define MYSQL_MYSQLXX_H

#include <string>
#include <vector>
#include <map>
#include <stdint.h>

// MysqlXX mysql;
// mysql.Connect();
// mysql.Query();
// for (mysql::iterator i = mysql.begin(); i != mysql.end(); ++i) {
//     printf("id %s\n", (*i)[id]);
// }
// mysql.Close();
typedef struct st_mysql MYSQL;

class MysqlXX
{
public:
    typedef std::map<std::string, std::string> Row;
    typedef std::vector<Row>::iterator iterator;
    typedef std::vector<Row>::const_iterator const_iterator;

    MysqlXX();
    ~MysqlXX();

    int Connect(const std::string &host, 
                const std::string &user, 
                const std::string &passwd, 
                const std::string &db = "",
                unsigned int port = 3306,
                const std::string &unix_socket = "",
                unsigned long client_flag = 0);
    void Close();

    int SetNames(const std::string &csname);
    std::string GetNames() const;
    int Query(const std::string &sql);
    uint64_t GetAffectedRowCount() const;
    uint64_t GetInsertedId() const;

    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;
    size_t size() const;
    size_t count() const;

    const std::vector<Row>& CopyRows() const;
    
    int GetLastErrno() const;
    std::string GetLastError() const;

private:
    void Reset();

    MYSQL* mysql_;
    int errno_;
    std::string error_;
    std::string names_;
    uint64_t affected_row_count_;
    uint64_t inserted_id_;
    std::vector<Row> rows_;
};

#endif //MYSQL_MYSQLXX_H
