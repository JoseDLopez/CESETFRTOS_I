/*
 * mobile.c
 *
 *  Created on: Jun 18, 2020
 *      Author: jose
 */
#include "mobile.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "supporting_functions.h"
#include "task.h"
#include "semphr.h"
#include "sapi.h"
#include "vehicle.h"
#include "obd.h"

#define BLE UART_232					// Se asigna UART
#define BLE_IRQn USART3_IRQn			// Se indica cual es el request de interrupción
#define BLE_BAUDRATE 9600				// Se indica el BAUDRATE de 9600

SemaphoreHandle_t xSemaforoBLE_Receive;
SemaphoreHandle_t xSemaforoBLE_Send;
char caracter_recibido;

static void vBLERead (void *pvParameters);
static void vBLESend (void *pvParameters);
void vBLEReceiveCallBack(void *pvParameters);

bool_t bMobileInit(void){
	uartConfig(BLE, BLE_BAUDRATE);											// Configuramos el UART para el BLE y su baudrate
	uartCallbackSet(BLE, UART_RECEIVE, vBLEReceiveCallBack, NULL);			// Fijamos que cuando se reciba data por el uart se ejecute la tarea BLE_Receive_CallBack
	NVIC_SetPriority(BLE_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY + 5);	// Se asigna la prioridad de interrupciòn como nivel 5.
	NVIC_EnableIRQ(BLE_IRQn);												// Se habilita el request de interrupcion

	xSemaforoBLE_Receive = xSemaphoreCreateBinary();
	xSemaforoBLE_Send = xSemaphoreCreateBinary();
	if (xSemaforoBLE_Receive == NULL || xSemaforoBLE_Send == NULL){			// Corroboramos de que los semáforos fueron creados correctamente
		vPrintString("No fue posible crear el semaforo para el BLE.\r\n");
		return 0;
	}

	BaseType_t res1 = xTaskCreate(vBLERead,		// Función que se va a ejecutar cuando la tarea tome su lugar en el scheduler.
			(const char *)"BLE_READ",			// Identificador de la tarea
			configMINIMAL_STACK_SIZE,			// Memoria asignada
			NULL,								// No se envía variable.
			tskIDLE_PRIORITY+1,					// Prioridad mínima
			NULL);								// No se envía handle para modificar prioridad de tarea

	BaseType_t res2 = xTaskCreate(vBLESend,		// Función que se va a ejecutar cuando la tarea tome su lugar en el scheduler.
				(const char *)"BLE_SEND",		// Identificador de la tarea
				configMINIMAL_STACK_SIZE * 2,	// Memoria asignada
				NULL,							// No se envía variable.
				tskIDLE_PRIORITY+4,				// Prioridad mínima
				NULL);							// No se envía handle para modificar prioridad de tarea

	if(res1 == pdFAIL || res2 == pdFAIL){		// Confirmamos si la tarea pudo ser creada correctamente.
		vPrintString("No fue posible crear la tarea para lectura/escritura por BLE.\r\n");
		return 0;								// Devolvemos 0 ya  que no fue posible crearla.
	}

	gpioWrite(LEDB,ON);
	return 1;
}

void vBLEReceiveCallBack(void *pvParameters){
	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;
	uint8_t c = uartRxRead(BLE);
	if ((c != '\n') && (c != '\r')){
		caracter_recibido = c;
		xSemaphoreGiveFromISR(xSemaforoBLE_Receive, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}

static void vBLERead (void *pvParameters){
	BaseType_t res;
	for(;;){
		xSemaphoreTake(xSemaforoBLE_Receive, portMAX_DELAY);		// Tratamos de tomar el semáforo para leer
		switch (caracter_recibido){
			case '1':
				if (bOpenDoorsRequest()){
					vPrintString("Se realiza la solicitud de apertura de puertas correctamente.\r\n");
				}else{
					vPrintString("Hubo un error con la solicitud de apertura de puertas.\r\n");
				}
				break;
			case '2':
				if (bCloseDoorsRequest()){
					vPrintString("Se realiza la solicitud de apertura de puertas correctamente.\r\n");
				}else{
					vPrintString("Hubo un error con la solicitud de apertura de puertas.\r\n");
				}
				break;
			case '3':
				if (bTurnImmoONRequest()){
					vPrintString("Se realiza la solicitud de inmovilizacion del auto.\r\n");
				}else{
					vPrintString("Hubo un error con la solicitud de inmovilizacion del auto.\r\n");
				}
				break;
			case '4':
				if (bTurnImmoOFFRequest()){
					vPrintString("Se realiza la solicitud de des-inmovilizacion del auto.\r\n");
				}else{
					vPrintString("Hubo un error con la solicitud de des-inmovilizacion del auto.\r\n");
				}
				break;
			case '5':
				res = xSemaphoreGive(xSemaforoBLE_Send);		// Tratamos de hacer entrega del semáforo.
				if (res == pdPASS){									// Verificamos que pudo ser entregado.
					vPrintString("Se recibe la solicitud de informacion de estado de motor.\r\n");
				}else{
					vPrintString("Hubo un error con la solicitud de informacion de estado de motor.\r\n");
				}
				break;
			case '6':
				if (bModifyOBDSampleTime(5000)){
					vPrintString("Se realiza la solicitud de cambio de frecuencia de muestreo a 5 segundos.\r\n");
				}else{
					vPrintString("Hubo un error con la solicitud de cambio de frecuencia de muestreo a 5 segundos.\r\n");
				}
				break;
			case '7':
				if(bModifyOBDSampleTime(10000)){
					vPrintString("Se realiza la solicitud de cambio de frecuencia de muestreo a 10 segundos.\r\n");
				}else{
					vPrintString("Hubo un error con la solicitud de cambio de frecuencia de muestreo a 10 segundos.\r\n");
				}
				break;
			default:
				printf( "Recibimos <<%c>> por UART\r\n", caracter_recibido );
				break;
		}
	}
}


static void vBLESend (void *pvParameters){
	for(;;){
		xSemaphoreTake(xSemaforoBLE_Send, portMAX_DELAY);		// Tratamos de tomar el semáforo para leer
		taskENTER_CRITICAL();
		char message[50];
		sprintf(message, "Motor: %d. ",bEngineStatusRequest());
		uartWriteString(BLE, message);
		sprintf(message, "RPM: %d.\r\n",iGetOBDInfo());
		uartWriteString(BLE, message);
		taskEXIT_CRITICAL();
	}
}
