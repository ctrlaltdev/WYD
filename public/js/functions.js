console.warn('I have become self aware, human.')

/**
 * Make a XHR call
 * @param {string} url 
 * @param {boolean} post 
 */
function XHR(url, post=false){
  return new Promise((resolve, reject) => {
    const xhr = new XMLHttpRequest();
    if(post){xhr.open('POST', url);}
    else{xhr.open('GET', url);}
    xhr.onload = () => resolve(xhr.responseText);
    xhr.onerror = () => reject(xhr.statusText);
    xhr.send();
  });
}