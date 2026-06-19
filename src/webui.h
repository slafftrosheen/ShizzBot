// ============================================================
// ShizzBot - WebUI v2: SPY-KIDS CONTROL CENTER
// WebSocket-driven, tabbed, mobile-first, self-balance support
// ============================================================
#pragma once

const char WEBUI_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no">
<title>ShizzBot Control Center</title>
<link href="https://fonts.googleapis.com/css2?family=Orbitron:wght@400;700;900&family=Share+Tech+Mono&display=swap" rel="stylesheet">
<style>
:root{
  --bg:#060b18;--panel:rgba(8,16,36,0.85);
  --cyan:#00f0ff;--magenta:#ff00aa;--lime:#00ff88;
  --orange:#ff8800;--red:#ff2244;--txt:#c8d4e0;--dim:#3a4a60;
  --glow-cyan:0 0 8px rgba(0,240,255,.5);
  --glow-mag:0 0 8px rgba(255,0,170,.5);
}
*,*::before,*::after{margin:0;padding:0;box-sizing:border-box}
html,body{height:100%;overflow:hidden;background:var(--bg);color:var(--txt);
  font-family:'Share Tech Mono',monospace;-webkit-tap-highlight-color:transparent;
  touch-action:manipulation}
body{display:flex;flex-direction:column}

/* ===== SCANLINE OVERLAY ===== */
body::after{content:'';position:fixed;inset:0;pointer-events:none;z-index:9999;
  background:repeating-linear-gradient(0deg,transparent,transparent 2px,rgba(0,0,0,.03) 2px,rgba(0,0,0,.03) 4px);
}

/* ===== HUD HEADER ===== */
.hdr{display:flex;align-items:center;padding:6px 12px;gap:8px;
  background:linear-gradient(180deg,rgba(0,240,255,.06),transparent);
  border-bottom:1px solid rgba(0,240,255,.15)}
.logo{font-family:'Orbitron',sans-serif;font-weight:900;font-size:18px;
  background:linear-gradient(90deg,var(--cyan),var(--magenta));
  -webkit-background-clip:text;-webkit-text-fill-color:transparent;
  animation:glitch 4s infinite;letter-spacing:2px;flex-shrink:0}
@keyframes glitch{
  0%,93%,95%,97%,100%{opacity:1;transform:none}
  94%{opacity:.8;transform:translateX(-2px)}
  96%{opacity:.9;transform:translateX(1px)}
}
.hdr-spacer{flex:1}
.badge{font-size:10px;padding:3px 10px;border-radius:10px;font-weight:700;letter-spacing:1px;text-transform:uppercase}
.badge-mode{background:rgba(0,255,136,.1);border:1px solid rgba(0,255,136,.4);color:var(--lime)}
.badge-batt{background:rgba(255,136,0,.1);border:1px solid rgba(255,136,0,.4);color:var(--orange)}
.badge-conn{display:flex;align-items:center;gap:4px;background:rgba(0,240,255,.08);border:1px solid rgba(0,240,255,.3);color:var(--cyan)}
.dot{width:6px;height:6px;border-radius:50%;background:var(--lime);animation:pulse 1.5s infinite}
.dot.off{background:var(--red);animation:none}
@keyframes pulse{0%,100%{opacity:1}50%{opacity:.3}}
.latency{font-size:10px;color:var(--dim)}

/* ===== TAB BAR ===== */
.tabs{display:flex;border-bottom:1px solid rgba(0,240,255,.1);background:var(--panel)}
.tab{flex:1;padding:10px 0;text-align:center;font-size:12px;letter-spacing:1.5px;
  text-transform:uppercase;color:var(--dim);cursor:pointer;border:none;background:none;
  font-family:'Share Tech Mono',monospace;transition:color .2s;position:relative}
.tab.active{color:var(--cyan)}
.tab.active::after{content:'';position:absolute;bottom:0;left:20%;right:20%;height:2px;
  background:var(--cyan);box-shadow:var(--glow-cyan);border-radius:1px}
.tab:active{opacity:.7}

/* ===== CONTENT ===== */
.content{flex:1;overflow-y:auto;overflow-x:hidden;padding:10px;display:flex;flex-direction:column;gap:10px}
.page{display:none;flex-direction:column;gap:10px}
.page.active{display:flex}

/* ===== PANELS ===== */
.pnl{position:relative;background:var(--panel);border:1px solid rgba(0,240,255,.1);
  border-radius:4px;padding:12px;backdrop-filter:blur(6px)}
.pnl::before,.pnl::after,
.pnl .corner-bl,.pnl .corner-br{
  content:'';position:absolute;width:14px;height:14px;pointer-events:none}
.pnl::before{top:-1px;left:-1px;border-top:2px solid var(--cyan);border-left:2px solid var(--cyan)}
.pnl::after{top:-1px;right:-1px;border-top:2px solid var(--cyan);border-right:2px solid var(--cyan)}
.pnl .corner-bl{bottom:-1px;left:-1px;border-bottom:2px solid var(--cyan);border-left:2px solid var(--cyan)}
.pnl .corner-br{bottom:-1px;right:-1px;border-bottom:2px solid var(--cyan);border-right:2px solid var(--cyan)}
.pnl-title{font-family:'Orbitron',sans-serif;font-size:10px;letter-spacing:2px;
  text-transform:uppercase;color:var(--cyan);margin-bottom:10px;
  padding-bottom:6px;border-bottom:1px solid rgba(0,240,255,.1)}

/* ===== JOYSTICK ===== */
.joy-wrap{display:flex;flex-direction:column;align-items:center}
.joy-outer{position:relative;width:min(240px,65vw);height:min(240px,65vw);
  border-radius:50%;background:rgba(0,240,255,.03);
  border:2px solid rgba(0,240,255,.12);touch-action:none;cursor:grab}
.joy-outer::before{content:'';position:absolute;inset:25%;border-radius:50%;
  border:1px dashed rgba(0,240,255,.08)}
/* crosshair */
.joy-outer::after{content:'';position:absolute;top:50%;left:10%;right:10%;height:1px;
  background:linear-gradient(90deg,transparent,rgba(0,240,255,.1) 30%,rgba(0,240,255,.1) 70%,transparent)}
.joy-cross-v{position:absolute;left:50%;top:10%;bottom:10%;width:1px;
  background:linear-gradient(180deg,transparent,rgba(0,240,255,.1) 30%,rgba(0,240,255,.1) 70%,transparent);pointer-events:none}
/* direction arrows */
.dir-arrow{position:absolute;font-size:16px;opacity:.15;transition:opacity .15s;pointer-events:none}
.dir-arrow.n{top:6px;left:50%;transform:translateX(-50%)}
.dir-arrow.s{bottom:6px;left:50%;transform:translateX(-50%)}
.dir-arrow.e{right:6px;top:50%;transform:translateY(-50%)}
.dir-arrow.w{left:6px;top:50%;transform:translateY(-50%)}
.dir-arrow.lit{opacity:.9;text-shadow:var(--glow-cyan)}
.joy-thumb{position:absolute;width:56px;height:56px;border-radius:50%;
  background:radial-gradient(circle at 40% 35%,rgba(0,240,255,.6),rgba(255,0,170,.5));
  box-shadow:0 0 20px rgba(0,240,255,.35);
  top:50%;left:50%;transform:translate(-50%,-50%);pointer-events:none;
  transition:box-shadow .15s}
.joy-thumb.active{box-shadow:0 0 30px rgba(255,0,170,.5)}

/* speed bars */
.speed-row{display:flex;gap:12px;margin-top:10px;width:100%}
.speed-box{flex:1;text-align:center;padding:8px;border-radius:4px;
  background:rgba(0,240,255,.04);border:1px solid rgba(0,240,255,.08)}
.speed-lbl{font-size:9px;letter-spacing:1px;color:var(--dim);text-transform:uppercase}
.speed-num{font-family:'Orbitron',sans-serif;font-size:24px;font-weight:700;margin-top:2px}
.speed-num.l-color{color:var(--cyan)}
.speed-num.r-color{color:var(--magenta)}

/* ===== QUICK ACTIONS ===== */
.actions{display:grid;grid-template-columns:1fr 1fr 1fr;gap:8px}
.act{padding:14px 8px;border-radius:4px;border:1px solid;font-family:'Share Tech Mono',monospace;
  font-size:12px;font-weight:700;letter-spacing:1px;text-transform:uppercase;
  cursor:pointer;transition:all .15s;background:transparent;display:flex;
  align-items:center;justify-content:center;gap:6px}
.act:active{transform:scale(.95)}
.act-fwd{color:var(--lime);border-color:rgba(0,255,136,.3)}
.act-fwd:hover{background:rgba(0,255,136,.1)}
.act-rev{color:var(--orange);border-color:rgba(255,136,0,.3)}
.act-rev:hover{background:rgba(255,136,0,.1)}
.act-spin{color:var(--magenta);border-color:rgba(255,0,170,.3)}
.act-spin:hover{background:rgba(255,0,170,.1)}
.act-horn{color:var(--cyan);border-color:rgba(0,240,255,.3)}
.act-horn:hover{background:rgba(0,240,255,.1)}

/* ===== ESTOP ===== */
.estop-bar{padding:8px 12px;display:flex;align-items:center;gap:10px;
  border-top:1px solid rgba(255,34,68,.2);background:rgba(255,34,68,.04)}
.estop{flex:1;padding:14px;border-radius:4px;border:2px solid var(--red);
  background:rgba(255,34,68,.1);color:var(--red);font-family:'Orbitron',sans-serif;
  font-size:14px;font-weight:900;letter-spacing:3px;cursor:pointer;
  transition:all .15s;text-transform:uppercase}
.estop:hover{background:rgba(255,34,68,.2)}
.estop:active{transform:scale(.97);background:rgba(255,34,68,.3)}

/* ===== TELEMETRY GAUGES ===== */
.tgrid{display:grid;grid-template-columns:1fr 1fr;gap:8px}
.tcard{text-align:center;padding:12px 8px;border-radius:4px;
  background:rgba(0,240,255,.03);border:1px solid rgba(0,240,255,.06)}
.tcard-icon{font-size:20px;margin-bottom:4px}
.tcard-lbl{font-size:9px;letter-spacing:1px;color:var(--dim);text-transform:uppercase}
.tcard-val{font-family:'Orbitron',sans-serif;font-size:20px;font-weight:700;color:var(--cyan);margin-top:2px}
.tcard-unit{font-size:10px;color:var(--dim)}
.tcard.warn .tcard-val{color:var(--orange)}
.tcard.danger .tcard-val{color:var(--red)}

/* IMU horizon */
.horizon-wrap{position:relative;width:100%;max-width:200px;margin:0 auto;aspect-ratio:1}
.horizon-ring{width:100%;height:100%;border-radius:50%;border:2px solid rgba(0,240,255,.15);
  position:relative;overflow:hidden;background:rgba(0,240,255,.02)}
.horizon-sky{position:absolute;inset:0;background:linear-gradient(180deg,
  rgba(0,100,200,.15) 0%,rgba(0,240,255,.05) 50%,rgba(100,50,0,.15) 100%);
  transition:transform .1s}
.horizon-line{position:absolute;left:10%;right:10%;top:50%;height:2px;
  background:var(--cyan);box-shadow:var(--glow-cyan);transform:translateY(-50%)}
.horizon-dot{position:absolute;width:8px;height:8px;border-radius:50%;background:var(--magenta);
  top:50%;left:50%;transform:translate(-50%,-50%);box-shadow:var(--glow-mag);z-index:2}
.horizon-lbl{position:absolute;bottom:6px;width:100%;text-align:center;
  font-size:10px;color:var(--dim)}

/* ===== CONFIG PAGE ===== */
.cfg-section{margin-bottom:6px}
.cfg-label{display:flex;justify-content:space-between;align-items:center;
  font-size:11px;color:var(--dim);text-transform:uppercase;letter-spacing:1px;margin-bottom:6px}
.cfg-val{color:var(--cyan);font-family:'Orbitron',sans-serif;font-weight:700}

/* toggle switch */
.toggle{position:relative;width:48px;height:26px;cursor:pointer;flex-shrink:0}
.toggle input{display:none}
.toggle-track{position:absolute;inset:0;border-radius:13px;background:rgba(255,255,255,.08);
  border:1px solid rgba(255,255,255,.1);transition:all .3s}
.toggle input:checked+.toggle-track{background:rgba(0,240,255,.2);border-color:rgba(0,240,255,.5)}
.toggle-thumb{position:absolute;top:3px;left:3px;width:20px;height:20px;border-radius:50%;
  background:var(--dim);transition:all .3s;box-shadow:0 0 4px rgba(0,0,0,.3)}
.toggle input:checked~.toggle-thumb{left:25px;background:var(--cyan);box-shadow:var(--glow-cyan)}

.cfg-row{display:flex;align-items:center;justify-content:space-between;
  padding:10px 0;border-bottom:1px solid rgba(255,255,255,.04)}
.cfg-row-label{font-size:12px;color:var(--txt)}

/* slider */
input[type=range]{-webkit-appearance:none;width:100%;height:6px;border-radius:3px;
  background:rgba(0,240,255,.1);outline:none;margin:8px 0}
input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:20px;height:20px;
  border-radius:50%;background:var(--cyan);box-shadow:var(--glow-cyan);cursor:pointer}

/* PID group */
.pid-grid{display:grid;grid-template-columns:1fr 1fr 1fr;gap:8px}
.pid-item{display:flex;flex-direction:column;align-items:center}
.pid-item label{font-size:9px;color:var(--dim);letter-spacing:1px;margin-bottom:4px}
.pid-item input[type=number]{width:70px;text-align:center;padding:6px;border-radius:4px;
  border:1px solid rgba(0,240,255,.15);background:rgba(0,240,255,.04);color:var(--cyan);
  font-family:'Share Tech Mono',monospace;font-size:14px}
.pid-item input[type=number]:focus{outline:none;border-color:var(--cyan)}

/* WiFi info */
.wifi-info{font-size:11px;color:var(--dim);line-height:1.8}
.wifi-info b{color:var(--cyan)}

/* ===== RESPONSIVE ===== */
@media(max-width:380px){
  .joy-outer{width:min(200px,60vw);height:min(200px,60vw)}
  .speed-num{font-size:20px}
  .tcard-val{font-size:16px}
  .actions{grid-template-columns:1fr 1fr}
  .actions-4{grid-template-columns:1fr 1fr}
}
</style>
</head>
<body>

<!-- HEADER -->
<div class="hdr">
  <div class="logo">SHIZZBOT</div>
  <div class="hdr-spacer"></div>
  <div class="badge badge-batt" id="batt-badge">🔋 <span id="batt-txt">--%</span></div>
  <div class="badge badge-mode" id="mode-badge">SKID STEER</div>
  <div class="badge badge-conn"><div class="dot" id="conn-dot"></div><span id="conn-txt">---</span></div>
  <div class="latency" id="latency">--ms</div>
</div>

<!-- TABS -->
<div class="tabs">
  <button class="tab active" data-tab="drive">🎮 DRIVE</button>
  <button class="tab" data-tab="emotes">🎭 EMOTES</button>
  <button class="tab" data-tab="sensors">📡 SENSORS</button>
  <button class="tab" data-tab="config">⚙️ CONFIG</button>
</div>

<!-- ============ DRIVE TAB ============ -->
<div class="content">
<div class="page active" id="pg-drive">

  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">Drive Control</div>
    <div class="joy-wrap">
      <div class="joy-outer" id="joy">
        <div class="joy-cross-v"></div>
        <div class="dir-arrow n" id="da-n">▲</div>
        <div class="dir-arrow s" id="da-s">▼</div>
        <div class="dir-arrow e" id="da-e">▶</div>
        <div class="dir-arrow w" id="da-w">◀</div>
        <div class="joy-thumb" id="joy-thumb"></div>
      </div>
      <div class="speed-row">
        <div class="speed-box"><div class="speed-lbl">Left Motor</div>
          <div class="speed-num l-color" id="sp-l">0</div></div>
        <div class="speed-box"><div class="speed-lbl">Right Motor</div>
          <div class="speed-num r-color" id="sp-r">0</div></div>
      </div>
    </div>
  </div>

  <div class="actions actions-4" style="grid-template-columns:1fr 1fr 1fr 1fr">
    <button class="act act-fwd" onclick="qAct('fwd')">▲ FWD</button>
    <button class="act act-spin" onclick="qAct('spin')">↻ SPIN</button>
    <button class="act act-horn" onclick="qAct('horn')">🔊 HORN</button>
    <button class="act act-rev" onclick="qAct('rev')">▼ REV</button>
  </div>

  <div class="pnl" style="padding:6px 12px">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="cfg-label"><span>Max Speed</span><span class="cfg-val" id="maxsp-v">50%</span></div>
    <input type="range" id="maxsp" min="10" max="100" value="50">
  </div>

</div>

<!-- ============ EMOTES & SOUNDS TAB ============ -->
<div class="page" id="pg-emotes">
  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">Soundboard</div>
    <div class="actions actions-4" style="grid-template-columns:1fr 1fr; gap:10px; margin-bottom:15px">
      <button class="act act-horn" onclick="sendSnd('horn')">🔊 HORN</button>
      <button class="act act-horn" onclick="sendSnd('chirp')">🤖 CHIRP</button>
      <button class="act act-spin" onclick="sendSnd('scan')">📡 SCAN</button>
      <button class="act act-rev" onclick="sendSnd('err')">❌ ERROR</button>
    </div>
  </div>

  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">Face Override</div>
    <div class="actions actions-4" style="grid-template-columns:1fr 1fr; gap:10px">
      <button class="act act-fwd" onclick="sendEmo('happy')">😄 HAPPY</button>
      <button class="act act-rev" onclick="sendEmo('angry')">😠 ANGRY</button>
      <button class="act act-spin" onclick="sendEmo('surprised')">😲 SURPRISE</button>
      <button class="act act-horn" onclick="sendEmo('dizzy')">😵‍💫 DIZZY</button>
      <button class="act" style="grid-column: span 2; border-color:var(--dim); color:var(--txt)" onclick="sendEmo('idle')">😐 NORMAL</button>
    </div>
  </div>
</div>

<!-- ============ SENSORS TAB ============ -->
<div class="page" id="pg-sensors">

  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">Motor Telemetry</div>
    <div class="tgrid">
      <div class="tcard" id="tc-volt"><div class="tcard-icon">⚡</div>
        <div class="tcard-lbl">Voltage</div>
        <div class="tcard-val" id="tv-v">--</div><div class="tcard-unit">V</div></div>
      <div class="tcard" id="tc-temp"><div class="tcard-icon">🌡️</div>
        <div class="tcard-lbl">Motor Temp</div>
        <div class="tcard-val" id="tv-t">--</div><div class="tcard-unit">°C</div></div>
      <div class="tcard"><div class="tcard-icon">🔄</div>
        <div class="tcard-lbl">RPM Left</div>
        <div class="tcard-val" id="tv-sl">0</div><div class="tcard-unit">RPM</div></div>
      <div class="tcard"><div class="tcard-icon">🔄</div>
        <div class="tcard-lbl">RPM Right</div>
        <div class="tcard-val" id="tv-sr">0</div><div class="tcard-unit">RPM</div></div>
      <div class="tcard"><div class="tcard-icon">🔌</div>
        <div class="tcard-lbl">Current L</div>
        <div class="tcard-val" id="tv-cl">--</div><div class="tcard-unit">mA</div></div>
      <div class="tcard"><div class="tcard-icon">🔌</div>
        <div class="tcard-lbl">Current R</div>
        <div class="tcard-val" id="tv-cr">--</div><div class="tcard-unit">mA</div></div>
    </div>
  </div>

  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">IMU — Attitude</div>
    <div class="horizon-wrap">
      <div class="horizon-ring">
        <div class="horizon-sky" id="hz-sky"></div>
        <div class="horizon-line"></div>
        <div class="horizon-dot" id="hz-dot"></div>
      </div>
      <div class="horizon-lbl" id="hz-lbl">P: 0.0°  R: 0.0°</div>
    </div>
  </div>

</div>

<!-- ============ CONFIG TAB ============ -->
<div class="page" id="pg-config">

  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">Drive Mode</div>
    <div class="cfg-row">
      <span class="cfg-row-label">Self-Balancing Mode</span>
      <label class="toggle"><input type="checkbox" id="cfg-balance" onchange="sendCfg()">
        <div class="toggle-track"></div><div class="toggle-thumb"></div></label>
    </div>
    <div class="cfg-row">
      <span class="cfg-row-label">Invert Left Motor</span>
      <label class="toggle"><input type="checkbox" id="cfg-invL" onchange="sendCfg()">
        <div class="toggle-track"></div><div class="toggle-thumb"></div></label>
    </div>
    <div class="cfg-row">
      <span class="cfg-row-label">Invert Right Motor</span>
      <label class="toggle"><input type="checkbox" id="cfg-invR" onchange="sendCfg()">
        <div class="toggle-track"></div><div class="toggle-thumb"></div></label>
    </div>
  </div>

  <div class="pnl" id="pid-panel">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">Balance PID Tuning</div>
    <div class="pid-grid">
      <div class="pid-item"><label>KP</label>
        <input type="number" id="pid-kp" value="12" step="0.5" onchange="sendPid()"></div>
      <div class="pid-item"><label>KI</label>
        <input type="number" id="pid-ki" value="0.4" step="0.1" onchange="sendPid()"></div>
      <div class="pid-item"><label>KD</label>
        <input type="number" id="pid-kd" value="0.6" step="0.1" onchange="sendPid()"></div>
    </div>
  </div>

  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">Network Info</div>
    <div class="wifi-info" id="wifi-info">Connecting...</div>
  </div>

  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">System</div>
    <button class="act act-fwd" onclick="window.location.href='/update'" style="width:100%;margin-top:5px">⬆️ FIRMWARE UPDATE</button>
  </div>

</div>
</div><!-- /content -->

<!-- ESTOP FOOTER -->
<div class="estop-bar">
  <button class="estop" id="btn-estop">🛑 EMERGENCY STOP</button>
</div>

<script>
// ===== STATE =====
let ws=null,wsOk=false,maxPct=50,joyActive=false;
let sL=0,sR=0,lastPing=0,lat=0;

// ===== TABS =====
document.querySelectorAll('.tab').forEach(t=>{
  t.addEventListener('click',()=>{
    document.querySelectorAll('.tab').forEach(x=>x.classList.remove('active'));
    document.querySelectorAll('.page').forEach(x=>x.classList.remove('active'));
    t.classList.add('active');
    document.getElementById('pg-'+t.dataset.tab).classList.add('active');
  });
});

// ===== WEBSOCKET =====
function wsConnect(){
  const h=location.host||'192.168.4.1';
  ws=new WebSocket('ws://'+h+'/ws');
  ws.onopen=()=>{
    wsOk=true;
    document.getElementById('conn-dot').classList.remove('off');
    document.getElementById('conn-txt').textContent='ONLINE';
  };
  ws.onclose=()=>{wsOk=false;
    document.getElementById('conn-dot').classList.add('off');
    document.getElementById('conn-txt').textContent='OFFLINE';
    setTimeout(wsConnect,1500);
  };
  ws.onerror=()=>{ws.close()};
  ws.onmessage=(e)=>{
    lat=Date.now()-lastPing;
    document.getElementById('latency').textContent=lat+'ms';
    try{
      const d=JSON.parse(e.data);
      // Voltage: comes as mV int → display as V with 1 decimal
      const volts=(d.v/1000).toFixed(1);
      document.getElementById('tv-v').textContent=volts;
      const vc=document.getElementById('tc-volt');
      vc.classList.toggle('warn',d.v<11000&&d.v>0);
      vc.classList.toggle('danger',d.v<10000&&d.v>0);

      // Temp: comes as °C*10 int → display as °C with 1 decimal
      const temp=(d.t/10).toFixed(1);
      document.getElementById('tv-t').textContent=temp;
      const tc=document.getElementById('tc-temp');
      tc.classList.toggle('warn',d.t>500);
      tc.classList.toggle('danger',d.t>700);

      // Speed: already in RPM
      document.getElementById('tv-sl').textContent=d.sl;
      document.getElementById('tv-sr').textContent=d.sr;

      // Current: already in mA
      document.getElementById('tv-cl').textContent=d.cl;
      document.getElementById('tv-cr').textContent=d.cr;

      // IMU
      updateHorizon(d.p||0,d.r||0);

      // Mode
      const isBal=d.m==='balance';
      document.getElementById('mode-badge').textContent=isBal?'SELF-BALANCE':'SKID STEER';
      document.getElementById('mode-badge').style.borderColor=isBal?'rgba(255,0,170,.5)':'rgba(0,255,136,.4)';
      document.getElementById('mode-badge').style.color=isBal?'var(--magenta)':'var(--lime)';

      // Battery
      if (d.batt !== undefined) {
        document.getElementById('batt-txt').textContent = d.batt + '%';
        const bb = document.getElementById('batt-badge');
        if (d.batt < 20) {
          bb.style.borderColor = 'rgba(255,34,68,.8)';
          bb.style.color = 'var(--red)';
        } else {
          bb.style.borderColor = 'rgba(255,136,0,.4)';
          bb.style.color = 'var(--orange)';
        }
      }

      // WiFi info
      if(d.ip) document.getElementById('wifi-info').innerHTML=
        'SSID: <b>'+d.ssid+'</b><br>IP: <b>'+d.ip+'</b><br>RSSI: <b>'+d.rssi+' dBm</b><br>Clients: <b>'+d.wc+'</b>';

      // One-time config sync on connection
      if(d.kp !== undefined && !window.cfgSynced) {
        window.cfgSynced = true;
        document.getElementById('cfg-balance').checked = (d.m === 'balance');
        document.getElementById('cfg-invL').checked = (d.invL === 1);
        document.getElementById('cfg-invR').checked = (d.invR === 1);
        document.getElementById('pid-kp').value = d.kp;
        document.getElementById('pid-ki').value = d.ki;
        document.getElementById('pid-kd').value = d.kd;
        document.getElementById('maxsp').value = d.maxSp;
        document.getElementById('maxsp-v').textContent = d.maxSp + '%';
        maxPct = d.maxSp;
      }
    }catch(ex){}
  };
}
wsConnect();

function wsSend(obj){
  if(ws&&ws.readyState===1){
    lastPing=Date.now();
    ws.send(JSON.stringify(obj));
  }
}

// ===== JOYSTICK =====
const joy=document.getElementById('joy');
const thumb=document.getElementById('joy-thumb');
const arrows={n:document.getElementById('da-n'),s:document.getElementById('da-s'),
              e:document.getElementById('da-e'),w:document.getElementById('da-w')};

function joyMove(cx,cy){
  const r=joy.getBoundingClientRect();
  const hw=r.width/2,hh=r.height/2;
  const tR=28;
  let dx=cx-(r.left+hw),dy=cy-(r.top+hh);
  const maxR=hw-tR;
  const dist=Math.sqrt(dx*dx+dy*dy);
  if(dist>maxR){dx=dx/dist*maxR;dy=dy/dist*maxR;}
  thumb.style.left=`calc(50% + ${dx}px)`;
  thumb.style.top=`calc(50% + ${dy}px)`;

  const nx=dx/maxR,ny=-dy/maxR; // normalized -1..1, Y inverted
  
  // True proportional tank mix
  // Preserves curves and avoids clipping in diagonal directions
  let v = (1 - Math.abs(nx)) * ny + ny;
  let w = (1 - Math.abs(ny)) * nx + nx;
  sL = Math.round(((v + w) / 2) * maxPct);
  sR = Math.round(((v - w) / 2) * maxPct);

  document.getElementById('sp-l').textContent=sL;
  document.getElementById('sp-r').textContent=sR;

  // Direction arrows
  arrows.n.classList.toggle('lit',ny>0.25);
  arrows.s.classList.toggle('lit',ny<-0.25);
  arrows.e.classList.toggle('lit',nx>0.25);
  arrows.w.classList.toggle('lit',nx<-0.25);
}

function joyReset(){
  thumb.style.left='50%';thumb.style.top='50%';
  thumb.classList.remove('active');
  sL=0;sR=0;
  document.getElementById('sp-l').textContent='0';
  document.getElementById('sp-r').textContent='0';
  Object.values(arrows).forEach(a=>a.classList.remove('lit'));
  wsSend({c:'m',l:0,r:0});
}

joy.addEventListener('pointerdown',e=>{joyActive=true;thumb.classList.add('active');joy.setPointerCapture(e.pointerId);joyMove(e.clientX,e.clientY);});
joy.addEventListener('pointermove',e=>{if(joyActive)joyMove(e.clientX,e.clientY);});
joy.addEventListener('pointerup',()=>{joyActive=false;joyReset();});
joy.addEventListener('pointercancel',()=>{joyActive=false;joyReset();});

// Send motor commands at max 20Hz, only when changed, plus a heartbeat
let lastSentL = null, lastSentR = null;
setInterval(()=>{
  if(joyActive){
    if(sL !== lastSentL || sR !== lastSentR) {
      wsSend({c:'m',l:sL,r:sR});
      lastSentL = sL; lastSentR = sR;
    }
  } else {
    if(Date.now() - lastPing > 500) {
      wsSend({c:'m',l:0,r:0}); // Heartbeat
    }
  }
}, 50);

// ===== MAX SPEED =====
const slider=document.getElementById('maxsp');
slider.addEventListener('change',()=>{
  maxPct=parseInt(slider.value);
  document.getElementById('maxsp-v').textContent=maxPct+'%';
  sendCfg(); // Send config when slider is released
});
slider.addEventListener('input',()=>{
  document.getElementById('maxsp-v').textContent=slider.value+'%';
});

// ===== QUICK ACTIONS =====
function qAct(a){
  if(a==='fwd'){sL=maxPct;sR=maxPct;}
  else if(a==='rev'){sL=-maxPct;sR=-maxPct;}
  else if(a==='spin'){sL=maxPct;sR=-maxPct;}
  else if(a==='horn'){sendSnd('horn'); return;}
  
  document.getElementById('sp-l').textContent=sL;
  document.getElementById('sp-r').textContent=sR;
  wsSend({c:'m',l:sL,r:sR});
  setTimeout(()=>{sL=0;sR=0;
    document.getElementById('sp-l').textContent='0';
    document.getElementById('sp-r').textContent='0';
    wsSend({c:'m',l:0,r:0});
  },1200);
}

// ===== EMOTES & SOUNDS =====
function sendSnd(snd) {
  wsSend({c:'snd', s:snd});
}

function sendEmo(emo) {
  wsSend({c:'emo', e:emo});
}

// ===== ESTOP =====
document.getElementById('btn-estop').addEventListener('click',()=>{
  wsSend({c:'stop'});
  sL=0;sR=0;
  document.getElementById('sp-l').textContent='0';
  document.getElementById('sp-r').textContent='0';
  if(navigator.vibrate)navigator.vibrate(200);
});

// ===== CONFIG =====
function sendCfg(){
  wsSend({c:'cfg',
    bal:document.getElementById('cfg-balance').checked?1:0,
    invL:document.getElementById('cfg-invL').checked?1:0,
    invR:document.getElementById('cfg-invR').checked?1:0,
    maxSp:parseInt(document.getElementById('maxsp').value)
  });
}
function sendPid(){
  wsSend({c:'pid',
    kp:parseFloat(document.getElementById('pid-kp').value),
    ki:parseFloat(document.getElementById('pid-ki').value),
    kd:parseFloat(document.getElementById('pid-kd').value)
  });
}

// ===== IMU HORIZON =====
function updateHorizon(pitch,roll){
  const sky=document.getElementById('hz-sky');
  const dot=document.getElementById('hz-dot');
  const lbl=document.getElementById('hz-lbl');
  // Shift sky based on pitch (clamped to ±45°)
  const pClamped=clamp(pitch,-45,45);
  const rClamped=clamp(roll,-45,45);
  sky.style.transform=`translateY(${pClamped*1.5}%) rotate(${rClamped}deg)`;
  // Dot shows roll+pitch offset
  const dotX=clamp(rClamped/45*40,-40,40);
  const dotY=clamp(pClamped/45*40,-40,40);
  dot.style.transform=`translate(calc(-50% + ${dotX}px),calc(-50% + ${dotY}px))`;
  lbl.textContent=`P: ${(pitch/10).toFixed(1)}°  R: ${(roll/10).toFixed(1)}°`;
}

function clamp(v,lo,hi){return Math.max(lo,Math.min(hi,v));}
</script>
</body>
</html>
)rawliteral";
