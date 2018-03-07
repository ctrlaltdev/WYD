#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "wyd.h"

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

// an unused option passed to list() instead of "-a" to avoid seg faults on NULL
static struct sArg unused = { -1, "" };

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



int createDB(struct sArg *args) {
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

int list(struct sArg *args) {
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

	if (args && args[0].str && strncmp(args[0].str, "-a", 2) == 0)
    s = TASKS_SHOW_ALL;

  sqlite3_prepare_v2(db, SQLstatements[s].statement, -1, &stmt, NULL);

	int done = 0;
	while (!done ) {
		switch (sqlite3_step (stmt)) {
			case SQLITE_ROW:
				if ( s == TASKS_SHOW_ALL ) {
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

int create(struct sArg *args) {
	int rc;
	sqlite3_stmt *stmt;

	rc = open_database(SQLITE_OPEN_READWRITE);
	
	if ( rc ) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    goto out;
	}

	sqlite3_prepare_v2(db, SQLstatements[TASK_CREATE].statement, -1, &stmt, NULL);
	sqlite3_bind_text(stmt, 1, args[0].str, -1, SQLITE_STATIC);
	rc = sqlite3_step(stmt); 
	if (rc != SQLITE_DONE) {
		printf("ERROR creating task: %s\n", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);
	list(&unused);
out:
  return rc;
}

int delete(struct sArg *args) {
	int rc;
	sqlite3_stmt *stmt;

	rc = open_database(SQLITE_OPEN_READWRITE);
	
	if ( rc ) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    goto out;
	}

	sqlite3_prepare_v2(db, SQLstatements[TASK_DELETE].statement, -1, &stmt, NULL);
	sqlite3_bind_int(stmt, 1, args[0].num);
	rc = sqlite3_step(stmt); 
	if (rc != SQLITE_DONE) {
		fprintf(stderr, "ERROR deleting task: %s\n", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);
	list(&unused);
out:
  return rc;
}

int done(struct sArg *args) {
	int rc;
	sqlite3_stmt *stmt;

	rc = open_database(SQLITE_OPEN_READWRITE);
	
	if ( rc ) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    goto out;
	}

	sqlite3_prepare_v2(db, SQLstatements[TASK_DONE].statement, -1, &stmt, NULL);
	sqlite3_bind_int(stmt, 1, args[0].num);
	rc = sqlite3_step(stmt); 
	if (rc != SQLITE_DONE) {
		fprintf(stderr, "ERROR closing task: %s\n", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);
	list(&unused);

out:
  return rc;
}

int update(struct sArg *args) {
	int rc;
	sqlite3_stmt *stmt;

	rc = sqlite3_open_v2(DBfile, &db, SQLITE_OPEN_READWRITE, NULL);
	
	if ( rc ) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    goto out;
	}

	sqlite3_prepare_v2(db, SQLstatements[TASK_UPDATE].statement, -1, &stmt, NULL);
	sqlite3_bind_int(stmt, 2, args[0].num);
	sqlite3_bind_text(stmt, 1, args[1].str, -1, SQLITE_STATIC);
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		fprintf(stderr, "ERROR updating task: %s\n", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);
	list(&unused);

out:
  return rc;
}

static struct sOptions options[] = {
  { "init",   NULL,   createDB, 0, "\t\tCreate database"             },
  { "create", "add",  create,   1, "\"Task\"\t\tNew task"            },
  { "update", "up",   update,   2, "ID \"Task\" Update task content" },
  { "delete", "rm",   delete,   1, "ID\t\tDelete task"               },
  { "close",  "done", done,     1, "ID\t\tMark task done"            },
  { "list",   "ls",   list,     1, "[-a]\t\tList open or all tasks"  },
  { NULL,     NULL,   NULL,     0,   NULL                            },
};

void help() {
	fprintf(stdout, "%s",
"usage: wyd command\n"
"\tcommand\t | alias\thelp\n");

  struct sOptions *opt = &options[0];
  while (opt->name) {
    fprintf(stdout, "\t%s %s%s %s\n",
        opt->name,
        opt->alias ? "| " : "",
        opt->alias ? opt->alias : "",
        opt->help ? opt->help : ""
        );
    opt++;
  }
}

int main(int argc, char* argv[]) {
  int ret = 0;

  if (argc >= 2) {
    struct sArg args[argc];
    for (int i = 0; i < argc - 2; ++i) {
      args[i].num = -1;
      sscanf(argv[i + 2], "%d", &args[i].num);
      args[i].str = argv[i + 2];
    }
    args[argc - 1].num = -1;
    args[argc - 1].str = NULL;

    struct sOptions *opt = &options[0];
    while (opt->name) {
      if (strncmp(opt->name, argv[1], strlen(opt->name)) == 0 ||
          ( opt->alias && strncmp(opt->alias, argv[1], strlen(opt->alias)) == 0)) {
        if (argc - 1 >= opt->arg_count)
          opt->func(args);
        else
          fprintf(stderr, "%s\n", "Incorrect number of arguments.");
      }
      opt++;
    }
  } else
    help();

  return ret;
}
