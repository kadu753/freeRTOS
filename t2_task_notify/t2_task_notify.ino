#include <Arduino_FreeRTOS.h>
#include <queue.h>

#define configUSE_TASK_NOTIFICATIONS 1

/* The tasks to be created. */
static void barberTask( void *pvParameters );
static void clientTask( void *pvParameters );

int numClients = 2;

int arriveTimeClient_1 = 200;
int arriveTimeClient_2 = 400;

int cuttingTimeBarber_1 = 200;
int cuttingTimeBarber_2 = 500;

TaskHandle_t HandlerTask = NULL;
QueueHandle_t waitingRoom;

void setup( void )
{
  Serial.begin(9600);

  waitingRoom = xQueueCreate(numClients, sizeof(int));

  xTaskCreate( barberTask, "Barber", 200, NULL, 1, &HandlerTask );

  xTaskCreate( clientTask, "Clients", 200, NULL, 1, NULL );

  vTaskStartScheduler();

  for ( ;; );
}

static void barberTask( void *pvParameters ) {
  for ( ;; ) {
    if ( ulTaskNotifyTake(pdFALSE, portMAX_DELAY) != 0 ) {
      vPrintString("O barbeiro acordou\n");
      while (xQueueReceive(waitingRoom, &HandlerTask, 0) == pdPASS) {
        vPrintString("Barbeiro cortando o cabelo de um cliente\n");
        ulTaskNotifyTake(pdFALSE, 0);
        vTaskDelay(random(cuttingTimeBarber_1, cuttingTimeBarber_2) / portTICK_PERIOD_MS);
      }
      vPrintString("Ninguém na sala de espera. Barbeiro indo dormir\n");
    }
  }
}


static void clientTask( void *pvParameters) {
  for ( ;; )
  {
    vTaskDelay(random(arriveTimeClient_1, arriveTimeClient_2) / portTICK_PERIOD_MS);
    vPrintString("Um cliente acabou de chegar\n");
    if (xQueueSendToBack(waitingRoom, &HandlerTask, 0) == pdPASS) {
      vPrintString("O cliente foi esperar na sala de espera\n");
      xTaskNotifyGive(HandlerTask);
    } else {
      vPrintString("Não tinha mais lugar o cliente foi embora\n");
    }
  }
}

void vPrintString( const char *pcString ) {
  vTaskSuspendAll();
  {
    Serial.print(pcString);
    Serial.flush();
  }
  xTaskResumeAll();
}

void loop() {}
