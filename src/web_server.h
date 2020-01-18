AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void sendJSON(AsyncWebSocketClient *client, JsonDocument &doc)
{
  size_t len = measureJson(doc);
  AsyncWebSocketMessageBuffer *buffer = ws.makeBuffer(len);
  if (buffer)
  {
    serializeJson(doc, (char *)buffer->get(), len + 1);
    if (client)
    {
      client->text(buffer);
    }
    else
    {
      ws.textAll(buffer);
    }
  }
}

JsonDocument getCurrentSettings()
{
  DynamicJsonDocument doc(500);
  JsonObject settings = doc.createNestedObject("settings");

  settings["current_mode"] = newMode;
  settings["max_bright"] = max_bright;
  settings["demo_mode"] = demorun;
  settings["demo_duration"] = demo_duration;

  JsonArray modes = settings.createNestedArray("my_modes");

  for (size_t i = 0; i < my_mode_count; i++)
  {
    modes.add(my_modes[i]);
  }

  return doc;
}

JsonDocument processInputMessage(char *data)
{
  StaticJsonDocument<300> doc;
  DeserializationError error = deserializeJson(doc, data);
  if (error)
  {
    PRINT("deserializeJson() failed: ");
    PRINTLN(error.c_str());

    DynamicJsonDocument res(300);
    res["message"] = "deserializeJson() failed: " + String(error.c_str());

    return res;
  }

  String cmd = doc["cmd"];
  PRINT("cmd: ");
  PRINTLN(cmd);

  if (cmd != "null")
  {
    if (cmd == "RESET_ESP")
    {
      ESP.restart();
    }
    else if (cmd == "PREVIOUS_MODE")
    {
      previousMode();
    }
    else if (cmd == "NEXT_MODE")
    {
      nextMode();
    }
    else if (cmd == "CURRENT_MODE")
    {
      uint8_t value = doc["value"];
      SetMode(value);
    }
    else if (cmd == "DEMO_MODE")
    {
      demorun = doc["value"];
    }
    else if (cmd == "DEMO_DURATION")
    {
      demo_duration = doc["value"];
    }
    else if (cmd == "MAX_BRIGHT")
    {
      int value = doc["value"];
      setBrightness(value);
    }

    DynamicJsonDocument res(200);
    res["message"] = cmd;

    return res;
  }

  StaticJsonDocument<60> res;

  res["message"] = "unsupported string";

  return res;
}


void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  if (type == WS_EVT_CONNECT)
  {
    //client connected
    IPAddress ip = client->remoteIP();
    uint32_t num = client->id();
    const char *url = server->url();

    PRINTF("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], url);

    String msg = "Connected\r\nBuild: " + String(SRC_REV) +
                 "\r\nBuild date: " + String(SRC_DATE) +
                 "\r\nLast reset reason: " + getLastResetReason() +
                 "\r\nESP.getFreeHeap() : " + ESP.getFreeHeap() +
                 "\r\nesp_get_free_heap_size() : " + esp_get_free_heap_size() +
                 "\r\nesp_get_minimum_free_heap_size() : " + esp_get_minimum_free_heap_size();

    // send message to client
    DynamicJsonDocument doc(400);
    doc["message"] = msg;
    sendJSON(client, doc);

    JsonDocument settingsJson = getCurrentSettings();
    sendJSON(client, settingsJson);
  }
  else if (type == WS_EVT_DISCONNECT)
  {
    //client disconnected
    PRINTF("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
  }
  else if (type == WS_EVT_ERROR)
  {
    //error was received from the other end
    PRINTF("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t *)arg), (char *)data);
  }
  else if (type == WS_EVT_PONG)
  {
    //pong message was received (in response to a ping request maybe)
    PRINTF("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char *)data : "");
  }
  else if (type == WS_EVT_DATA)
  {
    //data packet
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len)
    {
      //the whole message is in a single frame and we got all of it's data
      PRINTF("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT) ? "text" : "binary", info->len);
      if (info->opcode == WS_TEXT)
      {
        data[len] = 0;
        PRINTF("%s\n", (char *)data);

        JsonDocument res = processInputMessage((char *)data);

        PRINTLN(measureJson(res));
        sendJSON(client, res);
      }
      else
      {
        for (size_t i = 0; i < info->len; i++)
        {
          PRINTF("%02x ", data[i]);
        }
        PRINTF("\n");
      }
      if (info->opcode == WS_TEXT)
        client->text("I got your text message");
      else
        client->binary("I got your binary message");
    }
    else
    {
      //message is comprised of multiple frames or the frame is split into multiple packets
      if (info->index == 0)
      {
        if (info->num == 0)
          PRINTF("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
        PRINTF("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
      }

      PRINTF("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT) ? "text" : "binary", info->index, info->index + len);
      if (info->message_opcode == WS_TEXT)
      {
        data[len] = 0;
        PRINTF("%s\n", (char *)data);
      }
      else
      {
        for (size_t i = 0; i < len; i++)
        {
          PRINTF("%02x ", data[i]);
        }
        PRINTF("\n");
      }

      if ((info->index + len) == info->len)
      {
        PRINTF("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
        if (info->final)
        {
          PRINTF("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
          JsonDocument res = processInputMessage((char *)data);
          sendJSON(client, res);
          if (info->message_opcode == WS_TEXT)
            client->text("I got your text message");
          else
            client->binary("I got your binary message");
        }
      }
    }
  }
}

void initWebServer()
{
  SPIFFS.begin(true);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html");
  });
  server.on("/routine.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/Routine.js");
  });

  server.onNotFound([](AsyncWebServerRequest *request) { request->send(404); });

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  server.begin();
}
