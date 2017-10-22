'use strict';
var bodyParser = require('body-parser');
var conf = require('./conf');
var cookie = require('cookie-parser');
var express = require('express');
var flash = require('connect-flash');
var helmet = require('helmet');
var http = require('http');
var log4js = require("log4js");
var morgan = require('morgan');
var path = require('path');
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
var sass = require('node-sass-middleware');

// Server
var app = express();
if (conf.app.secure) {app.set('trust proxy', 1);}
var server = http.createServer(app);

app.set('views', path.join(__dirname, 'views'));
app.set('view engine', 'pug');
app.use(flash());
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({extended: false}));
app.use(sass({
  src: __dirname + '/sass',
  dest: __dirname + '/public/style',
  prefix: '/style',
  debug: true,
  indentedSyntax: true,
  outputStyle: 'compressed'
}));
app.use(express.static(__dirname +'/public'));

// Logging
var appLogger = log4js.getLogger();
if (process.env.env == 'dev') {
  appLogger.level = 'debug';
  var httpLogger = morgan('dev', {
    stream: {
      write: function(str) { appLogger.debug(str); }
    }
  });
}else{
  var httpLogger = morgan('combined');
}
app.use(httpLogger);

// Helmet
app.use(helmet());
app.disable('x-powered-by');

// Routes
var index = require('./routes/index');
var tasks = require('./routes/tasks');
app.use('/', index);
app.use('/tasks', tasks);

// Errors
app.use(function(req, res, next) {
  var err = new Error('Not Found');
  err.status = 404;
  next(err);
});
if (process.env.env === 'dev'){
  app.use(function(err, req, res, next){
    res.status(err.status || 500);
    res.render('error', {
      message: err.message,
      error: err
    });
  });
}
app.use(function(err, req, res, next){
  res.status(err.status || 500);
  res.render('error', {
    message: err.message,
    error: ''
  });
});

// Start
console.log(process.env.name, "started.");
if(!conf.app.secure){
  console.warn("The app is not set to secure, you shouldn't use authentification without SSL/TLS.");
}

server.listen(process.env.port || 1337);
module.exports = app;