/*
 * vehicle.h
 *
 *  Created on: Jun 18, 2020
 *      Author: jose
 */

#ifndef PROGRAMS_TF_RTOSI_INC_VEHICLE_H_
#define PROGRAMS_TF_RTOSI_INC_VEHICLE_H_

#include "FreeRTOS.h"
#include "semphr.h"
#include "sapi.h"

typedef struct {
	gpioMap_t ABRIR;					// Referencia a la salida que afecta la apertura de puertas
	SemaphoreHandle_t xSemaforoAbrir;	// Semáforo que permite la ejecución de la tarea de apertura de puertas
	gpioMap_t CERRAR;					// Referencia a la salida que afecta el cierre de puertas
	SemaphoreHandle_t xSemaforoCerrar;	// Semáforo que permite la ejecución de la tarea de cierre de puertas
	gpioMap_t IMMO;						// Variable de salida que inmoviliza el vehículo.
	gpioMap_t ENGINE;					// Pin de entrada que lee el estado del motor.
}vehicle_t;

#define DOORS_DELAY 3000

bool_t bVehicleInit(void);			// Prototipo de función de inicialización del módulo vehicle del proyecto.
bool_t bOpenDoorsRequest(void);		// Prototipo de función que recibe solicitud de apertura de puertas.
bool_t bCloseDoorsRequest(void);		// Prototipo de función que recibe solicitud de cierre de puertas..
bool_t bTurnImmoONRequest(void);		// Prototipo de función que recibe solicitud de habilitación de inmovilizador.
bool_t bTurnImmoOFFRequest(void);	// Prototipo de función que recibe solicitud de deshabilitación de inmovilizador.
bool_t bEngineStatusRequest(void);	// Prototipo de función que recibe solicitud deñ estado del motor.

#endif /* PROGRAMS_TF_RTOSI_INC_VEHICLE_H_ */
