#include "Server/Dashboard.h"
#undef CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <algorithm>

DashboardServer::DashboardServer(std::uint16_t port)
    : m_port(port)
{
}

DashboardServer::~DashboardServer()
{
    stop();
}

void DashboardServer::start()
{
    m_running = true;
    m_thread = std::thread(&DashboardServer::run, this);
    std::cout << "[Dashboard] HTTP dashboard running at http://localhost:" << m_port << std::endl;
}

void DashboardServer::stop()
{
    m_running = false;
    if (m_thread.joinable())
        m_thread.join();
}

void DashboardServer::updateStats(const DashboardStats& stats)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_stats = stats;
    // Sync the rolling log into stats so buildJson can see it
    m_stats.recentLog.clear();
    for (const auto& entry : m_logBuffer)
        m_stats.recentLog.push_back(entry);
}

void DashboardServer::pushLog(const std::string& message)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_logBuffer.push_front(message);
    if (m_logBuffer.size() > 20)
        m_logBuffer.pop_back();
}

void DashboardServer::run()
{
    httplib::Server svr;

    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(serveHtml(), "text/html");
        });

    svr.Get("/stats", [this](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(m_mutex);
        res.set_content(buildJson(), "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");
        });

    svr.listen("0.0.0.0", m_port);
}

std::string DashboardServer::buildJson() const
{
    std::ostringstream j;
    j << "{";
    j << "\"playerCount\":" << m_stats.playerCount << ",";
    j << "\"maxPlayers\":" << m_stats.maxPlayers << ",";
    j << "\"enemyCount\":" << m_stats.enemyCount << ",";
    j << "\"npcCount\":" << m_stats.npcCount << ",";
    j << "\"uptime\":" << m_stats.uptimeSeconds << ",";
    j << "\"seed\":" << m_stats.worldSeed << ",";
    j << "\"gameState\":\"" << m_stats.gameState << "\",";
    j << "\"packetsPerSec\":" << m_stats.packetsPerSec << ",";

    j << "\"players\":[";
    for (size_t i = 0; i < m_stats.players.size(); ++i)
    {
        const auto& p = m_stats.players[i];
        if (i > 0) j << ",";
        j << "{";
        j << "\"id\":" << p.id << ",";
        j << "\"name\":\"" << p.name << "\",";
        j << "\"x\":" << static_cast<int>(p.x) << ",";
        j << "\"y\":" << static_cast<int>(p.y) << ",";
        j << "\"health\":" << static_cast<int>(p.health) << ",";
        j << "\"isHost\":" << (p.isHost ? "true" : "false");
        j << "}";
    }
    j << "],";

    j << "\"log\":[";
    for (size_t i = 0; i < m_stats.recentLog.size(); ++i)
    {
        if (i > 0) j << ",";
        // Escape quotes in log strings
        std::string entry = m_stats.recentLog[i];
        std::string escaped;
        for (char c : entry)
        {
            if (c == '"') escaped += "\\\"";
            else if (c == '\\') escaped += "\\\\";
            else escaped += c;
        }
        j << "\"" << escaped << "\"";
    }
    j << "]";

    j << "}";
    return j.str();
}

std::string DashboardServer::serveHtml()
{
    return R"HTML(<!DOCTYPE html>
<html lang="en">
    <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Lands Of P(ixel) - Server Dashboard</title>
 
        <style>
          *{box-sizing:border-box;margin:0;padding:0}
          body{background:#0f1932;color:#e8d5a0;font-family:'Courier New',Courier,monospace;font-size:13px;padding:20px;min-height:100vh}
          .stars{position:fixed;top:0;left:0;width:100%;height:100%;pointer-events:none;z-index:0}
          .content{position:relative;z-index:1}
          .header{text-align:center;padding:20px 0 24px;border-bottom:2px solid #daa520;margin-bottom:24px}
          .title{font-size:22px;color:#daa520;letter-spacing:2px;text-shadow:2px 2px 0 #7a4a1e;margin-bottom:8px}
          .title-line{height:2px;background:#daa520;margin:8px auto;width:60%}
          .subtitle{font-size:11px;color:#c8a84a;margin-top:6px;letter-spacing:1px}
          .status-dot{display:inline-block;width:8px;height:8px;border-radius:50%;background:#3fb950;margin-right:6px;animation:pulse 2s infinite}
          @keyframes pulse{0%,100%{opacity:1}50%{opacity:0.3}}
          .stats-grid{display:grid;grid-template-columns:repeat(4,1fr);gap:12px;margin-bottom:24px}
          .stat{background:#3d2a0f;border:2px solid #7a4a1e;padding:14px 10px;text-align:center}
          .stat-label{color:#c8a84a;font-size:10px;margin-bottom:10px;letter-spacing:1px}
          .stat-value{font-size:22px;color:#daa520;text-shadow:1px 1px 0 #7a4a1e}
          .section{margin-bottom:20px}
          .section-title{color:#daa520;font-size:11px;letter-spacing:2px;margin-bottom:10px;border-bottom:1px solid #7a4a1e;padding-bottom:6px}
          .panel{background:#5c3a12;border:2px solid #8b5e1a;padding:12px}
          .player-row{display:flex;align-items:center;gap:10px;background:#3d2a0f;border:1px solid #7a4a1e;padding:8px 10px;margin-bottom:6px;font-size:12px}
          .player-id{color:#c8a84a;min-width:24px}
          .player-name{flex:1;color:#e8d5a0}
          .host-badge{font-size:10px;padding:2px 6px;background:#4b3580;color:#daa520;border:1px solid #daa520}
          .hp-wrap{width:60px;height:6px;background:#1a0f00;border:1px solid #7a4a1e;overflow:hidden}
          .hp-fill{height:100%;transition:width 0.5s}
          .hp-high{background:#3fb950}
          .hp-mid{background:#daa520}
          .hp-low{background:#c0392b}
          .player-pos{color:#c8a84a;min-width:120px;text-align:right;font-size:11px}
          .two-col{display:grid;grid-template-columns:1fr 1fr;gap:14px}
          .log-entry{padding:4px 0;border-bottom:1px solid #7a4a1e;font-size:11px;line-height:1.8}
          .log-entry:last-child{border-bottom:none}
          .log-time{color:#c8a84a;margin-right:6px}
          .log-ok{color:#3fb950}
          .log-info{color:#daa520}
          .log-warn{color:#c0392b}
          .no-players{color:#c8a84a;padding:12px 0;text-align:center;font-size:11px}
          .world-row{display:flex;justify-content:space-between;padding:5px 0;border-bottom:1px solid #7a4a1e;font-size:11px}
          .world-row:last-child{border-bottom:none}
          .world-key{color:#c8a84a}
          .world-val{color:#daa520}
        </style>
    </head>
<body>
    <canvas class="stars" id="stars"></canvas>
    <div class="content">
 
        <div class="header">
          <div class="title-line"></div>
          <div class="title">Lands OF P(ixel)</div>
          <div class="title-line"></div>
          <div class="subtitle"><span class="status-dot"></span>server dashboard  -  port 53001</div>
        </div>
 
        <div class="stats-grid">
          <div class="stat"><div class="stat-label">players</div><div class="stat-value" id="s-players">-</div></div>
          <div class="stat"><div class="stat-label">uptime</div><div class="stat-value" id="s-uptime">-</div></div>
          <div class="stat"><div class="stat-label">packets/s</div><div class="stat-value" id="s-pps">-</div></div>
          <div class="stat"><div class="stat-label">enemies</div><div class="stat-value" id="s-enemies">-</div></div>
        </div>
 
        <div class="section">
          <div class="section-title">connected players</div>
          <div id="players-list"><div class="no-players">no players connected</div></div>
        </div>
 
        <div class="two-col">
          <div class="section">
            <div class="section-title">server log</div>
            <div class="panel" id="log-box"></div>
          </div>
          <div class="section">
            <div class="section-title">world info</div>
            <div class="panel" id="world-box"></div>
          </div>
        </div>
 
    </div>
        <script>
        const canvas = document.getElementById('stars');
        const ctx = canvas.getContext('2d');
        function resizeStars() {
          canvas.width = window.innerWidth;
          canvas.height = window.innerHeight;
          ctx.clearRect(0,0,canvas.width,canvas.height);
          for(let i=0;i<80;i++){
            ctx.fillStyle='rgba(255,255,255,'+(Math.random()*0.5+0.1)+')';
            ctx.beginPath();
            ctx.arc(Math.random()*canvas.width,Math.random()*canvas.height,Math.random()*1.5,0,Math.PI*2);
            ctx.fill();
          }
        }
        resizeStars();
        window.addEventListener('resize', resizeStars);
 
        function fmtUptime(s){
          const h=Math.floor(s/3600),m=Math.floor((s%3600)/60),ss=s%60;
          return String(h).padStart(2,'0')+':'+String(m).padStart(2,'0')+':'+String(ss).padStart(2,'0');
        }
        function hpClass(hp){ return hp>60?'hp-high':hp>30?'hp-mid':'hp-low'; }
        function logClass(msg){
          const l=msg.toLowerCase();
          if(l.includes('error')||l.includes('fail')||l.includes('timeout')) return 'log-warn';
          if(l.includes('host')||l.includes('seed')||l.includes('start')||l.includes('transfer')) return 'log-info';
          return 'log-ok';
        }
 
        async function refresh(){
          try{
            const r=await fetch('/stats');
            const d=await r.json();
 
            document.getElementById('s-players').textContent=d.playerCount+'/'+d.maxPlayers;
            document.getElementById('s-uptime').textContent=fmtUptime(d.uptime);
            document.getElementById('s-pps').textContent=d.packetsPerSec;
            document.getElementById('s-enemies').textContent=d.enemyCount;
 
            const list=document.getElementById('players-list');
            if(!d.players.length){
              list.innerHTML='<div class="no-players">no players connected</div>';
            } else {
              list.innerHTML=d.players.map(p=>{
                const hp=Math.min(100,Math.max(0,p.health));
                return '<div class="player-row">'
                  +'<span class="player-id">#'+p.id+'</span>'
                  +'<span class="player-name">'+p.name+'</span>'
                  +(p.isHost?'<span class="host-badge">host</span>':'')
                  +'<div class="hp-wrap"><div class="hp-fill '+hpClass(hp)+'" style="width:'+hp+'%"></div></div>'
                  +'<span class="player-pos">('+p.x+', '+p.y+')</span>'
                  +'</div>';
              }).join('');
            }
 
            document.getElementById('log-box').innerHTML=d.log.map(l=>
              '<div class="log-entry"><span class="'+logClass(l)+'">'+l+'</span></div>'
            ).join('')||'<div class="log-entry" style="color:#c8a84a">no events yet</div>';
 
            document.getElementById('world-box').innerHTML=
              '<div class="world-row"><span class="world-key">seed</span><span class="world-val">'+(d.seed||'not set')+'</span></div>'
              +'<div class="world-row"><span class="world-key">state</span><span class="world-val">'+d.gameState+'</span></div>'
              +'<div class="world-row"><span class="world-key">npcs</span><span class="world-val">'+d.npcCount+'</span></div>'
              +'<div class="world-row"><span class="world-key">enemies</span><span class="world-val">'+d.enemyCount+'</span></div>';
 
          } catch(e){
            document.querySelector('.subtitle').textContent='connection lost - retrying...';
          }
        }
 
        refresh();
        setInterval(refresh,1000);
        </script>
    </body>
</html>)HTML";
}