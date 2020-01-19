
WiFiManager wm;

void saveConfigCallback()
{
  PRINTLN("saveConfigCallback");
  ESP.restart();
}

bool isAP = false;
void initWiFi()
{
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  wm.setDebugOutput(PRINT_DEBUG);

  //reset settings - wipe credentials for testing
  if (digitalRead(BTN_PIN) == LOW)
  {
    PRINTLN("resetSettings");
    delay(1000);
    wm.resetSettings();
    PRINTLN("reboot");
    delay(2000);
    ESP.restart();
    return;
  }

  
  wm.setConfigPortalBlocking(false);
  wm.setSaveConfigCallback(saveConfigCallback);

  //automatically connect using saved credentials if they exist
  //If connection fails it starts an access point with the specified name
  if (wm.autoConnect(WIFI_AP_SSID, WIFI_AP_PASSWORD))
  {
    PRINTLN("connected...yeey :)");
  }
  else
  {
    PRINTLN("Configportal running");
    isAP = true;
  }
  PRINT("isAP: ");
  PRINTLN(isAP);

  if (!MDNS.begin(WIFI_HOST_NAME))
  {
    PRINTLN("Error setting up MDNS responder!");
    while (1)
    {
      delay(1000);
    }
  }

  PRINTLN("mDNS responder started");
  MDNS.setInstanceName(WIFI_HOST_DESCRIPTION);

  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);
}

/*
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
*/
