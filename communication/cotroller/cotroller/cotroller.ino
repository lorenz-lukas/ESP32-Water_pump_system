/*
  Heltec.LoRa Multiple Communication

  This example provide a simple way to achieve one to multiple devices
  communication.

  Each devices send datas in broadcast method. Make sure each devices
  working in the same BAND, then set the localAddress and destination
  as you want.
  
  Sends a message every half second, and polls continually
  for new incoming messages. Implements a one-byte addressing scheme,
  with 0xFD as the broadcast address. You can set this address as you
  want.

  Note: while sending, Heltec.LoRa radio is not listening for incoming messages.
  
  by Aaron.Lee from HelTec AutoMation, ChengDu, China
  成都惠利特自动化科技有限公司
  www.heltec.cn
  
  this project also realess in GitHub:
  https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series
*/
#include "heltec.h"


#define BAND    433E6  //you can set band here directly,e.g. 868E6,915E6
/* Offset de linhas no display OLED */
#define LINE1     0
#define LINE2     10
#define LINE3     20
#define LINE4     30
#define LINE5     40
#define LINE6     50

String outgoing;              // outgoing message

byte localAddress = 0xFD;     // address of this device
byte destination = 0xBB;      // destination to send to

byte msgCount = 0;            // count of outgoing messages
long lastSendTime = 0;        // last send time
int interval = 2000;          // interval between sends
String message = "Hello,I'm coming!";   // send a message

bool opmode = true;               //modo: 0->automatico / 1->manual
int state = 0 ;               //estado: 0->vazio / 1->enchendo / 2->cheio / 3->erro
bool toggle_pump;              //usuario->bomba: ativar/desativar bomba no modo manual
bool pump_state=true;

int pump = 23;
int capac = 13;
int floater = 38;

    
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
  
  pinMode(pump, OUTPUT);
  pinMode(capac, INPUT);
  pinMode(floater, INPUT);

  digitalWrite(pump,HIGH);
 
}

void loop()
{
  while(state!=4){ 
    if (millis() - lastSendTime > interval)
    {
      sendMessage(message);
      Serial.println("Sending " + message);
      lastSendTime = millis();            // timestamp the message
      interval = random(2000) + 1000;    // 2-3 seconds
    }
  
    // parse for a packet, and call onReceive with the result:
    onReceive(LoRa.parsePacket());
    state = control();
  }
  Heltec.display->clear();
  Heltec.display->drawString(20, LINE3, "Erro, veja sensores");
  Heltec.display->display();
  delay(1000);
  while(1){}
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
  if(opmode){
    Heltec.display->drawString(0, LINE2, "Modo: Automatico");
  }
  else if(!opmode){
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
  else if(pump_state){
    Heltec.display->drawString(0, LINE4, "Bomba: Desativada");
  }
  else if(!pump_state){
    Heltec.display->drawString(0, LINE4, "Bomba: Ativada");
  }
  Heltec.display->display();

}

int control()
{
  bool top,bottom;
  
  bottom = digitalRead(capac);
  bottom=!bottom;
  top = digitalRead(floater);
  top = !top;
  if (top && !bottom){          //erro
    digitalWrite(pump,HIGH);
    pump_state=false;
    return 4;
  }
  else if (top && bottom){     //cheio
    digitalWrite(pump,HIGH);    //desliga bomba
    pump_state=false;
    toggle_pump = false;
    return 3;   
  }
  else if (!opmode && !toggle_pump){//manual e switch bomba off
    digitalWrite(pump,HIGH);         //desliga bomba
    pump_state=false;
    if (!bottom) return 0;       //vazio
    else if (bottom) return 2 ; //esvaziando
  }
  else if(!opmode && toggle_pump){//manual e switch bomba on
    digitalWrite(pump,LOW);      //liga bomba
    pump_state=true;
    return 1; //enchendo
  }
  else if(opmode && !bottom){ //automatico e vazio
    digitalWrite(pump,LOW);  //liga bomba
    pump_state=true;
    return 1;                  //enchendo
  }
  else if(opmode && bottom) return 2; //automatico e esvaziando
}
