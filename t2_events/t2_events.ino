#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include <event_groups.h>
#include <timers.h>

/* The tasks to be created. */
static void barberTask( void *pvParameters );
static void clientTask( void *pvParameters );

int arriveTimeClient_1 = 200;
int arriveTimeClient_2 = 400;

int cuttingTimeBarber_1 = 200;
int cuttingTimeBarber_2 = 500;

EventGroupHandle_t xEventGroup;

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:

  /* Before an event group can be used it must first be created. */
  xEventGroup = xEventGroupCreate();

  /* Create the task that sets event bits in the event group. */
  xTaskCreate( clientTask, "Bit Setter", 100, NULL, 1, NULL );
  /* Create the task that waits for event bits to get set in the event group. */
  xTaskCreate( barberTask, "Bit Reader", 100, NULL, 1, NULL );

  /* Start the scheduler so the created tasks start executing. */
  vTaskStartScheduler();
}

static void barberTask( void *pvParameters ) {
  EventBits_t xEventGroupValue;
  for ( ;; )
  {
    if (xEventGroupGetBits(xEventGroup) != 0) {
      vPrintString("O barbeiro acordou\n");
      while (xEventGroupValue = xEventGroupWaitBits(xEventGroup, B11 , pdTRUE, pdFALSE, 0 )) {
        if ( ( xEventGroupValue & B01 ) != 0 ) {
          vPrintString("Barbeiro cortando o cabelo do cliente\n");
          vTaskDelay(random(cuttingTimeBarber_1, cuttingTimeBarber_2) / portTICK_PERIOD_MS);
        }
        if ( ( xEventGroupValue & B10 ) != 0 ) {
          vPrintString("Barbeiro cortando o cabelo do cliente\n");
          vTaskDelay(random(cuttingTimeBarber_1, cuttingTimeBarber_2) / portTICK_PERIOD_MS);
        }
      }
      vPrintString("Barbeiro indo dormir\n");
    }
  }
}

static void clientTask( void *pvParameters) {
  EventBits_t xEventGroupValue;
  for ( ;; )
  {
    vTaskDelay(random(arriveTimeClient_1, arriveTimeClient_2) / portTICK_PERIOD_MS);
    vPrintString("Um cliente acabou de chegar\n");

    xEventGroupValue = xEventGroupGetBits(xEventGroup);

    if(xEventGroupValue == B01){
      vPrintString("O cliente foi esperar na sala de espera\n");
      xEventGroupSetBits( xEventGroup, B10);
    } else if(xEventGroupValue == B00){
      vPrintString("O cliente foi esperar na sala de espera\n");
      xEventGroupSetBits( xEventGroup, B01);
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


  if ( Serial.available() )  {
    vTaskEndScheduler();
  }
}

void loop() {}
