[![Issues](https://img.shields.io/github/issues/implose/AMA.svg?style=flat-square)](https://github.com/implose/AMA/issues)
[![I love badges](https://img.shields.io/badge/I%20love-badges-FF00FF.svg?style=flat-square)](https://shields.io)

# Ask Me Anything

## Installation

### Requirements
* Node.js 6.11+
* Rethinkdb 2.3.6+

### How to install
* Clone this repo: ```git clone https://github.com/implose/AMA.git```
* Move inside the directory: ```cd AMA```
* Install the npm packages: ```npm i```
* Customize the config.json file - note that you have to create the rethinkdb database on your own, you just have to reference the name here.
* Customize the pm2.json file if you're going to use pm2, if not, note that the app with be accessible on the default port: 1337 and logging will be set to production mode.
* Customize the sass/_config.sass
* Start the server: ```npm start``` or ```node server.js```
* Go to http://localhost:1337/install
* Pet a cat
* Enjoy life

### TO DO
* [x] Installation script
* [x] Authentification
* [x] Conf to enable/disable questions moderation
* [x] Conf to enable/disable answers validation
* [ ] ADMIN: Users management
* [ ] FRONT: Form to submit a question
* [ ] FRONT: Questions list
* [ ] FRONT: Vote for a question feature
* [ ] ADMIN: Questions moderation
* [ ] ADMIN: Answer submission/publication
* [ ] API: Public questions listing
* [ ] API: Public question submission
* [ ] API: Questions update/moderation
* [ ] Everything else!