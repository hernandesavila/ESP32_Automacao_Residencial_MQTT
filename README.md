# 🏠 Automação Residencial MQTT com ESP32

Automação Residencial MQTT com ESP32 é um projeto de firmware que transforma o microcontrolador em um hub doméstico conectado, unindo sensores ambientais, controle de ventilação, acionamento de climatizador por infravermelho e notificações de segurança via MQTT e e-mail.

O sistema conecta-se automaticamente a uma rede Wi-Fi e a um broker MQTT, publica telemetria (temperatura, umidade e estado dos dispositivos), recebe comandos remotos e reage a eventos críticos de gás ou chama, enviando alertas imediatos.

---

## 🛠️ Tecnologias Utilizadas

<p align="center">
  <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/arduino/arduino-original.svg" alt="Arduino IDE" width="30" height="30"/>
  <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/cplusplus/cplusplus-original.svg" alt="C/C++" width="30" height="30"/>
  <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/googlecloud/googlecloud-original.svg" alt="MQTT (broker)" width="30" height="30"/>
</p>

- **ESP32 + Arduino Core** – plataforma alvo do firmware.
- **C/C++ (Arduino)** – linguagem utilizada no sketch principal (`.ino`).
- **MQTT (PubSubClient)** – protocolo de mensagens para telemetria e comandos.
- **Bibliotecas adicionais**: `WiFi`, `DHT`, `SH1106Wire`, `IRremoteESP8266`, `NTPClient`, `EEPROM`, `ESP32_MailClient`.

---

## 📂 Estrutura do Projeto

- `Automacao_Residencial_MQTT.ino` – sketch principal com leitura de sensores, controle MQTT, automação IR, persistência em EEPROM e envio de e-mails.
- `LICENSE` – arquivo de licença (MIT).

---

## ✅ Pré-requisitos

- Placa **ESP32** com os sensores e atuadores conectados conforme definido no sketch (DHT22, OLED SH1106, emissor IR, MQ-6, sensor de chama e ventilador).
- **Arduino IDE** (ou PlatformIO) com suporte à placa ESP32 instalado.
- Bibliotecas: `PubSubClient`, `DHT sensor library`, `ESP8266 and ESP32 OLED driver for SSD1306 displays (SH1106)`, `IRremoteESP8266`, `NTPClient`, `ESP32 Mail Client`.
- Rede Wi-Fi disponível e acesso a um broker MQTT.
- Conta SMTP para envio de e-mails de alerta.

---

## ⚙️ Configuração

1. **Credenciais e serviços**
   - Atualize `SSID` e `PASSWORD` com a rede Wi-Fi local.
   - Ajuste `BROKER_MQTT` e `BROKER_PORT` com o broker desejado (padrão `broker.emqx.io:1883`).
   - Defina as credenciais de e-mail (`EMAIL_SENDER_ACCOUNT`, `EMAIL_SENDER_PASSWORD`, `EMAIL_RECIPIENT`, `SMTP_SERVER`, `SMTP_SERVER_PORT`, `EMAIL_SUBJECT`).
   - Altere o servidor NTP (`servidorNTP`) caso utilize outro serviço.

2. **Tópicos MQTT**
   - Assine os tópicos de controle (`topico_ventilador`, `topico_cli_onoff`, `topico_cli_cooling`, `topico_cli_speed`, `topico_cli_swing`, `topico_update_data`).
   - Publique medições e estados em tópicos específicos para temperatura, umidade, hora, ventilador, climatizador, alertas e modo automático.

3. **EEPROM e limites**
   - Persista limites de temperatura mínima e máxima e a data da última atualização para restaurar dados após reinicializações.

---

## 🛠️ Compilação e Upload

1. Abra `Automacao_Residencial_MQTT.ino` no **Arduino IDE**.
2. Selecione a placa ESP32 apropriada e a porta serial.
3. Instale/importar as bibliotecas listadas em Pré-requisitos.
4. Compile e envie o sketch para o dispositivo.
5. Acompanhe o Monitor Serial (115200 bps) para verificar logs de conexão e eventos.

---

## ▶️ Execução

1. Após o boot, o ESP32 inicializa sensores, display, emissor IR, conecta-se ao Wi-Fi e ao broker MQTT, sincroniza a hora e ativa o modo automático do ventilador.
2. Em cada ciclo, registra temperatura/umidade, atualiza a EEPROM com valores mínimo/máximo diários e mostra as informações no display OLED.
3. O ventilador pode operar automaticamente dentro da faixa configurada ou manualmente via comandos MQTT; o climatizador recebe sequências IR para ligar/desligar, ativar resfriamento, ajustar velocidade e swing.
4. Sensores de gás e chama disparam alertas imediatos, publicam mensagens nos tópicos e enviam e-mails de emergência.
5. Quando solicitado, o firmware responde com o IP público e o estado completo dos dispositivos para o painel MQTT.

---

## 🔎 Funcionamento Interno

- O callback MQTT interpreta comandos numéricos para alternar modos automáticos, ligar/desligar dispositivos e confirmar alterações publicando os estados atualizados.
- Leituras inválidas do DHT retornam um valor sentinela, evitando registros inconsistentes.
- A função `printData` monta a tela do OLED com temperatura, umidade, limites configurados, hora e indicadores de falha.
- Alertas e relatórios utilizam `ESP32_MailClient` e serviços HTTP para recuperar o IP público.

---

## 📌 Observações

- Ajuste `MIN_TEMP` e `MAX_TEMP` para adaptar o modo automático ao ambiente.
- Regrave as sequências IR se o climatizador alvo utilizar códigos diferentes.
- Personalize os tópicos MQTT conforme a sua estrutura de broker/painel.

---

## 📄 Licença

Este projeto está licenciado sob a [MIT License](LICENSE).
