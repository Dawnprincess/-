#pragma once

#include <iostream>
#include <string>
#include <mysql.h>
using namespace std;
class Employee {
private:

public:
    Employee(){
    };
    ~Employee(){
    };

    static void showEmployeeView(MYSQL_RES *res,MYSQL_ROW row,MYSQL *conn,const string &worker_id) {
        string query = "SELECT * FROM employee_view WHERE worker_id = '" + worker_id + "';";
        if (mysql_query(conn, query.c_str()) == 0) {
            res = mysql_store_result(conn);
            if((row = mysql_fetch_row(res))) {
                cout << "Worker ID: " << row[0] << "\nName: " << row[1] << "\nGender: " << row[2] << "\nPhone: " << row[3] << "\nDepartment: " << row[4] << "\nBase Salary: " << row[5] << "\nBonus: " << row[6] << endl;
            }
            else{
                //如果没有找到worker_id对应的视图信息，但是可以登陆，则一定是工资salary信息没有录入，提示信息
                cout << "The salary information is not available for this employee." << endl;
            }
            mysql_free_result(res);
        } else {
            cerr << "Error displaying employee view: " << mysql_error(conn) << endl;
        }
    }
};