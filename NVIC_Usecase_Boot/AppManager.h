/*
 * AppManager.h						v.1.0.
 *
 *  Created on: 26 Oct 2023
 *      Author: Balazs Farkas
 */

#ifndef INC_APPMANAGER_CUSTOM_H_
#define INC_APPMANAGER_CUSTOM_H_

#include "main.h"
#include "stdint.h"
#include "stdio.h"



//LOCAL CONSTANT
static const uint32_t App_Section_Start_Addr = 0x8008000;					//this is the app section's address. It is defined in the linker files.
static const uint32_t Boot_Section_Start_Addr = 0x8000000;					//this is the boot section's address. It is defined in the boot's linker file.

//LOCAL VARIABLE


//EXTERNAL VARIABLE

//FUNCTION PROTOTYPES
void GoToApp(void);
void ReBoot(void);

#endif /* INC_APPMANAGER_CUSTOM_H_ */
