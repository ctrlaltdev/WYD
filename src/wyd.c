#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

void help() {
	fprintf(stdout, "usage: wyd command\n" \
	"	commands:\n" \
	"		add \"Task\"		alias of create\n" \
	"		create \"Task\"		create a task\n" \
	"		init			create the database\n" \
	"		list [options]		list the open tasks\n" \
	"			-a		list all tasks\n" \
	"		ls			alias of list\n" \
	);
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

	 rc = sqlite3_open_v2("./.WYD.db", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	 
	 if( rc ) {
			fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
			return(0);
	 }

	 sql = "CREATE TABLE TASKS("  \
				 "ID INTEGER PRIMARY KEY AUTOINCREMENT," \
				 "TASK TEXT NOT NULL," \
				 "STATUS INT DEFAULT 0 NOT NULL);";

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
	int rc;
	sqlite3_stmt *stmt;

	rc = sqlite3_open_v2("./.WYD.db", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	
	if ( rc ) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return(1);
	}

	sqlite3_prepare_v2(db, "INSERT INTO TASKS (TASK) VALUES(?1);", -1, &stmt, NULL);
	sqlite3_bind_text(stmt, 1, task, -1, SQLITE_STATIC);
	rc = sqlite3_step(stmt); 
	if (rc != SQLITE_DONE) {
		printf("ERROR inserting data: %s\n", sqlite3_errmsg(db));
		return 1;
	}

	sqlite3_finalize(stmt);
	printf("Task created.\n");
	sqlite3_close(db);
	return 0;
}

int list(int showall) {
	sqlite3 *db;
	int rc;
	int id;
	const unsigned char* task;
	sqlite3_stmt *stmt;

	rc = sqlite3_open_v2("./.WYD.db", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	
	if ( rc ) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		return(1);
	}

	if ( showall == 1 ) {
		sqlite3_prepare_v2(db, "SELECT ID, TASK FROM TASKS;", -1, &stmt, NULL);
	}
	else {
		sqlite3_prepare_v2(db, "SELECT ID, TASK FROM TASKS WHERE STATUS = 0;", -1, &stmt, NULL);
	}

	int done = 0;
	while ( done == 0 ) {
		switch (sqlite3_step (stmt)) {
			case SQLITE_ROW:
				id = sqlite3_column_int64(stmt, 0);
				task  = sqlite3_column_text(stmt, 1);
				printf ("#%d - %s\n", id, task);
				break;

			case SQLITE_DONE:
				done = 1;
				break;

			default:
				fprintf(stderr, "Failed.\n");
				return 1;
			}
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);
	return 0;
}

int main(int argc, char* argv[]) {

	if( argc == 2 ) {
		if ( strcmp(argv[1], "init") == 0 ) {
			createDB(argc, argv);
		}
		else if ( strcmp(argv[1], "create") == 0  || strcmp(argv[1], "add") == 0 ) {
			create(argv[2]);
		}
		else if ( strcmp(argv[1], "list") == 0  || strcmp(argv[1], "ls") == 0 ) {
			list(0);
		}
		else {
			help();
		}
	}
	else if ( argc == 3 ) {
		if ( strcmp(argv[1], "create") == 0  || strcmp(argv[1], "add") == 0 ) {
			create(argv[2]);
		}
		else if ( ( strcmp(argv[1], "list") == 0  || strcmp(argv[1], "ls") == 0 ) && strcmp(argv[2], "-a") == 0 ) {
			list(1);
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