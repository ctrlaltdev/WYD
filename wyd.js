'use strict';
var fs = require('fs');
var argv = require('minimist')(process.argv.slice(2));
var cmd = (argv._[0]) ? argv._[0] : false;
var sqlite3 = require('sqlite3').verbose();
var intel = require('intel');
var confExist = fs.existsSync('./.conf.json');
if (!confExist) {
  intel.warn(`
  Your conf file is not set up.

  REMOTE:
    To set up the server url:   wyd set --url="url:port"
    To set up the user:         wyd set --user="your name"

  LOCAL:
    To set up a local install:  wyd set --local

  LOGGING:
    To make wyd more verbal:    wyd set --dev
  `);
  var conf = null;
  intel.setLevel(intel.WARN);
} else {
  var conf = require('./.conf.json');
  if (conf.dev) {
    intel.setLevel(intel.TRACE);
  } else {
    intel.setLevel(intel.WARN);
  }
}

class DBConnect {

  constructor () {
    if (conf.local) {
      this.db = new sqlite3.Database('./.WYD.db', (err) => {
        if (err) {
          intel.warn(err.message);
        }
        intel.info('Local DB');
      });
    } else {
      intel.info('Remote DB')
    }
  }

  deconstructor () {
    if (conf.local) {
      this.db.close((err) => {
        if (err) {
          console.error(err.message);
        }
        intel.info('Local DB closed');
      });
    } else {
      intel.info('Remote DB closed');
    }
  }

  close (id) {
    this.deconstructor();
  }

  create (task) {
    this.deconstructor();
  }

  delete (id) {
    this.deconstructor();
  }

  list () {
    this.deconstructor();
  }

  update (id, task) {
    this.deconstructor();
  }

}

if (cmd && cmd == "set") {

  if (argv.url) {
    if (confExist) {
      try {
        let newconf = JSON.parse(fs.readFileSync("./.conf.json", "utf8"));
        newconf.url = argv.url;
        fs.writeFileSync("./.conf.json", JSON.stringify(newconf));
        intel.info('Url set to:',argv.url);
      } catch (err) {
        intel.warn(err);
      }
    } else {
      try {
        let newconf = {url: argv.url};
        fs.writeFileSync("./.conf.json", JSON.stringify(newconf));
        intel.info('Url set to:',argv.url);
      } catch (err) {
        intel.warn(err);
      }
    }
  } else if (argv.user) {
    if (confExist) {
      try {
        let newconf = JSON.parse(fs.readFileSync("./.conf.json", "utf8"));
        newconf.user = argv.user;
        fs.writeFileSync("./.conf.json", JSON.stringify(newconf));
        intel.info('User set to:',argv.user);
      } catch (err) {
        intel.warn(err);
      }
    } else {
      try {
        let newconf = {user: argv.user};
        fs.writeFileSync("./.conf.json", JSON.stringify(newconf));
        intel.info('User set to:',argv.user);
      } catch (err) {
        intel.warn(err);
      }
    }
  } else if (argv.local) {
    if (confExist) {
      try {
        let newconf = JSON.parse(fs.readFileSync("./.conf.json", "utf8"));
        newconf.local = true;
        fs.writeFileSync("./.conf.json", JSON.stringify(newconf));
        intel.info('Installation set to local');
      } catch (err) {
        intel.warn(err);
      }
    } else {
      try {
        let newconf = {local: true};
        fs.writeFileSync("./.conf.json", JSON.stringify(newconf));
        intel.info('Installation set to local');
      } catch (err) {
        intel.warn(err);
      }
    }
  } else if (argv.dev) {
    if (confExist) {
      try {
        let newconf = JSON.parse(fs.readFileSync("./.conf.json", "utf8"));
        newconf.dev = true;
        fs.writeFileSync("./.conf.json", JSON.stringify(newconf));
        intel.info('Installation set to dev');
      } catch (err) {
        intel.warn(err);
      }
    } else {
      try {
        let newconf = {dev: true};
        fs.writeFileSync("./.conf.json", JSON.stringify(newconf));
        intel.info('Installation set to dev');
      } catch (err) {
        intel.warn(err);
      }
    }
  } else {
    intel.warn(`
  The option has not been recognized, consult the help to check allowed options for the command set.
    `);
  }

}

if (cmd && confExist) {
  var DBC = new DBConnect();

  switch (cmd) {

    case "done":
    case "close":
      DBC.close(argv._[1]);
      break;

    case "new":
    case "create":
      DBC.create(argv._[1]);
      break;

    case "delete":
    case "del":
    case "rm":
      DBC.delete(argv._[1]);
      break;

    case "list":
    case "ls":
      DBC.list();
      break;

    case "update":
      DBC.update(argv._[1], argv._[2]);
      break;

    default:
      intel.warn(`
      This command is not recognized, maybe there's a typo?
      Do not hesitate to check wyd -h
      `);
      break;

  }

} else {
  if (argv.h) {
    intel.info(`
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
          e.g.:     wyd delete 76

        DONE:
          e.g.:     wyd done 431

        LIST:
          e.g.:     wyd list

        NEW:
          e.g.:     wyd new "Pet the cat"

        UPDATE:
          e.g.:     wyd update 42 "Pet my cat"
    `);
  }
}
