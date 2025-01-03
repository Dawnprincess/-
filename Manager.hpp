#pragma once

#include <map>
#include <iostream>
#include <string>
#include <mysql.h>
#include <iomanip>
#include "DM.hpp"
using namespace std;

class Manager{
private:
public:
    Manager(){};
    ~Manager(){};
    static bool selectWorker(MYSQL *conn, const map<string, string>& filters) {
        string query = "SELECT * FROM employee";
    
        // 如果有筛选条件，则构建 WHERE 子句
        if (!filters.empty()) {
            query += " WHERE";
            bool first = true;
        
            for (const auto& filter : filters) {
                if (!first) {
                    query += " AND";  // 连接条件
                }
                query += " " + filter.first + " LIKE '%" + filter.second + "%'";  // 构建筛选条件
                first = false;
            }
        }
    
        // 执行查询
        if (mysql_query(conn, query.c_str()) == 0) {
            MYSQL_RES *result = mysql_store_result(conn);
            if (result == NULL) {
                cerr << "No workers found.\n";
                return false;
            }
            MYSQL_ROW row;

            // 输出表头
            cout << setw(10) << "Worker ID" << setw(15) << "Name" << setw(10) << "Gender" 
                << setw(15) << "Phone" << setw(15) << "Department" << setw(15) << "Password" << endl;

            while ((row = mysql_fetch_row(result))) {
                cout << setw(10) << row[0] << setw(15) << row[1] << setw(10) << row[2] 
                    << setw(15) << row[3] << setw(15) << row[4] << setw(15) << row[5] << endl;
            }

            mysql_free_result(result);
            return true;
        } else {
            cerr << "Error selecting workers: " << mysql_error(conn) << endl;
            return false;
        }
    }
    static void insertWorker(MYSQL *conn, const string &worker_id, const string &name, const string &gender, const string &phone, const string &department, const string &password) {
        string query = "INSERT INTO employee (worker_id, name, gender, phone, department, password) VALUES ('" + worker_id + "', '" + name + "', '" + gender + "', '" + phone + "', '" + department + "', '" + password + "');";
        if (mysql_query(conn, query.c_str()) == 0) {
            cout << "Worker added successfully!\n";
        } else {
            cerr << "Error adding worker: " << mysql_error(conn) << endl;
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
        //删除该用户
        string dropUserQuery = "DROP USER '" + worker_id + "'@'localhost';";
        if (mysql_query(conn, dropUserQuery.c_str()) == 0) {
            cout << "User deleted successfully!\n";
        } else {
            cerr << "Error deleting user: " << mysql_error(conn) << endl;
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
            if(!selectWorker(conn, {{"worker_id", new_worker_id}})){
                //新ID已存在
                cout << "The new Worker ID already exists.\n";
                continue;
            }
            string query = "UPDATE employee SET worker_id = '" + new_worker_id + "' WHERE worker_id = '" + worker_id + "';";
            if (mysql_query(conn, query.c_str()) == 0) {
                cout << "Worker ID updated successfully!\n";
                //更新用户
                string updateUserQuery = "UPDATE mysql.user SET User = '" + new_worker_id + "' WHERE User = '" + worker_id + "';";
                if (mysql_query(conn, updateUserQuery.c_str()) == 0) {
                    cout << "User updated successfully!\n";
                    return;
                } else {
                    cerr << "Error updating user: " << mysql_error(conn) << endl;
                    //用户更新失败，复原worker_id
                    string revertQuery = "UPDATE employee SET worker_id = '" + worker_id + "' WHERE worker_id = '" + new_worker_id + "';";
                    if (mysql_query(conn, revertQuery.c_str()) == 0) {
                        cout << "Worker ID reverted successfully!\n";
                    } else {
                        cerr << "Error reverting worker ID: " << mysql_error(conn) << endl;
                    }
                    return;
                }
                //更新后的用户ID不再适用，再进行其他更新操作无效，退出该函数
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



    static bool selectSalary(MYSQL *conn, const map<string, string>& filters) {
        string query = "SELECT * FROM salary";
    
        // 如果有筛选条件，则构建 WHERE 子句
        if (!filters.empty()) {
            query += " WHERE";
            bool first = true;
        
            for (const auto& filter : filters) {
                if (!first) {
                    query += " AND";  // 连接条件
                }
                query += " " + filter.first + " LIKE '%" + filter.second + "%'";  // 构建筛选条件
                first = false;
            }
        }
    
        // 执行查询
        if (mysql_query(conn, query.c_str()) == 0) {
            MYSQL_RES *result = mysql_store_result(conn);
            if (result == NULL) {
                cerr << "No workers found.\n";
                return false;
            }
            MYSQL_ROW row;

            // 输出表头
            cout << setw(10) << "Worker ID" << setw(15) << "Name" << setw(10) << "Gender" 
                << setw(15) << "Phone" << setw(15) << "Department" << setw(15) << "Password" << endl;

            while ((row = mysql_fetch_row(result))) {
                cout << setw(10) << row[0] << setw(15) << row[1] << setw(10) << row[2] 
                    << setw(15) << row[3] << setw(15) << row[4] << setw(15) << row[5] << endl;
            }

            mysql_free_result(result);
            return true;
        } else {
            cerr << "Error selecting workers: " << mysql_error(conn) << endl;
            return false;
        }
    }
   
    static void executeManagerOption(MYSQL *conn) {
        int choice = 0;
        do {
            if(choice == 0){
                cout << "\nManager Options:\n"
                     << "1. Add Worker  2. Delete Worker 3. Update Worker 4. Select Worker\n"
                     << "5. Add salary  6. Delete salary 7. Update salary 8. Select salary\n";
                cin.ignore();
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
                //检查worker_id是否存在
                if(!selectWorker(conn,{{"worker_id",worker_id}})){
                    cout << "Worker not found.\n";
                    break;
                }
                updateWorker(conn,worker_id);
                choice = 0;
                continue;
            }
            case 4:
                {
                    map<string, string> filters;
                    string field, value;
        
                    cout << "Enter fields to filter (Name, Gender, Phone, Department) separated by commas, or * for all: ";
                    getline(cin, field);
        
                    if (field != "*") {
                        stringstream ss(field);
                        string individualField;
                        //禁止password字段作为筛选条件
                        if(field.find("password") != string::npos){
                            cout << "password is not allowed as a filter field." << endl;
                            break;
                        }
            
                        // 解析用户输入的多个字段
                        while (getline(ss, individualField, ',')) {
                            cout << "Enter value for " << individualField << ": ";
                            getline(cin, value);
                            filters[individualField] = value;  // 将字段和对应的值加入筛选条件
                        }
                    }
        
                    selectWorker(conn, filters);  // 调用查询函数
                    choice = 0;
                    continue;
                }
            case 5:{
                cout << "Enter Worker ID.\n";
                return;
            }
            default:
                cout << "Invalid choice. Try again.\n";
                choice = 0;
            }
        } while (choice != 4);
    }
};