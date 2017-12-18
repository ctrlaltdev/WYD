'use strict';
var conf = require('./conf'),
  argv = require('minimist')(process.argv.slice(2)),
  cmd = (argv._[0]) ? argv._[0] : false;

if (cmd) {

  switch (cmd) {

    case "":
      break;

    default:
      break;

  }

} else {
  if (argv.h) {
    console.log(`
      WYD is CLI client for a What You Doin server.
      You can check below the availables commands and options.

      COMMANDS:   list, new, done, delete, update

      GLOBAL OPTIONS:
        -h        Show this help

      COMMANDS LIST:
        close     alias of done
        create    alias of new
        delete    delete a task
        del       alias of delete
        done      close an open tasks
        list      list the tasks
        ls        alias of list
        new       create a new task
        rm        alias of delete
        update    overwrite the task details

      COMMANDS DETAILS:
      
        DELETE:
          e.g.:     wyd delete #76

        DONE:
          e.g.:     wyd done #431

        LIST:
          e.g.:     wyd list

        NEW:
          e.g.:     wyd new "Pet the cat"

        UPDATE:
          e.g.:     wyd update #42 "Pet my cat"
    `);
  } else {
    console.warn('Please, check the help to see the available commands: wyd -h');
  }
}
