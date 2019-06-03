#include <Arduino_FreeRTOS.h>
#include "semphr.h"

#define configUSE_COUNTING_SEMAPHORES       1

static void barberTask( void *pvParameters );
static void clientTask( void *pvParameters );

SemaphoreHandle_t xCountingSemaphoreClients;
SemaphoreHandle_t xMutexBarberWorking;
SemaphoreHandle_t xMutexBarberSleeping;

int numClients = 2;

int arriveTimeClient_1 = 200;
int arriveTimeClient_2 = 400;

int cuttingTimeBarber_1 = 200;
int cuttingTimeBarber_2 = 500;

void setup( void )
{
  Serial.begin(9600);

  xCountingSemaphoreClients = xSemaphoreCreateCounting( numClients, 0 );
  xMutexBarberSleeping = xSemaphoreCreateMutex();
  xMutexBarberWorking = xSemaphoreCreateMutex();

  xSemaphoreTake(xMutexBarberSleeping, portMAX_DELAY);

  if ( xCountingSemaphoreClients != NULL && xMutexBarberSleeping != NULL && xMutexBarberWorking != NULL)
  {

    xTaskCreate( barberTask, "Barber", 200, NULL, 2, NULL );

    xTaskCreate( clientTask, "Clients", 200, NULL, 2, NULL );

    vTaskStartScheduler();
  }

  for ( ;; );
}


static void barberTask( void *pvParameters ) {
  for ( ;; ) {

    vPrintString("O barbeiro acordou\n");

    xSemaphoreTake(xMutexBarberWorking, portMAX_DELAY);
    while (xSemaphoreTake(xCountingSemaphoreClients, 0) == pdPASS) {
      vPrintString("Barbeiro cortando o cabelo de um cliente\n");
      vTaskDelay(random(cuttingTimeBarber_1, cuttingTimeBarber_2) / portTICK_PERIOD_MS);
    }
    xSemaphoreGive(xMutexBarberWorking);

    vPrintString("Ninguém na sala de espera. Barbeiro indo dormir\n");
    xSemaphoreTake(xMutexBarberSleeping, portMAX_DELAY);
  }
}


static void clientTask( void *pvParameters) {

  for ( ;; )
  {

    vTaskDelay(random(arriveTimeClient_1, arriveTimeClient_2) / portTICK_PERIOD_MS);
    vPrintString("Um cliente acabou de chegar\n");

    if (xSemaphoreTake(xMutexBarberWorking, 0) == pdFAIL) {
      if (xSemaphoreGive(xCountingSemaphoreClients) == pdPASS) {
        vPrintString("O cliente foi esperar na sala de espera\n");
      } else {
        vPrintString("Não tinha mais lugar o cliente foi embora\n");
      }
    } else {
      vPrintString("O cliente foi esperar na sala de espera\n");
      xSemaphoreGive(xCountingSemaphoreClients);
      xSemaphoreGive(xMutexBarberSleeping);
      xSemaphoreGive(xMutexBarberWorking);
    }
  }
}

void vPrintString( const char *pcString )
{
  vTaskSuspendAll();
  {
    Serial.print(pcString);
    Serial.flush();
  }
  xTaskResumeAll();
}

void loop() {}
