#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Wire.h>
#include <SH1106Wire.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <NTPClient.h>
#include <EEPROM.h>
#include <ESP32_MailClient.h>
#include <driver/adc.h>

#define PIN_VENTILADOR   18
#define PIN_DHT          27     
#define PIN_SH1106_SDA   21
#define PIN_SH1106_SCK   22 
#define PIN_IR_SEND      13
#define PIN_MQ6          26
#define PIN_CHAMA        36

#define TYPE_DHT         DHT22
#define IR_FREQUENCY_KHZ 38
#define MIN_TEMP         28
#define MAX_TEMP         29
#define MAX_EPRON_BUFFER 100

#define TOPICO_SUBSCRIBE_VENTILADOR    "topico_ventilador"
#define TOPICO_SUBSCRIBE_CLI_ONOFF     "topico_cli_onoff"
#define TOPICO_SUBSCRIBE_CLI_COOLING   "topico_cli_cooling"
#define TOPICO_SUBSCRIBE_CLI_SPEED     "topico_cli_speed"
#define TOPICO_SUBSCRIBE_CLI_SWING     "topico_cli_swing"
#define TOPICO_SUBSCRIBE_UPDATE_DATA   "topico_update_data"
#define TOPICO_PUBLISH_MODO_STS_VENT   "topico_status_ventilador"
#define TOPICO_PUBLISH_TEMPERATURA     "topico_temperatura"
#define TOPICO_PUBLISH_TEMPERATURA_MIN "topico_temperatura_min"
#define TOPICO_PUBLISH_TEMPERATURA_MAX "topico_temperatura_max"  
#define TOPICO_PUBLISH_UMIDADE         "topico_umidade"  
#define TOPICO_PUBLISH_STS_CLI_ONOFF   "topico_stscli_onoff"
#define TOPICO_PUBLISH_STS_CLI_COOLING "topico_stscli_cooling"
#define TOPICO_PUBLISH_STS_CLI_SPEED   "topico_stscli_speed"
#define TOPICO_PUBLISH_STS_CLI_SWING   "topico_stscli_swing"
#define TOPICO_PUBLISH_RELOGIO         "topico_relogio"
#define TOPICO_PUBLISH_GAS             "topico_gas"    
#define TOPICO_PUBLISH_CHAMA           "topico_chama"
#define TOPICO_PUBLISH_IP              "topico_ip"
#define TOPICO_PUBLISH_STS_VENT_AUTO   "topico_sts_vent_auto"    
#define ID_MQTT                        "mqttesp-cz58690eesp32"

#define EMAIL_SENDER_ACCOUNT           "ENDERECO_EMAIL_ENVIO"    
#define EMAIL_SENDER_PASSWORD          "SENHA_EMAIL_ENVIO"
#define EMAIL_RECIPIENT                "ENDERECO_EMAIL_ALERTA"
#define SMTP_SERVER                    "SMTP_EMAIL_ENVIO"
#define SMTP_SERVER_PORT               587
#define EMAIL_SUBJECT                  "Alerta Automação ESP32"

const char* SSID = "SSID_WIFI";
const char* PASSWORD = "SENHA_WIFI";
 
const char* BROKER_MQTT = "broker.emqx.io";
int BROKER_PORT = 1883; 

const char* servidorNTP = "a.st1.ntp.br";

int valor_gas_ant = 1;
int valor_chama_ant = 1;
float temperatura = 0;
float umidade = 0;
float temp_min = 0;
float temp_max = 0;
bool modo_aut;
bool atualizar_dados = false;
float temp_ant = 0;
String dataAnterior = "";

bool stsClimatizadorOn = false;
bool stsClimatizadorCooling = false;
int stsClimatizadorSpeed = 1;
bool stsClimatizadorSwing = false;

uint16_t climatizadorOnOff[191] = {1352, 460,  1356, 430,  448, 1274,  1360, 426,  1356, 432,  446, 1274,  446, 1274,  1390, 398,  482, 1238,  484, 1236,  480, 1240,  480, 8144,  1358, 430,  1328, 458,  444, 1276,  1328, 458,  1330, 458,  472, 1246,  418, 1302,  1330, 456,  444, 1274,  444, 1276,  416, 1302,  446, 8178,  1328, 458,  1328, 458,  420, 1300,  1326, 460,  1356, 430,  444, 1276,  444, 1276,  1352, 434,  444, 1276,  470, 1250,  418, 1302,  418, 8204,  1330, 456,  1330, 458,  444, 1276,  1328, 458,  1328, 458,  418, 1302,  446, 1274,  1328, 458,  416, 1302,  444, 1276,  418, 1302,  418, 8206,  1328, 458,  1328, 458,  420, 1300,  1328, 458,  1328, 458,  446, 1272,  418, 1300,  1330, 458,  418, 1302,  444, 1274,  448, 1272,  416, 8206,  1330, 456,  1360, 428,  420, 1300,  1360, 426,  1330, 458,  450, 1270,  448, 1270,  1358, 428,  450, 1268,  482, 1238,  480, 1240,  484, 8140,  1352, 434,  1354, 432,  448, 1272,  1354, 428,  1356, 432,  444, 1276,  442, 1276,  1352, 436,  416, 1304,  442, 1276,  442, 1276,  444, 8180,  1324, 460,  1356, 432,  470, 1250,  1352, 434,  1326, 460,  440, 1280,  440, 1276,  1354, 434,  444, 1276,  442, 1276,  498, 1220,  412};  
uint16_t climatizadorCooling[191] = {1350, 462,  1358, 428,  444, 1274,  1328, 462,  1354, 434,  446, 1274,  446, 1274,  442, 1278,  1352, 436,  416, 1304,  444, 1276,  472, 8152,  1356, 432,  1354, 434,  446, 1274,  1354, 432,  1356, 434,  416, 1304,  442, 1276,  478, 1244,  1352, 434,  444, 1276,  446, 1276,  442, 8180,  1328, 462,  1352, 434,  444, 1274,  1356, 434,  1358, 430,  442, 1278,  414, 1304,  442, 1278,  1356, 432,  442, 1280,  442, 1278,  442, 8184,  1356, 432,  1356, 432,  442, 1280,  1328, 460,  1358, 432,  444, 1278,  440, 1278,  446, 1274,  1328, 460,  444, 1276,  444, 1276,  422, 8202,  1354, 436,  1356, 432,  446, 1274,  1358, 430,  1358, 430,  442, 1278,  442, 1278,  444, 1276,  1326, 460,  444, 1276,  444, 1276,  444, 8180,  1354, 434,  1354, 434,  446, 1276,  1352, 434,  1354, 434,  446, 1272,  442, 1278,  444, 1274,  1354, 434,  444, 1276,  446, 1274,  444, 8180,  1326, 458,  1326, 460,  414, 1306,  1352, 434,  1352, 434,  414, 1304,  442, 1278,  442, 1276,  1352, 432,  442, 1276,  442, 1278,  476, 8148,  1352, 434,  1350, 434,  474, 1248,  1348, 434,  1354, 434,  442, 1276,  414, 1304,  416, 1302,  1350, 436,  440, 1278,  412, 1306,  442};
uint16_t climatizadorSpeed[191] = {1350, 432,  1318, 462,  412, 1302,  1348, 432,  1344, 434,  410, 1306,  412, 1304,  442, 1272,  414, 1302,  1344, 434,  414, 1302,  472, 8136,  1318, 462,  1316, 460,  410, 1304,  1346, 432,  1344, 434,  412, 1302,  442, 1272,  442, 1276,  412, 1302,  1318, 460,  414, 1302,  412, 8172,  1368, 434,  1318, 460,  412, 1302,  1346, 432,  1318, 460,  384, 1332,  412, 1302,  412, 1298,  442, 1276,  1344, 430,  442, 1274,  414, 8194,  1344, 432,  1320, 458,  468, 1248,  1318, 458,  1348, 430,  414, 1302,  442, 1272,  442, 1274,  442, 1272,  1324, 454,  444, 1272,  440, 8146,  1370, 428,  1348, 430,  474, 1242,  1350, 426,  1380, 398,  440, 1274,  444, 1268,  446, 1270,  476, 1236,  1380, 398,  446, 1270,  478, 8130,  1382, 394,  1354, 424,  440, 1274,  1380, 396,  1390, 388,  540, 1172,  474, 1240,  478, 1238,  480, 1234,  1384, 394,  474, 1240,  478, 8132,  1350, 426,  1386, 390,  414, 1302,  1380, 396,  1354, 424,  474, 1240,  478, 1236,  474, 1238,  480, 1236,  1348, 426,  478, 1240,  474, 8134,  1374, 402,  1352, 426,  524, 1192,  1346, 428,  1346, 430,  444, 1272,  442, 1272,  476, 1238,  470, 1244,  1346, 430,  442, 1274,  412};
uint16_t climatizadorSwing[191] = {1382, 432,  1354, 434,  444, 1276,  1354, 434,  1324, 464,  442, 1278,  416, 1304,  444, 1276,  444, 1276,  414, 1306,  1354, 434,  442, 8160,  1374, 436,  1352, 436,  414, 1308,  1354, 432,  1342, 446,  416, 1304,  440, 1278,  442, 1278,  440, 1278,  418, 1304,  1356, 428,  448, 8156,  1374, 434,  1356, 432,  442, 1278,  1356, 432,  1354, 432,  444, 1276,  444, 1276,  444, 1276,  444, 1276,  444, 1276,  1384, 402,  448, 8176,  1358, 430,  1388, 400,  482, 1240,  1388, 398,  1386, 402,  512, 1208,  484, 1236,  478, 1240,  484, 1236,  450, 1270,  1330, 456,  450, 8176,  1356, 430,  1330, 458,  420, 1302,  1354, 432,  1330, 458,  446, 1274,  442, 1276,  418, 1302,  442, 1278,  470, 1248,  1354, 434,  442, 8182,  1354, 434,  1356, 432,  442, 1278,  1354, 434,  1356, 432,  422, 1298,  468, 1250,  444, 1278,  442, 1276,  444, 1276,  1354, 432,  442, 8182,  1382, 406,  1358, 430,  442, 1278,  1382, 406,  1354, 434,  442, 1276,  442, 1278,  442, 1276,  444, 1278,  442, 1278,  1352, 434,  444, 8182,  1382, 404,  1354, 434,  442, 1278,  1382, 406,  1382, 406,  444, 1276,  470, 1250,  442, 1278,  442, 1278,  472, 1248,  1378, 408,  442};

DHT dht(PIN_DHT, TYPE_DHT);
SH1106Wire display(0x3c, PIN_SH1106_SDA, PIN_SH1106_SCK);
IRsend irsend(PIN_IR_SEND);
WiFiClient espClient; 
PubSubClient MQTT(espClient);

WiFiUDP ntpUDP; 
NTPClient timeClient(ntpUDP, servidorNTP, -3 * 3600, 60000);

SMTPData smtpData;

float faz_leitura_temperatura(void);
float faz_leitura_umidade(void);
void initWiFi(void);
void initMQTT(void);
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void reconnectMQTT(void);
void reconnectWiFi(void);
void VerificaConexoesWiFIEMQTT(void);
void printData(float, float);
void sendMailCallback(SendStatus info);
 
float faz_leitura_temperatura(void)
{
    float t = dht.readTemperature();
    float result;
     
    if (! (isnan(t)) )
        result = t;
    else
        result = -99.99;
 
    return result;
}
 
float faz_leitura_umidade(void)
{
    float h = dht.readHumidity();    
    float result;
     
    if (! (isnan(h)) )
        result = h;
    else
        result = -99.99;
 
    return result;
}

void initWiFi(void) 
{
    delay(10);
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");
      
    reconnectWiFi();
}
 

void initMQTT(void) 
{
    MQTT.setServer(BROKER_MQTT, BROKER_PORT); 
    MQTT.setCallback(mqtt_callback); 
}
 
void mqtt_callback(char* topic, byte* payload, unsigned int length) 
{
    String msg;
    char cliSpeed[10] = {0};
    
    for(int i = 0; i < length; i++) 
    {
       char c = (char)payload[i];
       msg += c;
    }
 
    Serial.print("Chegou a seguinte string via MQTT: ");
    Serial.println(msg);
    
    if (msg.equals("1"))
    {
        modo_aut = false;
        digitalWrite(PIN_VENTILADOR, LOW);
        Serial.println("Ventilador ligado mediante comando MQTT");
        MQTT.publish(TOPICO_PUBLISH_MODO_STS_VENT, "ON");
    }

    if (msg.equals("2"))
    {
        modo_aut = false;
        digitalWrite(PIN_VENTILADOR, HIGH);    
        Serial.println("Ventilador desligado mediante comando MQTT"); 
        MQTT.publish(TOPICO_PUBLISH_MODO_STS_VENT, "OFF");
    }

    if (msg.equals("3"))
    {
        modo_aut = true;
        Serial.println("Modo automatico ligado mediante comando MQTT");

        if(digitalRead(PIN_VENTILADOR) == HIGH)
            MQTT.publish(TOPICO_PUBLISH_MODO_STS_VENT, "OFF");
        else 
            MQTT.publish(TOPICO_PUBLISH_MODO_STS_VENT, "ON");
    }

    if (msg.equals("4"))
    {
        stsClimatizadorOn = !stsClimatizadorOn;
        irsend.sendRaw(climatizadorOnOff, 191, IR_FREQUENCY_KHZ);

        if(stsClimatizadorOn) 
            MQTT.publish(TOPICO_PUBLISH_STS_CLI_ONOFF, "ON");          
        else 
            MQTT.publish(TOPICO_PUBLISH_STS_CLI_ONOFF, "OFF");
    }

    if (msg.equals("5"))
    {
        stsClimatizadorCooling = !stsClimatizadorCooling;
        irsend.sendRaw(climatizadorCooling, 191, IR_FREQUENCY_KHZ);

        if(stsClimatizadorCooling) 
            MQTT.publish(TOPICO_PUBLISH_STS_CLI_COOLING, "ON");          
        else 
            MQTT.publish(TOPICO_PUBLISH_STS_CLI_COOLING, "OFF");
    }

    if (msg.equals("6"))
    {
        stsClimatizadorSpeed++;

        if(stsClimatizadorSpeed > 3) 
            stsClimatizadorSpeed = 0;          

        sprintf(cliSpeed, "%d", stsClimatizadorSpeed);
          
        irsend.sendRaw(climatizadorSpeed, 191, IR_FREQUENCY_KHZ);
        MQTT.publish(TOPICO_PUBLISH_STS_CLI_SPEED, cliSpeed);
    }

    if (msg.equals("7"))
    {
        stsClimatizadorSwing = !stsClimatizadorSwing;
        irsend.sendRaw(climatizadorSwing, 191, IR_FREQUENCY_KHZ);

        if(stsClimatizadorSwing) 
            MQTT.publish(TOPICO_PUBLISH_STS_CLI_SWING, "ON");          
        else 
            MQTT.publish(TOPICO_PUBLISH_STS_CLI_SWING, "OFF");
    }

    if (msg.equals("8"))
        atualizar_dados = true;
}
 
void reconnectMQTT(void) 
{
    while (!MQTT.connected()) 
    {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        
        if (MQTT.connect(ID_MQTT)) 
        {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPICO_SUBSCRIBE_VENTILADOR);
            MQTT.subscribe(TOPICO_SUBSCRIBE_CLI_ONOFF);
            MQTT.subscribe(TOPICO_SUBSCRIBE_CLI_COOLING);
            MQTT.subscribe(TOPICO_SUBSCRIBE_CLI_SPEED);
            MQTT.subscribe(TOPICO_SUBSCRIBE_CLI_SWING);
            MQTT.subscribe(TOPICO_SUBSCRIBE_UPDATE_DATA);  
        } 
        else
        {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Havera nova tentatica de conexao em 2s");
            delay(2000);
        }
    }
}

void VerificaConexoesWiFIEMQTT(void)
{
    if (!MQTT.connected()) 
        reconnectMQTT(); 
      
     reconnectWiFi(); 
}
 
void reconnectWiFi(void) 
{
    if (WiFi.status() == WL_CONNECTED)
        return;
    
    if (strcmp(PASSWORD, "") == 0) {
      Serial.println("Conexão sem senha!");    
      WiFi.begin(SSID);
    }
    else {
      Serial.println("Conexão com senha!");
      WiFi.begin(SSID, PASSWORD);
    }
      
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(100);
        Serial.print(".");
    }
    
    Serial.println();
    Serial.print("Conectado com sucesso na rede ");
    Serial.print(SSID);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());
}

void printData(float temperatura, float umidade, String horario, float temp_min, float temp_max) 
{
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, "Temperatura:");
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 10, ((String)temperatura) + "  °C");
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 35, "Umidade:");
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 45, ((String)umidade) + "  %");
    display.setFont(ArialMT_Plain_10);
    display.drawString(70, 0, "Temp. Mín:");
    display.drawString(70, 10, (String)temp_min + "  °C");
    display.drawString(70, 25, "Temp. Máx:");
    display.drawString(70, 35, (String)temp_max + "  °C");
    display.drawString(75, 50, horario);
    display.display();  
}

void gravarDadosEEPROM(String &str) 
{
  int i;

  EEPROM.write(0, str.length());
  
  for(i = 0; i < str.length(); i++) 
  {
    EEPROM.write(i + 1, str[i]);    
  }

  EEPROM.commit();
}

String lerDadosEEPROM()
{
  char buf[MAX_EPRON_BUFFER];
  int tam;
  int i;

  tam = EEPROM.read(0);
  
  for(i = 0; i < tam; i++)
  {
    buf[i] = (char)byte(EEPROM.read(i + 1));
  }

  buf[i] = '\0';
  
  return String(buf);
}

String splitString(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void sendMailMessage(String strMailMessage) 
{
    smtpData.setLogin(SMTP_SERVER, SMTP_SERVER_PORT, EMAIL_SENDER_ACCOUNT, EMAIL_SENDER_PASSWORD);
    smtpData.setSender("ESP32", EMAIL_SENDER_ACCOUNT);
    smtpData.setSubject(EMAIL_SUBJECT);
    smtpData.addRecipient(EMAIL_RECIPIENT);
    smtpData.setPriority("High");
    smtpData.setSendCallback(sendMailMessageCallback);   

    smtpData.setMessage("<div style=\"color:#2f4468;\"><h1>" + strMailMessage + "</h1><p>- Sent from ESP32 board</p></div>", true);    

    if(!MailClient.sendMail(smtpData))
        Serial.println("Error sending Email, " + MailClient.smtpErrorReason());

    smtpData.empty();
}

void sendMailMessageCallback(SendStatus msg) 
{
    Serial.println(msg.info());
    
    if (msg.success()) 
    {
        Serial.println("Email enviado!");
    }
}

String getPublicIP()
{
    WiFiClient client;
    
    if (client.connect("api.ipify.org", 80)) 
    {
        Serial.println("Connecting in api.ipify.org...");
        client.println("GET / HTTP/1.0");
        client.println("Host: api.ipify.org");
        client.println();
    } else {
        Serial.println("Connection to ipify.org failed");
        return String();
    }
    
    delay(5000);
    
    String line;
    
    while(client.available())
    {
      line = client.readStringUntil('\n');
      Serial.println(line);
    }
    
    return line;
}

void setup() 
{
    Serial.begin(115200);  
    EEPROM.begin(MAX_EPRON_BUFFER);
      
    pinMode(PIN_VENTILADOR, OUTPUT);
    pinMode(PIN_MQ6, INPUT_PULLUP);
    pinMode(PIN_CHAMA, INPUT_PULLUP);
    
    digitalWrite(PIN_VENTILADOR, HIGH);

    dht.begin();  
    display.init();
    irsend.begin();  
    
    display.flipScreenVertically();
    display.setContrast(255);
  
    initWiFi();
    initMQTT();

    delay(1000);

    VerificaConexoesWiFIEMQTT();

    timeClient.begin();
    timeClient.forceUpdate();
    
    modo_aut = true;
    MQTT.publish(TOPICO_SUBSCRIBE_VENTILADOR, "3");
    MQTT.publish(TOPICO_PUBLISH_STS_CLI_ONOFF, "OFF");
    MQTT.publish(TOPICO_PUBLISH_STS_CLI_COOLING, "OFF");
    MQTT.publish(TOPICO_PUBLISH_STS_CLI_SPEED, "2");
    MQTT.publish(TOPICO_PUBLISH_STS_CLI_SWING, "OFF"); 
    MQTT.publish(TOPICO_PUBLISH_GAS, "NÃO");
    MQTT.publish(TOPICO_PUBLISH_CHAMA, "NÃO");
	
	Serial.println("ESP32 iniciado!");
}
 
void loop() 
{
    char temperatura_str[10] = {0};
    char temperatura_min_str[10] = {0};
    char temperatura_max_str[10] = {0};
    char umidade_str[10] = {0};
    char horario_str[10] = {0};
    char ip_str[10] = {0};
    char cliSpeed[10] = {0};
    String horario = "";
    String dataAtual = "";
    String dadosEPROM = "";
    String dadosEPROMNovo = "";
    String publicIP = "";
    unsigned long epochTime;
    struct tm *ptm;
    int valor_gas = 0;
    int valor_chama = 0;
	
	VerificaConexoesWiFIEMQTT();
	MQTT.loop();
	
    horario = timeClient.getFormattedTime();
    epochTime = timeClient.getEpochTime();
    ptm = gmtime((time_t *)&epochTime);
    dataAtual = (String)ptm->tm_mday + "/" + (ptm->tm_mon + 1) + "/" + (ptm->tm_year + 1900);
    
    horario.toCharArray(horario_str, horario.length() + 1);

    dadosEPROM = lerDadosEEPROM();

    if(dadosEPROM.indexOf(';') <= 0) 
    {
        dadosEPROM = "00.00;00.00;00/00/0000";
        
        gravarDadosEEPROM(dadosEPROM);
    }
    
    temp_min = splitString(dadosEPROM, ';', 0).toFloat();
    temp_max = splitString(dadosEPROM, ';', 1).toFloat();
    dataAnterior = splitString(dadosEPROM, ';', 2);
    temperatura = faz_leitura_temperatura();
    umidade = faz_leitura_umidade();

    if(temp_ant > 0) 
        if(abs(temp_ant - temperatura) >= 0.5)
            temperatura = temp_ant;            
    else 
        temp_ant = temperatura;

    if(modo_aut) 
    {
        if(temperatura > MAX_TEMP && digitalRead(PIN_VENTILADOR) == HIGH) 
        {        
            digitalWrite(PIN_VENTILADOR, LOW);
            MQTT.publish(TOPICO_PUBLISH_MODO_STS_VENT, "ON");
        }            
        else if(temperatura <= MIN_TEMP && digitalRead(PIN_VENTILADOR) == LOW) 
        {
            digitalWrite(PIN_VENTILADOR, HIGH);
            MQTT.publish(TOPICO_PUBLISH_MODO_STS_VENT, "OFF");
        }
    }

    temp_ant = temperatura;
              
    if(temperatura < temp_min) 
        temp_min = temperatura;

    if(temperatura > temp_max) 
        temp_max = temperatura;

    if((String)temp_min == "-99.99")
        temp_min = temperatura;

    if((String)temp_max == "-99.99") 
        temp_max = temperatura;
        
    if(dataAnterior != dataAtual) 
    {
        dataAnterior = dataAtual;
        temp_min = temperatura;
        temp_max = temperatura;
    }
    
    dadosEPROMNovo = (String)temp_min + ";" + temp_max + ";" + dataAnterior;
    
    if(dadosEPROM != dadosEPROMNovo)
        gravarDadosEEPROM(dadosEPROMNovo);

    sprintf(temperatura_str,"%.2f", temperatura);
    sprintf(temperatura_min_str,"%.2f", temp_min);
    sprintf(temperatura_max_str,"%.2f", temp_max);
    sprintf(umidade_str,"%.2f", umidade);
    
    MQTT.publish(TOPICO_PUBLISH_TEMPERATURA, temperatura_str);
    MQTT.publish(TOPICO_PUBLISH_TEMPERATURA_MIN, temperatura_min_str);
    MQTT.publish(TOPICO_PUBLISH_TEMPERATURA_MAX, temperatura_max_str);
    MQTT.publish(TOPICO_PUBLISH_UMIDADE, umidade_str);
    MQTT.publish(TOPICO_PUBLISH_RELOGIO, horario_str);
    
    printData(temperatura, umidade, horario, temp_min, temp_max);

    valor_gas = digitalRead(PIN_MQ6);
    valor_chama = analogRead(PIN_CHAMA);
    
    if(valor_gas != valor_gas_ant) 
    {
        valor_gas_ant = valor_gas;
        
        if(valor_gas != 1) 
        {
            MQTT.publish(TOPICO_PUBLISH_GAS, "SIM");     
            sendMailMessage("Gás Detectado!");
        }
        else 
            MQTT.publish(TOPICO_PUBLISH_GAS, "NÃO");      
    }

    if(valor_chama <= 0)
        valor_chama = 1;
    else
        valor_chama = 0; 
        
    if(valor_chama != valor_chama_ant)
    {
        valor_chama_ant = valor_chama;

        if(valor_chama != 0)
        {
            MQTT.publish(TOPICO_PUBLISH_CHAMA, "SIM");
            sendMailMessage("Fogo Detectado!");
        }
        else 
            MQTT.publish(TOPICO_PUBLISH_CHAMA, "NÃO");
    }

    if(atualizar_dados) 
    {
        if(modo_aut)
            MQTT.publish(TOPICO_PUBLISH_STS_VENT_AUTO, "YES");
        else
            MQTT.publish(TOPICO_PUBLISH_STS_VENT_AUTO, "NO");
            
        if(digitalRead(PIN_VENTILADOR) == HIGH)
            MQTT.publish(TOPICO_PUBLISH_MODO_STS_VENT, "OFF");
        else
            MQTT.publish(TOPICO_PUBLISH_MODO_STS_VENT, "ON");

        if(stsClimatizadorOn) 
            MQTT.publish(TOPICO_PUBLISH_STS_CLI_ONOFF, "ON");          
        else 
            MQTT.publish(TOPICO_PUBLISH_STS_CLI_ONOFF, "OFF");

        if(stsClimatizadorCooling) 
            MQTT.publish(TOPICO_PUBLISH_STS_CLI_COOLING, "ON");          
        else 
            MQTT.publish(TOPICO_PUBLISH_STS_CLI_COOLING, "OFF");

        if(stsClimatizadorSwing) 
            MQTT.publish(TOPICO_PUBLISH_STS_CLI_SWING, "ON");          
        else 
            MQTT.publish(TOPICO_PUBLISH_STS_CLI_SWING, "OFF");

        sprintf(cliSpeed, "%d", stsClimatizadorSpeed);
        MQTT.publish(TOPICO_PUBLISH_STS_CLI_SPEED, cliSpeed);

        if(valor_gas != 1) 
            MQTT.publish(TOPICO_PUBLISH_GAS, "SIM");     
        else 
            MQTT.publish(TOPICO_PUBLISH_GAS, "NÃO");

         if(valor_chama != 0)
            MQTT.publish(TOPICO_PUBLISH_CHAMA, "SIM");
        else 
            MQTT.publish(TOPICO_PUBLISH_CHAMA, "NÃO");
            
        publicIP = getPublicIP();

        if(publicIP != "") 
        {
            publicIP.toCharArray(ip_str, publicIP.length() + 1);
            MQTT.publish(TOPICO_PUBLISH_IP, ip_str);
        }

        atualizar_dados = false;
    }
}
