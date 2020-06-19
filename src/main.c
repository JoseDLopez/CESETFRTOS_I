/*=============================================================================
 * Copyright (c) 2020, José López <josedlopez11@gmail.com>
 * All rights reserved.
 * License: bsd-3-clause (see LICENSE.txt)
 * Date: 2020/06/18
 * Version: 1.0
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

/* Librerias de FreeRTOS. */
#include "FreeRTOS.h"					// Librería FreeRTOS
#include "task.h"						// Librería para creación de tareas FreeRTOS
#include "FreeRTOSConfig.h"				// Archivo de configuración para FreeRTOS
#include "supporting_functions.h"		// Librerias de soporte que vienen con FreeRTOS
#include "sapi.h"						// Librería SAPI
#include "obd.h"						// Libreria para extraer información simulada del OBD
#include "vehicle.h"					// Libreria que acciona los periféricos del vehículo
#include "mobile.h"						// Libreria que se comunica con el móvil usando BLE.


bool_t obd_start = 0;
bool_t vehicle_start = 0;
bool_t mobile_start = 0;

/*------------------ MAIN PRINCIPAL --------------------------*/
int main (){
	boardConfig(); 																	// Iniciamos la configuración de la tarjeta
	vPrintString("Inicio de programa. Inicializando distintos modulos:\r\n");

	obd_start = bOBDInit();															// Iniciamos el módulo OBD del proyecto.
	vehicle_start = bVehicleInit();													// Iniciamos el módulo Vehicle del proyecto.
	mobile_start = bMobileInit();													// Iniciamos el módulo Mobile del proyecto.

	if (obd_start && vehicle_start && mobile_start){
		vPrintString("Se inician correctamente los modulos.\r\n");
		vPrintString("Inicia el programador del SO:\r\n");
		vTaskStartScheduler();														// Se inica el programador de tareas.
	}else {
		vPrintString("Hubo un problema los modulos programa.\r\n");
	}

	for (;;);
	return 0;
}
/*------------------ FIN MAIN PRINCIPAL --------- -----------*/
