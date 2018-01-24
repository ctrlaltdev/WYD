'use strict';
var fs = require('fs');
var argv = require('minimist')(process.argv.slice(2));
var cmd = (argv._[0]) ? argv._[0] : false;
var confExist = fs.existsSync('./.conf.json');

if (!confExist) {
  console.warn(`
  Your conf file is not set up.

  REMOTE:
    To set up the server url:   wyd set --url="url:port"
    To set up the user:         wyd set --user="your name"

  LOCAL:
    To set up a local install:  wyd set --local
  `);
} else {
  var conf = require('./.conf.json');
  if (conf.local) {
    const sqlite3 = require('sqlite3').verbose();
    let db = new sqlite3.Database('./.WYD.db', (err) => {
      if (err) {
        console.warn(err.message);
      }
      console.log('Connected to the database.');
    });
    db.close((err) => {
      if (err) {
        console.error(err.message);
      }
      console.log('Close the database connection.');
    });
  }
}

if (cmd && cmd == "set") {

  if (argv.url) {
    if (confExist) {
      try {
        let newconf = fs.readFileSync("./.conf.json", "utf8");
        newconf.url = argv.url;
        fs.writeFileSync("./.conf.json", JSON.stringify(newconf));
        console.log('Url set to:',argv.url);
      } catch (err) {
        console.warn(err);
      }
    } else {
      try {
        let newconf = {url: argv.url};
        fs.writeFileSync("./.conf.json", JSON.stringify(newconf));
        console.log('Url set to:',argv.url);
      } catch (err) {
        console.warn(err);
      }
    }
  } else if (argv.user) {
    if (confExist) {
      try {
        let newconf = fs.readFileSync("./.conf.json", "utf8");
        newconf.user = argv.user;
        fs.writeFileSync("./.conf.json", JSON.stringify(newconf));
        console.log('User set to:',argv.user);
      } catch (err) {
        console.warn(err);
      }
    } else {
      try {
        let newconf = {user: argv.user};
        fs.writeFileSync("./.conf.json", JSON.stringify(newconf));
        console.log('User set to:',argv.user);
      } catch (err) {
        console.warn(err);
      }
    }
  } else if (argv.local) {
    if (confExist) {
      try {
        let newconf = fs.readFileSync("./.conf.json", "utf8");
        newconf.local = true;
        fs.writeFileSync("./.conf.json", JSON.stringify(newconf));
        console.log('Installation set to local');
      } catch (err) {
        console.warn(err);
      }
    } else {
      try {
        let newconf = {local: true};
        fs.writeFileSync("./.conf.json", JSON.stringify(newconf));
        console.log('Installation set to local');
      } catch (err) {
        console.warn(err);
      }
    }
  } else {
    console.warn(`
  The option has not been recognized, consult the help to check allowed options for the command set.
    `);
  }

}

if (cmd && confExist) {

  switch (cmd) {

    case "done":
    case "close":
      console.log('DONE');
      break;

    case "new":
    case "create":
      console.log('NEW');
      break;

    case "delete":
    case "del":
    case "rm":
      console.log('DELETE');
      break;

    case "list":
    case "ls":
      console.log('LIST');
      break;

    case "update":
      console.log('UPDATE');
      break;

    default:
      console.warn(`
      This command is not recognized, maybe there's a typo?
      Do not hesitate to check wyd -h
      `);
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
  }
}
