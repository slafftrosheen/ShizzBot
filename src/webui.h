// ============================================================
// ShizzBot Swarm - WebUI v4: Kids World Mega Expansion
// Drive, PT, Fun, Pet, and Config tabs.
// Voice changer, Music Box, Achievements, Story mode.
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
  color:var(--accent);letter-spacing:2px;flex-shrink:0;text-shadow:0 0 8px currentColor}
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
  text-transform:uppercase;letter-spacing:2px;margin-bottom:10px;text-shadow:0 0 6px currentColor}

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
.compass-box{width:60px;height:60px;position:relative;margin:0 auto}
.compass-box svg{width:100%;height:100%}

/* ACTIONS */
.actions{display:grid;gap:8px}
.act{border:1px solid;border-radius:4px;padding:10px 6px;font-family:'Share Tech Mono',monospace;
  font-size:12px;cursor:pointer;text-transform:uppercase;letter-spacing:1px;
  background:transparent;transition:all .15s;touch-action:manipulation;color:var(--txt)}
.act:active{transform:scale(.95)}
.act-fwd{border-color:var(--lime);color:var(--lime)}
.act-rev{border-color:var(--red);color:var(--red)}
.act-spin{border-color:var(--accent);color:var(--accent)}
.act-horn{border-color:var(--orange);color:var(--orange)}
.act-gray{border-color:var(--dim);color:var(--txt)}

/* SLIDERS */
.arm-slider-group{display:flex;flex-direction:column;gap:12px}
.arm-slider{display:flex;align-items:center;gap:10px}
.arm-slider label{font-size:10px;color:var(--dim);text-transform:uppercase;letter-spacing:1px;width:50px;text-align:right}
.arm-slider input[type=range]{flex:1;-webkit-appearance:none;height:8px;border-radius:4px;
  background:rgba(0,240,255,.1);outline:none}
.arm-slider input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:24px;height:24px;
  border-radius:50%;background:var(--accent);box-shadow:var(--glow-accent);cursor:pointer}
.arm-slider .arm-val{font-family:'Orbitron',sans-serif;font-size:14px;color:var(--accent);width:35px;text-align:center}

/* SENSORS & METERS */
.tgrid{display:grid;grid-template-columns:repeat(auto-fit,minmax(100px,1fr));gap:8px}
.tcard{text-align:center;padding:8px 4px;border-radius:4px;
  background:rgba(0,240,255,.03);border:1px solid rgba(0,240,255,.06)}
.tcard-icon{font-size:20px;margin-bottom:4px}
.tcard-lbl{font-size:9px;letter-spacing:1px;color:var(--dim);text-transform:uppercase}
.tcard-val{font-family:'Orbitron',sans-serif;font-size:18px;font-weight:700;color:var(--accent);margin-top:2px}
.tcard-unit{font-size:10px;color:var(--dim)}

/* PROGRESS BARS */
.bar-wrap{width:100%;height:8px;background:rgba(255,255,255,0.1);border-radius:4px;overflow:hidden;margin-top:4px;}
.bar-fill{height:100%;transition:width 0.3s;background:var(--accent);}

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
  background:rgba(0,240,255,.1);outline:none;margin:8px 0;touch-action:pan-y}
input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:20px;height:20px;
  border-radius:50%;background:var(--accent);box-shadow:var(--glow-accent);cursor:pointer}
input[type=text],input[type=password],select{width:100%;box-sizing:border-box;padding:8px;
  border:1px solid rgba(0,240,255,.15);background:rgba(0,240,255,.04);color:var(--accent);
  font-family:'Share Tech Mono',monospace;font-size:14px;outline:none;border-radius:4px;margin-bottom:8px}

.wifi-info{font-size:11px;color:var(--dim);line-height:1.8}
.wifi-info b{color:var(--accent)}

/* MISSION LOG & TOAST */
.mission-log{max-height:120px;overflow-y:auto;font-size:11px;color:var(--dim);
  line-height:1.6;padding:6px;background:rgba(0,0,0,.3);border-radius:4px}
.mission-log .entry{border-bottom:1px solid rgba(255,255,255,.03);padding:2px 0}
.mission-log .entry:last-child{border:none}
#toast{position:fixed;top:60px;left:50%;transform:translateX(-50%);background:var(--accent);
  color:#000;padding:10px 20px;border-radius:20px;font-weight:700;font-size:14px;
  box-shadow:var(--glow-accent);z-index:9999;opacity:0;transition:opacity 0.3s;pointer-events:none;}
#toast.show{opacity:1;}

/* MUSIC BOX */
.music-grid{display:grid;grid-template-columns:repeat(4,1fr);gap:10px;margin-top:10px}
.music-pad{aspect-ratio:1;border-radius:50%;border:2px solid;display:flex;align-items:center;
  justify-content:center;font-weight:bold;cursor:pointer;user-select:none;transition:transform 0.1s}
.music-pad:active{transform:scale(0.9)}

/* ACHIEVEMENTS */
.ach-grid{display:grid;grid-template-columns:1fr 1fr;gap:8px}
.ach-card{padding:8px;border-radius:4px;background:rgba(0,0,0,0.4);border:1px solid rgba(255,255,255,0.1);
  display:flex;align-items:center;gap:8px;opacity:0.4;transition:all 0.3s}
.ach-card.unlocked{opacity:1;border-color:var(--lime);background:rgba(0,255,136,0.1);box-shadow:0 0 10px rgba(0,255,136,0.2)}
.ach-icon{font-size:24px}
.ach-info{flex:1;overflow:hidden}
.ach-title{font-size:12px;font-weight:bold;color:var(--txt);white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
.ach-desc{font-size:9px;color:var(--dim);line-height:1.2;margin-top:2px}
.ach-card.unlocked .ach-title{color:var(--lime)}

/* MIC BUTTON */
#btn-mic{background:rgba(255,0,170,0.1);border-color:var(--magenta);color:var(--magenta);padding:15px;font-size:16px;}
#btn-mic.recording{background:var(--magenta);color:#fff;box-shadow:0 0 20px var(--magenta);animation:pulse 1s infinite;}

/* ESTOP */
.estop-bar{padding:8px 12px;background:var(--panel);border-top:1px solid rgba(255,0,0,.2);display:flex;gap:10px;}
.estop{flex:2;padding:14px;border:2px solid var(--red);border-radius:6px;
  background:rgba(255,34,68,.08);color:var(--red);font-family:'Orbitron',sans-serif;
  font-weight:700;font-size:16px;letter-spacing:3px;cursor:pointer;
  transition:all .15s;text-transform:uppercase;touch-action:manipulation}
.estop:active{background:rgba(255,34,68,.3);transform:scale(.97)}
.btn-swarm-estop{flex:1;background:rgba(255,100,0,0.1);border-color:var(--orange);color:var(--orange);}

@media(max-width:380px){
  .joy-outer{width:min(200px,60vw)}
  .actions{grid-template-columns:1fr 1fr}
}
</style>
</head>
<body>

<div id="toast">⭐ Achievement Unlocked!</div>

<!-- HEADER -->
<div class="hdr">
  <div class="logo" id="robot-logo">🤖 SHIZZBOT</div>
  <div class="hdr-spacer"></div>
  <div class="badge badge-batt" id="batt-badge">🔋 <span id="batt-txt">--%</span></div>
  <div class="badge badge-score" id="score-badge">⭐ <span id="score-txt">0</span></div>
  <div class="badge badge-conn"><div class="dot" id="conn-dot"></div><span id="conn-txt">---</span></div>
</div>

<!-- TABS -->
<div class="tabs">
  <button class="tab active" data-tab="drive">🏎️ DRIVE</button>
  <button class="tab" data-tab="pt">📸 HEAD</button>
  <button class="tab" data-tab="emotes">🎭 FUN</button>
  <button class="tab" data-tab="pet">🐾 PET</button>
  <button class="tab" data-tab="config">⚙️ CFG</button>
</div>

<div class="content">

<!-- ============ DRIVE TAB ============ -->
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
        <div class="gauge-box">
          <svg width="80" height="80" viewBox="0 0 100 100">
            <circle cx="50" cy="50" r="40" fill="none" stroke="rgba(0,240,255,0.1)" stroke-width="8"></circle>
            <circle cx="50" cy="50" r="40" fill="none" stroke="var(--accent)" stroke-width="8" id="svg-throttle" stroke-dasharray="251" stroke-dashoffset="251" stroke-linecap="round" style="transition:stroke-dashoffset 0.1s;"></circle>
          </svg>
          <div class="gauge-center"><div class="gauge-val" id="sp-throttle">0</div><div class="gauge-lbl">Throttle</div></div>
        </div>
        <div class="compass-box">
          <svg viewBox="0 0 100 100" id="compass-svg" style="cursor:crosshair">
            <circle cx="50" cy="50" r="45" fill="none" stroke="rgba(0,240,255,0.15)" stroke-width="2"/>
            <text x="50" y="12" text-anchor="middle" fill="var(--accent)" font-size="10" font-family="Orbitron">N</text>
            <line x1="50" y1="50" x2="50" y2="20" stroke="var(--accent)" stroke-width="2" id="compass-needle" style="transform-origin:50px 50px;transition:transform .15s"/>
            <line x1="50" y1="50" x2="50" y2="20" stroke="var(--magenta)" stroke-width="2" id="compass-target" stroke-dasharray="3,3" style="transform-origin:50px 50px;transition:transform .15s; display:none;"/>
          </svg>
        </div>
        <div class="gauge-box">
          <svg width="80" height="80" viewBox="0 0 100 100">
            <circle cx="50" cy="50" r="40" fill="none" stroke="rgba(255,0,170,0.1)" stroke-width="8"></circle>
            <circle cx="50" cy="50" r="40" fill="none" stroke="var(--magenta)" stroke-width="8" id="svg-steer" stroke-dasharray="251" stroke-dashoffset="251" stroke-linecap="round" style="transition:stroke-dashoffset 0.1s;"></circle>
          </svg>
          <div class="gauge-center"><div class="gauge-val" id="sp-steer" style="color:var(--magenta)">0</div><div class="gauge-lbl">Steer</div></div>
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

<!-- ============ PAN/TILT TAB ============ -->
<div class="page" id="pg-pt">
  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">Pan & Tilt Control</div>
    <div class="arm-slider-group">
      <div class="arm-slider">
        <label>Pan</label>
        <input type="range" id="pt-pan" min="-100" max="100" value="0">
        <div class="arm-val" id="pt-pan-v">0</div>
      </div>
      <div class="arm-slider">
        <label>Tilt</label>
        <input type="range" id="pt-tilt" min="-100" max="100" value="0">
        <div class="arm-val" id="pt-tilt-v">0</div>
      </div>
    </div>
  </div>
  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">Presets</div>
    <div class="actions" style="grid-template-columns:1fr 1fr">
      <button class="act act-fwd" onclick="ptPreset('center')">🎯 CENTER</button>
      <button class="act act-horn" onclick="ptPreset('scan')">📡 SCAN</button>
      <button class="act act-horn" onclick="ptPreset('nod')">👍 NOD</button>
      <button class="act act-fwd" onclick="ptPreset('shake')">👎 SHAKE</button>
    </div>
  </div>
</div>

<!-- ============ FUN TAB ============ -->
<div class="page" id="pg-emotes">
  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">Faces</div>
    <div class="actions" style="grid-template-columns:1fr 1fr 1fr 1fr; gap:6px;">
      <button class="act act-spin" onclick="sendEmo('love')">😍</button>
      <button class="act act-horn" onclick="sendEmo('silly')">🤪</button>
      <button class="act act-fwd" onclick="sendEmo('cool')">😎</button>
      <button class="act act-rev" onclick="sendEmo('crying')">😭</button>
      <button class="act act-gray" onclick="sendEmo('ninja')">🥷</button>
      <button class="act act-horn" onclick="sendEmo('happy')">😄</button>
      <button class="act act-spin" onclick="sendEmo('shocked')">😲</button>
      <button class="act act-fwd" onclick="sendEmo('wink')">😉</button>
      <button class="act act-gray" onclick="sendEmo('bored')">😒</button>
      <button class="act act-rev" onclick="sendEmo('party')">🥳</button>
      <button class="act act-lime" onclick="sendEmo('robot')">🤖</button>
      <button class="act act-gray" onclick="sendEmo('idle')">😐</button>
    </div>
  </div>

  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">Emoji Overlays</div>
    <div class="actions" style="grid-template-columns:1fr 1fr 1fr 1fr 1fr; gap:6px;">
      <button class="act" onclick="wsSend({c:'overlay',e:1})">⭐</button>
      <button class="act" onclick="wsSend({c:'overlay',e:2})">⚡</button>
      <button class="act" onclick="wsSend({c:'overlay',e:3})">🔥</button>
      <button class="act" onclick="wsSend({c:'overlay',e:4})">💩</button>
      <button class="act" onclick="wsSend({c:'overlay',e:5})">🌈</button>
    </div>
  </div>

  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">Speech Bubbles</div>
    <div style="display:flex; gap:8px;">
      <input type="text" id="speech-txt" placeholder="Say something..." maxlength="20" style="margin:0;">
      <button class="act act-spin" onclick="sendSpeech()">💬 SEND</button>
    </div>
    <div class="actions" style="grid-template-columns:1fr 1fr 1fr; margin-top:8px;">
      <button class="act act-gray" onclick="wsSend({c:'speech',t:'Hi!'})">Hi!</button>
      <button class="act act-gray" onclick="wsSend({c:'speech',t:'Feed me!'})">Feed me</button>
      <button class="act act-gray" onclick="wsSend({c:'speech',t:'Uh oh...'})">Uh oh</button>
    </div>
  </div>

  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">Soundboard</div>
    <div class="actions" style="grid-template-columns:1fr 1fr; gap:8px;">
      <button class="act act-rev" onclick="sendSnd('fart')">💨 FART</button>
      <button class="act act-horn" onclick="sendSnd('giggle')">🤭 GIGGLE</button>
      <button class="act act-spin" onclick="sendSnd('magic')">✨ MAGIC</button>
      <button class="act act-fwd" onclick="sendSnd('pewpew')">🔫 PEW PEW</button>
      <button class="act act-rev" onclick="sendSnd('burp')">🤢 BURP</button>
      <button class="act act-horn" onclick="sendSnd('whoopee')">🎈 WHOOPEE</button>
      <button class="act act-spin" onclick="sendSnd('tada')">🎺 TA-DA</button>
      <button class="act act-fwd" onclick="sendSnd('drumroll')">🥁 DRUMROLL</button>
      <button class="act act-gray" onclick="sendSnd('alien')">👽 ALIEN</button>
      <button class="act act-gray" onclick="sendSnd('boing')">🪀 BOING</button>
    </div>
  </div>

  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">Action Macros</div>
    <div class="actions" style="grid-template-columns:1fr 1fr;">
      <button class="act act-rev" onclick="wsSend({c:'macro', m:'fart_shake'})">💨 Fart+Shake</button>
      <button class="act act-spin" onclick="wsSend({c:'macro', m:'magic_reveal'})">✨ Reveal</button>
      <button class="act act-horn" onclick="wsSend({c:'macro', m:'victory_dance'})">🏆 Victory</button>
      <button class="act act-fwd" onclick="wsSend({c:'macro', m:'tantrum'})">😡 Tantrum</button>
      <button class="act act-gray" onclick="wsSend({c:'macro', m:'sneeze'})">🤧 Sneeze</button>
      <button class="act act-lime" onclick="wsSend({c:'macro', m:'belly_laugh'})">🤣 Laugh</button>
    </div>
  </div>

  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">Dance Sequences</div>
    <div class="actions" style="grid-template-columns:1fr 1fr;">
      <button class="act act-spin" onclick="wsSend({c:'dance', r:0})">🪩 Disco</button>
      <button class="act act-lime" onclick="wsSend({c:'dance', r:1})">🤖 Robot</button>
      <button class="act act-horn" onclick="wsSend({c:'dance', r:2})">〰️ Wiggle</button>
      <button class="act act-gray" onclick="wsSend({c:'dance', r:3})">🚶 Moonwalk</button>
    </div>
  </div>
</div>

<!-- ============ PET TAB ============ -->
<div class="page" id="pg-pet">
  
  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">Tamagotchi Status</div>
    <div style="display:flex; justify-content:space-between; margin-bottom:4px; font-size:12px">
      <span>Level: <b id="pet-lvl" class="cfg-val">Baby (0)</b></span>
      <span>XP: <b id="pet-xp" class="cfg-val">0</b></span>
    </div>
    <div class="cfg-label" style="margin-top:10px"><span>🍕 Hunger</span><span class="cfg-val" id="pet-hval">100%</span></div>
    <div class="bar-wrap"><div class="bar-fill" id="pet-hbar" style="background:var(--lime); width:100%"></div></div>
    
    <div class="actions" style="grid-template-columns:1fr 1fr; margin-top:15px;">
      <button class="act act-lime" onclick="wsSend({c:'pet', a:'feed'})">🍕 FEED</button>
      <button class="act act-spin" onclick="wsSend({c:'pet', a:'pat'})">🤗 PET</button>
    </div>
    <div class="cfg-row" style="margin-top:10px; padding-top:10px;">
      <span class="cfg-row-label">🐾 Auto-Pet Mode (Idle Behaviors)</span>
      <label class="toggle"><input type="checkbox" id="tgl-pet" onchange="wsSend({c:'pet', a:'toggle'})">
        <div class="toggle-track"></div><div class="toggle-thumb"></div></label>
    </div>
  </div>

  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">Voice Changer</div>
    <select id="vc-effect" style="margin-bottom:10px;">
      <option value="none">Normal Voice</option>
      <option value="robot">Robot</option>
      <option value="chipmunk">Chipmunk</option>
      <option value="monster">Monster</option>
      <option value="echo">Echo Room</option>
    </select>
    <button class="act" id="btn-mic" style="width:100%">🎤 HOLD TO TALK</button>
  </div>

  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">Story Mode</div>
    <div class="actions" style="grid-template-columns:1fr;">
      <button class="act act-fwd" onclick="wsSend({c:'story', id:0})">📖 The Brave Robot</button>
      <button class="act act-spin" onclick="wsSend({c:'story', id:1})">🚀 Space Mission</button>
      <button class="act act-horn" onclick="wsSend({c:'story', id:2})">🤪 The Silly Dance</button>
    </div>
  </div>

  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">RGB Mood Lighting</div>
    <div class="actions" style="grid-template-columns:1fr 1fr 1fr;">
      <button class="act" onclick="wsSend({c:'rgb', p:1})" style="border-color:#ff00ff">🌈 Rainbow</button>
      <button class="act" onclick="wsSend({c:'rgb', p:2})" style="border-color:#0000ff">🚓 Police</button>
      <button class="act" onclick="wsSend({c:'rgb', p:3})" style="border-color:#ff0000">❤️ Heart</button>
      <button class="act" onclick="wsSend({c:'rgb', p:4})" style="border-color:#ffff00">🪩 Disco</button>
      <button class="act" onclick="wsSend({c:'rgb', p:5})" style="border-color:#00ffff">💨 Breath</button>
      <button class="act" onclick="wsSend({c:'rgb', p:6})" style="border-color:#ff8800">🔥 Fire</button>
      <button class="act act-gray" onclick="wsSend({c:'rgb', p:0})" style="grid-column:span 3">❌ OFF (Auto-Sync)</button>
    </div>
  </div>

  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">Music Box</div>
    <div class="music-grid">
      <div class="music-pad" style="border-color:#ff0000; color:#ff0000" onclick="wsSend({c:'note',n:0})">C</div>
      <div class="music-pad" style="border-color:#ff8800; color:#ff8800" onclick="wsSend({c:'note',n:2})">D</div>
      <div class="music-pad" style="border-color:#ffff00; color:#ffff00" onclick="wsSend({c:'note',n:4})">E</div>
      <div class="music-pad" style="border-color:#00ff00; color:#00ff00" onclick="wsSend({c:'note',n:5})">F</div>
      <div class="music-pad" style="border-color:#00ffff; color:#00ffff" onclick="wsSend({c:'note',n:7})">G</div>
      <div class="music-pad" style="border-color:#0000ff; color:#0000ff" onclick="wsSend({c:'note',n:9})">A</div>
      <div class="music-pad" style="border-color:#ff00ff; color:#ff00ff" onclick="wsSend({c:'note',n:11})">B</div>
      <div class="music-pad" style="border-color:#ffffff; color:#ffffff" onclick="wsSend({c:'note',n:12})">C'</div>
    </div>
  </div>

  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">Achievements <button class="act act-spin" style="float:right; padding:2px 8px;" onclick="loadAch()">🔄</button></div>
    <div class="ach-grid" id="ach-list">
      <div class="cfg-label">Loading...</div>
    </div>
  </div>

</div>

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
    <div class="pnl-title">Face Customizer</div>
    <div class="cfg-row" style="padding-top:0">
      <span class="cfg-row-label">Base Style</span>
      <select id="cfg-faceType" onchange="sendFaceCust()" style="width:120px; margin:0">
        <option value="0">Boy</option>
        <option value="1">Girl</option>
      </select>
    </div>
    <div class="cfg-row">
      <span class="cfg-row-label">Eye Shape</span>
      <select id="cfg-fEyeShape" onchange="sendFaceCust()" style="width:120px; margin:0">
        <option value="0">Round</option>
        <option value="1">Square</option>
        <option value="2">Star</option>
      </select>
    </div>
    <div class="cfg-row">
      <span class="cfg-row-label">Mouth Style</span>
      <select id="cfg-fMouth" onchange="sendFaceCust()" style="width:120px; margin:0">
        <option value="0">Normal</option>
        <option value="1">Zigzag</option>
        <option value="2">Fangs</option>
      </select>
    </div>
    <div class="cfg-row">
      <span class="cfg-row-label">Accessory</span>
      <select id="cfg-fAcc" onchange="sendFaceCust()" style="width:120px; margin:0">
        <option value="0">Default</option>
        <option value="1">Top Hat</option>
        <option value="2">Pink Bow</option>
        <option value="3">Antenna</option>
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
      <label class="toggle"><input type="checkbox" id="cfg-invM" onchange="sendCfg()"><div class="toggle-track"></div><div class="toggle-thumb"></div></label>
    </div>
    <div class="cfg-row">
      <span class="cfg-row-label">Invert Steering</span>
      <label class="toggle"><input type="checkbox" id="cfg-invS" onchange="sendCfg()"><div class="toggle-track"></div><div class="toggle-thumb"></div></label>
    </div>
    <div class="cfg-row">
      <span class="cfg-row-label" style="color:var(--magenta)">Stealth Mode</span>
      <label class="toggle"><input type="checkbox" id="cfg-stealth" onchange="sendCfg()"><div class="toggle-track"></div><div class="toggle-thumb"></div></label>
    </div>
  </div>
  
  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">Telemetry</div>
    <div class="tgrid">
      <div class="tcard"><div class="tcard-icon">⚡</div><div class="tcard-lbl">Volts</div><div class="tcard-val"><span id="tv-v">--</span><span class="tcard-unit">V</span></div></div>
      <div class="tcard"><div class="tcard-icon">🌡️</div><div class="tcard-lbl">Temp</div><div class="tcard-val"><span id="tv-bmeT">--</span><span class="tcard-unit">°C</span></div></div>
      <div class="tcard"><div class="tcard-icon">💨</div><div class="tcard-lbl">Air</div><div class="tcard-val" id="tv-bmeA">--<span class="tcard-unit">%</span></div></div>
      <div class="tcard"><div class="tcard-icon">🧭</div><div class="tcard-lbl">Hdg</div><div class="tcard-val" id="tv-hdg">--<span class="tcard-unit">°</span></div></div>
    </div>
    <button class="act act-spin" onclick="wsSend({c:'zero'})" style="width:100%;margin-top:8px">🧭 ZERO HEADING</button>
  </div>

  <div class="pnl">
    <div class="corner-bl"></div><div class="corner-br"></div>
    <div class="pnl-title">System</div>
    <div class="wifi-info" id="wifi-info" style="margin-bottom:10px">Connecting...</div>
    <input type="text" id="wifi-ssid" placeholder="Home WiFi SSID">
    <input type="password" id="wifi-pass" placeholder="Password">
    <button class="act act-spin" onclick="saveWiFi()" style="width:100%; padding:10px; margin-bottom:10px;">💾 SAVE & REBOOT</button>
    <button class="act act-fwd" onclick="window.location.href='/update'" style="width:100%">⬆️ FIRMWARE UPDATE</button>
  </div>
</div>

</div><!-- /content -->

<!-- ESTOP FOOTER -->
<div class="estop-bar">
  <button class="estop" id="btn-estop">🛑 LOCAL STOP</button>
  <button class="estop btn-swarm-estop" onclick="wsSend({c:'swarm_estop'})">🛑 SWARM</button>
</div>

<script>
// ===== STATE =====
let ws=null,wsOk=false,maxPct=50,joyActive=false;
let sThrottle=0,sSteer=0,lastPing=0,lat=0;

// ===== COLOR THEMES =====
const THEMES = {
  0: {accent:'#00f0ff', glow:'0 0 8px rgba(0,240,255,.5)'},
  1: {accent:'#ff00aa', glow:'0 0 8px rgba(255,0,170,.5)'},
  2: {accent:'#00ff88', glow:'0 0 8px rgba(0,255,136,.5)'},
  3: {accent:'#ff8800', glow:'0 0 8px rgba(255,136,0,.5)'}
};

function applyTheme(colorId) {
  const t = THEMES[colorId] || THEMES[0];
  document.documentElement.style.setProperty('--accent', t.accent);
  document.documentElement.style.setProperty('--glow-accent', t.glow);
}

function updateGauge(id, val) {
  const c = document.getElementById(id);
  if(c) c.style.strokeDashoffset = 251 - (251 * Math.abs(val) / 100);
}

// ===== TABS =====
document.querySelectorAll('.tab').forEach(t=>{
  t.addEventListener('click',()=>{
    document.querySelectorAll('.tab').forEach(x=>x.classList.remove('active'));
    document.querySelectorAll('.page').forEach(x=>x.classList.remove('active'));
    t.classList.add('active');
    document.getElementById('pg-'+t.dataset.tab).classList.add('active');
    if(t.dataset.tab === 'pet') loadAch();
  });
});

// ===== TOAST NOTIFICATION =====
function showToast(msg) {
  const t = document.getElementById('toast');
  t.textContent = msg;
  t.classList.add('show');
  setTimeout(()=>t.classList.remove('show'), 3000);
}

// ===== WEBSOCKET =====
function wsConnect(){
  const h=location.host||'192.168.4.1';
  ws=new WebSocket('ws://'+h+'/ws');
  ws.onopen=()=>{
    wsOk=true;
    document.getElementById('conn-dot').classList.remove('off');
    document.getElementById('conn-txt').textContent='ONLINE';
  };
  ws.onclose=()=>{
    wsOk=false;
    document.getElementById('conn-dot').classList.add('off');
    document.getElementById('conn-txt').textContent='OFFLINE';
    setTimeout(wsConnect,1500);
  };
  ws.onerror=()=>{ws.close()};
  ws.onmessage=(e)=>{
    lat=Date.now()-lastPing;
    try{
      const d=JSON.parse(e.data);
      
      // Handle Achievement List response
      if(d.ach) {
        renderAchievements(d.ach);
        return;
      }

      // Achievement notification
      if(d.achNew === 1) {
        showToast('⭐ Achievement Unlocked!');
        loadAch(); // refresh list
      }

      // Telemetry
      if(d.v) document.getElementById('tv-v').textContent=(d.v/1000).toFixed(1);
      if(d.bmeT !== undefined) document.getElementById('tv-bmeT').textContent=d.bmeT.toFixed(1);
      if(d.bmeA !== undefined) document.getElementById('tv-bmeA').textContent=d.bmeA+'%';
      if(d.mic !== undefined && document.getElementById('tv-mic')) document.getElementById('tv-mic').textContent=d.mic+'%';
      
      // Pet Status
      if(d.lvl !== undefined) {
        const lvls = ['Baby (0)', 'Teen (1)', 'Adult (2)'];
        document.getElementById('pet-lvl').textContent = lvls[d.lvl] || 'Max';
        document.getElementById('pet-xp').textContent = d.xp;
        
        const hVal = d.petHunger;
        document.getElementById('pet-hval').textContent = hVal + '%';
        const hBar = document.getElementById('pet-hbar');
        hBar.style.width = hVal + '%';
        hBar.style.background = hVal < 30 ? 'var(--red)' : (hVal < 60 ? 'var(--orange)' : 'var(--lime)');
      }
      
      if(d.petOn !== undefined) document.getElementById('tgl-pet').checked = (d.petOn === 1);

      // Heading
      if(d.hdg !== undefined) {
        const hdg = (d.hdg/10).toFixed(0);
        document.getElementById('tv-hdg').textContent=hdg+'°';
        const needle = document.getElementById('compass-needle');
        if(needle) needle.style.transform = 'rotate('+hdg+'deg)';
      }

      // Battery
      if (d.batt !== undefined) {
        document.getElementById('batt-txt').textContent = d.batt + '%';
        const bb = document.getElementById('batt-badge');
        bb.style.borderColor = d.batt < 20 ? 'rgba(255,34,68,.8)' : 'rgba(255,136,0,.4)';
        bb.style.color = d.batt < 20 ? 'var(--red)' : 'var(--orange)';
      }

      // Score
      if (d.score !== undefined) document.getElementById('score-txt').textContent = d.score;

      // WiFi info
      if(d.ip) document.getElementById('wifi-info').innerHTML=
        'SSID: <b>'+d.ssid+'</b><br>IP: <b>'+d.ip+'</b><br>RSSI: <b>'+d.rssi+' dBm</b><br>Clients: <b>'+d.wc+'</b>';

      // One-time config sync
      if(d.name !== undefined && !window.cfgSynced) {
        window.cfgSynced = true;
        document.getElementById('cfg-name').value = d.name;
        document.getElementById('cfg-color').value = d.color;
        document.getElementById('cfg-invM').checked = (d.invM === 1);
        document.getElementById('cfg-invS').checked = (d.invS === 1);
        document.getElementById('cfg-stealth').checked = (d.stealth === 1);
        
        document.getElementById('maxsp').value = d.maxSp;
        document.getElementById('maxsp-v').textContent = d.maxSp + '%';
        maxPct = d.maxSp;
        document.getElementById('robot-logo').textContent = '🤖 ' + d.name.toUpperCase();
        applyTheme(d.color);
        
        if(d.fType !== undefined) {
             document.getElementById('cfg-faceType').value = d.fType;
             document.getElementById('cfg-fEyeShape').value = d.fEyeShape || 0;
             document.getElementById('cfg-fMouth').value = d.fMouth || 0;
             document.getElementById('cfg-fAcc').value = d.fAcc || 0;
             
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

// ===== ACHIEVEMENTS =====
function loadAch() { wsSend({c:'getAch'}); }
function renderAchievements(list) {
  const html = list.map(a => `
    <div class="ach-card ${a.u ? 'unlocked' : ''}">
      <div class="ach-info">
        <div class="ach-title">${a.u ? '⭐' : '🔒'} ${a.n}</div>
        <div class="ach-desc">${a.p} / ${a.t}</div>
      </div>
    </div>
  `).join('');
  document.getElementById('ach-list').innerHTML = html;
}

// ===== SPEECH BUBBLE =====
function sendSpeech() {
  const txt = document.getElementById('speech-txt').value.trim();
  if (txt) {
    wsSend({c:'speech', t:txt});
    document.getElementById('speech-txt').value = '';
  }
}

// ===== FACE CUSTOMIZER =====
function sendFaceCust(){
  wsSend({
    c:'faceCust',
    eye: parseInt(document.getElementById('cfg-fEyeShape').value),
    mouth: parseInt(document.getElementById('cfg-fMouth').value),
    acc: parseInt(document.getElementById('cfg-fAcc').value)
  });
  // Also send faceType update just in case
  const ft = parseInt(document.getElementById('cfg-faceType').value);
  wsSend({c:'faceCfg', type:ft, eye:parseInt(document.getElementById('cfg-fEye').value), blink:50, bounce:50});
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

// ===== JOYSTICK =====
const joy=document.getElementById('joy');
const thumb=document.getElementById('joy-thumb');
const arrows={n:document.getElementById('da-n'),s:document.getElementById('da-s'),e:document.getElementById('da-e'),w:document.getElementById('da-w')};

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
  const nx=dx/maxR,ny=-dy/maxR;
  sThrottle = Math.round(ny * maxPct);
  sSteer = Math.round(nx * 100);
  document.getElementById('sp-throttle').textContent=sThrottle;
  document.getElementById('sp-steer').textContent=sSteer;
  updateGauge('svg-throttle', sThrottle);
  updateGauge('svg-steer', sSteer);
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

joy.addEventListener('pointerdown',e=>{joyActive=true;thumb.classList.add('active');joy.setPointerCapture(e.pointerId);joyMove(e.clientX,e.clientY);});
joy.addEventListener('pointermove',e=>{if(joyActive)joyMove(e.clientX,e.clientY);});
joy.addEventListener('pointerup',()=>{joyActive=false;joyReset();});
joy.addEventListener('pointercancel',()=>{joyActive=false;joyReset();});

let lastSentT = null, lastSentS = null;
setInterval(()=>{
  if(joyActive && (sThrottle!==lastSentT || sSteer!==lastSentS)){
    wsSend({c:'m', t:sThrottle, s:sSteer});
    lastSentT=sThrottle; lastSentS=sSteer;
  }
},50);

document.getElementById('maxsp').addEventListener('input',(e)=>{
  maxPct=parseInt(e.target.value);
  document.getElementById('maxsp-v').textContent=maxPct+'%';
  wsSend({c:'cfg', maxSp:maxPct});
});

['pt-pan','pt-tilt'].forEach(id=>{
  const el = document.getElementById(id);
  const valEl = document.getElementById(id+'-v');
  el.addEventListener('input', ()=>{
    valEl.textContent = el.value;
    wsSend({c:'pt', p:parseInt(document.getElementById('pt-pan').value), t:parseInt(document.getElementById('pt-tilt').value)});
  });
  el.addEventListener('change', ()=>{
    el.value = 0;
    wsSend({c:'pt', p:0, t:0});
  });
});

function ptPreset(p) { wsSend({c:'ptPreset', p:p}); }

function qAct(a){
  if(a==='fwd'){sThrottle=maxPct;sSteer=0;}
  else if(a==='rev'){sThrottle=-maxPct;sSteer=0;}
  else if(a==='spin'){sThrottle=30;sSteer=100;}
  else if(a==='horn'){wsSend({c:'snd',s:'horn'});return;}
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

function sendSnd(s){wsSend({c:'snd',s:s});}
function sendEmo(e){wsSend({c:'emo',e:e});}

function sendCfg(){
  wsSend({c:'cfg',
    invM:document.getElementById('cfg-invM').checked?1:0,
    invS:document.getElementById('cfg-invS').checked?1:0,
    stealth:document.getElementById('cfg-stealth').checked?1:0
  });
}

function saveName(){
  const n = document.getElementById('cfg-name').value.trim();
  const c = parseInt(document.getElementById('cfg-color').value);
  if(n){
    wsSend({c:'name', n:n});
    wsSend({c:'color', v:c});
    document.getElementById('robot-logo').textContent = '🤖 ' + n.toUpperCase();
    applyTheme(c);
  }
}

function saveWiFi(){
  const s = document.getElementById('wifi-ssid').value.trim();
  const p = document.getElementById('wifi-pass').value;
  if(s){ wsSend({c:'wifi', s:s, p:p}); alert('Credentials saved! Rebooting...'); }
}

document.getElementById('btn-estop').addEventListener('click',()=>{
  wsSend({c:'stop'}); sThrottle=0;sSteer=0;
  document.getElementById('sp-throttle').textContent='0';
  document.getElementById('sp-steer').textContent='0';
  updateGauge('svg-throttle', 0); updateGauge('svg-steer', 0);
  ['pt-pan','pt-tilt'].forEach(id=>{
    document.getElementById(id).value=0;
    document.getElementById(id+'-v').textContent='0';
  });
  if(navigator.vibrate)navigator.vibrate(200);
});

// ===== VOICE CHANGER =====
let audioCtx = null;
let micStream = null;
let sourceNode = null;
let biquadFilter = null;
let pitchShiftNode = null; // Fake it with playbackRate logic if using AudioBuffer
const btnMic = document.getElementById('btn-mic');

btnMic.addEventListener('pointerdown', async (e) => {
  e.preventDefault();
  if (navigator.mediaDevices && navigator.mediaDevices.getUserMedia) {
    try {
      micStream = await navigator.mediaDevices.getUserMedia({ audio: true });
      audioCtx = new (window.AudioContext || window.webkitAudioContext)();
      sourceNode = audioCtx.createMediaStreamSource(micStream);
      
      const effect = document.getElementById('vc-effect').value;
      biquadFilter = audioCtx.createBiquadFilter();
      
      // Simple routing to phone speaker (Web Audio handles output)
      if (effect === 'robot') {
        biquadFilter.type = 'lowpass';
        biquadFilter.frequency.value = 800;
        const osc = audioCtx.createOscillator();
        osc.type = 'sawtooth';
        osc.frequency.value = 50;
        const gain = audioCtx.createGain();
        osc.connect(gain);
        gain.connect(biquadFilter.detune);
        osc.start();
      } else if (effect === 'chipmunk') {
        biquadFilter.type = 'highpass';
        biquadFilter.frequency.value = 1500;
      } else if (effect === 'monster') {
        biquadFilter.type = 'lowpass';
        biquadFilter.frequency.value = 300;
      } else if (effect === 'echo') {
        biquadFilter = audioCtx.createDelay(1.0);
        biquadFilter.delayTime.value = 0.3;
        const fb = audioCtx.createGain();
        fb.gain.value = 0.6;
        biquadFilter.connect(fb);
        fb.connect(biquadFilter);
      } else {
        biquadFilter.type = 'allpass';
      }
      
      sourceNode.connect(biquadFilter);
      biquadFilter.connect(audioCtx.destination);
      
      btnMic.classList.add('recording');
      btnMic.textContent = "🎙️ TALKING...";
      wsSend({c:'emo', e:'happy'}); // Make robot face animate
    } catch (err) {
      alert("Mic access denied or unsupported.");
    }
  }
});

function stopMic() {
  if (micStream) {
    micStream.getTracks().forEach(t => t.stop());
    micStream = null;
  }
  if (audioCtx) {
    audioCtx.close();
    audioCtx = null;
  }
  btnMic.classList.remove('recording');
  btnMic.textContent = "🎤 HOLD TO TALK";
  wsSend({c:'emo', e:'idle'}); // Stop animation
}

btnMic.addEventListener('pointerup', stopMic);
btnMic.addEventListener('pointercancel', stopMic);
btnMic.addEventListener('pointerleave', stopMic);

</script>
</body>
</html>
)rawliteral";
