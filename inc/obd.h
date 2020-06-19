/*
 * obd.h
 *
 *  Created on: Jun 18, 2020
 *      Author: jose
 */

#ifndef PROGRAMS_TF_RTOSI_INC_OBD_H_
#define PROGRAMS_TF_RTOSI_INC_OBD_H_
#include "sapi.h"


bool_t bOBDInit(void);				// Prototipo de función de inicialización del módulo obd del proyecto.
uint16_t iGetOBDInfo(void);			// Prototipo de función para la entrega de información recolectada.
bool_t bModifyOBDSampleTime(uint16_t new_sample_delay);	// Prototipo de función que modifica el sample time del OBD.


#endif /* PROGRAMS_TF_RTOSI_INC_OBD_H_ */
