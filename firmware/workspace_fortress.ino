#include <WiFi.h>
#include <WebServer.h>
#include "DHT.h"
#include <ESP32Servo.h>

const char* ssid     = "YOUR_NETWORK_SSID";     
const char* password = "YOUR_NETWORK_PASSWORD"; 

#define TRIG_PIN 18  
#define ECHO_PIN 19  
#define DHTPIN 4     
#define DHTTYPE DHT11
#define LDR_PIN 34   
#define SERVO_PIN 26 
#define ACTIVE_BUZZER 25   
#define VIBRA_MOTOR 27     
#define PASSIVE_BUZZER 32  

DHT dht(DHTPIN, DHTTYPE);
Servo statusDial; 
WebServer server(80); 

float slouchThresholdCm = 50.0;          
float thermalMaxLimitCelsius = 30.0;     
const int passiveBuzzerChannel = 2;

int pomodoroState = 0;                   
int slouchCounter = 0;
bool isSlouching = false;
unsigned long slouchStartTime = 0;

unsigned long lastMovementTime = 0;
float lastStoredDistance = 0.0;
bool staticFatigueAlert = false;

unsigned long absenceStartTime = 0;
bool isAbsent = false;
int distractionEvents = 0;

bool thermalAlertActive = false;
bool contrastAlertActive = false;
unsigned long lastContrastChimeTime = 0;
unsigned long lastMildHeatChimeTime = 0;

String getHTMLPage() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>Fortress OS v2.0</title>";
  html += "<style>body{font-family:-apple-system,sans-serif; text-align:center; background:#0d0e12; color:#e1e1e6; padding:10px;}";
  html += ".card{background:#16171d; padding:20px; border-radius:12px; margin:15px auto; max-width:460px; border: 1px solid #232530; text-align:left;}";
  html += ".btn{color:white; padding:12px 24px; font-size:14px; border-radius:6px; display:inline-block; margin:5px; text-decoration:none; font-weight:bold; cursor:pointer; border:none; transition:0.2s;}";
  html += ".btn-start{background:#2a9d8f;} .btn-start:hover{background:#1f7369;}";
  html += ".btn-stop{background:#e63946;} .btn-stop:hover{background:#b52d37;}";
  html += ".btn-reset{background:#f4a261;} .btn-reset:hover{background:#c98449;}";
  html += ".metric{font-size:24px; font-weight:bold; color:#f4a261; float:right;} .label-title{font-weight:600; color:#9aa0a6;}";
  html += "input[type=range]{width:100%; margin:12px 0; accent-color:#2a9d8f;}";
  html += ".alert-banner{background:#e63946; padding:12px; border-radius:8px; font-weight:bold; text-align:center; margin-bottom:15px; display:none;}";
  html += ".warn-banner{background:#e9c46a; color:#121214; padding:12px; border-radius:8px; font-weight:bold; text-align:center; margin-bottom:15px; display:none;}</style>";
  
  html += "<script>";
  html += "var timerInstance; var timeLeft = 1500; var isRunning = false;";
  
  html += "function startPomodoro() {";
  html += "  if(!isRunning) { isRunning = true; fetch('/pomo-state?mode=1'); ";
  html += "    timerInstance = setInterval(function() {";
  html += "      if(timeLeft <= 0) { clearInterval(timerInstance); isRunning = false; fetch('/pomo-state?mode=2'); alert('Focus Session Complete! Commencing research-backed 5-minute cognitive reset break.'); }";
  html += "      else { timeLeft--; updateTimerUI(); }";
  html += "    }, 1000);";
  html += "  }";
  html += "}";
  
  html += "function stopPomodoro() { isRunning = false; clearInterval(timerInstance); fetch('/pomo-state?mode=0'); }";
  html += "function resetPomodoro() { stopPomodoro(); timeLeft = 1500; updateTimerUI(); }";
  html += "function updateTimerUI() { var mins = Math.floor(timeLeft / 60); var secs = timeLeft % 60; document.getElementById('timer_display').innerHTML = (mins < 10 ? '0':'') + mins + ':' + (secs < 10 ? '0':'') + secs; }";
  
  html += "function pollFortressNode() {";
  html += "  var xhttp = new XMLHttpRequest();";
  html += "  xhttp.onreadystatechange = function() {";
  html += "    if (this.readyState == 4 && this.status == 200) {";
  html += "      var data = JSON.parse(this.responseText);";
  html += "      document.getElementById('dist_val').innerHTML = data.distance + ' cm';";
  html += "      document.getElementById('temp_val').innerHTML = data.temp + ' &deg;C';";
  html += "      document.getElementById('slouch_val').innerHTML = data.slouches;";
  html += "      document.getElementById('distract_val').innerHTML = data.distractions;";
  
  html += "      var luxStr = data.lux + ' Raw ';";
  html += "      if(data.lux < 1250) luxStr += '(Too Dark - Strain Hazard)';";
  html += "      else if(data.lux <= 3300) luxStr += '(Optimal Illuminance)';";
  html += "      else luxStr += '(Excessive Glare Hazard)';";
  html += "      document.getElementById('lux_val').innerHTML = luxStr;";
  
  html += "      var score = 100 - (data.slouches * 5) - (data.distractions * 10); if(score < 0) score = 0;";
  html += "      document.getElementById('score_val').innerHTML = score + '%';";
  
  html += "      localStorage.setItem('fortress_cached_slouches', data.slouches);";
  html += "      localStorage.setItem('fortress_cached_score', score);";
  
  html += "      document.getElementById('thermal_banner').style.display = (data.thermal_alert == 1) ? 'block' : 'none';";
  html += "      document.getElementById('fatigue_banner').style.display = (data.fatigue_alert == 1) ? 'block' : 'none';";
  html += "    }";
  html += "  };";
  html += "  xhttp.open('GET', '/readSensors', true);";
  html += "  xhttp.send();";
  html += "}";
  
  html += "function updateSliders() {";
  html += "  var slouch = document.getElementById('slouch_range').value;";
  html += "  var temp = document.getElementById('temp_range').value;";
  html += "  document.getElementById('slouch_lbl').innerHTML = slouch + ' cm';";
  html += "  document.getElementById('temp_lbl').innerHTML = temp + ' &deg;C';";
  html += "  var xhttp = new XMLHttpRequest();";
  html += "  xhttp.open('GET', '/calibrate?slouch=' + slouch + '&temp=' + temp, true);";
  html += "  xhttp.send();";
  html += "}";
  
  html += "setInterval(pollFortressNode, 1000);";
  html += "</script>";
  
  html += "</head><body>";
  html += "<h2>Workspace Fortress: Privacy-First OS</h2>";
  html += "<div id='thermal_banner' class='alert-banner'>CRITICAL HEAT ALERT (COGNITIVE DROP): ENFORCING ACTIVE SIREN OVERRIDE</div>";
  html += "<div id='fatigue_banner' class='warn-banner'>STATIC FATIGUE WARNING: YOU HAVE HELD THIS POSITION FOR 20 MINS. SHIFT POSTURE NOW.</div>";
  
  html += "<div class='card'><h3>Pomodoro Focus Synchronization</h3>";
  html += "<p><span class='label-title'>Time Block Remaining:</span> <span id='timer_display' class='metric' style='font-size:28px; color:#e9c46a;'>25:00</span></p><br>";
  html += "<button onclick='startPomodoro()' class='btn btn-start'>Start Sprint</button>";
  html += "<button onclick='stopPomodoro()' class='btn btn-stop'>Pause Block</button>";
  html += "<button onclick='resetPomodoro()' class='btn btn-reset'>Reset Layout</button></div>";
  
  html += "<div class='card'><h3>Non-Optical Telemetry Array</h3>";
  html += "<p><span class='label-title'>AOA Radar Distance:</span> <span id='dist_val' class='metric'>--</span></p>";
  html += "<p><span class='label-title'>Cognitive Microclimate:</span> <span id='temp_val' class='metric'>--</span></p>";
  html += "<p><span class='label-title'>Asymmetric Optical Contrast:</span> <span id='lux_val' class='metric' style='font-size:16px;'>--</span></p></div>";

  html += "<div class='card'><h3>Biomechanical Habit Analytics</h3>";
  html += "<p><span class='label-title'>Ergonomic Distance Violations:</span> <span id='slouch_val' class='metric'>0</span></p>";
  html += "<p><span class='label-title'>Desk Absence/Distraction Counts:</span> <span id='distract_val' class='metric'>0</span></p>";
  html += "<p><span class='label-title'>Aggregate Workspace Health Score:</span> <span id='score_val' class='metric' style='color:#2a9d8f;'>100%</span></p></div>";
  
  html += "<div class='card'><h3>Over-The-Air Hardware Calibration</h3>";
  html += "<p><span class='label-title'>Slouch Distance Cutoff:</span> <span id='slouch_lbl' class='metric'>50 cm</span></p>";
  html += "<input type='range' id='slouch_range' min='30' max='80' value='50' oninput='updateSliders()'>";
  html += "<p><span class='label-title'>Thermal Alarm Trigger:</span> <span id='temp_lbl' class='metric'>30 &deg;C</span></p>";
  html += "<input type='range' id='temp_range' min='22' max='45' value='30' oninput='updateSliders()'></div>";
  
  html += "</body></html>";
  return html;
}

void handleRoot() { server.send(200, "text/html", getHTMLPage()); }

void handleJSONData() {
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  float distance = pulseIn(ECHO_PIN, HIGH) * 0.034 / 2;
  if(distance > 400 || distance < 2) distance = 0;

  float temperature = dht.readTemperature();
  int ldrValue = analogRead(LDR_PIN);
  if (isnan(temperature)) temperature = 0.0;

  String json = "{\"distance\":" + String(distance, 1) + ",";
  json += "\"temp\":" + String(temperature, 1) + ",";
  json += "\"lux\":" + String(ldrValue) + ",";
  json += "\"slouches\":" + String(slouchCounter) + ",";
  json += "\"distractions\":" + String(distractionEvents) + ",";
  json += "\"thermal_alert\":" + String(thermalAlertActive ? 1 : 0) + ",";
  json += "\"fatigue_alert\":" + String(staticFatigueAlert ? 1 : 0);
  json += "}";
  server.send(200, "application/json", json);
}

void handleCalibration() {
  if (server.hasArg("slouch")) slouchThresholdCm = server.arg("slouch").toFloat();
  if (server.hasArg("temp")) thermalMaxLimitCelsius = server.arg("temp").toFloat();
  server.send(200, "text/plain", "OK");
}

void handlePomoState() {
  if (server.hasArg("mode")) {
    pomodoroState = server.arg("mode").toInt();
    if(pomodoroState == 1) { 
      ledcWriteTone(PASSIVE_BUZZER, 600); delay(100); ledcWriteTone(PASSIVE_BUZZER, 800); delay(150); ledcWriteTone(PASSIVE_BUZZER, 0);
      lastMovementTime = millis(); staticFatigueAlert = false;
    } else if(pomodoroState == 2) { 
      ledcWriteTone(PASSIVE_BUZZER, 800); delay(100); ledcWriteTone(PASSIVE_BUZZER, 1000); delay(200); ledcWriteTone(PASSIVE_BUZZER, 0);
    }
  }
  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(115200);
  delay(1000); 
  Serial.println("INIT_PINS");
  
  pinMode(TRIG_PIN, OUTPUT); pinMode(ECHO_PIN, INPUT); pinMode(LDR_PIN, INPUT);
  pinMode(ACTIVE_BUZZER, OUTPUT); pinMode(VIBRA_MOTOR, OUTPUT);
  
  digitalWrite(ACTIVE_BUZZER, LOW); digitalWrite(VIBRA_MOTOR, LOW);
  
  Serial.println("INIT_SERVO");
  ESP32PWM::allocateTimer(2);    
  statusDial.setPeriodHertz(50);    
  statusDial.attach(SERVO_PIN, 500, 2400); 
  statusDial.write(0); 

  Serial.println("INIT_LEDC");
  ledcAttachChannel(PASSIVE_BUZZER, 2000, 8, passiveBuzzerChannel);

  Serial.println("INIT_WIFI");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
  }
  
  Serial.println("");
  Serial.println("WIFI_OK");
  Serial.println(WiFi.localIP());

  Serial.println("INIT_DHT");
  dht.begin(); 

  Serial.println("INIT_SERVER");
  server.on("/", handleRoot);
  server.on("/readSensors", handleJSONData);
  server.on("/calibrate", handleCalibration);
  server.on("/pomo-state", handlePomoState);
  server.begin();
  
  Serial.println("SYS_RUNNING");
}

void loop() {
  server.handleClient(); 
  
  float temperature = dht.readTemperature();
  int currentLight = analogRead(LDR_PIN);
  
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  float currentDistance = pulseIn(ECHO_PIN, HIGH) * 0.034 / 2;
  if (currentDistance > 400 || currentDistance < 2) currentDistance = 0;

  // --- Thermal Safety Matrix ---
  if (!isnan(temperature) && temperature >= thermalMaxLimitCelsius) {
    thermalAlertActive = true;
    statusDial.write(180);              
    digitalWrite(ACTIVE_BUZZER, HIGH);  
  } else {
    if(thermalAlertActive) {
      thermalAlertActive = false;
      digitalWrite(ACTIVE_BUZZER, LOW);
      statusDial.write(0);
    }
  }

  // --- Contrast Guard & Mild Heat Chimes (Pomodoro Active Only) ---
  if (!thermalAlertActive && pomodoroState == 1) {
    if (currentLight < 1250) { 
      if (millis() - lastContrastChimeTime >= 300000) { 
        contrastAlertActive = true;
        ledcWriteTone(PASSIVE_BUZZER, 400); delay(150); ledcWriteTone(PASSIVE_BUZZER, 300); delay(150); ledcWriteTone(PASSIVE_BUZZER, 0);
        lastContrastChimeTime = millis();
      }
    } else { contrastAlertActive = false; }

    if (temperature >= 25.0 && temperature < thermalMaxLimitCelsius) {
      if (millis() - lastMildHeatChimeTime >= 300000) { 
        ledcWriteTone(PASSIVE_BUZZER, 520); delay(80); ledcWriteTone(PASSIVE_BUZZER, 0); delay(50); ledcWriteTone(PASSIVE_BUZZER, 520); delay(80); ledcWriteTone(PASSIVE_BUZZER, 0);
        lastMildHeatChimeTime = millis();
      }
    }
  }

  // --- Servo State Machine & Biomechanical Monitoring ---
  if (!thermalAlertActive) {
    if (pomodoroState == 1) {
      statusDial.write(90); // Focus mode: needle at 90°
      
      // Absence / Distraction Detection
      if (currentDistance == 0 || currentDistance > 120.0) {
        if (!isAbsent) { absenceStartTime = millis(); isAbsent = true; }
        else if (millis() - absenceStartTime >= 10000) { 
          distractionEvents++;
          pomodoroState = 0; 
          isAbsent = false;
          ledcWriteTone(PASSIVE_BUZZER, 250); delay(400); ledcWriteTone(PASSIVE_BUZZER, 0); 
        }
      } else { isAbsent = false; }

      // AOA Proximity / Slouch Guard
      if (currentDistance > 0 && currentDistance < slouchThresholdCm) {
        if (!isSlouching) { slouchStartTime = millis(); isSlouching = true; } 
        else if (millis() - slouchStartTime >= 3000) { 
          digitalWrite(VIBRA_MOTOR, HIGH); delay(200); digitalWrite(VIBRA_MOTOR, LOW); delay(300); 
          slouchCounter++;
          slouchStartTime = millis(); 
        }
      } else {
        if (isSlouching) { isSlouching = false; digitalWrite(VIBRA_MOTOR, LOW); }
      }

      // Static Fatigue Monitor (20 min threshold)
      if (abs(currentDistance - lastStoredDistance) > 3.0 && currentDistance > 0) {
        lastMovementTime = millis(); 
        lastStoredDistance = currentDistance;
        staticFatigueAlert = false;
      } 
      else if (millis() - lastMovementTime >= 1200000) { 
        staticFatigueAlert = true;
        statusDial.write(45); 
        digitalWrite(VIBRA_MOTOR, HIGH); delay(1000); digitalWrite(VIBRA_MOTOR, LOW); 
        lastMovementTime = millis(); 
      }
    } 
    else if (pomodoroState == 2) {
      statusDial.write(135); // Break mode: needle at 135°
      digitalWrite(VIBRA_MOTOR, LOW);
      staticFatigueAlert = false;
    }
    else {
      statusDial.write(0);  // Idle: needle at 0°
      digitalWrite(VIBRA_MOTOR, LOW);
      staticFatigueAlert = false;
    }
  }
  
  delay(20); 
}
