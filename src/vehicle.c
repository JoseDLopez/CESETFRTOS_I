/*
 * vehicle.c
 *
 *  Created on: Jun 18, 2020
 *      Author: jose
 */


#include "vehicle.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "supporting_functions.h"
#include "task.h"
#include "semphr.h"
#include "sapi.h"
#include "obd.h"

static void vSampleOBD (void *pvParameters );		// Prototipo de la función de muestreo de OBD
static void vAbrirPuertas (void *pvParameters);		// Prototipo de la función de apertura de puertas.
static void vCerrarPuertas (void *pvParameters);	// Prototipo de la función de cierre de puertas.

uint16_t last_sampled_value;						// Valor global para guardar última muestra realizada
uint16_t sample_ms;									// Valor que almacena en ms cada cuanto tiempo se debe realizar la muestra.
vehicle_t vehicle;									// Instanciamos una variable vehiculo de tipo vehicle.


/** Función de incialización del móduo de vehículo. Inicializa la variable vehicle
 * asignando los pines correspondientes, y crea además los semáforos que sincronizan las tareas
 * También crea las tareas correspondientes.
 */
bool_t bVehicleInit(void){
	BaseType_t res;											// Variable que nos permite saber si la tarea fue creada correctamente.
	vehicle.ABRIR = LED1;									// Se asigna pin LED1 para el abrir puertas.
	vehicle.CERRAR = LED2;									// Se asigna pin LED2 para el cierre de puertas.
	vehicle.IMMO = LED3;									// Se asigna pin LED3 para señalizar la inmovilización del vehículo.
	vehicle.ENGINE = GPIO3;									// Se asigna pin GPIO3 como entrada del estado del motor.
	gpioInit(vehicle.ENGINE,GPIO_INPUT);					// Se inicializa el GPI03 como pin de entrada.
	vehicle.xSemaforoAbrir = xSemaphoreCreateBinary();		// Se crea semáforo para abrir puertas.
	vehicle.xSemaforoCerrar = xSemaphoreCreateBinary();		// Se crea semáforo para cerrar puertas.

	if (vehicle.xSemaforoAbrir == NULL || vehicle.xSemaforoCerrar == NULL){		// Corroboramos de que los semáforos fueron creados correctamente
		vPrintString("No fue posible crear los semáforos para el auto\r\n");
		return 0;
	}

	res = xTaskCreate(vAbrirPuertas,					// Función que se va a ejecutar cuando la tarea tome su lugar en el scheduler.
					(const char *)"T_OPEN_DOORS",		// Identificador de la tarea
					configMINIMAL_STACK_SIZE,			// Memoria asignada
					NULL,								// No se envía variable.
					tskIDLE_PRIORITY+1,					// Prioridad mínima
					NULL);								// No se envía handle para modificar prioridad de tarea
	if(res == pdFAIL){									// Confirmamos si la tarea pudo ser creada correctamente.
		vPrintString("No fue posible crear la tarea para abrir puertas del auto.\r\n");
		return 0;										// Devolvemos 0 ya  que no fue posible crearla.
	}

	res = xTaskCreate(vCerrarPuertas,					// Función que se va a ejecutar cuando la tarea tome su lugar en el scheduler.
					(const char *)"T_CLOSE_DOORS",		// Identificador de la tarea
					configMINIMAL_STACK_SIZE,			// Memoria asignada
					NULL,								// No se envía variable.
					tskIDLE_PRIORITY+1,					// Prioridad mínima
					NULL);								// No se envía handle para modificar prioridad de tarea
	if(res == pdFAIL){									// Confirmamos si la tarea pudo ser creada correctamente.
		vPrintString("No fue posible crear la tarea para cerrar puertas del auto.\r\n");
		return 0;										// Devolvemos 0 ya  que no fue posible crearla.
	}

	gpioWrite(LEDG,ON);									// Led que señaliza que no hubo problemas inicializando el módulo.
	return 1;											// Retornamos 1 para indicar que no hubo problema iniciando el auto.

}

/** Función privada que realiza la tarea de apertura de puertas del auto
 * Va a encender y apagar el led (representando el pulso que excita la tarjeta
 * de cierre centralizado del vehículo
 */
static void vAbrirPuertas(void *pvParameters){
	const TickType_t xActionDelay = pdMS_TO_TICKS( DOORS_DELAY );	// Tiempo que va a durar el led encendido.
	for(;;){
		xSemaphoreTake(vehicle.xSemaforoAbrir, portMAX_DELAY);		// Tratamos de tomar el semáforo para ejecutar la tarea.
		vPrintString("Se realiza apertura de puertas\r\n");
		gpioWrite(vehicle.ABRIR, ON);								// Encendemos el led.
		vTaskDelay(xActionDelay);									// Esperamos que se cumpla el segundo.
		gpioWrite(vehicle.ABRIR, OFF);								// Apagamos el led.
	}
}


/** Función privada que realiza la tarea de cierre de puertas del auto
 * Va a encender y apagar el led (representando el pulso que excita la tarjeta
 * de cierre centralizado del vehículo
 */
static void vCerrarPuertas(void *pvParameters){
	const TickType_t xActionDelay = pdMS_TO_TICKS( DOORS_DELAY );	// Tiempo que va a durar el led encendido.
	for(;;){
		xSemaphoreTake(vehicle.xSemaforoCerrar, portMAX_DELAY);		// Tratamos de tomar el semáforo para ejecutar la tarea.
		vPrintString("Se realiza cierre de puertas\r\n");
		gpioWrite(vehicle.CERRAR, ON);								// Encendemos el led.
		vTaskDelay(xActionDelay);									// Esperamos que se cumpla el segundo.
		gpioWrite(vehicle.CERRAR, OFF);								// Apagamos el led.
	}
}


/** Función pública que recibe la solicitud de apertura de puertas
 * Permite que el semáforo de la tarea de apertura de puertas sea tomado.
 */
bool_t bOpenDoorsRequest(){
	BaseType_t res;									// Variable que obtiene resultado de la entrega del semáforo.
	res = xSemaphoreGive(vehicle.xSemaforoAbrir);	// Tratamos de hacer entrega del semáforo.
	if (res == pdPASS){								// Verificamos que pudo ser entregado.
		vPrintString("Se recibe la solicitud de apertura de puertas.\r\n");
		return 1;									// Devolvemos True indicando que fue entregado el semáforo.
	}else{
		vPrintString("No se pudo recibir una nueva solicitud de apertura de puertas.\r\n");
		return 0;									// El semáforo no pudo ser entregado.
	}
}

/** Función pública que recibe la solicitud de cierre de puertas
 * Permite que el semáforo de la tarea de cierre de puertas sea tomado.
 */
bool_t bCloseDoorsRequest(){
	BaseType_t res;										// Variable que obtiene resultado de la entrega del semáforo.
	res = xSemaphoreGive(vehicle.xSemaforoCerrar);		// Tratamos de hacer entrega del semáforo.
	if (res == pdPASS){									// Verificamos que pudo ser entregado.
		vPrintString("Se recibe la solicitud de cierre de puertas.\r\n");
		return 1;										// Devolvemos True indicando que fue entregado el semáforo.
	}else{
		vPrintString("No se pudo recibir una nueva solicitud de cierre de puertas.\r\n");
		return 0;										// El semáforo no pudo ser entregado.
	}
}


/** Función pública que resuelve la solicitud de inmovilización del vehículo */
bool_t bTurnImmoONRequest(){
	gpioWrite(vehicle.IMMO, ON);
	vPrintString("Se habilita el inmovilizador del auto.\r\n");
	return 1;
}

/** Función pública que resuelve la solicitud de desinmovilización del vehículo */
bool_t bTurnImmoOFFRequest(){
	gpioWrite(vehicle.IMMO, OFF);
	vPrintString("Se deshabilita el inmovilizador del auto.\r\n");
	return 1;
}

/** Función pública que resuelve la solicitud de estado del motor */
bool_t bEngineStatusRequest(){
	bool_t status = gpioRead(vehicle.ENGINE);
	vPrintString("Se recibe el estado del motor.\r\n");
	return status;
}

