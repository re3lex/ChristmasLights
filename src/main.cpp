/*
   notamesh - IR controlled 'synchronized' led lighting effects using FastLED across multiple Arduino controllers.

         By: Andrew Tuline
       Date: October, 2018
        URL: www.tuline.com
      Email: atuline@gmail.com
       Gist: https://gist.github.com/atuline
     GitHub: https://github.com/atuline
   Pastebin: http://pastebin.com/u/atuline
    Youtube: https://www.youtube.com/user/atuline/videos
*/

//  Переделка и дополнение
//  Декабрь 2018
//  Бикин Дмитрий
//  d.bikin@bk.ru
//  SysWork.ru
//  Как подключить пульт
//  https://mysku.ru/blog/aliexpress/68990.html

// Добавлено управление одной кнопкой, декабрь 2019
// AlexGyver
// https://AlexGyver.ru/
// https://www.youtube.com/c/alexgyvershow/

// Проект портирован на ESP32 и PlatformIO.
// Добавлена (планируется) поддержка управления через Web-интерфейс
// Январь 2020
// re3lex
// Внимание: поддержка IR и аналоговых кнопок портирована лишь частично.
// этот функционал не работает!

#include <Arduino.h>
#include "initialization.h"
#include "includes.h"

void initWiFi()
{
  WiFi.persistent(false); // Do not write new connections to FLASH
  WiFi.mode(WIFI_STA);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    ledSwap();
    delay(500);
    PRINT(".");
  }

  PRINTLN("");
  PRINTLN("WiFi connected");
  PRINT("IP address: ");
  PRINTLN(WiFi.localIP());
}

void setup()
{
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  PRINTLN("***** Setup start *****");

  initWiFi();
  initWebServer();
  initOTA();
  //initTasks();

  initRoutine();

  PRINTLN("***** Setup end *****");
}

void loop()
{
  ArduinoOTA.handle();
  ws.cleanupClients();
  
  handleRoutine();
}
