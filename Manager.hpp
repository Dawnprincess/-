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
        }

        // 赋予employee_role角色权限
        string grantRoleQuery = "GRANT employee_role TO '" + worker_id + "'@'localhost';";
        if (mysql_query(conn, grantRoleQuery.c_str()) == 0) {
            //激活角色
            string activate = "SET DEFAULT ROLE employee_role TO '" + worker_id + "'@'localhost';";
            if (mysql_query(conn, activate.c_str()) == 0) 
                cout << "Role activated successfully!\n";
            cout << "Role granted successfully!\n";
        } else {
            cerr << "Error granting role: " << mysql_error(conn) << endl;
            return;
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

    static void updateWorker(MYSQL *conn, const string &worker_id) {
        int choice = 0;
        while(true){
        if(choice == 0){
            cout << "Update Worker Options:\n1. Change Worker ID\n2. Change Name\n3. Change Gender\n4. Change Phone\n5. Change Department\n6. Change Password\n7. Exit\nEnter your choice: ";
            cin >> choice;
        }
        switch (choice) {
        case 1: {
            string new_worker_id;
            cout << "Enter new Worker ID: ";
            cin >> new_worker_id;
            if(new_worker_id[0] == '0'){
                //只能更新普通员工
                cout << "out of permission" << endl;
                continue;
            }
            string query = "UPDATE employee SET worker_id = '" + new_worker_id + "' WHERE worker_id = '" + worker_id + "';";
            if (mysql_query(conn, query.c_str()) == 0) {
                cout << "Worker ID updated successfully!\n";
                choice = 0;
                continue;
            } else {
                cerr << "Error updating worker ID: " << mysql_error(conn) << endl;
            }
            break;
        }
        case 2: {
            string new_name;
            cout << "Enter new Name: ";
            cin >> new_name;
            cin.ignore();
            string query = "UPDATE employee SET name = '" + new_name + "' WHERE worker_id = '" + worker_id + "';";
            if (mysql_query(conn, query.c_str()) == 0) {
                cout << "Name updated successfully!\n";
                choice = 0;
                continue;
            } else {
                cerr << "Error updating name: " << mysql_error(conn) << endl;
            }
            break;
        }
        case 3: {
            string new_gender;
            cout << "Enter new Gender: ";
            cin >> new_gender;
            string query = "UPDATE employee SET gender = '" + new_gender + "' WHERE worker_id = '" + worker_id + "';";
            if (mysql_query(conn, query.c_str()) == 0) {
                cout << "Gender updated successfully!\n";
                choice = 0;
                continue;
            } else {
                cerr << "Error updating gender: " << mysql_error(conn) << endl;
            }
            break;
        }
        case 4: {
            string new_phone;
            cout << "Enter new Phone: ";
            cin >> new_phone;
            string query = "UPDATE employee SET phone = '" + new_phone + "' WHERE worker_id = '" + worker_id + "';";
            if (mysql_query(conn, query.c_str()) == 0) {
                cout << "Phone updated successfully!\n";
                choice = 0;
                continue;
            } else {
                cerr << "Error updating phone: " << mysql_error(conn) << endl;
            }
            break;
        }
        case 5: {
            string new_department;
            cout << "Enter new Department: ";
            cin >> new_department;
            string query = "UPDATE employee SET department = '" + new_department + "' WHERE worker_id = '" + worker_id + "';";
            if (mysql_query(conn, query.c_str()) == 0) {
                cout << "Department updated successfully!\n";
                choice = 0;
                continue;
            } else {
                cerr << "Error updating department: " << mysql_error(conn) << endl;
            }
            break;
        }
        case 6: {
            string new_password;
            cout << "Enter new Password: ";
            cin >> new_password;
            string query = "UPDATE employee SET password = '" + new_password + "' WHERE worker_id = '" + worker_id + "';";
            if (mysql_query(conn, query.c_str()) == 0) {
                cout << "Password updated successfully!\n";
                choice = 0;
                continue;
            } else {
                cerr << "Error updating password: " << mysql_error(conn) << endl;
            }
            break;
        }
        case 7:
            {
            cout << "Exiting Update Worker.\n";
            return;
            }
        default:{
            cout << "Invalid choice. Try again.\n";
            continue;
            }
        }
    }
}
    static void executeManagerOption(MYSQL *conn) {
        int choice = 0;
        do {
            if(choice == 0){
                cout << "\nManager Options:\n1. Add Worker\n2. Delete Worker\n3. Update Worker\n4. Back\nEnter your choice: ";
                cin >> choice;
                cin.ignore();
            }

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
                choice = 0;
                continue;
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
                choice = 0;
                continue;
            }
            case 3:{
                cout << "Enter Worker ID to update: ";
                string worker_id;
                cin >> worker_id;
                if(worker_id[0] == '0'){
                    //只能更新普通员工
                    cout << "out of permission" << endl;
                    break;
                }
                updateWorker(conn,worker_id);
                choice = 0;
                continue;
            }
            default:
                cout << "Invalid choice. Try again.\n";
            }
        } while (choice != 3);
    }
};