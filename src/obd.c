/*
 * obd.c
 *
 *  Created on: Jun 18, 2020
 *      Author: jose
 */

#include "obd.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "supporting_functions.h"
#include "task.h"
#include "sapi.h"

static void vSampleOBD (void *pvParameters );		// Prototipo de la función de muestreo de OBD
uint16_t last_sampled_value;						// Valor global para guardar última muestra realizada
uint16_t sample_ms;									// Valor que almacena en ms cada cuanto tiempo se debe realizar la muestra.


/** Función pública de inicialización de OBD
 * Contempla la inicialización de variables globales y la creación
 * de la tarea de muestreo, validando que fue creada correctamente.
 * Devuelve 0 si NO pudo crear.
 * Devuelve 1 si crea la tarea ok.
 */
bool_t bOBDInit(void){
	last_sampled_value = 0;							// Inicializamos el último valor guardado en 0.
	sample_ms = 10000;								// Inicializamos que la frecuencia de muestreo va a ser cada 10ms.
	adcConfig( ADC_ENABLE );						// Configuramos el ADC ya que de acá vamos a simular el valor de las revoluciones obtenidas del OBD
	BaseType_t res = xTaskCreate(vSampleOBD,		// Función que se va a ejecutar cuando la tarea tome su lugar en el scheduler.
				(const char *)"Muestra de OBD",		// Identificador de la tarea
				configMINIMAL_STACK_SIZE,			// Memoria asignada
				NULL,								// No se envía variable.
				tskIDLE_PRIORITY+1,					// Prioridad mínima
				NULL);								// No se envía handle para modificar prioridad de tarea

	if(res == pdFAIL){								// Confirmamos si la tarea pudo ser creada correctamente.
		vPrintString("No fue posible crear la tarea para muestreo de OBD\r\n");
		return 0;									// Devolvemos 0 ya  que no fue posible crearla.
	}else{
		gpioWrite(LEDR,ON);							// Led señalizador de que esta módulo fue inicializado correctamente.s
		return 1;									// Devolvemos 1 ya que fue creda ok.
	}
}


/** Función privada de Muestreo de OBD
 * Esta función se ejecuta cada vez que la tarea Muestra de OBD tiene
 * lugar en el scheduler. La función se bloquea cada sample_ms milisegundos
 * Realiza la lectura del ADC y guarda el valor tomado en una variable
 * llamada last_sampled_value.
 */
static void vSampleOBD(void *pvParameters){
	TickType_t xLastWakeTime;									// Creamos la varaible para definir la última vez que se ejecutó la tarea
	xLastWakeTime = xTaskGetTickCount();						// Esta variable guarda el last Tick Count
	for(;;){
		TickType_t xTicksToWait = pdMS_TO_TICKS( sample_ms );	// Definimos en tickets cuanto tiempo debe esperar la tarea para ejecutarse
		last_sampled_value  = adcRead( CH1 );					// Realizamos lectura del ADC y guardamos el valor en la variable.
		vPrintStringAndNumber("Se lee rpm: ",last_sampled_value);
		vTaskDelayUntil (&xLastWakeTime, xTicksToWait);			// Indicamos que la tarea se bloqueará sample_ms milisegundos desde la última vez que se ejecutó.
	}
}


/** Función que devuelve el valor guardado
 * recolectado del OBD
 */
uint16_t iGetOBDInfo(void){
	return last_sampled_value;					// Retorno de valor solicitado.
}


/** Función que permite modificar la tasa de
 * muestreo del OBD. Recibe un valor que debe ser mayor a 5 segundos
 * y menor a 10 minutos. En caso de modificar correctamente el valor
 * se devuelve un 1. Si no es posible, devuelve 0.
 */
bool_t bModifyOBDSampleTime(uint16_t new_sample_delay){
	if (new_sample_delay > 4999 && new_sample_delay < 100000){		// Comparación para verificar que el nuevo ms esta dentro del rango.
		sample_ms = new_sample_delay;								// Establecemos el nuevo valor en la variable sample_ms
		vPrintString("Se realiza el cambio de tasa de muestreo\r\n");
		return 1;
	}else{
		return 0;
	}
}

