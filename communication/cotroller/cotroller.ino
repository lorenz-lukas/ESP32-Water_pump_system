#include "heltec.h"


#define BAND    433E6  //you can set band here directly,e.g. 868E6,915E6
/* Offset de linhas no display OLED */
#define LINE1     0
#define LINE2     10
#define LINE3     20
#define LINE4     30
#define LINE5     40
#define LINE6     50

#define pump 21
#define capac 13
#define floater 37

String outgoing;              // outgoing message

byte localAddress = 0xFD;     // address of this device
byte destination = 0xBB;      // destination to send to

byte msgCount = 0;            // count of outgoing messages
long lastSendTime = 0;        // last send time
int interval = 1000;          // interval between sends
String message = "Hello,I'm coming!";   // send a message

int opmode = 0;               //modo: 0->automatico / 1->manual
int state = 0 ;               //estado: 0->vazio / 1->enchendo / 2->esvaziando / 3->cheio / 4->erro
int toggle_pump;              //usuario->switch bomba: off->1 /on->0 bomba no modo manual
int pump_state=0;



    
void setup()
{
  //Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
   //WIFI Kit series V1 not support Vext control
  Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.LoRa Enable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
  
  Heltec.display->init();
  Heltec.display->flipScreenVertically();  
  Heltec.display->setFont(ArialMT_Plain_10);
  
  Serial.println("Heltec.LoRa Duplex");
  
  Heltec.display->clear();
  Heltec.display->drawString(20, LINE3, "Aguarde...");
  Heltec.display->display();
  delay(1000);
  
  //pinMode(pump, OUTPUT);
  pinMode(capac, INPUT);
  pinMode(floater, INPUT);
  pinMode(pump, OUTPUT);
  digitalWrite(pump,HIGH);
  digitalWrite( LED ,HIGH );
  delay(10000);
  digitalWrite(pump,LOW);
  digitalWrite( LED ,LOW );
  delay(10000);
}

int ini = 0;

void loop()
{

  if (ini==0){
    digitalWrite( pump ,HIGH ); delay(1000);
    ini=1;
  }
  if (millis() - lastSendTime > interval)
  {
    sendMessage(message);
    Serial.println("Sending " + String(pump_state));
    lastSendTime = millis();            // timestamp the message
  }
  // parse for a packet, and call onReceive with the result:
  onReceive(LoRa.parsePacket());
  state = control();

  if(state == 4){
    error();    
  }

}

void error(){
  Heltec.display->clear();
  Heltec.display->drawString(20, LINE3, "Erro, veja sensores");
  Heltec.display->display();
  delay(1000);
  while(1){
      LoRa.beginPacket();                   // start packet
      LoRa.write(destination);              // add destination address
      LoRa.write(localAddress);             // add sender address
      LoRa.write(msgCount);                 // add message ID
      LoRa.write(outgoing.length());        // add payload length
      
      LoRa.write(state);                    // manda estado
      LoRa.write(pump_state);               // manda estado da bomba
      if (state==3) LoRa.write(toggle_pump); // se cheio forca toggle do usuario
      
      LoRa.print(outgoing);                 // add payload
      LoRa.endPacket();                     // finish packet and send it
      msgCount++;                           // increment message ID
 }
}

void sendMessage(String outgoing)
{
  LoRa.beginPacket();                   // start packet
  LoRa.write(destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(outgoing.length());        // add payload length
  
  LoRa.write(state);                    // manda estado
  LoRa.write(pump_state);               // manda estado da bomba
  if (state==3) LoRa.write(toggle_pump); // se cheio forca toggle do usuario
  
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
  opmode = LoRa.read();                 // modo de operacao
  if (!opmode) toggle_pump = LoRa.read(); // se manual vê toggle
  

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

int control()
{
  int top,bottom;
  
  bottom = digitalRead(capac);
  top = digitalRead(floater);
  ////////////////////////// ERRO
  if (top==1 && bottom==0){          //erro
    digitalWrite( pump ,HIGH ); delay(1000);// desliga bomba
    pump_state=0;
    Serial.println("ERRO, top"+ String(top)+"bot"+ String(bottom)+"pump"+ String(pump_state));
    return 4;
  }
  /////////// CHEIO MANUAL E AUTOMÁTICO
  else if (top==1 && bottom==1){     //cheio
    digitalWrite( pump ,HIGH ); delay(1000);   //desliga bomba
    pump_state=0;
    toggle_pump = 0;
    Serial.println("cheio, top"+ String(top)+"bot"+ String(bottom)+"pump"+ String(pump_state));
    return 3;   
  }
  ////////// AUTOMÁTICO
  else if(opmode==0 && bottom==0){ //automatico e vazio
    digitalWrite( pump ,LOW ); delay(1000);  //liga bomba
    pump_state=1;
    Serial.println("vazio-auto, top"+ String(top)+"bot"+ String(bottom)+"pump"+ String(pump_state));
    return 0;                  //vazio
  }
  else if(opmode==0 && bottom==1 && pump_state==0){
    pump_state=0;
    Serial.println("esvaziando-auto, top"+ String(top)+"bot"+ String(bottom)+"pump"+ String(pump_state));
    return 2; //automatico e esvaziando
  }
  else if(opmode==0 && bottom==1 && pump_state==1){
    pump_state=1;
    Serial.println("enchendo-auto, top"+ String(top)+"bot"+ String(bottom)+"pump"+ String(pump_state));
    return 1; //automatico e enchendo
  }
  //////////////////// MANUAL
  else if (opmode==1 && toggle_pump==0){//manual e switch bomba off
    digitalWrite( pump ,HIGH ); delay(1000);        //desliga bomba
    pump_state=0;
    if (bottom==0){
      Serial.println("vazio-manual-off, top"+ String(top)+"bot"+ String(bottom)+"pump"+ String(pump_state));
      return 0;       //vazio
    }
    else if (bottom==1){
      Serial.println("esvaziando-manual-off, top"+ String(top)+"bot"+ String(bottom)+"pump"+ String(pump_state));
      return 2 ; //esvaziando
    }
  }
  else if(opmode==1 && toggle_pump==0){//manual e switch bomba on
    digitalWrite( pump ,LOW ); delay(1000);      //liga bomba
    pump_state=1;
    Serial.println("enchendo-manual, top"+ String(top)+"bot"+ String(bottom)+"pump"+ String(pump_state));
    return 1; //enchendo
  }
}
