CS 405 – Module Two SQL Injection Activity
------------------------------------------

Student Name: [Christian Busca]  
Course: CS 405 – Secure Coding  
Assignment: Module Two – SQL Injection Defensive Coding  

Overview:
---------
This assignment demonstrates defensive coding techniques to prevent SQL injection attacks using C++ and SQLite. Specifically, it addresses the classic injection pattern `' OR value=value`, which can be used to bypass query filters and gain unauthorized access to data.

The program creates an in-memory SQLite database and executes queries against a USERS table. Input handling was modified to detect and block malicious query patterns before execution.

Files Included:
---------------
1. SQLInjection.cpp  
   - The modified source file with logic to detect and prevent SQL injection attacks.

2. SQLInjection  
   - Compiled executable of the project for demonstration and testing.

3. sqlite3.c / sqlite3.h  
   - SQLite engine source files, required to build the in-memory database.

4. sql_injection_test  
   - (Optional) File used to simulate or verify SQL injection attempts.

5. 2-2.docx  
   - Word document summarizing the problem, the solution implemented, and including a screenshot of the terminal output showing successful SQL injection defense.

6. cs405_sql_injection_fixed.zip  
   - Final ZIP file of the project contents for submission.

How to Compile:
---------------
If rebuilding from source, use the following command:

g++ -o SQLInjection SQLInjection.cpp sqlite3.c -lpthread -ldl


How to Run:
-----------

./SQLInjection


Expected Output:
----------------
- The USERS table is created and populated with four entries.
- Multiple test queries are run.
- SQL injection attempts are blocked, and a message is shown indicating that a potential injection was detected.

Important Notes:
----------------
- The `run_query()` function was updated to include detection of the pattern `' OR ... = ...`.
- If this pattern is detected, the program skips query execution and logs a warning message.
- This demonstrates defensive programming but should not replace the use of parameterized queries in real-world systems.

