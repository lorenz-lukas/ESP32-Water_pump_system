#include <LoRa.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
 
/* Definicoes para comunicação com radio LoRa */
#define SCK_LORA           5
#define MISO_LORA          19
#define MOSI_LORA          27
#define RESET_PIN_LORA     14
#define SS_PIN_LORA        18
 
#define HIGH_GAIN_LORA     20  /* dBm */
#define BAND               915E6  /* 433MHz de frequencia */

/* Definicoes do OLED */
#define OLED_SDA_PIN    4
#define OLED_SCL_PIN    15
#define SCREEN_WIDTH    128 
#define SCREEN_HEIGHT   64  
#define OLED_ADDR       0x3C 
#define OLED_RESET      16
 
/* Offset de linhas no display OLED */
#define OLED_LINE1     0
#define OLED_LINE2     10
#define OLED_LINE3     20
#define OLED_LINE4     30
#define OLED_LINE5     40
#define OLED_LINE6     50 
/* Definicoes gerais */
#define DEBUG_SERIAL_BAUDRATE    9600
 
/* Variaveis globais */
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
int informacao_a_ser_enviada = 0;
 
/* Local prototypes */
void display_init(void);
bool init_comunicacao_lora(void);
int control(int opmode, int toggle_pump);

/* Funcao: 
 * Parametros: 
 * Retorno: 
 *          
 *          
 */

 int control(int opmode, int toggle_pump)
 {
    
 }
 
/* Funcao: inicia comunicação com chip LoRa
 * Parametros: nenhum
 * Retorno: true: comunicacao ok
 *          false: falha na comunicacao
*/

void display_init(void)
{
    if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) 
    {
        Serial.println("[LoRa Receiver] Falha ao inicializar comunicacao com OLED");        
    }
    else
    {
        Serial.println("[LoRa Receiver] Comunicacao com OLED inicializada com sucesso");
     
        /* Limpa display e configura tamanho de fonte */
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
    }
}

bool init_comunicacao_lora(void)
{
    bool status_init = false;
    Serial.println("[LoRa Sender] Tentando iniciar comunicacao com o radio LoRa...");
    SPI.begin(SCK_LORA, MISO_LORA, MOSI_LORA, SS_PIN_LORA);
    LoRa.setPins(SS_PIN_LORA, RESET_PIN_LORA, LORA_DEFAULT_DIO0_PIN);
     
    if (!LoRa.begin(BAND)) 
    {
        Serial.println("[LoRa Sender] Comunicacao com o radio LoRa falhou. Nova tentativa em 1 segundo...");        
        delay(1000);
        status_init = false;
    }
    else
    {
        /* Configura o ganho do receptor LoRa para 20dBm, o maior ganho possível (visando maior alcance possível) */
        LoRa.setTxPower(HIGH_GAIN_LORA); 
        Serial.println("[LoRa Sender] Comunicacao com o radio LoRa ok");
        status_init = true;
    }
 
    return status_init;
}
 
/* Funcao de setup */
void setup() 
{   
    /* Configuracao da I²C para o display OLED */
    Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);

    Serial.begin(DEBUG_SERIAL_BAUDRATE);
    while (!Serial);
    delay(100);
    Serial.println("\n Inicia com. Serial");
 
    /* Display init */
    display_init();
 
    /* Print message telling to wait */
    display.clearDisplay();    
    display.setCursor(0, OLED_LINE1);
    display.print("Aguarde...");
    display.display();
    delay(1000);
   
 
    /* Tenta, até obter sucesso, comunicacao com o chip LoRa */
    while(init_comunicacao_lora() == false);      

    display.clearDisplay();    
    display.setCursor(0, OLED_LINE1);
    display.print("Inic. LoRa concl.");
    display.display();
    delay(1000);
}
 
/* Programa principal */
void loop() 
{
    int lora_rssi = 0;
    /*mode: 0->automatico / 1->manual*/
    int opmode = 0;
    /*estado: 0->vazio / 1->enchendo / 2->cheio*/
    int state = 1 ;
    /*toggle_pump: ativar/desativar bomba no modo manual*/
    int toggle_pump;

    //state = control(opmode, toggle_pump);
    
    /* Envia a informação */
    LoRa.beginPacket();
    LoRa.write(state);
    //LoRa.write(informacao_a_ser_enviada);
    LoRa.endPacket();
    
    
    lora_rssi = LoRa.packetRssi();
    display.clearDisplay();   
    display.setCursor(0, OLED_LINE1);
    display.print("Sender-RSSI: ");
    display.println(lora_rssi);
    display.setCursor(0, OLED_LINE2);
    display.print("Enviado: ");
    //display.setCursor(0, OLED_LINE3);
    //display.println(informacao_a_ser_enviada);
    if(opmode==0){
      display.setCursor(0, OLED_LINE3);
      display.println("Automatico");
    }
    else if(opmode==1){
      display.setCursor(0, OLED_LINE3);
      display.println("Manual");
    }
    if(state==0){
      display.setCursor(0, OLED_LINE4);
      display.println("Vazio");
    }
    else if(state==1){
      display.setCursor(0, OLED_LINE4);
      display.println("Enchendo");
    }
    else if(state==2){
      display.setCursor(0, OLED_LINE4);
      display.println("Cheio");
    }
    display.display();
    
    /* Incrementa a informação para o próximo envio e aguarda 
       1 segundo até enviar a próxima informação */
    informacao_a_ser_enviada++;
    delay(10000);
     
}
