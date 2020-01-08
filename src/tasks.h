
void ledSwapTask(void *pvParameters)
{
  while (true)
  {
    ledSwap();

    PRINTF("ledSwapTask complete running on Core %d\r\n",xPortGetCoreID());
    vTaskDelay(1000 / portTICK_RATE_MS);
  }
}

void task2(void *pvParameters)
{
  while (true)
  {
    PRINTF("task2 complete running on Core %d\r\n", xPortGetCoreID());

    vTaskDelay(1500 / portTICK_RATE_MS);
  }
}

TaskHandle_t TaskA, TaskB;

void initTasks(){
  xTaskCreatePinnedToCore(
      ledSwapTask, /* pvTaskCode */
      "Workload1", /* pcName */
      1000,        /* usStackDepth */
      NULL,        /* pvParameters */
      1,           /* uxPriority */
      &TaskA,      /* pxCreatedTask */
      0);          /* xCoreID */

  xTaskCreatePinnedToCore(
      task2,       /* pvTaskCode */
      "Workload2", /* pcName */
      1000,        /* usStackDepth */
      NULL,        /* pvParameters */
      1,           /* uxPriority */
      &TaskB,      /* pxCreatedTask */
      1);          /* xCoreID */
}