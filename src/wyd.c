#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

void help() {
	fprintf(stdout, "usage: wyd command\n" \
	"	commands:\n" \
	"		init			create the database\n" \
	"		create \"Task\"		create a task\n" \
	"		add \"Task\"		alias of create\n");
}

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
	 int i;
	 for(i = 0; i<argc; i++) {
			printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	 }
	 printf("\n");
	 return 0;
}

int createDB(int argc, char* argv[]) {
	 sqlite3 *db;
	 char *zErrMsg = 0;
	 int rc;
	 char *sql;

	 /* Open database */
	 rc = sqlite3_open_v2("./.WYD.db", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	 
	 if( rc ) {
			fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
			return(0);
	 }

	 /* Create SQL statement */
	 sql = "CREATE TABLE TASKS("  \
				 "ID INT PRIMARY KEY NOT NULL," \
				 "TASK TEXT NOT NULL," \
				 "STATUS INT DEFAULT 0 NOT NULL);";

	 /* Execute SQL statement */
	 rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
	 
	 if( rc != SQLITE_OK ){
	 fprintf(stderr, "SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
	 } else {
			fprintf(stdout, "Database created\n");
	 }
	 sqlite3_close(db);
	 return 0;
}

int create(char* task) {
	sqlite3 *db;
	 char *zErrMsg = 0;
	 int rc;
	 char *stmt;

	 rc = sqlite3_open_v2("./.WYD.db", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	 
	 if( rc ) {
			fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
			return(0);
	 }

	 stmt = "INSERT INTO TASKS (TASK) VALUES('');";

	 rc = sqlite3_exec(db, stmt, callback, 0, &zErrMsg);
	 
	 if( rc != SQLITE_OK ){
	 fprintf(stderr, "SQL error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
	 } else {
			fprintf(stdout, "Database created\n");
	 }
	 sqlite3_close(db);
	 return 0;
}

int main(int argc, char* argv[]) {

	if( argc > 1 ) {
		if ( strcmp(argv[1], "init") == 0 ) {
			createDB(argc, argv);
		}
		else if ( strcmp(argv[1], "create") == 0 ) {
			create(argv[2]);
		}
		else {
			help();
		}
	}
	else {
		printf("No command detected.\n");
		help();
		return 0;
	}
}