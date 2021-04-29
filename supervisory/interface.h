const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP32 - Supervisório</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    p { font-size: 1.2rem;}
    body {  margin: 0;}
    .topnav { overflow: hidden; background-color: #50B8B4; color: white; font-size: 1rem; }
    .content { padding: 20px; }
    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); }
    .cards { max-width: 800px; margin: 0 auto; display: grid; grid-gap: 2rem; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); }
    .reading { font-size: 1.4rem; }
  </style>
</head>
<body>
  <div class="topnav">
    <h1>ESP32 - Sistema Supervisorio</h1>
    <h3> Alunos: Arthur Lima e Lukas Lorenz <h3>
  </div>
  <div class="content">
    <div class="cards">
      <div class="card">
        <p><i class="fas fa-thermometer-half" style="color:#059e8a;"></i> TEMPERATURE</p>
        <p><span class="reading"><span id="temp">%TEMPERATURE%</span> &deg;C</span></p>
      </div>
      <div class="card">
        <p><i class="fas fa-tint" style="color:#00add6;"></i> HUMIDITY</p>
        <p><span class="reading"><span id="hum">%HUMIDITY%</span> &percnt;</span></p>
      </div>
      <div class="card">
        <p><i class="fas fa-angle-double-down" style="color:#e1e437;"></i> PRESSURE</p>
        <p><span class="reading"><span id="pres">%PRESSURE%</span> hPa</span></p>
      </div>
    </div>
    %BUTTONPLACEHOLDER%
  </div>
<script>
// FUNÇÔES REATIVIDADE LED
function controlOutput(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/output?state=1", true); }
  else { xhr.open("GET", "/output?state=0", true); }
  xhr.send();
}
function toggleLed(element) {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/toggle", true);
  xhr.send();
}
// 
if (!!window.EventSource) {
 var source = new EventSource('/events'); 

 source.addEventListener('open', function(e) {
  console.log("Events Connected");
 }, false);
 source.addEventListener('error', function(e) {
  if (e.target.readyState != EventSource.OPEN) {
    console.log("Events Disconnected");
  }
 }, false);
 
 source.addEventListener('message', function(e) {
  console.log("message", e.data);
 }, false);
 
 source.addEventListener('temperature', function(e) {
  console.log("temperature", e.data);
  document.getElementById("temp").innerHTML = e.data;
 }, false);
 
 source.addEventListener('humidity', function(e) {
  console.log("humidity", e.data);
  document.getElementById("hum").innerHTML = e.data;
 }, false);
 
 source.addEventListener('pressure', function(e) {
  console.log("pressure", e.data);
  document.getElementById("pres").innerHTML = e.data;
 }, false);
 source.addEventListener('led_state', function(e) {
  console.log("led_state", e.data);
  var inputChecked;
  if( e.data == 1){ inputChecked = true; }
  else { inputChecked = false; }
  document.getElementById("led").checked = inputChecked;
 }, false);
}
</script>
</body>
</html>)rawliteral";
