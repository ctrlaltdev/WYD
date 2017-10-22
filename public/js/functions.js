console.warn('I have become self aware, human.')

/**
 * Make a XHR call
 * @param {string} url 
 * @param {boolean} post 
 */
function XHR(url, post=false){
  return new Promise(function(resolve, reject) {
    var request = new XMLHttpRequest();
    if(post){request.open('POST', url);}
    else{request.open('GET', url);}
    request.responseType = 'json';
    request.onload = function() {
      if (request.status === 200) {
        resolve(request.response);
      } else {
        reject(Error('JSON didn\'t load successfully; error code:' + request.statusText));
      }
    };
    request.onerror = function() {
        reject(Error('There was a network error.'));
    };
    request.send();
  });
}

/**
 * Load Tasks in UI
 */
function loadTasks(){
  XHR('/tasks/').then((responseText) => {
    console.log(responseText);
  }).catch((error) => {
    console.log(error);
  });
}

loadTasks();