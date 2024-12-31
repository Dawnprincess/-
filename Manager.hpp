#pragma once

#include <iostream>
#include <string>
#include <mysql.h>
#include "DM.hpp"
using namespace std;

class Manager{
private:
public:
    Manager(){};
    ~Manager(){};

    static void insertWorker(MYSQL *conn, const string &worker_id, const string &name, const string &gender, const string &phone, const string &department, const string &password) {
        string query = "INSERT INTO employee (worker_id, name, gender, phone, department, password) VALUES ('" + worker_id + "', '" + name + "', '" + gender + "', '" + phone + "', '" + department + "', '" + password + "');";
        if (mysql_query(conn, query.c_str()) == 0) {
            cout << "Worker added successfully!\n";
        } else {
            cerr << "Error adding worker: " << mysql_error(conn) << endl;
        }
        // 创建MySQL用户，并将password作为登录密码
        string createUserQuery = "CREATE USER '" + worker_id + "'@'localhost' IDENTIFIED BY '" + password + "';";
        if (mysql_query(conn, createUserQuery.c_str()) == 0) {
            cout << "User created successfully!\n";
        } else {
            cerr << "Error creating user: " << mysql_error(conn) << endl;
            return; // 如果创建用户失败，直接返回
        }

        // 赋予employee_role角色权限
        string grantRoleQuery = "GRANT employee_role TO '" + worker_id + "'@'localhost';";
        if (mysql_query(conn, grantRoleQuery.c_str()) == 0) {
            cout << "Role granted successfully!\n";
        } else {
            cerr << "Error granting role: " << mysql_error(conn) << endl;
        }
    }

    static void deleteWorker(MYSQL *conn, const string &worker_id) {
        string query = "DELETE FROM employee WHERE worker_id = '" + worker_id + "';";
        if (mysql_query(conn, query.c_str()) == 0) {
            cout << "Worker deleted successfully!\n";
        } else {
            cerr << "Error deleting worker: " << mysql_error(conn) << endl;
        }
    }

    static void executeManagerOption(MYSQL *conn) {
        int choice;
        do {
            cout << "\nManager Options:\n1. Add Worker\n2. Delete Worker\n3. Exit\nEnter your choice: ";
            cin >> choice;
            cin.ignore();

            switch (choice) {
            case 1: {
                //string类型不能存储中文字符，这里注意，暂时先不改进了
                string worker_id, name, gender, phone, department, password;
                cout << "Enter Worker ID: ";
                cin >> worker_id;
                if(worker_id[0] == '0'){
                    //只能创建普通员工
                    cout << "out of permission" << endl;
                    break;
                }
                cout << "Enter Name: ";
                cin >> name;
                cout << "Enter Gender: ";
                cin >> gender;
                cout << "Enter Phone: ";
                cin >> phone;
                cout << "Enter Department: ";
                cin >> department;
                cout << "Enter Password: ";
                cin >> password;
                insertWorker(conn, worker_id, name, gender, phone, department, password);
                break;
            }
            case 2: {
                string worker_id;
                cout << "Enter Worker ID to delete: ";
                cin >> worker_id;
                if(worker_id[0] == '0'){
                    //只能删除普通员工
                    cout << "out of permission" << endl;
                    break;
                }
                deleteWorker(conn,worker_id);
                break;
            }
            case 3:
                cout << "Exiting Manager Options.\n";
                break;
            default:
                cout << "Invalid choice. Try again.\n";
            }
        } while (choice != 3);
    }
};