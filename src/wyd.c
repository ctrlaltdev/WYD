#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

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
static char* const unused = "unused";

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



int createDB(int unused) {
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

int list(char *option) {
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

	if (strncmp(option, "-a", 2) == 0)
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
	list(unused);
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
	list(unused);
out:
  return rc;
}

int done(int id) {
	int rc;
	sqlite3_stmt *stmt;

	rc = open_database(SQLITE_OPEN_READWRITE);
	
	if ( rc ) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    goto out;
	}

	sqlite3_prepare_v2(db, SQLstatements[TASK_DONE].statement, -1, &stmt, NULL);
	sqlite3_bind_int(stmt, 1, id);
	rc = sqlite3_step(stmt); 
	if (rc != SQLITE_DONE) {
		fprintf(stderr, "ERROR closing task: %s\n", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);
	list(unused);

out:
  return rc;
}

int update(int id, char* task) {
	int rc;
	sqlite3_stmt *stmt;

	rc = sqlite3_open_v2(DBfile, &db, SQLITE_OPEN_READWRITE, NULL);
	
	if ( rc ) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    goto out;
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
	list(unused);

out:
  return rc;
}

struct sOptions {
  const char* const name;
  const char* const alias;
  int (*func_int)(int);
  int (*func_str)(char *);
  int (*func_int_str)(int, char *);
  const char* const help;
};

static struct sOptions options[] = {
  { "init",   NULL,   createDB, NULL,   NULL,   "\t\tCreate database"             },
  { "create", "add",  NULL,     create, NULL,   "\"Task\"\t\tNew task"            },
  { "update", "up",   NULL,     NULL,   update, "ID \"Task\" Update task content" },
  { "delete", "rm",   delete,   NULL,   NULL,   "ID\t\tDelete task"               },
  { "close",  "done", done,     NULL,   NULL,   "ID\t\tMark task done"            },
  { "list",   "ls",   NULL,     list,   NULL,   "[-a]\t\tList open or all tasks"  },
  { NULL,     NULL,   NULL,     NULL,   NULL,   NULL                              },
};

void help() {
	fprintf(stdout, "%s",
"usage: wyd command\n"
"\tcommand\t [alias]\thelp\n");

  struct sOptions *opt = &options[0];
  while (opt->name) {
    fprintf(stdout, "\t%s %s%s%s %s\n",
        opt->name,
        opt->alias ? "[" : "",
        opt->alias ? opt->alias : "",
        opt->alias ? "]" : "",
        opt->help ? opt->help : ""
        );
    opt++;
  }
}


int main(int argc, char* argv[]) {
  int ret = 0;

  if (argc >= 2) {
    // set default known values
    int id = -1;
    char *task = NULL;
    if (argc >= 3) {
      if (sscanf(argv[2], "%d", &id)) {
        // if sscanf returned > 0 it must have put a valid decimal number in 'id'
        if (argc >= 4)
          // there should be a text string following
          task = argv[3];
      } else {
        // first argument not a number
        task = argv[2];
      }
    }

    struct sOptions *opt = &options[0];
    while (opt->name) {
      if (strncmp(opt->name, argv[1], strlen(opt->name)) == 0 ||
          ( opt->alias && strncmp(opt->alias, argv[1], strlen(opt->alias)) == 0)) {
        if (argc >= 4 && opt->func_int_str && task)
         ret = opt->func_int_str(id, task);
        else if (argc >= 2 && argc <= 3 && opt->func_str && id < 0) {
          if (!task)
            task = (char *)opt->name; // task just needs to be non-null and this is the easiest way!
          ret = opt->func_str(task);
        } else if (argc >= 2 && argc <= 3 && opt->func_int)
          ret = opt->func_int(id);
        }
      opt++;
    }
  } else
    help();

  return ret;
}
