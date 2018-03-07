struct sSQLstatement {
  const char *statement;
};

struct sArg {
  int num;
  char *str;
};

struct sOptions {
  const char* const name;
  const char* const alias;
  int (*func)(struct sArg *);
  int arg_count;
  const char* const help;
};

