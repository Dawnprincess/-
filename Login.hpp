#pragma once

#include <iostream>
#include <string>
#include "DM.hpp"
#include "Employee.hpp"
#include "Manager.hpp"

using namespace std;
class Login {
private:
    Database db;

public:
    void authenticate() {
        string worker_id, password;
        //清屏
        //system("cls");
        cout << "Welcome to Employee Management System!\n";
        

        while(1){
        cout << "Input '#' to exit.\n"; 
        cout << "Worker ID: ";
        cin >> worker_id;
        cin.ignore();
        if(worker_id[0] == '#'){
            cout << "Thanks for using the system!\n";
            return;
        }
        cout << "Password: ";
        cin >> password;
        cin.ignore();

        if (!db.checkWorkerExists(worker_id)) {
            //system("cls");
            cerr << "Worker ID does not exist or is invalid.\n";
            continue;
        }

        if (!db.verifyPassword(worker_id, password)) {
            //system("cls");
            cerr << "Incorrect password!\n";
            continue;
        }
        else
        {
            //system("cls");
            break;
        }
    }
    //验证身份后就不再使用root账号登录，改为使用worker_id登录
    db.ConnectDB(worker_id, password);

    if (worker_id[0] == '0') {
        cout << "Welcome, Manager!\n";
        Manager::executeManagerOption(db.conn);
    } else if (worker_id[0] == '1') {
        cout << "Welcome, Employee!\n";
        //调用Employee类中的showEmployeeView()方法 
        Employee::showEmployeeView(db.res, db.row, db.conn,worker_id);
    } else {
        cerr << "Invalid Worker ID format.\n";
        }
    }
};