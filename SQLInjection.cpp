#include <algorithm>
#include <iostream>
#include <locale>
#include <tuple>
#include <vector>

#include "sqlite3.h"

typedef std::tuple<std::string, std::string, std::string> user_record;
const std::string str_where = " where ";

static int callback(void* possible_vector, int argc, char** argv, char** azColName)
{
  if (possible_vector == NULL)
  {
    for (int i = 0; i < argc; i++)
    {
      std::cout << azColName[i] << " = " << (argv[i] ? argv[i] : "NULL") << std::endl;
    }
    std::cout << std::endl;
  }
  else
  {
    std::vector< user_record >* rows = static_cast<std::vector< user_record > *>(possible_vector);
    rows->push_back(std::make_tuple(argv[0], argv[1], argv[2]));
  }
  return 0;
}

bool initialize_database(sqlite3* db)
{
  char* error_message = NULL;
  std::string sql = "CREATE TABLE USERS(" \
    "ID INT PRIMARY KEY     NOT NULL," \
    "NAME           TEXT    NOT NULL," \
    "PASSWORD       TEXT    NOT NULL);";

  int result = sqlite3_exec(db, sql.c_str(), callback, NULL, &error_message);
  if (result != SQLITE_OK)
  {
    std::cout << "Failed to create USERS table. ERROR = " << error_message << std::endl;
    sqlite3_free(error_message);
    return false;
  }
  std::cout << "USERS table created." << std::endl;

  sql = "INSERT INTO USERS (ID, NAME, PASSWORD)" \
    "VALUES (1, 'Fred', 'Flinstone');" \
    "INSERT INTO USERS (ID, NAME, PASSWORD)" \
    "VALUES (2, 'Barney', 'Rubble');" \
    "INSERT INTO USERS (ID, NAME, PASSWORD)" \
    "VALUES (3, 'Wilma', 'Flinstone');" \
    "INSERT INTO USERS (ID, NAME, PASSWORD)" \
    "VALUES (4, 'Betty', 'Rubble');";

  result = sqlite3_exec(db, sql.c_str(), callback, NULL, &error_message);
  if (result != SQLITE_OK)
  {
    std::cout << "Data failed to insert to USERS table. ERROR = " << error_message << std::endl;
    sqlite3_free(error_message);
    return false;
  }

  return true;
}

// Secure query execution using prepared statements
bool run_query(sqlite3* db, const std::string& sql, std::vector< user_record >& records)
{
  records.clear();
  sqlite3_stmt* stmt;

  // Detect dynamic values and simulate prepared queries
  if (sql.find("WHERE NAME=") != std::string::npos)
  {
    size_t start = sql.find("='");
    size_t end = sql.find("'", start + 2);
    std::string name = sql.substr(start + 2, end - (start + 2));

    std::string safe_sql = "SELECT ID, NAME, PASSWORD FROM USERS WHERE NAME = ?";
    if (sqlite3_prepare_v2(db, safe_sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
      std::cout << "Failed to prepare SQL statement.\n";
      return false;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
      int id = sqlite3_column_int(stmt, 0);
      const unsigned char* name = sqlite3_column_text(stmt, 1);
      const unsigned char* pwd = sqlite3_column_text(stmt, 2);

      records.push_back(std::make_tuple(
        std::to_string(id),
        std::string(reinterpret_cast<const char*>(name)),
        std::string(reinterpret_cast<const char*>(pwd))
      ));
    }

    sqlite3_finalize(stmt);
    return true;
  }

  // For non-parameterized queries (like SELECT * FROM USERS)
  char* error_message;
  if (sqlite3_exec(db, sql.c_str(), callback, &records, &error_message) != SQLITE_OK)
  {
    std::cout << "Data failed to be queried from USERS table. ERROR = " << error_message << std::endl;
    sqlite3_free(error_message);
    return false;
  }

  return true;
}

// Simulates an injection attempt but now fails to bypass prepared statement
bool run_query_injection(sqlite3* db, const std::string& sql, std::vector< user_record >& records)
{
  std::string injectedSQL(sql);
  std::string localCopy(sql);
  std::transform(localCopy.begin(), localCopy.end(), localCopy.begin(), ::tolower);

  if (localCopy.find("where") != std::string::npos)
  {
    switch (rand() % 4)
    {
    case 1:
      injectedSQL.append(" or 2=2;");
      break;
    case 2:
      injectedSQL.append(" or 'hi'='hi';");
      break;
    case 3:
      injectedSQL.append(" or 'hack'='hack';");
      break;
    case 0:
    default:
      injectedSQL.append(" or 1=1;");
      break;
    }
  }

  return run_query(db, injectedSQL, records);
}

void dump_results(const std::string& sql, const std::vector< user_record >& records)
{
  std::cout << std::endl << "SQL: " << sql << " ==> " << records.size() << " records found." << std::endl;

  for (auto record : records)
  {
    std::cout << "User: " << std::get<1>(record) << " [UID=" << std::get<0>(record) << " PWD=" << std::get<2>(record) << "]" << std::endl;
  }
}

void run_queries(sqlite3* db)
{
  std::vector< user_record > records;

  std::string sql = "SELECT * from USERS";
  if (!run_query(db, sql, records)) return;
  dump_results(sql, records);

  sql = "SELECT ID, NAME, PASSWORD FROM USERS WHERE NAME='Fred'";
  if (!run_query(db, sql, records)) return;
  dump_results(sql, records);

  for (auto i = 0; i < 5; ++i)
  {
    if (!run_query_injection(db, sql, records)) continue;
    dump_results(sql, records);
  }
}

int main()
{
  srand(time(nullptr));

  int return_code = 0;
  std::cout << "SQL Injection Example" << std::endl;

  sqlite3* db = NULL;

  int result = sqlite3_open(":memory:", &db);
  if (result != SQLITE_OK)
  {
    std::cout << "Failed to connect to the database and terminating. ERROR=" << sqlite3_errmsg(db) << std::endl;
    return -1;
  }

  std::cout << "Connected to the database." << std::endl;

  if (!initialize_database(db))
  {
    std::cout << "Database Initialization Failed. Terminating." << std::endl;
    return_code = -1;
  }
  else
  {
    run_queries(db);
  }

  if (db != NULL)
  {
    sqlite3_close(db);
  }

  return return_code;
}
