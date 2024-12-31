#pragma once

#include <iostream>
#include <mysql.h>
#include <string>

using namespace std;


class Database {
private:
    
public:
    MYSQL_ROW row;
    MYSQL_RES *res;
    MYSQL *conn;
    Database() {
        conn = mysql_init(0);
        conn = mysql_real_connect(conn, "localhost", "root", "123456", "em", 3306, NULL, 0);
        if (conn) {
            cout << "Database connected successfully!\n";
        } else {
            cerr << "Failed to connect to database: " << mysql_error(conn) << endl;
            exit(1);
        }
    }
    ~Database() {

        mysql_close(conn);
    }
    void ConnectDB( string user, string password,string host = "localhost", string database = "em", int port = 3306){
        mysql_close(conn);
        conn = mysql_init(0);
        conn = mysql_real_connect(conn, host.c_str(), user.c_str(), password.c_str(), database.c_str(), port, NULL, 0);
        if (conn) {
            //添加user欢迎信息
            cout << "Hello " << user << "!\n";
        } else {
            cerr << "Failed to connect to database: " << mysql_error(conn) << endl;
            exit(1);
        }
    }

    bool checkWorkerExists(const string &worker_id) {
        //循环检查worker_id每一位是否为数字
        for(int i=0;i<worker_id.length();i++){
            if(worker_id[i]<'0' || worker_id[i]>'9'){
                return false;
            }
        }
        
        string query = "SELECT * FROM employee WHERE worker_id = '" + worker_id + "';";
        if (mysql_query(conn, query.c_str()) == 0) {
            res = mysql_store_result(conn);
            if (mysql_num_rows(res) > 0) {
                mysql_free_result(res);
                return true;
            }
        }
        return false;
    }

    bool verifyPassword(const string &worker_id, const string &password) {
        string query = "SELECT * FROM employee WHERE worker_id = '" + worker_id + "' AND password = '" + password + "';";
        if (mysql_query(conn, query.c_str()) == 0) {
            res = mysql_store_result(conn);
            if (mysql_num_rows(res) > 0) {
                mysql_free_result(res);
                return true;
            }
        }
        return false;
    }

};

