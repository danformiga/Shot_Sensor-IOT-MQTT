//---Contador IOT REV 27072021
#include <Wire.h>
#include <PubSubClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "images.h" // carrega imagens 128 X 64
#include <ESP8266WiFi.h>
#define OLED_RESET -1 // OLED Reset, Deixar -1 para D1 Mini
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define MSG_BUFFER_SIZE  (50)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//--------WIFI-------------
const char* ssid = "Formiga";
const char* password = "*********";
const char* mqtt_server = "broker.mqtt-dashboard.com"; //Servidor MQTT
WiFiClient espClient;
PubSubClient client(espClient);
//unsigned long lastMsg = 0;

//----------Buffer dos concatenadores-----------
char msg[MSG_BUFFER_SIZE]; 
char msg1[MSG_BUFFER_SIZE];
char msg2[MSG_BUFFER_SIZE];
char msg3[MSG_BUFFER_SIZE];
char msg4[MSG_BUFFER_SIZE];

//--Config. servidor NTP------------------------------------------------------
static const char ntpServerName[] = "us.pool.ntp.org";
const int timeZone = -3;     // Central European Time
WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets
time_t getNtpTime();
void digitalClockDisplay();
void printDigits(int digits);
void sendNTPpacket(IPAddress &address);

//-----------Logica e entrada sensores-----------------------------------------
int sensorPin = A0; // Pino Input
int counter = 0;
int res = 0; //Reset
int sensorValue = 0;
int val = 0;
int value = 0;
int diaatual = 0;
int mesatual = 0;
int anoatual = 0;
int horaatual = 0;
int minatual = 0;
int segatual =0;

//----Conexao WIFI-------------------------------------------------
void setup_wifi() { 

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Conectando em ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    
//----Mensagem erro no OLED de conexao Wifi após 6 tentatias--------
    value++;
    delay(500);
    if (value > 5) {
      display.clearDisplay(); //Limpa display
      display.setTextSize(2); //Tamanho da fonte
      display.setTextColor(WHITE);//Cor da fonte, no monocromático é sempre branco
      display.setCursor(0, 0); //Cursor
      display.println(" ATENCAO!");  //Escrita
      display.setTextSize(2); 
      display.setTextColor(WHITE);
      display.setCursor(0, 16);
      display.print(("VERIFIQUE A CONEXAO DO WIFI.."));
      display.display();   //Mostra no display
      delay(2000);
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0, 0);
      display.println(" ATENCAO!");  
      display.setTextSize(2); 
      display.setTextColor(WHITE);
      display.setCursor(0, 16);
      display.print(("E RESET O APARELHO."));
      display.display();     
      delay(2000);   
    }
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop até conectar
  while (!client.connected()) {
    Serial.print("Conectando servidor MQTT...");
    //Cria um ID de cliente aleatório
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Tentando conectar
    if (client.connect(clientId.c_str())) {
      Serial.println(" Conectado");
      Serial.println(msg);
      // Ao conectar publicar - Conectado
      client.publish("outTopic03200", "Conectado");
      client.publish("outTopic03200", msg1);
    } else {
      Serial.print(" Falhou, rc=");
      Serial.print(client.state());
      Serial.println(" reconexão em 5 segundos");
      // Espera 5 segundos
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0, 0);
//Display static text
      display.print("  ATENCAO!");
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0, 20);
      display.println("Sem internet!");
      display.println("Tentando reconectar..");
      display.display();
      delay(5000);
    }
  }
}
//-------------Mqtt^^-----------
void setup()
{
  //----------------LCD-----------------
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
//--------Splash Screen----------------------------
  display.clearDisplay();
  display.drawBitmap(0, 1,  image5, 128, 64, WHITE);
  display.display();
  delay(5000);
//----------------------------------------------
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
// Display static text
  display.print("Contador");
  display.setTextSize(1);
  display.println(" IOT");
  display.setCursor(0, 20);
  display.println("Estabelecendo conexao");
  display.println("Rede:");
  display.setCursor(0, 40);
  display.setTextSize(2);
  display.println(ssid);
  display.display();
//----------------------------
  pinMode(sensorPin, INPUT); // declara sensorPin
  Serial.begin(9600);
  //-------------MQTT----------------
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  
 // while (!Serial) ; // Somente Leonardo
  delay(250);
  Serial.println("TimeNTP ");
  Serial.print("Conectando a: ");
  Serial.println(ssid);
 
  //------------------LCD-----------
  display.clearDisplay();
  delay(1000);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("Contador");
  display.setTextSize(1);
  display.println(" IOT");
  display.setCursor(0, 20);
  display.println("Conectado!");
  display.println("Endereco de IP:");
  display.println(WiFi.localIP());
  display.println("Endereco de MAC:");
  display.println(WiFi.macAddress());
  display.display();
  delay(3000);

//---------NTP-----------------------------
  Serial.print("IP atribuido por DHCP: ");
  Serial.println(WiFi.localIP());
  Serial.println("Inicializando UDP");
  Udp.begin(localPort);
  Serial.print("Porta local: ");
  Serial.println(Udp.localPort());
  Serial.println("Aguardando sync");
  setSyncProvider(getNtpTime); 
  setSyncInterval(300);
}

time_t prevDisplay = 0; // when the digital clock was displayed

void loop()
{
  //---------------MQTT---------------
if (!client.connected()) {
    reconnect();
  }
  client.loop();

  //----------------Loop sensor--------
  sensorValue = analogRead(sensorPin);    
  val = analogRead(sensorPin);
  //Serial.println(val);
  //Serial.print("  ");
  //Serial.print(counter);
  snprintf (msg4, MSG_BUFFER_SIZE, " %03d  ", counter); 
  //----Lógica de zerar o contador-- 
  if (segatual == 59 && minatual == 59 && horaatual == 23 ){ 
    counter = 0;
  }
  //--Lógica de acionamento, concatenagem da msg, escrita no servidor MQTT e display-----
  if (val > 600){
 // if (x = 1){
    counter = counter + 1;
    ++value;
    snprintf (msg, MSG_BUFFER_SIZE, "Cont:%1d | %02d:%02d:%02d | %02d/%02d/%02d",counter, horaatual, minatual, segatual, diaatual, mesatual, anoatual);
    snprintf (msg1, MSG_BUFFER_SIZE, "  Ultimo: %02d:%02d:%02d ", horaatual, minatual, segatual); // %02d:%02d:%02d duas casas decimais
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("outTopic03200", msg);
    display.setCursor(0, 30);
    display.setTextSize(1);
    display.println("P");
    display.display();
    delay(2000); //Intervalo de uma contagem para outra
  }
  
  else if (timeStatus() != timeNotSet) {
    if (now() != prevDisplay) { //update the display only if time has changed
      prevDisplay = now();
      digitalClockDisplay();
    }
  }
}

//---Lógica biblioteca NTP e escrita no display
void digitalClockDisplay()
{
  diaatual = day();
  mesatual = month();
  anoatual = year();
  horaatual = hour();
  minatual = minute();
  segatual = second();
    
  snprintf (msg2, MSG_BUFFER_SIZE, "Shot Sensor  %02d:%02d:%02d", horaatual, minatual, segatual);
  snprintf (msg3, MSG_BUFFER_SIZE, "CONTADOR   %02d/%02d/%04d", diaatual, mesatual, anoatual);
//---DEBUG---------------
//Serial.print(msg2);
//Serial.print(msg3);
//-------------LCD---------
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.print(msg3);
  display.print(msg2);
  display.setCursor(5, 22);
  display.setTextSize(4);
  display.println(msg4);
  display.setCursor(0, 56);
  display.setTextSize(1);
  display.println(msg1);
  display.display();
}

void printDigits(int digits)
{
  // utility for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if (digits < 10)
   Serial.print('0');
   Serial.print(digits);
}

/*-------------------- NTP code ----------------------------------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmitindo pedido NTP ");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Recebido resposta NTP");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("Sem resposta servidor NTP :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
