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

// program-wide variables
static char *DBfile = NULL;
static const char* const filename = "/.WYD.db";
static sqlite3 *db;


struct sSQLstatement {
  const char *statement;
};

// these enums are the indexes into SQLstatements[]
enum eSQLstatements {
  DB_CREATE,
  TASKS_SHOW_ALL,
  TASKS_STATUS_0,
  TASK_CREATE,
  TASK_DELETE,
  TASK_DONE,
  TASK_UPDATE,
};

// list of SQL statements
static struct sSQLstatement SQLstatements[] = {
  { "CREATE TABLE TASKS(ID INTEGER PRIMARY KEY AUTOINCREMENT,TASK TEXT NOT NULL,STATUS INT DEFAULT 0 NOT NULL);" },
  {	"SELECT ID, TASK, STATUS FROM TASKS;" },
  { "SELECT ID, TASK FROM TASKS WHERE STATUS = 0;" },
  { "INSERT INTO TASKS (TASK) VALUES(?1);" },
  { "DELETE FROM TASKS WHERE ID = ?1;" },
  { "UPDATE TASKS SET STATUS = 1 WHERE ID = ?1;" },
  { "UPDATE TASKS SET TASK = ?1 WHERE ID = ?2;" },
  { NULL },
};

// custom error numbers
enum errornum {
  ENOHOME = 128,
  ENOMEM,
  ENODB,
};

int open_database(int flags)
{
  int ret = 0;
  // if this is the 1st call figure out the DB path
  if (!DBfile) {
    char *home = getenv("HOME");
    if (!home) {
      ret = -ENOHOME;
      goto out;
    }
    if ((DBfile = (char *)calloc(1, strlen(home) + strlen(filename) + 1)) == NULL) {
      ret = -ENOMEM;
      goto out;
    }
    strncpy(DBfile, home, strlen(home));
    strncat(DBfile, filename, strlen(filename));
  }

	ret = sqlite3_open_v2(DBfile, &db, flags, NULL);

  if (ret) {
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    ret = -ENODB;
  }

out:
  return ret;
}



int createDB() {
	char *zErrMsg = 0;
	int rc;

	rc = open_database(SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);

  if (!rc) {
    rc = sqlite3_exec(db, SQLstatements[DB_CREATE].statement, callback, 0, &zErrMsg);
	
    if(rc == SQLITE_OK) {
      fprintf(stdout, "What You Doin\' initialized\n");
    } else {
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
		  sqlite3_free(zErrMsg);
    }
    sqlite3_close(db);
	}
	return rc;
}

int list(int showall) {
	int rc;
	int id;
	const unsigned char* task;
	int status;
	sqlite3_stmt *stmt;
  int s = TASKS_STATUS_0;

	rc = open_database(SQLITE_OPEN_READONLY);
	
	if (rc) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    goto out;
	}

	if (showall)
    s = TASKS_SHOW_ALL;

  sqlite3_prepare_v2(db, SQLstatements[s].statement, -1, &stmt, NULL);

	int done = 0;
	while (!done ) {
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

out:
  return rc;
}

int create(char* task) {
	int rc;
	sqlite3_stmt *stmt;

	rc = open_database(SQLITE_OPEN_READWRITE);
	
	if ( rc ) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    goto out;
	}

	sqlite3_prepare_v2(db, SQLstatements[TASK_CREATE].statement, -1, &stmt, NULL);
	sqlite3_bind_text(stmt, 1, task, -1, SQLITE_STATIC);
	rc = sqlite3_step(stmt); 
	if (rc != SQLITE_DONE) {
		printf("ERROR creating task: %s\n", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);
	list(0);
out:
  return rc;
}

int delete(int id) {
	int rc;
	sqlite3_stmt *stmt;

	rc = open_database(SQLITE_OPEN_READWRITE);
	
	if ( rc ) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    goto out;
	}

	sqlite3_prepare_v2(db, SQLstatements[TASK_DELETE].statement, -1, &stmt, NULL);
	sqlite3_bind_int(stmt, 1, id);
	rc = sqlite3_step(stmt); 
	if (rc != SQLITE_DONE) {
		fprintf(stderr, "ERROR deleting task: %s\n", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);
	list(0);
out:
  return rc;
}

void done(int id) {
	int rc;
	sqlite3_stmt *stmt;

	rc = open_database(SQLITE_OPEN_READWRITE);
	
	if ( rc ) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	}

	sqlite3_prepare_v2(db, SQLstatements[TASK_DONE].statement, -1, &stmt, NULL);
	sqlite3_bind_int(stmt, 1, id);
	rc = sqlite3_step(stmt); 
	if (rc != SQLITE_DONE) {
		fprintf(stderr, "ERROR closing task: %s\n", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);
	list(0);
}

void update(int id, char* task) {
	int rc;
	sqlite3_stmt *stmt;

	rc = sqlite3_open_v2(DBfile, &db, SQLITE_OPEN_READWRITE, NULL);
	
	if ( rc ) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	}

	sqlite3_prepare_v2(db, SQLstatements[TASK_UPDATE].statement, -1, &stmt, NULL);
	sqlite3_bind_text(stmt, 1, task, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 2, id);
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		fprintf(stderr, "ERROR updating task: %s\n", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);
	list(0);
}

int main(int argc, char* argv[]) {

	if( argc == 2 ) {
		if ( strcmp(argv[1], "init") == 0 ) {
			createDB();
		}
		else if ( strcmp(argv[1], "list") == 0  || strcmp(argv[1], "ls") == 0 ) {
			list(0);
			return 0;
		}
		else {
			help();
			return 0;
		}
	}
	else if ( argc == 3 ) {
		if ( strcmp(argv[1], "create") == 0  || strcmp(argv[1], "add") == 0 ) {
			create(argv[2]);
			return 0;
		}
		else if ( ( strcmp(argv[1], "list") == 0  || strcmp(argv[1], "ls") == 0 ) && strcmp(argv[2], "-a") == 0 ) {
			list(1);
			return 0;
		}
		else if ( strcmp(argv[1], "delete") == 0  || strcmp(argv[1], "rm") == 0 ) {
			delete(atoi(argv[2]));
			return 0;
		}
		else if ( strcmp(argv[1], "close") == 0  || strcmp(argv[1], "done") == 0 ) {
			done(atoi(argv[2]));
			return 0;
		}
		else {
			help();
			return 0;
		}
	}
	else if ( argc == 4 ) {
		if ( strcmp(argv[1], "update") == 0  || strcmp(argv[1], "ud") == 0 ) {
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
