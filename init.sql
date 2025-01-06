-- Active: 1735128813752@@127.0.0.1@3306
CREATE DATABASE EM
    DEFAULT CHARACTER SET = 'utf8mb4';
USE EM;
-- 创建基本表
CREATE TABLE department (
    department_id INT(1) NOT NULL,
    department_name VARCHAR(20) NOT NULL UNIQUE,
    PRIMARY KEY (department_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE employee (
    worker_id VARCHAR(5) NOT NULL,
    name VARCHAR(20) NOT NULL,
    gender ENUM('M','F') NOT NULL,
    phone VARCHAR(11) NOT NULL,
    department VARCHAR(20) NOT NULL,
    password VARCHAR(6) NOT NULL,
    PRIMARY KEY (worker_id),
    FOREIGN KEY (department) REFERENCES department(department_name) ON DELETE CASCADE ON UPDATE CASCADE,
    CHECK (LENGTH(worker_id) = 5),
    CHECK (LENGTH(phone) = 11)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE salary (
    salary_id INT(5) NOT NULL,
    worker_id VARCHAR(5) NOT NULL,
    base_salary DECIMAL(10,2) NOT NULL,
    bonus DECIMAL(10,2),
    PRIMARY KEY (salary_id),
    FOREIGN KEY (worker_id) REFERENCES employee(worker_id) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE attendance (
    attendance_id INT(5) NOT NULL,
    attendance_type ENUM('1','2') NOT NULL,
    PRIMARY KEY (attendance_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE register (
    attendance_id INT(5) NOT NULL,
    worker_id VARCHAR(5) NOT NULL,
    register_time DATETIME NOT NULL,
    FOREIGN KEY (attendance_id) REFERENCES attendance(attendance_id) ON DELETE CASCADE ON UPDATE CASCADE,
    FOREIGN KEY (worker_id) REFERENCES employee(worker_id) ON DELETE CASCADE ON UPDATE CASCADE,
    PRIMARY KEY (attendance_id,worker_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE VIEW employee_view AS
SELECT employee.worker_id,employee.name,employee.gender,employee.phone,employee.department,salary.base_salary,salary.bonus
FROM employee
INNER JOIN salary ON employee.worker_id = salary.worker_id;
--初始化一个部门
INSERT INTO department (department_id, department_name) VALUES (1, 'hq');
--初始化一个经理账号
INSERT INTO employee (worker_id, name, gender, phone, department, password) VALUES ('00000', 'Jack','m', '12345678910', 'hq', '123456');

--创建模版角色
CREATE ROLE IF NOT EXISTS 'employee_role';
GRANT SELECT on em.employee_view TO 'employee_role';
CREATE ROLE IF NOT EXISTS 'manager_role';
GRANT SELECT,INSERT,UPDATE,DELETE ON em.* TO 'manager_role';
GRANT SELECT,UPDATE ON mysql.user TO 'manager_role';
GRANT CREATE USER,ROLE_ADMIN ON *.* TO 'manager_role';
CREATE USER '00000'@'localhost' IDENTIFIED BY '123456';
GRANT 'manager_role'@'%' TO '00000'@'localhost';
SET DEFAULT ROLE 'manager_role' TO '00000'@'localhost';
FLUSH PRIVILEGES;
