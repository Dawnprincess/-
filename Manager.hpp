#pragma once

#include <map>
#include <limits>
#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <mysql.h>
#include <iomanip>
#include "DM.hpp"
using namespace std;

class Manager{
private:
    static vector<string> split(const string& str, const char& delimiter) {
        vector<string> result;
        stringstream ss(str);
        string item;
        while (getline(ss, item, delimiter)) {
            result.push_back(item);
        }
        return result;
    }
    static vector<string> getColnames(MYSQL *conn, const string& table_name) {
        vector<string> colnames;
        string query = "SHOW COLUMNS FROM " + table_name + ";";
        if (mysql_query(conn, query.c_str()) == 0) {
            MYSQL_RES *result = mysql_store_result(conn);
            if (result == NULL) {
                cerr << "No information found.\n";
                return colnames;
            }
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                colnames.push_back(row[0]);
            }
            mysql_free_result(result);
        } else {
            cerr << "Error selecting: " << mysql_error(conn) << endl;
        }
        return colnames;
    }
    static map<string, string> proceeInput(MYSQL *conn, const string& table_name){
        map<string, string> fv;
        string field, value;
        //声明一个字符串数组保存当前表的各字段名
        vector<string> fields,values;
        fields = getColnames(conn, table_name);
                    
        cout << "The table has the following fields (separated by space): ";
        // 输出当前表的各字段名
        for (int i = 0; i < fields.size(); i++) {
            cout << fields[i] << " ";
        }
        cout << endl;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        // 用户选择的字段名用','分隔, 存在fields数组中，并作为fv的第一个元素
        cout << "Please enter the fields (separated by comma, * for all fields): \n";
        getline(cin, field);
        fields = split(field, ',');
        // 如果用户输入*，则将fields设置为所有字段名
        if (field == "*") {
            fields = getColnames(conn, table_name);
        }
        // 用户输入的各值用','分隔, 存在values数组中，并作为fv的第二个元素
        cout << "Please enter the values for the fields (separated by comma,* to skip): \n";
        getline(cin, value);
        values = split(value, ',');
        //将fields和values组成fv
        for(int i = 0; i < fields.size(); i++){
            fv[fields[i]] = values[i];
        }
        return fv;
    }
public:
    Manager(){};
    ~Manager(){};
    static bool selectTable(MYSQL *conn, const map<string, string>& filters, const string& table_name) {
        string query = "SELECT * FROM " + table_name;
    
        // 检查filters键值是否有*，如果有，则不添加WHERE子句
        if ( filters.find("*") == filters.end()) {
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
                cerr << "No information found.\n";
                return false;
            }
            MYSQL_ROW row;

            // 输出表头
            vector<string> columns = getColnames(conn, table_name);
            for(const auto& column : columns) {
                cout << setw(15) << column;
            }
            cout << endl;
            while ((row = mysql_fetch_row(result))) {
                for (int i = 0; i < mysql_num_fields(result); i++) {
                    cout << setw(15) << row[i];
                }
                cout << endl;
            }
            mysql_free_result(result);
            return true;
        } else {
            cerr << "Error selecting workers: " << mysql_error(conn) << endl;
            return false;
        }
    }
    static void insertTable(MYSQL *conn, const map<string, string>& values, const string& table_name) {
        string query = "INSERT INTO " + table_name + " (";
        bool first = true;
        for (const auto& value : values) {
            if (!first) {
                query += ", ";
            }
            query += value.first;
            first = false;
        }
        query += ") VALUES (";
        first = true;
        for (const auto& value : values) {
            if (!first) {
                query += ", ";
            }
            query += "'" + value.second + "'";
            first = false;
        }
        query += ")";
        if (mysql_query(conn, query.c_str()) == 0) {
            cout << "Data inserted successfully!\n";
        } else {
            cerr << "Error inserting data: " << mysql_error(conn) << endl;
        }
        //如果是插入员工，还需要创建用户
        if(table_name == "employee"){
            string createUserQuery = "CREATE USER '" + values.at("worker_id") + "'@'localhost' IDENTIFIED BY '" + values.at("password") + "';";
        if (mysql_query(conn, createUserQuery.c_str()) == 0) {
            cout << "Worker added successfully!\n";
        } else {
            cerr << "Error adding worker: " << mysql_error(conn) << endl;
        }

        // 赋予employee_role角色权限
        string grantRoleQuery = "GRANT employee_role TO '" + values.at("worker_id") + "'@'localhost';";
        if (mysql_query(conn, grantRoleQuery.c_str()) == 0) {
            //激活角色
            string activate = "SET DEFAULT ROLE employee_role TO '" + values.at("worker_id") + "'@'localhost';";
            if (mysql_query(conn, activate.c_str()) == 0) 
                cout << "Role activated successfully!\n";
            cout << "Role granted successfully!\n";
        } else {
            cerr << "Error granting role: " << mysql_error(conn) << endl;
            return;
        }
    }
    }
    static void deleteTable(MYSQL *conn, const map<string, string>& filters, const string& table_name, const vector<string>& columns) {
        if(selectTable(conn, filters, table_name)){
            cout << "Please confirm the information to be deleted:\n";
        }
        string query = "DELETE FROM " + table_name;
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
        if (mysql_query(conn, query.c_str()) == 0) {
            cout << "Worker deleted successfully!\n";
        } else {
            cerr << "Error deleting worker: " << mysql_error(conn) << endl;
        }
        if(table_name == "employee"){
        //如果是删除员工，还需要删除该用户
        string dropUserQuery = "DROP USER '" + filters.at("worker_id") + "'@'localhost';";
        if (mysql_query(conn, dropUserQuery.c_str()) == 0) {
            cout << "User deleted successfully!\n";
        } else {
            cerr << "Error deleting user: " << mysql_error(conn) << endl;
        }
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
            /*if(!selectTable(conn, {{"worker_id", new_worker_id}})){
                //新ID已存在
                cout << "The new Worker ID already exists.\n";
                continue;
            }
            */
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
 
    static void executeManagerOption(MYSQL *conn) {
        int choice = 0,table = 0;
        string table_name = "";
        while(true){
            //先选择要操作的表
            if(table == 0){
                cout << "Select Table:\n1. Employee\n2. Salary\n3. Attendance\n4. Register\n5.Department\n6.Exit.\nEnter your choice: ";
                cin >> table;
                // 检查输入是否为整数
                if(cin.fail()){
                cin.clear(); // 清除错误状态
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); // 忽略错误输入
                cout << "Invalid input. Please enter a number between 1 and 6.\n";
                table = 0; // 重置table以重新提示输入
                continue; // 重新开始循环
                }
            }
            switch (table) {
            case 1:
                {
                    table_name = "employee";
                    break;
                }
            case 2:
                {
                    table_name = "salary";
                    break;
                }
            case 3:
                {
                    table_name = "attendance";
                    break;
                }
            case 4:
                {
                    table_name = "register";
                    break;
                }
            case 5:
                {
                    table_name = "department";
                    break;
                }
            case 6:
                {
                    cout << "Exiting Manager.\n";
                    return;
                }
            default:
                cout << "Invalid choice. Try again.\n";
                table = 0;
                continue;
            }
            do {
                if(choice == 0){
                    cout << "\nManager Options:\n"
                        << "1. Add Information  2. Delete Information 3. Update Information 4. Query Information\n"
                        << "5. Back.\nEnter your choice: ";
                    cin >> choice;
                    if(cin.fail()){
                        cin.clear(); // 清除错误状态
                        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // 忽略错误输入
                        cout << "Invalid input. Please enter a number between 1 and 6.\n";
                        table = 0; // 重置table以重新提示输入
                        continue; // 重新开始循环
                    }
                }
                switch (choice) {
                case 1: {
                    cout << "You are adding a new entry to the " << table_name << " table.\n";
                    map<string, string> fv = proceeInput(conn, table_name);  // 调用输入函数
                    /*map<string, string> fv;                        
                    string field, value;
                    //声明一个字符串数组保存当前表的各字段名
                    vector<string> fields;
                    //用table_name去查询当前表的各字段名，然后保存到fields中
                    string query = "SHOW COLUMNS FROM " + table_name + ";";
                    if (mysql_query(conn, query.c_str()) == 0) {
                        MYSQL_RES *result = mysql_store_result(conn);
                        if (result == NULL) {
                            cerr << "No information found.\n";
                            return ;
                        }
                        MYSQL_ROW row;
                        while ((row = mysql_fetch_row(result))) {
                            fields.push_back(row[0]);
                        }
                        mysql_free_result(result);
                    } else {
                        cerr << "Error selecting: " << mysql_error(conn) << endl;
                        return;
                    }
                    
                    cout << "Fill all the required fields (";
                    // 输出当前表的各字段名
                    for (int i = 0; i < fields.size(); i++) {
                        cout << fields[i] << " ";
                    }
                    cout << ") with ',' to separate: \n";
                    cin.ignore();
                    getline(cin, value);
        
                    // 解析用户输入的多个值
                    stringstream ss(value);
                    string individualValue;
                    int fieldIndex = 0;

                    // 将键值对存入 fv 中
                    while (getline(ss, individualValue, ',')) {
                        if (fieldIndex < fields.size()) {
                            // 对应字段和值之间存储到 fv 中
                            fv[fields[fieldIndex]] = individualValue;
                            fieldIndex++;
                        }
                    }

                    // 确保每个字段都有对应的值
                    if (fieldIndex != fields.size()) {
                        cerr << "Error: Not all fields have corresponding values.\n";
                        return;
                    }
                    */
                    insertTable(conn, fv, table_name);  // 调用插入函数
                    table = 0;
                    choice = 0;
                    continue;
                }
                case 2: {
                    cout << "You are deleting an entry from the " << table_name << " table.\n";
                    string worker_id;
                    cout << "Enter Worker ID to delete: ";
                    cin >> worker_id;
                    if(worker_id[0] == '0'){
                        //只能删除普通员工
                        cout << "out of permission" << endl;
                        break;
                    }
                    //deleteTable(conn,worker_id);
                    choice = 0;
                    continue;
                }
                case 3:{
                    cout << "You are updating the " << table_name << "table.\n";

                    cout << "Enter Worker ID to update: ";
                    string worker_id;
                    cin >> worker_id;
                    if(worker_id[0] == '0'){
                        //只能更新普通员工
                        cout << "out of permission" << endl;
                        break;
                    }
                    //检查worker_id是否存在
                    /*if(!selectTable(conn,{{"worker_id",worker_id}})){
                        cout << "Worker not found.\n";
                        break;
                    }
                    */
                    updateWorker(conn,worker_id);
                    choice = 0;
                    continue;
                }
                case 4:
                    {
                        cout << "You are querying the "<< table_name << " table.\n";
                        map<string, string> filters = proceeInput(conn, table_name);  // 调用输入函数
                        selectTable(conn, filters, table_name);  // 调用查询函数
                        choice = 0;
                        continue;
                    }
                case 5:{
                    cout << "Back.\n";
                    break;
                }
                default:
                    cout << "Invalid choice. Try again.\n";
                    choice = 0;
                }
            } while (choice != 5);
            table = 0;
            choice = 0;
        }
    }
};