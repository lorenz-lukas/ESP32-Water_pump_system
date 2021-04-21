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
        <p><i class="fas fa-thermometer-half" style="color:#059e8a;"></i> ESTADO</p>
        
        <p>
          <span class="reading">
            <span id="state">%state:%
            </span> 
          </span>
        </p> 
      </div>
      <div class="card">
        <p><i class="fas fa-tint" style="color:#00add6;"></i> ESTADO BOMBA</p>
        
        <p>
          <span class="reading">
            <span id="state_pump">%state_pump%
            </span> 
          </span>
        </p>
      </div>
      <div class="card">
        <p><i class="fas fa-angle-double-down" style="color:#e1e437;"></i> ATIVA BOMBA</p>
        
        <p>
          <span class="reading">
            <span id="toggle_pump">%toggle_pump%
            </span>
          </span>
        </p>
      </div>
    </div>
  </div>
<script>
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
 
 source.addEventListener('state', function(e) {
  console.log("state", e.data);
  document.getElementById("state").innerHTML = e.data;
 }, false);
 
 source.addEventListener('state_pump', function(e) {
  console.log("state_pump", e.data);
  document.getElementById("state_pump").innerHTML = e.data;
 }, false);
 
 source.addEventListener('toggle_pump', function(e) {
  console.log("toggle_pump", e.data);
  document.getElementById("toggle_pump").innerHTML = e.data;
 }, false);
}
</script>
</body>
</html>)rawliteral";
