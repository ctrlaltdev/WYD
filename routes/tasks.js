'use strict';
var conf = require('../conf');
var express = require('express');
var router = express.Router();
var r = require('rethinkdbdash')({
  servers:[
    {
      host: conf.database.host,
      port: conf.database.port,
      user: conf.database.user,
      password: conf.database.password,
      db: conf.database.database
    }
  ],
  silent: true
});

// GET: /
router.get('/', function(req, res) {
  r.table('tasks').run().then((result) => {
    res.json(result);
  }).catch((error) => {
    console.log(error);
  });
});

module.exports = router;