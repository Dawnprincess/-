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
    static void clearInputBuffer() {
        if (cin.peek() == '\n') {
            cin.ignore();
        }
    }
    static map<string, string> proceeInput(MYSQL *conn, const string& table_name) {
        map<string, string> fv;
        string fields_input, values_input;
        vector<string> fields = getColnames(conn, table_name);

        cout << "The table has the following fields (separated by space): ";
        for (const string& field : fields) {
            cout << field << " ";
        }
        cout << endl;

        // 用户输入字段名
        cout << "Please enter the fields (separated by comma, * for all fields): \n";
        clearInputBuffer(); // 只在必要时清理缓冲区
        getline(cin, fields_input);

        // 用户输入值
        cout << "Please enter the corresponding values (separated by comma): \n";
        clearInputBuffer(); // 只在必要时清理缓冲区
        getline(cin, values_input);

        // 处理输入并返回 fv（代码保持不变）
        vector<string> selected_fields;
        if (fields_input == "*") {
            fv["*"] = '*';
        } else {
            stringstream ss(fields_input);
            string field;
            while (getline(ss, field, ',')) {
                selected_fields.push_back(field);
            }
        }

        //初始化selected_values为fields_input的大小，防止fields_input为*时，selected_values为空
        vector<string> selected_values = selected_fields;
        stringstream ss(values_input);
        string value;
        while (getline(ss, value, ',')) {
            selected_values.push_back(value);
        }
        //field_input没有*，selected_fields和selected_values的大小应该相等
        if (selected_fields.size() != selected_values.size() && fields_input!= "*") {
            cerr << "Error: The number of fields does not match the number of values!" << endl;
            return fv;
        }

        for (size_t i = 0; i < selected_fields.size(); ++i) {
            fv[selected_fields[i]] = selected_values[i];
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
            cerr << "Error selecting: " << mysql_error(conn) << endl;
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
    static void deleteTable(MYSQL *conn, const map<string, string>& filters, const string& table_name) {
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
        //查询符合筛选条件的worker_id
        if(table_name == "employee"){
            vector<string> worker_ids;
            string query2 = "SELECT worker_id FROM employee WHERE";
            bool first = true;
            for (const auto& filter : filters) {
                if (!first) {
                    query2 += " AND";  // 连接条件
                }
                query2 += " " + filter.first + " LIKE '%" + filter.second + "%'";  // 构建筛选条件
                first = false;
            }
            if (mysql_query(conn, query2.c_str()) == 0) {
                MYSQL_RES *result = mysql_store_result(conn);
                if (result == NULL) {
                    cerr << "No information found.\n";
                    return;
                }
                MYSQL_ROW row;
                while ((row = mysql_fetch_row(result))) {
                    worker_ids.push_back(row[0]);
                }
                mysql_free_result(result);
            } else {
                cerr << "Error selecting workers: " << mysql_error(conn) << endl;
                return;
            }
            //删除用户
            for(const auto& worker_id : worker_ids){
                string dropUserQuery = "DROP USER '" + worker_id + "'@'localhost';";
                if (mysql_query(conn, dropUserQuery.c_str()) == 0) {
                    cout << "User deleted successfully!\n";
                } else {
                    cerr << "Error deleting user: " << mysql_error(conn) << endl;
                }
            }
        }
    }
    static void updateTable(MYSQL *conn, const map<string, string>& filters,const map<string, string>& values, const string& table_name) {
        //filters筛选要修改的行，values是要修改的字段名和值
        //如果涉及更新employee表中的worker_id，则需要先获取旧的worker_id
        string old_worker_id;
        if(table_name == "employee" && values.find("worker_id") != values.end()){
            string query = "SELECT worker_id FROM employee WHERE";
            bool first = true;
            for (const auto& value : values) {
                if (!first) {
                    query += " AND";  // 连接条件
                }
                query += " " + value.first + " LIKE '%" + value.second + "%'";  // 构建筛选条件
                first = false;
            }
            if (mysql_query(conn, query.c_str()) == 0) {
                MYSQL_RES *result = mysql_store_result(conn);
                if (result == NULL) {
                    cerr << "No information found.\n";
                    return;
                }
                MYSQL_ROW row;
                while ((row = mysql_fetch_row(result))) {
                    old_worker_id = row[0];
                }
                mysql_free_result(result);
            } else {
                cerr << "Error selecting workers: " << mysql_error(conn) << endl;
                return;
            }   
            //获取新的worker_id
            string new_worker_id = values.at("worker_id");
            string updateUserQuery = "UPDATE mysql.user SET User = '" + values.at("worker_id") + "' WHERE User = '" + old_worker_id + "';";
            if (mysql_query(conn, updateUserQuery.c_str()) == 0) {
                cout << "User updated successfully!\n";
            } else {
                cerr << "Error updating user: " << mysql_error(conn) << endl;
                return;
            }
        }
        //获取用户输入的更新字段名和值
        string query = "UPDATE " + table_name + " SET";
        // 构建 SET 子句
        bool first = true;
        for (const auto& value : values) {
            if (!first) {
                query += ", ";
            }
            query += " " + value.first + " = '" + value.second + "'";
            first = false;
        }
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
        // 执行更新
        if (mysql_query(conn, query.c_str()) == 0) {
            cout << "Data updated successfully!\n";
        } else {
            cerr << "Error updating data: " << mysql_error(conn) << endl;
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
                    cout << "Please fill in the required fields with ',' to separate:\n";
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
                    cout << "Please select the fields and the values to be deleted:\n";
                    map<string, string> filters = proceeInput(conn, table_name);  // 调用输入函数
                    deleteTable(conn,filters, table_name);
                    choice = 0;
                    continue;
                }
                case 3:{
                    cout << "You are updating the " << table_name << "table.\n";
                    //选择需要修改的项以及修改内容
                    cout << "Please fill the fields and the values to select:\n";
                    map<string, string> filters = proceeInput(conn, table_name);  // 调用输入函数
                    cout << "Please confirm the information to be updated:\n";
                    selectTable(conn, filters, table_name);
                    cout << "Please select the fields and enter the new values to update:\n";
                    map<string, string> updates = proceeInput(conn, table_name);  // 调用输入函数
                    updateTable(conn,filters ,updates,table_name);
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