// ============================================================
// ShizzBot Swarm - WebUI v3: SPY-KIDS SWARM COMMAND
// Single motor + steering, arm control, robot identity,
// gamification, heading compass. No self-balance.
// ============================================================
#pragma once

const char WEBUI_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no">
<title>ShizzBot Swarm</title>
<link href="https://fonts.googleapis.com/css2?family=Orbitron:wght@400;700;900&family=Share+Tech+Mono&display=swap" rel="stylesheet">
<style>
:root{
  --bg:#060b18;--panel:rgba(8,16,36,0.85);
  --cyan:#00f0ff;--magenta:#ff00aa;--lime:#00ff88;
  --orange:#ff8800;--red:#ff2244;--txt:#c8d4e0;--dim:#3a4a60;
  --accent:var(--cyan);
  --glow-accent:0 0 8px rgba(0,240,255,.5);
}
*,*::before,*::after{margin:0;padding:0;box-sizing:border-box}
html,body{height:100%;overflow:hidden;background:var(--bg);color:var(--txt);
  font-family:'Share Tech Mono',monospace;-webkit-tap-highlight-color:transparent;
  touch-action:manipulation}
body{display:flex;flex-direction:column}

/* SCANLINES */
body::after{content:'';position:fixed;inset:0;pointer-events:none;z-index:9999;
  background:repeating-linear-gradient(0deg,transparent,transparent 2px,rgba(0,0,0,.03) 2px,rgba(0,0,0,.03) 4px)}

/* HUD HEADER */
.hdr{display:flex;align-items:center;padding:6px 12px;gap:8px;
  background:linear-gradient(180deg,rgba(0,240,255,.06),transparent);
  border-bottom:1px solid rgba(0,240,255,.15)}
.logo{font-family:'Orbitron',sans-serif;font-weight:900;font-size:16px;
  color:var(--accent);letter-spacing:2px;flex-shrink:0;
  text-shadow:0 0 8px currentColor}
.hdr-spacer{flex:1}
.badge{font-size:10px;padding:3px 10px;border-radius:10px;font-weight:700;letter-spacing:1px;text-transform:uppercase}
.badge-batt{background:rgba(255,136,0,.1);border:1px solid rgba(255,136,0,.4);color:var(--orange)}
.badge-score{background:rgba(0,255,136,.1);border:1px solid rgba(0,255,136,.4);color:var(--lime)}
.badge-conn{display:flex;align-items:center;gap:4px;background:rgba(0,240,255,.08);border:1px solid rgba(0,240,255,.3);color:var(--accent)}
.dot{width:6px;height:6px;border-radius:50%;background:var(--lime);animation:pulse 1.5s infinite}
.dot.off{background:var(--red);animation:none}
@keyframes pulse{0%,100%{opacity:1}50%{opacity:.3}}
.latency{font-size:10px;color:var(--dim)}

/* TABS */
.tabs{display:flex;border-bottom:1px solid rgba(0,240,255,.1);background:var(--panel)}
.tab{flex:1;padding:10px 0;text-align:center;font-size:11px;letter-spacing:1.5px;
  text-transform:uppercase;color:var(--dim);cursor:pointer;border:none;background:none;
  font-family:'Share Tech Mono',monospace;transition:color .2s;position:relative}
.tab.active{color:var(--accent)}
.tab.active::after{content:'';position:absolute;bottom:0;left:20%;right:20%;height:2px;
  background:var(--accent);box-shadow:var(--glow-accent);border-radius:1px}
.tab:active{opacity:.7}

/* CONTENT */
.content{flex:1;overflow-y:auto;overflow-x:hidden;padding:10px;display:flex;flex-direction:column;gap:10px}
.page{display:none;flex-direction:column;gap:10px}
.page.active{display:flex}

/* PANELS */
.pnl{position:relative;background:var(--panel);border:1px solid rgba(0,240,255,.1);
  border-radius:4px;padding:12px;backdrop-filter:blur(6px)}
.pnl::before,.pnl::after,.pnl .corner-bl,.pnl .corner-br{
  content:'';position:absolute;width:14px;height:14px;pointer-events:none}
.pnl::before{top:-1px;left:-1px;border-top:2px solid var(--accent);border-left:2px solid var(--accent)}
.pnl::after{top:-1px;right:-1px;border-top:2px solid var(--accent);border-right:2px solid var(--accent)}
.pnl .corner-bl{bottom:-1px;left:-1px;border-bottom:2px solid var(--accent);border-left:2px solid var(--accent)}
.pnl .corner-br{bottom:-1px;right:-1px;border-bottom:2px solid var(--accent);border-right:2px solid var(--accent)}
.pnl-title{font-family:'Orbitron',sans-serif;font-size:11px;color:var(--accent);
  text-transform:uppercase;letter-spacing:2px;margin-bottom:10px;
  text-shadow:0 0 6px currentColor}

/* JOYSTICK */
.joy-wrap{display:flex;flex-direction:column;align-items:center;gap:12px}
.joy-outer{position:relative;width:min(220px,65vw);aspect-ratio:1;border-radius:50%;
  background:radial-gradient(circle,rgba(0,240,255,.04),rgba(0,240,255,.01));
  border:2px solid rgba(0,240,255,.15);margin:0 auto;touch-action:none;user-select:none}
.joy-cross-v{position:absolute;left:50%;top:8%;bottom:8%;width:1px;
  background:linear-gradient(180deg,transparent,rgba(0,240,255,.15),transparent);transform:translateX(-50%)}
.joy-thumb{position:absolute;width:56px;height:56px;border-radius:50%;
  background:radial-gradient(circle at 35% 35%,rgba(0,240,255,.3),rgba(0,240,255,.08));
  border:2px solid rgba(0,240,255,.5);left:50%;top:50%;
  transform:translate(-50%,-50%);transition:box-shadow .2s;cursor:grab}
.joy-thumb.active{box-shadow:0 0 20px rgba(0,240,255,.6),inset 0 0 10px rgba(0,240,255,.2);cursor:grabbing}
.dir-arrow{position:absolute;font-size:14px;color:rgba(0,240,255,.2);transition:color .15s,text-shadow .15s}
.dir-arrow.n{top:4%;left:50%;transform:translateX(-50%)}
.dir-arrow.s{bottom:4%;left:50%;transform:translateX(-50%)}
.dir-arrow.e{right:4%;top:50%;transform:translateY(-50%)}
.dir-arrow.w{left:4%;top:50%;transform:translateY(-50%)}
.dir-arrow.lit{color:var(--accent);text-shadow:var(--glow-accent)}

/* GAUGES */
.gauge-row{display:flex;justify-content:space-around;align-items:center;padding:5px 0}
.gauge-box{position:relative;width:80px;height:80px}
.gauge-box svg{position:absolute;inset:0;transform:rotate(-90deg)}
.gauge-center{position:absolute;inset:0;display:flex;flex-direction:column;align-items:center;justify-content:center}
.gauge-val{font-family:'Orbitron',sans-serif;font-size:20px;font-weight:700;color:var(--accent)}
.gauge-lbl{font-size:8px;color:var(--dim);text-transform:uppercase;letter-spacing:1px}

/* COMPASS */
.compass-box{width:60px;height:60px;position:relative;margin:0 auto}
.compass-box svg{width:100%;height:100%}

/* ACTIONS */
.actions{display:grid;gap:8px}
.act{border:1px solid;border-radius:4px;padding:10px 6px;font-family:'Share Tech Mono',monospace;
  font-size:12px;cursor:pointer;text-transform:uppercase;letter-spacing:1px;
  background:transparent;transition:all .15s;touch-action:manipulation}
.act:active{transform:scale(.95)}
.act-fwd{border-color:var(--lime);color:var(--lime)}
.act-rev{border-color:var(--red);color:var(--red)}
.act-spin{border-color:var(--accent);color:var(--accent)}
.act-horn{border-color:var(--orange);color:var(--orange)}

/* ARM SLIDERS */
.arm-slider-group{display:flex;flex-direction:column;gap:12px}
.arm-slider{display:flex;align-items:center;gap:10px}
.arm-slider label{font-size:10px;color:var(--dim);text-transform:uppercase;letter-spacing:1px;width:50px;text-align:right}
.arm-slider input[type=range]{flex:1;-webkit-appearance:none;height:8px;border-radius:4px;
  background:rgba(0,240,255,.1);outline:none}
.arm-slider input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:24px;height:24px;
  border-radius:50%;background:var(--accent);box-shadow:var(--glow-accent);cursor:pointer}
.arm-slider .arm-val{font-family:'Orbitron',sans-serif;font-size:14px;color:var(--accent);width:35px;text-align:center}

/* SENSOR CARDS */
.tgrid{display:grid;grid-template-columns:repeat(3,1fr);gap:8px}
.tcard{text-align:center;padding:8px 4px;border-radius:4px;
  background:rgba(0,240,255,.03);border:1px solid rgba(0,240,255,.06)}
.tcard-icon{font-size:20px;margin-bottom:4px}
.tcard-lbl{font-size:9px;letter-spacing:1px;color:var(--dim);text-transform:uppercase}
.tcard-val{font-family:'Orbitron',sans-serif;font-size:18px;font-weight:700;color:var(--accent);margin-top:2px}
.tcard-unit{font-size:10px;color:var(--dim)}
.tcard.warn .tcard-val{color:var(--orange)}
.tcard.danger .tcard-val{color:var(--red)}

/* CONFIG */
.cfg-label{display:flex;justify-content:space-between;align-items:center;
  font-size:11px;color:var(--dim);text-transform:uppercase;letter-spacing:1px;margin-bottom:6px}
.cfg-val{color:var(--accent);font-family:'Orbitron',sans-serif;font-weight:700}

.toggle{position:relative;width:48px;height:26px;cursor:pointer;flex-shrink:0}
.toggle input{display:none}
.toggle-track{position:absolute;inset:0;border-radius:13px;background:rgba(255,255,255,.08);
  border:1px solid rgba(255,255,255,.1);transition:all .3s}
.toggle input:checked+.toggle-track{background:rgba(0,240,255,.2);border-color:rgba(0,240,255,.5)}
.toggle-thumb{position:absolute;top:3px;left:3px;width:20px;height:20px;border-radius:50%;
  background:var(--dim);transition:all .3s;box-shadow:0 0 4px rgba(0,0,0,.3)}
.toggle input:checked~.toggle-thumb{left:25px;background:var(--accent);box-shadow:var(--glow-accent)}

.cfg-row{display:flex;align-items:center;justify-content:space-between;
  padding:10px 0;border-bottom:1px solid rgba(255,255,255,.04)}
.cfg-row-label{font-size:12px;color:var(--txt)}

input[type=range]{-webkit-appearance:none;width:100%;height:6px;border-radius:3px;
  background:rgba(0,240,255,.1);outline:none;margin:8px 0}
input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:20px;height:20px;
  border-radius:50%;background:var(--accent);box-shadow:var(--glow-accent);cursor:pointer}

input[type=text],input[type=password],select{width:100%;box-sizing:border-box;padding:8px;
  border:1px solid rgba(0,240,255,.15);background:rgba(0,240,255,.04);color:var(--accent);
  font-family:'Share Tech Mono',monospace;font-size:14px;outline:none;border-radius:4px}

.wifi-info{font-size:11px;color:var(--dim);line-height:1.8}
.wifi-info b{color:var(--accent)}

/* MISSION LOG */
.mission-log{max-height:120px;overflow-y:auto;font-size:11px;color:var(--dim);
  line-height:1.6;padding:6px;background:rgba(0,0,0,.3);border-radius:4px}
.mission-log .entry{border-bottom:1px solid rgba(255,255,255,.03);padding:2px 0}
.mission-log .entry:last-child{border:none}

/* ESTOP */
.estop-bar{padding:8px 12px;background:var(--panel);border-top:1px solid rgba(255,0,0,.2)}
.estop{width:100%;padding:14px;border:2px solid var(--red);border-radius:6px;
  background:rgba(255,34,68,.08);color:var(--red);font-family:'Orbitron',sans-serif;
  font-weight:700;font-size:16px;letter-spacing:3px;cursor:pointer;
  transition:all .15s;text-transform:uppercase;touch-action:manipulation}
.estop:active{background:rgba(255,34,68,.3);transform:scale(.97)}

/* RESPONSIVE */
@media(max-width:380px){
  .joy-outer{width:min(200px,60vw)}
  .tcard-val{font-size:16px}
  .actions{grid-template-columns:1fr 1fr}
}
</style>
</head>
<body>

<!-- HEADER -->
<div class="hdr">
  <div class="logo" id="robot-logo">🤖 SHIZZBOT</div>
  <div class="hdr-spacer"></div>
  <div class="badge badge-batt" id="batt-badge">🔋 <span id="batt-txt">--%</span></div>
  <div class="badge badge-score" id="score-badge">⭐ <span id="score-txt">0</span></div>
  <div class="badge badge-conn"><div class="dot" id="conn-dot"></div><span id="conn-txt">---</span></div>
  <div class="latency" id="latency">--ms</div>
</div>

<!-- TABS -->
<div class="tabs">
  <button class="tab active" data-tab="drive">🏎️ DRIVE</button>
  <button class="tab" data-tab="arm">🦾 ARM</button>
  <button class="tab" data-tab="emotes">🎭 FUN</button>
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
      <div class="gauge-row">
        <!-- Throttle Gauge -->
        <div class="gauge-box">
          <svg width="80" height="80" viewBox="0 0 100 100">
            <circle cx="50" cy="50" r="40" fill="none" stroke="rgba(0,240,255,0.1)" stroke-width="8"></circle>
            <circle cx="50" cy="50" r="40" fill="none" stroke="var(--accent)" stroke-width="8"
              id="svg-throttle" stroke-dasharray="251" stroke-dashoffset="251" stroke-linecap="round"
              style="transition:stroke-dashoffset 0.1s;"></circle>
          </svg>
          <div class="gauge-center">
            <div class="gauge-val" id="sp-throttle">0</div>
            <div class="gauge-lbl">Throttle</div>
          </div>
        </div>
        <!-- Compass -->
        <div class="compass-box">
          <svg viewBox="0 0 100 100" id="compass-svg" style="cursor:crosshair">
            <circle cx="50" cy="50" r="45" fill="none" stroke="rgba(0,240,255,0.15)" stroke-width="2"/>
            <text x="50" y="12" text-anchor="middle" fill="var(--accent)" font-size="10" font-family="Orbitron">N</text>
            <line x1="50" y1="50" x2="50" y2="20" stroke="var(--accent)" stroke-width="2" id="compass-needle"
              style="transform-origin:50px 50px;transition:transform .15s"/>
            <line x1="50" y1="50" x2="50" y2="20" stroke="var(--magenta)" stroke-width="2" id="compass-target" stroke-dasharray="3,3"
              style="transform-origin:50px 50px;transition:transform .15s; display:none;"/>
          </svg>
        </div>
        <!-- Steer Indicator -->
        <div class="gauge-box">
          <svg width="80" height="80" viewBox="0 0 100 100">
            <circle cx="50" cy="50" r="40" fill="none" stroke="rgba(255,0,170,0.1)" stroke-width="8"></circle>
            <circle cx="50" cy="50" r="40" fill="none" stroke="var(--magenta)" stroke-width="8"
              id="svg-steer" stroke-dasharray="251" stroke-dashoffset="251" stroke-linecap="round"
              style="transition:stroke-dashoffset 0.1s;"></circle>
          </svg>
          <div class="gauge-center">
            <div class="gauge-val" id="sp-steer" style="color:var(--magenta)">0</div>
            <div class="gauge-lbl">Steer</div>
          </div>
        </div>
      </div>
    </div>
  </div>

  <div class="actions" style="grid-template-columns:1fr 1fr 1fr 1fr">
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

<!-- ============ ARM TAB ============ -->
<div class="page" id="pg-arm">

  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">Manipulator Arm</div>
    <div class="arm-slider-group">
      <div class="arm-slider">
        <label>Base</label>
        <input type="range" id="arm-base" min="-100" max="100" value="0">
        <div class="arm-val" id="arm-base-v">0</div>
      </div>
      <div class="arm-slider">
        <label>Lift</label>
        <input type="range" id="arm-lift" min="-100" max="100" value="0">
        <div class="arm-val" id="arm-lift-v">0</div>
      </div>
      <div class="arm-slider">
        <label>Grip</label>
        <input type="range" id="arm-grip" min="-100" max="100" value="0">
        <div class="arm-val" id="arm-grip-v">0</div>
      </div>
    </div>
    <p style="font-size:10px;color:var(--dim);margin-top:8px;text-align:center">
      ⚡ Continuous rotation: 0 = STOP, ±100 = full speed
    </p>
  </div>

  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">Presets</div>
    <div class="actions" style="grid-template-columns:1fr 1fr">
      <button class="act act-fwd" onclick="armPreset('park')">🅿️ PARK</button>
      <button class="act act-horn" onclick="armPreset('grab')">🤏 GRAB</button>
      <button class="act act-spin" onclick="armPreset('wave')">👋 WAVE</button>
      <button class="act act-rev" onclick="armPreset('release')">✋ RELEASE</button>
    </div>
  </div>

  <div class="pnl" style="padding:6px 12px">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="cfg-label"><span>Servo Max Speed</span><span class="cfg-val" id="servomax-v">70%</span></div>
    <input type="range" id="servomax" min="10" max="100" value="70">
  </div>
</div>

<!-- ============ FUN TAB ============ -->
<div class="page" id="pg-emotes">
  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">Soundboard</div>
    <div class="actions" style="grid-template-columns:1fr 1fr; gap:10px; margin-bottom:15px">
      <button class="act act-horn" onclick="sendSnd('horn')">🔊 HORN</button>
      <button class="act act-horn" onclick="sendSnd('chirp')">🤖 CHIRP</button>
      <button class="act act-spin" onclick="sendSnd('scan')">📡 SCAN</button>
      <button class="act act-rev" onclick="sendSnd('err')">❌ ERROR</button>
    </div>
  </div>

  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">Face Override</div>
    <div class="actions" style="grid-template-columns:1fr 1fr; gap:10px">
      <button class="act act-fwd" onclick="sendEmo('happy')">😄 HAPPY</button>
      <button class="act act-rev" onclick="sendEmo('angry')">😠 ANGRY</button>
      <button class="act act-spin" onclick="sendEmo('surprised')">😲 SURPRISE</button>
      <button class="act act-horn" onclick="sendEmo('dizzy')">😵 DIZZY</button>
      <button class="act" style="grid-column: span 2; border-color:var(--dim); color:var(--txt)" onclick="sendEmo('idle')">😐 NORMAL</button>
    </div>
  </div>

  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">Macros</div>
    <div class="actions" style="grid-template-columns:1fr;">
      <button class="act act-horn" onclick="wsSend({c:'dance'})" style="padding:15px; font-size:16px;">🕺 DANCE SEQUENCE</button>
    </div>
  </div>

  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">Mission Log</div>
    <div class="mission-log" id="mission-log">
      <div class="entry">🚀 System online!</div>
    </div>
  </div>
</div>

<!-- ============ SENSORS (inside drive for now) ============ -->

<!-- ============ CONFIG TAB ============ -->
<div class="page" id="pg-config">

  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">🤖 Robot Identity</div>
    <div style="display:flex;flex-direction:column;gap:8px;">
      <input type="text" id="cfg-name" placeholder="Robot Name" maxlength="16">
      <select id="cfg-color">
        <option value="0">🔵 Cyber Blue</option>
        <option value="1">🩷 Neon Pink</option>
        <option value="2">💚 Hacker Green</option>
        <option value="3">🧡 Blaze Orange</option>
      </select>
      <button class="act act-spin" onclick="saveName()" style="width:100%;padding:10px;">💾 SAVE IDENTITY</button>
    </div>
  </div>

  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">Face Settings</div>
    
    <div class="cfg-row" style="padding-top:0">
      <span class="cfg-row-label">Face Style</span>
      <select id="cfg-faceType" onchange="sendFaceCfg()" style="width:120px">
        <option value="0">Boy</option>
        <option value="1">Girl</option>
      </select>
    </div>
    <div class="cfg-label" style="margin-top:10px"><span>Eye Size</span><span class="cfg-val" id="fEye-v">15</span></div>
    <input type="range" id="cfg-fEye" min="10" max="25" value="15" onchange="sendFaceCfg()">
    <div class="cfg-label"><span>Blink Speed</span><span class="cfg-val" id="fBlink-v">50</span></div>
    <input type="range" id="cfg-fBlink" min="0" max="100" value="50" onchange="sendFaceCfg()">
    <div class="cfg-label"><span>Bounciness</span><span class="cfg-val" id="fBounce-v">50</span></div>
    <input type="range" id="cfg-fBounce" min="0" max="100" value="50" onchange="sendFaceCfg()">
  </div>

  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">Drive Config</div>
    <div class="cfg-row">
      <span class="cfg-row-label">Invert Motor</span>
      <label class="toggle"><input type="checkbox" id="cfg-invM" onchange="sendCfg()">
        <div class="toggle-track"></div><div class="toggle-thumb"></div></label>
    </div>
    <div class="cfg-row">
      <span class="cfg-row-label">Invert Steering</span>
      <label class="toggle"><input type="checkbox" id="cfg-invS" onchange="sendCfg()">
        <div class="toggle-track"></div><div class="toggle-thumb"></div></label>
    </div>
    <div class="cfg-row">
      <span class="cfg-row-label" style="color:var(--magenta)">Stealth Mode</span>
      <label class="toggle"><input type="checkbox" id="cfg-stealth" onchange="sendCfg()">
        <div class="toggle-track"></div><div class="toggle-thumb"></div></label>
    </div>
  </div>

  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">Sensors</div>
    <div class="tgrid">
      <div class="tcard" id="tc-volt"><div class="tcard-icon">⚡</div>
        <div class="tcard-lbl">Volts</div><div class="tcard-val"><span id="tv-v">--</span><span class="tcard-unit">V</span></div></div>
      <div class="tcard" id="tc-temp"><div class="tcard-icon">🌡️</div>
        <div class="tcard-lbl">Temp</div><div class="tcard-val"><span id="tv-t">--</span><span class="tcard-unit">°C</span></div></div>
      <div class="tcard" id="tc-rpm"><div class="tcard-icon">⚙️</div>
        <div class="tcard-lbl">RPM</div><div class="tcard-val" id="tv-rpm">--</div></div>
      <div class="tcard"><div class="tcard-icon">🧭</div>
        <div class="tcard-lbl">Heading</div><div class="tcard-val" id="tv-hdg">--<span class="tcard-unit">°</span></div></div>
      <div class="tcard"><div class="tcard-icon">📐</div>
        <div class="tcard-lbl">Pitch</div><div class="tcard-val" id="tv-pitch">--<span class="tcard-unit">°</span></div></div>
      <div class="tcard"><div class="tcard-icon">🔋</div>
        <div class="tcard-lbl">Current</div><div class="tcard-val" id="tv-cur">--<span class="tcard-unit">mA</span></div></div>
    </div>
    <button class="act act-spin" onclick="wsSend({c:'zero'})" style="width:100%;margin-top:8px">🧭 ZERO HEADING</button>
  </div>

  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">Network Info</div>
    <div class="wifi-info" id="wifi-info">Connecting...</div>
  </div>

  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">WiFi Setup</div>
    <div style="display:flex; flex-direction:column; gap:8px;">
      <input type="text" id="wifi-ssid" placeholder="Home WiFi SSID">
      <input type="password" id="wifi-pass" placeholder="Password">
      <button class="act act-spin" onclick="saveWiFi()" style="width:100%; padding:10px;">💾 SAVE & REBOOT</button>
    </div>
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
let sThrottle=0,sSteer=0,lastPing=0,lat=0;

// ===== COLOR THEMES =====
const THEMES = {
  0: {accent:'#00f0ff', glow:'0 0 8px rgba(0,240,255,.5)'},   // Cyber Blue
  1: {accent:'#ff00aa', glow:'0 0 8px rgba(255,0,170,.5)'},    // Neon Pink
  2: {accent:'#00ff88', glow:'0 0 8px rgba(0,255,136,.5)'},     // Hacker Green
  3: {accent:'#ff8800', glow:'0 0 8px rgba(255,136,0,.5)'}      // Blaze Orange
};

function applyTheme(colorId) {
  const t = THEMES[colorId] || THEMES[0];
  document.documentElement.style.setProperty('--accent', t.accent);
  document.documentElement.style.setProperty('--glow-accent', t.glow);
}

// ===== GAUGE HELPER =====
function updateGauge(id, val) {
  const c = document.getElementById(id);
  if(!c) return;
  c.style.strokeDashoffset = 251 - (251 * Math.abs(val) / 100);
}

// ===== MISSION LOG =====
function logMission(msg) {
  const log = document.getElementById('mission-log');
  const entry = document.createElement('div');
  entry.className = 'entry';
  entry.textContent = msg;
  log.insertBefore(entry, log.firstChild);
  if(log.children.length > 30) log.removeChild(log.lastChild);
}

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
    logMission('🟢 Connected to robot!');
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
      // Voltage
      const volts=(d.v/1000).toFixed(1);
      document.getElementById('tv-v').textContent=volts;
      const vc=document.getElementById('tc-volt');
      vc.classList.toggle('warn',d.v<11000&&d.v>0);
      vc.classList.toggle('danger',d.v<10000&&d.v>0);

      // Temp
      const temp=(d.t/10).toFixed(1);
      document.getElementById('tv-t').textContent=temp;

      // RPM
      document.getElementById('tv-rpm').textContent=d.rpm;

      // Current
      document.getElementById('tv-cur').textContent=d.cur;

      // Heading
      const hdg = (d.hdg/10).toFixed(0);
      document.getElementById('tv-hdg').textContent=hdg+'°';
      const needle = document.getElementById('compass-needle');
      if(needle) needle.style.transform = 'rotate('+hdg+'deg)';

      const targetNeedle = document.getElementById('compass-target');
      if(targetNeedle) {
        if(d.hdgLck !== undefined && d.hdgLck >= 0) {
           targetNeedle.style.display = 'block';
           targetNeedle.style.transform = 'rotate('+(d.hdgLck/10).toFixed(0)+'deg)';
        } else {
           targetNeedle.style.display = 'none';
        }
      }

      // Pitch
      document.getElementById('tv-pitch').textContent=(d.p/10).toFixed(1);

      // Battery
      if (d.batt !== undefined) {
        document.getElementById('batt-txt').textContent = d.batt + '%';
        const bb = document.getElementById('batt-badge');
        bb.style.borderColor = d.batt < 20 ? 'rgba(255,34,68,.8)' : 'rgba(255,136,0,.4)';
        bb.style.color = d.batt < 20 ? 'var(--red)' : 'var(--orange)';
      }

      // Score
      if (d.score !== undefined) {
        document.getElementById('score-txt').textContent = d.score;
      }

      // WiFi info
      if(d.ip) document.getElementById('wifi-info').innerHTML=
        'SSID: <b>'+d.ssid+'</b><br>IP: <b>'+d.ip+'</b><br>RSSI: <b>'+d.rssi+' dBm</b><br>Clients: <b>'+d.wc+'</b>';

      // One-time config sync on connection
      if(d.name !== undefined && !window.cfgSynced) {
        window.cfgSynced = true;
        document.getElementById('cfg-name').value = d.name;
        document.getElementById('cfg-color').value = d.color;
        document.getElementById('cfg-invM').checked = (d.invM === 1);
        document.getElementById('cfg-invS').checked = (d.invS === 1);
        document.getElementById('cfg-stealth').checked = (d.stealth === 1);
        document.getElementById('maxsp').value = d.maxSp;
        document.getElementById('maxsp-v').textContent = d.maxSp + '%';
        document.getElementById('servomax').value = d.servoMax;
        document.getElementById('servomax-v').textContent = d.servoMax + '%';
        maxPct = d.maxSp;
        // Apply identity
        document.getElementById('robot-logo').textContent = '🤖 ' + d.name.toUpperCase();
        applyTheme(d.color);
        
        if(d.fType !== undefined) {
             document.getElementById('cfg-faceType').value = d.fType;
             document.getElementById('cfg-fEye').value = d.fEye;
             document.getElementById('fEye-v').textContent = d.fEye;
             document.getElementById('cfg-fBlink').value = d.fBlink;
             document.getElementById('fBlink-v').textContent = d.fBlink;
             document.getElementById('cfg-fBounce').value = d.fBounce;
             document.getElementById('fBounce-v').textContent = d.fBounce;
        }
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

  const nx=dx/maxR,ny=-dy/maxR; // normalized -1..1

  // Ackermann: Y = throttle, X = steering
  sThrottle = Math.round(ny * maxPct);
  sSteer = Math.round(nx * 100);

  document.getElementById('sp-throttle').textContent=sThrottle;
  document.getElementById('sp-steer').textContent=sSteer;
  updateGauge('svg-throttle', sThrottle);
  updateGauge('svg-steer', sSteer);

  // Direction arrows
  arrows.n.classList.toggle('lit',ny>0.25);
  arrows.s.classList.toggle('lit',ny<-0.25);
  arrows.e.classList.toggle('lit',nx>0.25);
  arrows.w.classList.toggle('lit',nx<-0.25);
}

function joyReset(){
  thumb.style.left='50%';thumb.style.top='50%';
  thumb.classList.remove('active');
  sThrottle=0;sSteer=0;
  document.getElementById('sp-throttle').textContent='0';
  document.getElementById('sp-steer').textContent='0';
  updateGauge('svg-throttle', 0);
  updateGauge('svg-steer', 0);
  Object.values(arrows).forEach(a=>a.classList.remove('lit'));
  wsSend({c:'m',t:0,s:0});
}

// ===== COMPASS INTERACTION =====
const compSvg = document.getElementById('compass-svg');
if (compSvg) {
  compSvg.addEventListener('pointerdown', (e) => {
    const rect = compSvg.getBoundingClientRect();
    const x = e.clientX - rect.left - rect.width/2;
    const y = e.clientY - rect.top - rect.height/2;
    let angle = Math.atan2(x, -y) * 180 / Math.PI;
    if (angle < 0) angle += 360;
    wsSend({c:'hdg', h:angle, active:1});
    logMission('🧭 Heading Lock: ' + Math.round(angle) + '°');
  });
  compSvg.addEventListener('dblclick', () => {
    wsSend({c:'hdg', h:0, active:0});
    logMission('🧭 Heading Lock: OFF');
  });
}

joy.addEventListener('pointerdown',e=>{joyActive=true;thumb.classList.add('active');joy.setPointerCapture(e.pointerId);joyMove(e.clientX,e.clientY);});
joy.addEventListener('pointermove',e=>{if(joyActive)joyMove(e.clientX,e.clientY);});
joy.addEventListener('pointerup',()=>{joyActive=false;joyReset();});
joy.addEventListener('pointercancel',()=>{joyActive=false;joyReset();});

// Send motor+steer at max 20Hz
let lastSentT = null, lastSentS = null;
setInterval(()=>{
  if(joyActive && (sThrottle!==lastSentT || sSteer!==lastSentS)){
    wsSend({c:'m', t:sThrottle, s:sSteer});
    lastSentT=sThrottle; lastSentS=sSteer;
  }
},50);

// Max speed slider
document.getElementById('maxsp').addEventListener('input',(e)=>{
  maxPct=parseInt(e.target.value);
  document.getElementById('maxsp-v').textContent=maxPct+'%';
  sendCfg();
});

// Servo max speed slider
document.getElementById('servomax').addEventListener('input',(e)=>{
  const v=parseInt(e.target.value);
  document.getElementById('servomax-v').textContent=v+'%';
  sendCfg();
});

// ===== ARM SLIDERS =====
['arm-base','arm-lift','arm-grip'].forEach(id=>{
  const el = document.getElementById(id);
  const valEl = document.getElementById(id+'-v');

  // Touch: send while dragging
  el.addEventListener('input', ()=>{
    valEl.textContent = el.value;
    sendArm();
  });

  // Release: snap back to zero (continuous rotation — stop on release)
  el.addEventListener('change', ()=>{
    el.value = 0;
    valEl.textContent = '0';
    sendArm();
  });
});

function sendArm(){
  wsSend({c:'arm',
    b:parseInt(document.getElementById('arm-base').value),
    l:parseInt(document.getElementById('arm-lift').value),
    g:parseInt(document.getElementById('arm-grip').value)
  });
}

function armPreset(p){
  wsSend({c:'armPreset', p:p});
  logMission('🦾 Arm: ' + p.toUpperCase());
}

// ===== QUICK ACTIONS =====
function qAct(a){
  if(a==='fwd'){sThrottle=maxPct;sSteer=0;}
  else if(a==='rev'){sThrottle=-maxPct;sSteer=0;}
  else if(a==='spin'){sThrottle=30;sSteer=100;}
  else if(a==='horn'){wsSend({c:'snd',s:'horn'});logMission('📢 HONK!');return;}

  document.getElementById('sp-throttle').textContent=sThrottle;
  document.getElementById('sp-steer').textContent=sSteer;
  updateGauge('svg-throttle', sThrottle);
  updateGauge('svg-steer', sSteer);
  wsSend({c:'m',t:sThrottle,s:sSteer});
  setTimeout(()=>{sThrottle=0;sSteer=0;
    document.getElementById('sp-throttle').textContent='0';
    document.getElementById('sp-steer').textContent='0';
    updateGauge('svg-throttle', 0);
    updateGauge('svg-steer', 0);
    wsSend({c:'m',t:0,s:0});
  },1200);
}

// ===== SOUNDS & EMOTES =====
function sendSnd(s){wsSend({c:'snd',s:s});logMission('🔊 Sound: '+s);}
function sendEmo(e){wsSend({c:'emo',e:e});logMission('🎭 Face: '+e);}

// ===== CONFIG =====
function sendCfg(){
  wsSend({c:'cfg',
    invM:document.getElementById('cfg-invM').checked?1:0,
    invS:document.getElementById('cfg-invS').checked?1:0,
    maxSp:parseInt(document.getElementById('maxsp').value),
    servoMax:parseInt(document.getElementById('servomax').value),
    stealth:document.getElementById('cfg-stealth').checked?1:0
  });
}

function sendFaceCfg(){
  const ft = parseInt(document.getElementById('cfg-faceType').value);
  const fe = parseInt(document.getElementById('cfg-fEye').value);
  const fb = parseInt(document.getElementById('cfg-fBlink').value);
  const fbo = parseInt(document.getElementById('cfg-fBounce').value);
  
  document.getElementById('fEye-v').textContent = fe;
  document.getElementById('fBlink-v').textContent = fb;
  document.getElementById('fBounce-v').textContent = fbo;
  
  wsSend({c:'faceCfg', type:ft, eye:fe, blink:fb, bounce:fbo});
}

// ===== IDENTITY =====
function saveName(){
  const n = document.getElementById('cfg-name').value.trim();
  const c = parseInt(document.getElementById('cfg-color').value);
  if(n){
    wsSend({c:'name', n:n});
    wsSend({c:'color', v:c});
    document.getElementById('robot-logo').textContent = '🤖 ' + n.toUpperCase();
    applyTheme(c);
    logMission('🏷️ Renamed to: ' + n);
  }
}

// ===== WIFI =====
function saveWiFi(){
  const s = document.getElementById('wifi-ssid').value.trim();
  const p = document.getElementById('wifi-pass').value;
  if(s){
    wsSend({c:'wifi', s:s, p:p});
    alert('Credentials saved! Rebooting...');
  } else {
    alert('Please enter an SSID.');
  }
}

// ===== ESTOP =====
document.getElementById('btn-estop').addEventListener('click',()=>{
  wsSend({c:'stop'});
  sThrottle=0;sSteer=0;
  document.getElementById('sp-throttle').textContent='0';
  document.getElementById('sp-steer').textContent='0';
  updateGauge('svg-throttle', 0);
  updateGauge('svg-steer', 0);
  // Reset arm sliders
  ['arm-base','arm-lift','arm-grip'].forEach(id=>{
    document.getElementById(id).value=0;
    document.getElementById(id+'-v').textContent='0';
  });
  if(navigator.vibrate)navigator.vibrate(200);
  logMission('🛑 EMERGENCY STOP!');
});

</script>
</body>
</html>
)rawliteral";
