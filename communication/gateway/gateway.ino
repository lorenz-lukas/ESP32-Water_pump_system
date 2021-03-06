#include "heltec.h"

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "interface.h"


#define BAND    433E6  //you can set band here directly,e.g. 868E6,915E6
/* Offset de linhas no display OLED */
#define LINE1     0
#define LINE2     10
#define LINE3     20
#define LINE4     30
#define LINE5     40
#define LINE6     50

//Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

String outgoing;              // outgoing message

byte localAddress = 0xBB;     // address of this device
byte destination = 0xFD;      // destination to send to

byte msgCount = 0;            // count of outgoing messages
long lastSendTime = 0;        // last send time
int interval = 500;          // interval between sends
String message = "What, already?!";   // send a message

int opmode = 0;               //modo: 0->automatico / 1->manual

int state = 0 ;               //estado: 0->vazio / 1->enchendo / 2->esvaziando / 3->cheio / 4->erro
String state_msg = "";

int toggle_pump = 0;              //usuario->switch bomba: off->1 /on->0 bomba no modo manual
String toggle_pump_msg = "";

int pump_state = 0;             //bomba desligada = 0, bomba ligada = 1
String pump_state_msg = "";

int man = 23;
int tog = 13;

///////////////////////// HTML

// Replace with your network credentials
const char* ssid = "Lemos";
const char* password = "mano7019";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create an Event Source on /events
AsyncEventSource events("/events");

void initWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');
        delay(1000);
    }
    Serial.println(WiFi.localIP());
}


String outputState(int gpio){
  if(digitalRead(gpio)){
    return "checked";
  }
  else {
    return "";
  }
}

String processor(const String& var){
  //Serial.println(var);
  if(var == "state"){
    return state_msg;
  }
  else if(var == "pump_state"){
    Serial.println(pump_state);
    return pump_state_msg;
  }
  else if(var == "toggle_pump"){
    return toggle_pump_msg;
  }else if(var == "BUTTONPLACEHOLDER"){
    String buttons;
    String outputStateValue = outputState(32);
    buttons+="<div class=\"card card-switch\"><h4><i class=\"fas fa-lightbulb\"></i> Modo De Operacao </h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"controlOutput(this)\" id=\"output\" " + outputStateValue + "><span class=\"slider\"></span></label></div>";
    outputStateValue = outputState(19);
    buttons+="<div class=\"card card-switch\"><h4><i class=\"fas fa-lightbulb\"></i> Ligar Bomba Manual </h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleLed(this)\" id=\"led\" " + outputStateValue + "><span class=\"slider\"></span></label></div>";
    
    return buttons;
  }
  return String();
}

void initWebSERVER(){
  
   initWiFi();
  // Handle Web Server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  
  // Send a GET request to control on board status LED <ESP_IP>/toggle
  server.on("/toggle", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if(toggle_pump == 0 && state < 3) toggle_pump = 1;
    else toggle_pump = 0;
    request->send(200, "text/plain", "OK");
  });
  // Send a GET request to control output socket <ESP_IP>/output?state=<inputMessage>
  server.on("/output", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if(opmode == 0 && state < 3) opmode = 1;
    else opmode = 0;
    request->send(200, "text/plain", "OK");
  });

  // Handle Web Server Events
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);
  server.begin();
}


//////////////////////////////// LORA
void setup()
{
  //Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
   //WIFI Kit series V1 not support Vext control   
  Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.LoRa Enable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);

  initWebSERVER();
  
  Heltec.display->init();
  Heltec.display->flipScreenVertically();  
  Heltec.display->setFont(ArialMT_Plain_10);
  
  Serial.println("Heltec.LoRa Duplex");
  
  Heltec.display->clear();
  Heltec.display->drawString(20, LINE3, "Aguarde...");
  Heltec.display->display();
  delay(1000);

  pinMode(man, INPUT);
  pinMode(tog, INPUT);
  updisplay();
 
}

void loop()
{
  if (millis() - lastSendTime > interval)
  {
    sendMessage(message);
    Serial.println("Sending " + message);
    lastSendTime = millis();            // timestamp the message
    
    // P??GINA HTML
    events.send("ping",NULL,millis());
    if(state == 0){
      state_msg = "Vazio";    
      events.send("Vazio", "state", millis());
    }else if(state == 3){
      state_msg = "Cheio";    
      events.send("Cheio", "state", millis());
    }else if(state == 1){
      state_msg = "Enchendo";    
      events.send("Enchendo", "state", millis());
    }else if(state == 2){
      state_msg = "Esvaziando";    
      events.send("Esvaziando", "state", millis());
    }else{
      state_msg = "Erro";    
      events.send("Erro", "state", millis());
    }

    if(pump_state == 0){
      pump_state_msg = "Desligada";
      events.send("Desligada", "pump_state", millis());
    }else{
      pump_state_msg = "Ligada";
      events.send("Ligada", "pump_state", millis());
    }

    if(opmode == 0){
      toggle_pump_msg = "Automatico";    
      events.send("Autom??tico", "toggle_pump",millis());
    }else{
      toggle_pump_msg = "Manual";    
      events.send("Manual", "toggle_pump",millis());
    }
    if(toggle_pump == 1){
      pump_state = 1;
    }
  }

  // parse for a packet, and call onReceive with the result:
  onReceive(LoRa.parsePacket());
  
}

void sendMessage(String outgoing)
{
  LoRa.beginPacket();                   // start packet
  LoRa.write(destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(outgoing.length());        // add payload length
  LoRa.write(opmode);                   //modo de operacao
  if (!opmode) LoRa.write(toggle_pump);   //se manual manda toggle  
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID
}

void onReceive(int packetSize)
{
  if (packetSize == 0) return;          // if there's no packet, return

  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length
  state = LoRa.read();                  // estado do sistema
  pump_state = LoRa.read();              // estado da bomba
  if (state==3) toggle_pump = LoRa.read(); // se cheio forca toggle

  String incoming = "";

  while (LoRa.available())
  {
    incoming += (char)LoRa.read();
  }

  if (incomingLength != incoming.length())
  {   // check length for error
    Serial.println("error: message length does not match length");
    return;                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF) {
    Serial.println("This message is not for me.");
    return;                             // skip rest of function
  }

  // if message is for this device, or broadcast, print details:
  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Sent to: 0x" + String(recipient, HEX));
  Serial.println("Message ID: " + String(incomingMsgId));
  Serial.println("Message length: " + String(incomingLength));
  Serial.println("Message: " + incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();
  updisplay();
//  Heltec.display->clear();
//  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
//  Heltec.display->drawString(0, LINE1, "Gateway-RSSI: " + String(LoRa.packetRssi()));
//  if(opmode){
//    Heltec.display->drawString(0, LINE2, "Modo: Automatico");
//  }
//  else if(!opmode){
//    Heltec.display->drawString(0, LINE2, "Modo: Manual");
//  }
//  if(state==0){
//    Heltec.display->drawString(0, LINE3, "Estado: Vazio");
//  }
//  else if(state==1){
//    Heltec.display->drawString(0, LINE3, "Estado: Enchendo");
//  }
//  else if(state==2){
//    Heltec.display->drawString(0, LINE3, "Estado: Esvaziando");
//  }
//  else if(state==3){
//    Heltec.display->drawString(0, LINE3, "Estado: Cheio");
//  }
//  else if(pump_state){
//    Heltec.display->drawString(0, LINE4, "Bomba: Desativada");
//  }
//  else if(!pump_state){
//    Heltec.display->drawString(0, LINE4, "Bomba: Ativada");
//  }
//  Heltec.display->display();
  
}
void updisplay()
{
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->drawString(0, LINE1, "Controller-RSSI: " + String(LoRa.packetRssi()));
  if(opmode==0){
    Heltec.display->drawString(0, LINE2, "Modo: Automatico");
  }
  else if(opmode==1){
    Heltec.display->drawString(0, LINE2, "Modo: Manual");
  }
  if(state==0){
    Heltec.display->drawString(0, LINE3, "Estado: Vazio");
  }
  else if(state==1){
    Heltec.display->drawString(0, LINE3, "Estado: Enchendo");
  }
  else if(state==2){
    Heltec.display->drawString(0, LINE3, "Estado: Esvaziando");
  }
  else if(state==3){
    Heltec.display->drawString(0, LINE3, "Estado: Cheio");
  }
  else if(pump_state==0){
    Heltec.display->drawString(0, LINE4, "Bomba: Desativada");
  }
  else if(pump_state==1){
    Heltec.display->drawString(0, LINE4, "Bomba: Ativada");
  }
  Heltec.display->display();
}
