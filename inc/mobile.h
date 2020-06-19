/*
 * mobile.h
 *
 *  Created on: Jun 18, 2020
 *      Author: jose
 */

#ifndef PROGRAMS_TF_RTOSI_INC_MOBILE_H_
#define PROGRAMS_TF_RTOSI_INC_MOBILE_H_

#include "sapi.h"

bool_t bMobileInit(void);			// Función de inicialización del módulo mobile del proyecto
void bleSend(void);					// Función para el envío de información a través de bluetooth (usando RS232)

#endif /* PROGRAMS_TF_RTOSI_INC_MOBILE_H_ */
