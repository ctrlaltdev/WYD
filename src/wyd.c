#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

void help() {
	fprintf(stdout, "usage: wyd command\n" \
	"	commands:\n" \
	"		add \"Task\"		alias of create\n" \
	"		close ID		mark a task as done\n" \
	"		create \"Task\"		create a task\n" \
	"		delete ID		delete a task\n" \
	"		done ID			alias of close\n" \
	"		init			create the database\n" \
	"		list [options]		list the open tasks\n" \
	"			-a		list all tasks\n" \
	"		ls			alias of list\n" \
	"		rm ID			alias of delete\n" \
	"		ud ID \"Task\"		alias of update\n" \
	"		update ID \"Task\"	update task content\n" \
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

int createDB() {
	char *DBfile;
	char *file = "/.WYD.db";
	DBfile = malloc(strlen(getenv("HOME") + strlen(file) + 1));
	strcpy(DBfile, getenv("HOME"));
	strcat(DBfile, file);
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	char *sql;

	rc = sqlite3_open_v2(DBfile, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	
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
		fprintf(stdout, "What You Doin\' initialized\n");
	}
	sqlite3_close(db);
	free(DBfile);
	return 0;
}

void list(int showall) {
	char *DBfile;
	char *file = "/.WYD.db";
	DBfile = malloc(strlen(getenv("HOME") + strlen(file) + 1));
	strcpy(DBfile, getenv("HOME"));
	strcat(DBfile, file);
	sqlite3 *db;
	int rc;
	int id;
	const unsigned char* task;
	int status;
	sqlite3_stmt *stmt;

	rc = sqlite3_open_v2(DBfile, &db, SQLITE_OPEN_READONLY, NULL);
	
	if ( rc ) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	}

	if ( showall == 1 ) {
		sqlite3_prepare_v2(db, "SELECT ID, TASK, STATUS FROM TASKS;", -1, &stmt, NULL);
	}
	else {
		sqlite3_prepare_v2(db, "SELECT ID, TASK FROM TASKS WHERE STATUS = 0;", -1, &stmt, NULL);
	}

	int done = 0;
	while ( done == 0 ) {
		switch (sqlite3_step (stmt)) {
			case SQLITE_ROW:
				if ( showall == 1 ) {
					id = sqlite3_column_int(stmt, 0);
					task  = sqlite3_column_text(stmt, 1);
					status = sqlite3_column_int(stmt, 2);
					if (status == 1) {
						printf ("#%d - [X] %s\n", id, task);
					}
					else {
						printf ("#%d - [ ] %s\n", id, task);
					}
				}
				else {
					id = sqlite3_column_int(stmt, 0);
					task  = sqlite3_column_text(stmt, 1);
					printf ("#%d - [ ] %s\n", id, task);
				}
				break;

			case SQLITE_DONE:
				done = 1;
				break;

			default:
				fprintf(stderr, "Failed.\n");
			}
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);
	free(DBfile);
}

void create(char* task) {
	char *DBfile;
	char *file = "/.WYD.db";
	DBfile = malloc(strlen(getenv("HOME") + strlen(file) + 1));
	strcpy(DBfile, getenv("HOME"));
	strcat(DBfile, file);
	sqlite3 *db;
	int rc;
	sqlite3_stmt *stmt;

	rc = sqlite3_open_v2(DBfile, &db, SQLITE_OPEN_READWRITE, NULL);
	
	if ( rc ) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	}

	sqlite3_prepare_v2(db, "INSERT INTO TASKS (TASK) VALUES(?1);", -1, &stmt, NULL);
	sqlite3_bind_text(stmt, 1, task, -1, SQLITE_STATIC);
	rc = sqlite3_step(stmt); 
	if (rc != SQLITE_DONE) {
		printf("ERROR creating task: %s\n", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);
	free(DBfile);
	list(0);
}

void delete(int id) {
	char *DBfile;
	char *file = "/.WYD.db";
	DBfile = malloc(strlen(getenv("HOME") + strlen(file) + 1));
	strcpy(DBfile, getenv("HOME"));
	strcat(DBfile, file);
	sqlite3 *db;
	int rc;
	sqlite3_stmt *stmt;

	rc = sqlite3_open_v2(DBfile, &db, SQLITE_OPEN_READWRITE, NULL);
	
	if ( rc ) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	}

	sqlite3_prepare_v2(db, "DELETE FROM TASKS WHERE ID = ?1;", -1, &stmt, NULL);
	sqlite3_bind_int(stmt, 1, id);
	rc = sqlite3_step(stmt); 
	if (rc != SQLITE_DONE) {
		fprintf(stderr, "ERROR deleting task: %s\n", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);
	free(DBfile);
	list(0);
}

void done(int id) {
	char *DBfile;
	char *file = "/.WYD.db";
	DBfile = malloc(strlen(getenv("HOME") + strlen(file) + 1));
	strcpy(DBfile, getenv("HOME"));
	strcat(DBfile, file);
	sqlite3 *db;
	int rc;
	sqlite3_stmt *stmt;

	rc = sqlite3_open_v2(DBfile, &db, SQLITE_OPEN_READWRITE, NULL);
	
	if ( rc ) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	}

	sqlite3_prepare_v2(db, "UPDATE TASKS SET STATUS = 1 WHERE ID = ?1;", -1, &stmt, NULL);
	sqlite3_bind_int(stmt, 1, id);
	rc = sqlite3_step(stmt); 
	if (rc != SQLITE_DONE) {
		fprintf(stderr, "ERROR closing task: %s\n", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);
	free(DBfile);
	list(0);
}

void update(int id, char* task) {
	char *DBfile;
	char *file = "/.WYD.db";
	DBfile = malloc(strlen(getenv("HOME") + strlen(file) + 1));
	strcpy(DBfile, getenv("HOME"));
	strcat(DBfile, file);
	sqlite3 *db;
	int rc;
	sqlite3_stmt *stmt;

	rc = sqlite3_open_v2(DBfile, &db, SQLITE_OPEN_READWRITE, NULL);
	
	if ( rc ) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	}

	sqlite3_prepare_v2(db, "UPDATE TASKS SET TASK = ?1 WHERE ID = ?2;", -1, &stmt, NULL);
	sqlite3_bind_text(stmt, 1, task, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 2, id);
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		fprintf(stderr, "ERROR updating task: %s\n", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);
	free(DBfile);
	list(0);
}

int main(int argc, char* argv[]) {

	if( argc == 2 ) {
		if ( strncmp(argv[1], "init", strlen("init")) == 0 ) {
			createDB();
		}
		else if ( strncmp(argv[1], "list", strlen("list")) == 0  || strncmp(argv[1], "ls", strlen("ls")) == 0 ) {
			list(0);
			return 0;
		}
		else {
			help();
			return 0;
		}
	}
	else if ( argc == 3 ) {
		if ( strncmp(argv[1], "create", strlen("create")) == 0  || strncmp(argv[1], "add", strlen("add")) == 0 ) {
			create(argv[2]);
			return 0;
		}
		else if ( ( strncmp(argv[1], "list", strlen("list")) == 0  || strncmp(argv[1], "ls", strlen("ls")) == 0 ) && strncmp(argv[2], "-a", strlen("-a")) == 0 ) {
			list(1);
			return 0;
		}
		else if ( strncmp(argv[1], "delete", strlen("delete")) == 0  || strncmp(argv[1], "rm", strlen("rm")) == 0 ) {
			delete(atoi(argv[2]));
			return 0;
		}
		else if ( strncmp(argv[1], "close", strlen("close")) == 0  || strncmp(argv[1], "done", strlen("done")) == 0 ) {
			done(atoi(argv[2]));
			return 0;
		}
		else {
			help();
			return 0;
		}
	}
	else if ( argc == 4 ) {
		if ( strncmp(argv[1], "update", strlen("update")) == 0  || strncmp(argv[1], "ud", strlen("ud")) == 0 ) {
			update(atoi(argv[2]),argv[3]);
			return 0;
		}
		else {
			help();
			return 0;
		}
	}
	else {
		printf("No command detected.\n");
		help();
		return 0;
	}
}