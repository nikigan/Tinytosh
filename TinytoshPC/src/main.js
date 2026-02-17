const { invoke } = window.__TAURI__.core;
const { listen } = window.__TAURI__.event;

async function initAutostart() {
  const cb = document.getElementById("autostart-cb");
  if (!cb) return;
  try {
    const isEnabled = await invoke("check_autostart");
    cb.checked = isEnabled;
    cb.addEventListener('change', async () => {
      try { await invoke("set_autostart", { enable: cb.checked }); } 
      catch (e) { cb.checked = !cb.checked; }
    });
  } catch (e) { cb.disabled = true; }
}

let isConnected = false;

function setUiStatus(text, color) {
    const status = document.getElementById("status-text");
    if(status) {
        status.innerText = text;
        status.style.color = color;
    }
}

async function loadPorts() {
  try {
    const statusObj = await invoke("get_ports"); 
    const select = document.getElementById("port-select");
    const currentVal = select.value;

    select.innerHTML = ""; 
    
    if (statusObj.ports.length === 0) {
      let opt = document.createElement("option");
      opt.text = "No Ports Found";
      select.add(opt);
    } else {
      statusObj.ports.forEach((p) => {
        let opt = document.createElement("option");
        opt.value = p;
        opt.text = p;
        select.add(opt);
      });
      if (currentVal && statusObj.ports.includes(currentVal)) select.value = currentVal;
    }

    if (statusObj.connected) {
        if (!isConnected) {
             isConnected = true;
             select.value = statusObj.connected;
             
             const btn = document.getElementById("conn-btn");
             if(btn) { btn.innerText = "Disconnect"; btn.className = "btn-red"; }
             if(select) select.disabled = true;
             setUiStatus("Connected to " + statusObj.connected, "#10b981");
        }
    } else {
        if (isConnected) {
            isConnected = false;
            const btn = document.getElementById("conn-btn");
            if(btn) { btn.innerText = "Connect"; btn.className = "btn-blue"; }
            if(select) select.disabled = false;
        }
        
        if (statusObj.status_text && statusObj.status_text.length > 0) {
            const lowerText = statusObj.status_text.toLowerCase();
            
            if (lowerText.includes("failed")) {
                setUiStatus(statusObj.status_text, "#ef4444"); 
            } else {
                setUiStatus(statusObj.status_text, "#888"); 
            }
        } else {
            setUiStatus("Waiting for connection...", "#888");
        }
    }
  } catch (e) {}
}

async function updateStats() {
  try {
    const jsonStr = await invoke("get_stats");
    if (!jsonStr || jsonStr === "{}") return;
    const data = JSON.parse(jsonStr);
    if (data.cpu_percent !== undefined) document.getElementById("cpu").innerText = Math.round(data.cpu_percent) + "%";
   if (data.net_down_kb !== undefined) {
      if (data.net_down_kb >= 1024) {
          document.getElementById("dl-val").innerText = (data.net_down_kb / 1024).toFixed(1);
          document.getElementById("dl-unit").innerText = "MB/s";
      } else {
          document.getElementById("dl-val").innerText = data.net_down_kb;
          document.getElementById("dl-unit").innerText = "KB/s";
      }
    }
    if (data.mem_percent !== undefined) document.getElementById("ram").innerText = Math.round(data.mem_percent) + "%";
    if (data.disk_percent !== undefined) document.getElementById("disk").innerText = Math.round(data.disk_percent) + "%";
  } catch (e) { }
}

async function toggleConnection() {
  const select = document.getElementById("port-select");
  if (!isConnected) {
    const port = select.value;
    if (!port || port === "No Ports Found") return;
    
    try {
        await invoke("toggle_connection", { portName: port, connect: true });
        isConnected = true; 
        const btn = document.getElementById("conn-btn");
        if(btn) { btn.innerText = "Disconnect"; btn.className = "btn-red"; }
        if(select) select.disabled = true;
        setUiStatus("Connected to " + port, "#10b981");
    } catch (error) {
        setUiStatus(error, "#ef4444");
    }
  } else {
    try {
        await invoke("toggle_connection", { portName: "", connect: false });
        isConnected = false;
        const btn = document.getElementById("conn-btn");
        if(btn) { btn.innerText = "Connect"; btn.className = "btn-blue"; }
        if(select) select.disabled = false;
        setUiStatus("Disconnected", "#888");
    } catch(e) {}
  }
}

let pomodoroActive = false;

async function togglePomodoro() {
  const btn = document.getElementById("pomo-btn");
  try {
    const nowActive = await invoke("toggle_pomodoro");
    pomodoroActive = nowActive;
    if (btn) {
      btn.innerText = nowActive ? "Stop Pomodoro" : "Start Pomodoro";
      btn.className = nowActive ? "btn-pomo btn-red" : "btn-pomo btn-blue";
    }
  } catch (e) {
    if (btn) setUiStatus(String(e), "#ef4444");
  }
}

window.addEventListener("DOMContentLoaded", () => {
  const btn = document.getElementById("conn-btn");
  if(btn) btn.addEventListener("click", toggleConnection);
  const pomoBtn = document.getElementById("pomo-btn");
  if(pomoBtn) pomoBtn.addEventListener("click", togglePomodoro);
  initAutostart();
  loadPorts();
  setInterval(loadPorts, 2000);
  setInterval(updateStats, 1000);

  listen("pomodoro-changed", (event) => {
    pomodoroActive = event.payload;
    const pb = document.getElementById("pomo-btn");
    if (pb) {
      pb.innerText = pomodoroActive ? "Stop Pomodoro" : "Start Pomodoro";
      pb.className = pomodoroActive ? "btn-pomo btn-red" : "btn-pomo btn-blue";
    }
  });
});