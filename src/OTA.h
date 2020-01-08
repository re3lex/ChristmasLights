

void initOTA()
{
  ArduinoOTA.setHostname("ESP32_TEST");

  ArduinoOTA.onStart([]() {
    //ledOn();
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
    {
      type = "sketch";
    }
    else
    { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using
    // SPIFFS.end()
    PRINTLN("Start updating " + type);
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    //ledSwap();
    PRINTF("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onEnd([]() {
    // ledOff();
    PRINTLN("\nEnd");
  });
  ArduinoOTA.onError([](ota_error_t error) {
    // ledOff();
    PRINTF("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)
    {
      PRINTLN("Auth Failed");
    }
    else if (error == OTA_BEGIN_ERROR)
    {
      PRINTLN("Begin Failed");
    }
    else if (error == OTA_CONNECT_ERROR)
    {
      PRINTLN("Connect Failed");
    }
    else if (error == OTA_RECEIVE_ERROR)
    {
      PRINTLN("Receive Failed");
    }
    else if (error == OTA_END_ERROR)
    {
      PRINTLN("End Failed");
    }
  });
  ArduinoOTA.begin();
}