#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "DHT.h"
#include <FastBot.h>

#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Структура для хранения конфигурационных параметров
struct Configuration {
  const char* ssid;      // SSID Wi-Fi сети
  const char* password;  // Пароль Wi-Fi сети
  int DHTPin;            // Пин для DHT датчика
  int maxTemp;           // Порог критической температуры
};

// Задаем конфигурацию
Configuration config = {
  "NAME_SSID",  // SSID
  "PASSWORD",   // пароль
  D1,           // пин для DHT
  30,           // порог критической температуры
};


// Раскомментируйте одну из строк ниже в зависимости от того, какой датчик вы используете!
// #define DHTTYPE DHT11  // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
#define DHTTYPE DHT22  // DHT 22  (AM2302), AM2321


// Установите логин и пароль при открытии WEB-адреса.
const char* www_username = "username";
const char* www_password = "password";


// Создаем объект веб-сервера CreateAsync на порту 80
AsyncWebServer server(80);

// датчик DHT
DHT dht(config.DHTPin, DHTTYPE);

// текущая температура и влажность, обновляемые в цикле loop()
float t = 0.0;
float h = 0.0;

// иницилизация критической температуры
int temp;
int maxTemp = config.maxTemp;  // порог критической температуры на которой начнем присылать уведомления в void criticalTemp().
const unsigned long TEMP_MTBS = 120000;
unsigned long temp_lasttime;
bool isCriticalMessageSent = false;
unsigned long lastCriticalMessageTime = 0;
const unsigned long CRITICAL_MESSAGE_INTERVAL = 120000;  // Интервал в миллисекундах (например, 2 минуты)

unsigned long startTime = 0;  // время запуска программы


// Значение быстро станет слишком большим для хранения в int
unsigned long previousMillis = 0;  // сохранить время последнего обновления DHT


// Обновляем показания DHT каждые 10 секунд
const long interval = 10000;


// Инициализируем Telegram BOT
#define BOT_TOKEN "token"  // токен Telegram-бота
FastBot bot(BOT_TOKEN);


const char* www_realm = "ESP8266_TS";


// HTML
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>

<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset="UTF-8">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
      font-family: Arial;
      display: inline-block;
      margin: 0px auto;
      text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>ESP8266 TS</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#0cc1a9;"></i> 
    <span class="dht-labels">Температура: </span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#29bee1;"></i> 
    <span class="dht-labels">Влажность: </span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">%</sup>
  </p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 10000 ) ;
</script>
<footer>
  <span class="dht-labels">Проект доступен на <a href="https://github.com/vladios13/ESP8266_TS" target="_blank">GitHub</a></span>
</footer>
</html>)rawliteral";


String processor(const String& var) {
  //Serial.println(var);
  if (var == "TEMPERATURE") {
    return String(t);
  } else if (var == "HUMIDITY") {
    return String(h);
  }
  return String();
}

// Сообщения в Telegram, команды: /t, /time.
void newMsg(FB_msg& msg) {
  if (msg.text == "/t") {
    int t = dht.readTemperature();
    int h = dht.readHumidity();
    String readDHT = "Показатели:\n";
    readDHT = "🌡 Температура: ";
    readDHT += int(t);
    readDHT += "°C\n";
    readDHT += "💦 Влажность: ";
    readDHT += int(h);
    readDHT += "%\n";

    Serial.print(msg.username);
    bot.sendMessage(readDHT, msg.chatID);
  } else if (msg.text == "/time") {
    unsigned long currentTime = millis();
    unsigned long uptime = currentTime - startTime;
    int seconds = uptime / 1000;
    int minutes = seconds / 60;
    int hours = minutes / 60;

    String uptimeMsg = "⏱ Время работы: ";
    uptimeMsg += hours;
    uptimeMsg += " часов, ";
    uptimeMsg += minutes % 60;
    uptimeMsg += " минут, ";
    uptimeMsg += seconds % 60;
    uptimeMsg += " секунд.";

    bot.sendMessage(uptimeMsg, msg.chatID);
  }

  Serial.println(msg.toString());
}

// Критическая температрура
void criticalTemp() {
  if (!isCriticalMessageSent && millis() - temp_lasttime > TEMP_MTBS) {
    if (temp >= maxTemp) {
      String crit_temp = "🚨 Критическая температура: " + String(temp) + "°C!";
      bot.sendMessage(crit_temp);
      isCriticalMessageSent = true;
      lastCriticalMessageTime = millis();
    }
    temp_lasttime = millis();
  }

  // Проверка для сброса флага через определенный интервал (критическая температура).
  if (isCriticalMessageSent && millis() - lastCriticalMessageTime > CRITICAL_MESSAGE_INTERVAL) {
    isCriticalMessageSent = false;
  }
}

void setupDHT() {
  pinMode(config.DHTPin, INPUT);
  dht.begin();
}

void setupWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(config.ssid, config.password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("WiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void setupBot() {
  bot.setChatID("1111111,2222222");  // ID-пользователей которые вправе запускать бота.
  bot.attach(newMsg);

  bot.sendMessage("🟢 ONLINE ESP8266");
  Serial.println("Telegram Connected");
}

void setupServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (!request->authenticate(www_username, www_password))
      return request->requestAuthentication(www_realm, "Authentication failed");

    AsyncWebServerResponse* response = request->beginResponse_P(200, "text/html", index_html, processor);
    response->addHeader("Cache-Control", "max-age=86400");
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (!request->authenticate(www_username, www_password))
      return request->requestAuthentication(www_realm, "Authentication failed");
    request->send_P(200, "text/plain", String(t).c_str());
  });

  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (!request->authenticate(www_username, www_password))
      return request->requestAuthentication(www_realm, "Authentication failed");
    request->send_P(200, "text/plain", String(h).c_str());
  });

  server.onNotFound([](AsyncWebServerRequest* request) {
    request->send(404, "text/plain", "Not Found");
  });

  server.begin();
  Serial.println("HTTP server started");
}

void setup() {
  Serial.begin(115200);
  delay(100);

  setupWiFi();
  setupBot();
  setupServer();
  setupDHT();

  startTime = millis();  // устанавливаем время запуска программы
}

void loop() {
  bot.tick();
  temp = int(t);

  // Если сработало сообщение о критической температуре, проверям и отправляем сообщение только в том случае, если температура восстановилась.
  unsigned long currentMillis = millis();  // текущее время
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    float newT = dht.readTemperature();
    float newH = dht.readHumidity();

    if (isnan(newT) || isnan(newH)) {
      Serial.println("Failed to read from DHT sensor!");
    } else {
      t = newT;
      h = newH;

      // Проверка на восстановление температуры
      if (isCriticalMessageSent && t < maxTemp) {
        isCriticalMessageSent = false;
        bot.sendMessage("🟢 Температура восстановилась в норму.");
      }
    }
  }

  criticalTemp();
}
