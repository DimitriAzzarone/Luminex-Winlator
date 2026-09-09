const input=document.getElementById('profile'),box=document.getElementById('profiles'),key='luminexProfiles';
const saved=()=>{try{return JSON.parse(localStorage.getItem(key)||'[]')}catch{return[]}};
function render(){box.textContent='';saved().forEach(n=>{const b=document.createElement('button');b.textContent=n;b.onclick=()=>luminex.openProfile(n);box.appendChild(b)})}
document.getElementById('open').onclick=()=>{const n=input.value.trim();if(!n)return;const a=saved();if(!a.includes(n)){a.push(n);localStorage.setItem(key,JSON.stringify(a));render()}luminex.openProfile(n)};
document.getElementById('incognito').onclick=()=>luminex.openIncognito();render();
