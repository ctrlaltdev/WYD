'use strict';
var conf = require('../conf');
var express = require('express');
var router = express.Router();

// GET: /
router.get('/', function(req, res) {
  res.render('index', {
    author: conf.app.author,
    url: conf.app.url + '/',
    name: conf.app.name,
    title: conf.app.name
  });
});

module.exports = router;