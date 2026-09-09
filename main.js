const {app,BrowserWindow,ipcMain}=require('electron');
const path=require('path');
app.commandLine.appendSwitch('disable-gpu-sandbox');
app.commandLine.appendSwitch('no-sandbox');
app.commandLine.appendSwitch('disable-gpu');
app.commandLine.appendSwitch('disable-gpu-compositing');
app.commandLine.appendSwitch('disable-features', 'CalculateNativeWinOcclusion');
const prefs=(extra={})=>({contextIsolation:true,nodeIntegration:false,sandbox:false,preload:path.join(__dirname,'preload.js'),...extra});
function browser(profile,incognito){
  const partition=incognito?`private-${Date.now()}-${Math.random()}`:`persist:profile-${profile.replace(/[^\w-]/g,'_')}`;
  const win=new BrowserWindow({width:1280,height:820,title:incognito?'Luminex - Incognito':`Luminex - ${profile}`,webPreferences:prefs({partition,webviewTag:true})});
  win.setMenuBarVisibility(false);
  win.loadFile('browser.html',{query:{profile,incognito:String(incognito)}});
}
app.whenReady().then(()=>{
  ipcMain.on('open-profile',(_e,name)=>browser(String(name||'Personale'),false));
  ipcMain.on('open-incognito',()=>browser('Incognito',true));
  const win=new BrowserWindow({width:560,height:520,title:'Luminex Windows',webPreferences:prefs()});
  win.setMenuBarVisibility(false); win.loadFile('index.html');
});
app.on('window-all-closed',()=>app.quit());
