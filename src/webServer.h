AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void indexrequest(AsyncWebServerRequest *request)
{
  request->send_P(200, "text/html", index_page);
}

String createMessage(String msg)
{
  return "{\"message\":\"" + msg + "\"}";
}

String processInputMessage(String str)
{
  int cmdIdx = str.indexOf("CMD:");
  if (cmdIdx == 0)
  {
    String cmd = str.substring(4, str.length());

    if (cmd == "RST")
    {
      ESP.restart();
    }
    else if (cmd == "switchLED")
    {
      ledSwap();
    }
    return createMessage(cmd);
  }
  return createMessage(str);
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
    String json = createMessage(msg);

    client->text(json);
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
        String res = processInputMessage(String((char *)data));
        client->text(res);
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
          String res = processInputMessage(String((char *)data));
          client->text(res);
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
  server.on("/", HTTP_GET, indexrequest);
  // server.on("/xml", HTTP_PUT, xmlrequest);
  server.onNotFound([](AsyncWebServerRequest *request) { request->send(404); });

  server.on("/hello", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hello World");
  });

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  server.begin();
}
