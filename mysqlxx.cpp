// Copyright (c) 2014. All rights reserved.

#include <stdio.h>
#include <mysql.h>
#include "mysql/mysqlxx.h"

using namespace std;

MysqlXX::MysqlXX() : mysql_(NULL), names_("utf8")
{
    mysql_ = mysql_init(mysql_);
    // TODO(check mysql_ is not NULL)

    my_bool value = 1;
    mysql_options(mysql_, MYSQL_OPT_RECONNECT, &value);
    // TODO(check return value)
}

MysqlXX::~MysqlXX()
{
    Close();
}

int MysqlXX::Connect(const std::string &host, 
                     const std::string &user, 
                     const std::string &passwd, 
                     const std::string &db,
                     unsigned int port,
                     const std::string &unix_socket,
                     unsigned long client_flag)
{
    if (mysql_real_connect(mysql_, 
                           host.c_str(), 
                           user.c_str(), 
                           passwd.c_str(), 
                           db.c_str(), 
                           port, 
                           unix_socket.c_str(),
                           client_flag) == NULL) {
        errno_ = mysql_errno(mysql_);
        error_ = mysql_error(mysql_);
        return -1;
    }

    return SetNames(names_);
}

void MysqlXX::Close()
{
    if (mysql_ != NULL) {
        mysql_close(mysql_);
        mysql_ = NULL;
    }
}

int MysqlXX::SetNames(const string& csname)
{
    int status;

    status = mysql_set_character_set(mysql_, csname.c_str());
    if (status != 0) {
        errno_ = mysql_errno(mysql_);
        error_ = mysql_error(mysql_);
        return -1;
    }

    names_ = csname;

    return 0;
}

string MysqlXX::GetNames() const
{
    //return mysql_character_set_name(mysql_);
    return names_;
}

int MysqlXX::Query(const std::string &sql)
{ 
    int status;

    Reset();

    status = mysql_real_query(mysql_, sql.c_str(), sql.size());    
    if (status != 0) {
        errno_ = mysql_errno(mysql_);
        error_ = mysql_error(mysql_);
        return -1;
    }

    MYSQL_RES *result = mysql_store_result(mysql_);
    unsigned int field_count = mysql_field_count(mysql_);
    if (result == NULL && field_count != 0) {
        // mysql_store_result() should have returned data
        errno_ = mysql_errno(mysql_);
        error_ = mysql_error(mysql_);
        return -1;
    } else if (result == NULL && field_count == 0) {
            // query does not return data (it was not a SELECT)
            affected_row_count_ = mysql_affected_rows(mysql_);
            inserted_id_ = mysql_insert_id(mysql_);
    } else {
        unsigned int i = 0;
        std::map<unsigned int, std::string> field_id_to_field_name;
        
        MYSQL_FIELD *field;
        while ((field = mysql_fetch_field(result))) {
            field_id_to_field_name[i] = field->name;
            ++i;
        }

        MYSQL_ROW mysql_row;
        Row row;
        while ((mysql_row = mysql_fetch_row(result))) {
            unsigned long *lengths = mysql_fetch_lengths(result);
            for (i = 0; i < field_count; ++i) {
                if (mysql_row[i] == NULL) {
                    row[field_id_to_field_name[i]] = "";
                } else {
                    row[field_id_to_field_name[i]] = string(mysql_row[i], lengths[i]);
                }
            }

            rows_.push_back(row);
        }

        mysql_free_result(result);
    }

    return 0;
}

uint64_t MysqlXX::GetAffectedRowCount() const
{
    return affected_row_count_;
}

uint64_t MysqlXX::GetInsertedId() const
{
    return inserted_id_;
}

MysqlXX::iterator MysqlXX::begin()
{
    return rows_.begin();
}

MysqlXX::const_iterator MysqlXX::begin() const
{
    return rows_.begin();
}

MysqlXX::iterator MysqlXX::end()
{
    return rows_.end();
}

MysqlXX::const_iterator MysqlXX::end() const
{
    return rows_.end();
}

void MysqlXX::Reset()
{
    errno_ = 0;
    error_.clear();
    affected_row_count_ = 0;
    inserted_id_ = 0;
    rows_.clear();
}

int MysqlXX::GetLastErrno() const
{
    return errno_;
}

std::string MysqlXX::GetLastError() const
{
    return error_;
}
