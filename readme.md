# Умный мониторинг ESP8266 и Telegram

Это маленький проект представляет собой систему мониторинга температуры и влажности с использованием платформы ESP8266, датчика DHT, и возможности отправки уведомлений в Telegram.

------------

## Описание

Проект включает в себя следующие компоненты:

- **ESP8266:** Микроконтроллер, используемый для чтения данных с датчика DHT и отправки уведомлений в Telegram.
- **Датчик DHT:** Измеряет температуру и влажность.
- **Telegram Bot:** Отправляет уведомления о текущей температуре и влажности.

------------

## Функциональность

- Отображение и обновление в реальном времени текущей температуры и влажности на веб-странице.
- HTTP-аутентификация по логину и паролю.
- Возможность мониторинга данных через Telegram.
- Уведомления о критической температуре.

------------

## Использование

1. **Подключение ESP8266:** Подключите ESP8266 к вашей Wi-Fi сети, указав SSID и пароль в конфигурации.
2. **Установка Telegram Bot:** [Создайте бота в Telegram](https://core.telegram.org/bots#how-do-i-create-a-bot "Создайте бота в Telegram") и укажите его токен в конфигурации.
3. **Настройка датчика DHT:** Укажите тип используемого датчика DHT в конфигурации.
4. **Загрузка кода:** Загрузите скомпилированный код на ESP8266.

------------

## Сборка проекта

1. Клонируйте репозиторий: `git clone https://github.com/vladios13/ESP8266_TS`
2. Откройте проект в Arduino IDE.
3. Установите необходимые библиотеки через менеджер библиотек Arduino. ([ESPAsyncTCP](https://github.com/me-no-dev/ESPAsyncTCP "ESPAsyncTCP")), ([ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer "ESPAsyncWebServer")), ([FastBot](https://github.com/GyverLibs/FastBot/ "FastBot")).
4. Скомпилируйте и загрузите код на ESP8266.

------------

## Благодарности

- [shameermohamed](https://github.com/Tech-Trends-Shameer/Esp-8266-Projects/blob/main/ESP-8266-Temperature-Web-Server/esp-8266-temperature-web-server.ino#L60 "shameermohamed") за код с асинхронным веб-сервером. 

------------
