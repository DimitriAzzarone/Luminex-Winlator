const {contextBridge,ipcRenderer}=require('electron');
contextBridge.exposeInMainWorld('luminex',{openProfile:n=>ipcRenderer.send('open-profile',n),openIncognito:()=>ipcRenderer.send('open-incognito')});
