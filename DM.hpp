#include <iostream>
#include <mysql.h>
#include <string>

using namespace std;

class Database {
private:
    MYSQL *conn;
    MYSQL_ROW row;
    MYSQL_RES *res;

public:
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

    void showEmployeeView(const string &worker_id) {
        string query = "SELECT * FROM employee_view WHERE worker_id = '" + worker_id + "';";
        if (mysql_query(conn, query.c_str()) == 0) {
            res = mysql_store_result(conn);
            if((row = mysql_fetch_row(res))) {
                cout << "Worker ID: " << row[0] << "\nName: " << row[1] << "\nDepartment: " << row[2] << "\nPhone: " << row[3] << "\nGender: " << row[4] << "\nBase Salary: " << row[5] << "\nBonus: " << row[6] << endl;
            }
            else{
                cout << "The salary information is not available for this employee." << endl;
            }
            mysql_free_result(res);
        } else {
            cerr << "Error displaying employee view: " << mysql_error(conn) << endl;
        }
    }

    /*void insertWorker(const string &worker_id, const string &name, const string &gender, const string &phone, const string &department) {
        string query = "INSERT INTO worker (worker_id, name, gender, phone, department) VALUES ('" + worker_id + "', '" + name + "', '" + gender + "', '" + phone + "', '" + department + "');";
        if (mysql_query(conn, query.c_str()) == 0) {
            cout << "Worker added successfully!\n";
        } else {
            cerr << "Error adding worker: " << mysql_error(conn) << endl;
        }
    }

    void deleteWorker(const string &worker_id) {
        string query = "DELETE FROM worker WHERE worker_id = '" + worker_id + "';";
        if (mysql_query(conn, query.c_str()) == 0) {
            cout << "Worker deleted successfully!\n";
        } else {
            cerr << "Error deleting worker: " << mysql_error(conn) << endl;
        }
    }

    void executeManagerOption() {
        int choice;
        do {
            cout << "\nManager Options:\n1. Add Worker\n2. Delete Worker\n3. Exit\nEnter your choice: ";
            cin >> choice;
            cin.ignore();

            switch (choice) {
            case 1: {
                string worker_id, name, gender, phone, department;
                cout << "Enter Worker ID: ";
                cin >> worker_id;
                cout << "Enter Name: ";
                cin >> name;
                cout << "Enter Gender: ";
                cin >> gender;
                cout << "Enter Phone: ";
                cin >> phone;
                cout << "Enter Department: ";
                cin >> department;
                insertWorker(worker_id, name, gender, phone, department);
                break;
            }
            case 2: {
                string worker_id;
                cout << "Enter Worker ID to delete: ";
                cin >> worker_id;
                deleteWorker(worker_id);
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
    */
};

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
        if(worker_id[0] == '#'){
            cout << "Thanks for using the system!\n";
            return;
        }
        cout << "Password: ";
        cin >> password;

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

        if (worker_id[0] == '0') {
            cout << "Welcome, Manager!\n";
            db.executeManagerOption();
        } else if (worker_id[0] == '1') {
            cout << "Welcome, Employee!\n";
            db.showEmployeeView(worker_id);
        } else {
            cerr << "Invalid Worker ID format.\n";
        }
    }
};