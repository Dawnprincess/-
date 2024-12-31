CREATE DATABASE EM
    DEFAULT CHARACTER SET = 'utf8mb4';
USE EM;
CREATE TABLE employee (
    worker_id VARCHAR(5) NOT NULL,
    name VARCHAR(20) NOT NULL,
    gender ENUM('M','F') NOT NULL,
    phone VARCHAR(11) NOT NULL,
    department VARCHAR(20) NOT NULL,
    password VARCHAR(6) NOT NULL,
    PRIMARY KEY (worker_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE department (
    department_id INT(1) NOT NULL,
    department_name VARCHAR(20) NOT NULL,
    PRIMARY KEY (department_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE salary (
    salary_id INT(5) NOT NULL,
    worker_id VARCHAR(5) NOT NULL,
    base_salary DECIMAL(10,2) NOT NULL,
    bonus DECIMAL(10,2) NOT NULL,
    PRIMARY KEY (salary_id),
    FOREIGN KEY (worker_id) REFERENCES employee(worker_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE attendance (
    attendance_id INT(5) NOT NULL,
    attendance_type VARCHAR(20) NOT NULL,
    PRIMARY KEY (attendance_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE register (
    attendance_id INT(5) NOT NULL,
    worker_id VARCHAR(5) NOT NULL,
    register_time DATETIME NOT NULL,
    FOREIGN KEY (attendance_id) REFERENCES attendance(attendance_id),
    FOREIGN KEY (worker_id) REFERENCES employee(worker_id),
    PRIMARY KEY (attendance_id,worker_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE VIEW employee_view AS
SELECT employee.worker_id,employee.name,employee.gender,employee.phone,employee.department,salary.base_salary,salary.bonus
FROM employee
INNER JOIN salary ON employee.worker_id = salary.worker_id
INNER JOIN register ON employee.worker_id = register.worker_id;