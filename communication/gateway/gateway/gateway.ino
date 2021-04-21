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



//Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

String outgoing;              // outgoing message

byte localAddress = 0xBB;     // address of this device
byte destination = 0xFD;      // destination to send to

byte msgCount = 0;            // count of outgoing messages
long lastSendTime = 0;        // last send time
int interval = 2000;          // interval between sends
String message = "What, already?!";   // send a message
bool opmode = 0;               //modo: 0->automatico / 1->manual
int state = 0 ;               //estado: 0->vazio / 1->enchendo / 2->esvaziando / 3->cheio / 4->erro
bool toggle_pump;              //usuario->switch bomba: off->1 /on->0 bomba no modo manual
bool pump_state=true;

int man = 23;
int tog = 13;


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
  pinMode(man, INPUT);
  pinMode(tog, INPUT);
 
}

void loop()
{
  if (millis() - lastSendTime > interval)
  {
    
    sendMessage(message);
    Serial.println("Sending " + message);
    lastSendTime = millis();            // timestamp the message
    interval = random(2000) + 1000;    // 2-3 seconds
  }

  // parse for a packet, and call onReceive with the result:
  onReceive(LoRa.parsePacket());
  opmode=digitalRead(man);
  toggle_pump=digitalRead(tog);
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
  pump_state= LoRa.read();              // estado da bomba
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
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->drawString(0, LINE1, "Gateway-RSSI: " + String(LoRa.packetRssi()));
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
