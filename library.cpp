// Includes
#include <iostream>
#include <mariadb/conncpp.hpp>

// Function to Alter Table
void alterTable(std::shared_ptr<sql::Statement> &stmnt) {
  try {
    // Alter contacts Table
    stmnt->executeUpdate(
        "ALTER TABLE test.contacts RENAME COLUMN first_name TO f_name");
  }

  // Catch Exception
  catch (sql::SQLException &e) {
    std::cerr << "Error altering table: " << e.what() << std::endl;
  }
}

// Main Process
int main(int argc, char **argv) {
  try {
    // Instantiate Driver
    sql::Driver *driver = sql::mariadb::get_driver_instance();

    // Configure Connection
    // The URL or TCP connection string format is
    // ``jdbc:mariadb://host:port/database``.
    sql::SQLString url("jdbc:mariadb://127.0.0.1:3306");

    // Use a properties map for the user name and password
    sql::Properties properties({{"user", "root"}, {"password", "root"}});

    // Establish Connection
    // Use a smart pointer for extra safety
    std::unique_ptr<sql::Connection> conn(driver->connect(url, properties));

    // Create a Statement
    // Use a smart pointer for extra safety
    std::shared_ptr<sql::Statement> stmnt(conn->createStatement());

    // Use Statement to alter table
    alterTable(stmnt);

    // Close Connection
    conn->close();
  }
  // Catch Exceptions
  catch (sql::SQLException &e) {
    std::cerr << "Error Connecting to the database: " << e.what() << std::endl;

    // Exit (Failed)
    return 1;
  }

  // Exit (Success)
  return 0;
}