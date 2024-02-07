/*
 *
 *  Created on: 26 Oct 2023
 *  Project: STM32_NVIC_Usecase/NVIC_Usecase_Boot
 *  File: AppManager.c
 *  Author: BalazsFarkas
 *  Processor: STM32L053R8
 *  Compiler: ARM-GCC (STM32 IDE)
 *  Program version: 1.0
 *  Program description: N/A
 *  Hardware description/pin distribution: N/A
 *  Modified from: N/A
 *  Change history: N/A
 *
 *  Code holds the control commands to transition to an app from a bootloader and vica versa.
 *
 * v.1.0.
 * Below is a simple function to allow the bootloader to control/update/restart the app section.
 * The two functions are practically the same, just the address control is what is different before we engage the NVICs. We assume that te boot and the app are standalone.
 * Resetting is possible by calling the GoToApp in the app or the ReBoot in the boot.
 */

#include "AppManager.h"


//1) Jump to app
/*
 *	What we do here is that we define a function pointer into which we copy the function pointer we (should) find at App_Section_Start_Addr + 4.
 *  From the NVIC table, an app placed by the linker at "App_Section_Start_Addr" will have the reset vector at App_Section_Start_Addr + 4.
 *  Once the copy was successful, we call the function pointer and thus switch to the app.
 *  Mind, the app is a stand-alone element that is limited to the FLASH area App_Section_Start_Addr and after, thanks to the app's linker.
 *
 *  Note: the vector tables will be updated after the jumps given the system files and the linkers are properly set.
 *  Note: after the jump, we start with the startup assembly file (so a full reset occurs)
 *
 * */

void GoToApp(void)
{
	uint32_t App_reset_vector_addr;																	//this is the address of the app's reset vector (which is also a function pointer!)
	void (*Start_App_func_ptr)(void);																//the local function pointer we define

	if((*(uint32_t*)App_Section_Start_Addr) == 0x20002000)											//we check, what is stored at the App_Section_Addr. It should be the very first word of the app's code.
																									//This value should be the reset value of the stack pointer in RAM.
																									//Note: the exact value stored at the App_Section_Addr needs to be checked (it seems to be 0x20002000)
																									//Note: the memory monitor reads out the memory values upside-down! (there is an endian switch during the process)
	{
		printf("APP found. Starting...\r\n");
		App_reset_vector_addr = *(uint32_t*)(App_Section_Start_Addr + 4);							//we define a pointer to APP_ADDR + 4 and then dereference it to extract the reset vector for the app
																									//JumpAddress will hold the reset vector address (which won't be the same as APP_ADDR + 4, the address is just stored there)
		Start_App_func_ptr = App_reset_vector_addr;													//we call the local function pointer with the address of the app's reset vector
																									//Note: for the bootloader, this address is an integer. In reality, it will be a function pointer once the app is placed.
		__set_MSP(*(uint32_t*) App_Section_Start_Addr);												//we move the stack pointer to the APP address
		Start_App_func_ptr();																		//here we call the APP reset function through the local function pointer
	} else {
		printf("No APP found. \r\n");
	}

}

//2) Reboot
/*
 *	This function reboots the microcontroller using software.
 *	It is slower than using hardware reset, but the outcome is the same.
 *	The code is practically the same as boot jump, just the other direction. Since rebooting could occur from the app, there is an address check to see if we have a bootloader.
 *
 * */

void ReBoot(void)
{
	uint32_t Boot_reset_vector_addr;																//this is the address of the app's reset vector (which is also a function pointer!)
	void (*Start_Boot_func_ptr)(void);																//the local function pointer we define

	if((*(uint32_t*)Boot_Section_Start_Addr) == 0x20002000)											//we check, what is stored at the Boot_Section_Addr. It should be the very first word of the app's code.
																									//we do this check since we may run a device without a bootloader
	{
		printf("Rebooting...\r\n");
		Boot_reset_vector_addr = *(uint32_t*)(Boot_Section_Start_Addr + 4);							//we define a pointer to APP_ADDR + 4 and then dereference it to extract the reset vector for the app
																									//JumpAddress will hold the reset vector address (which won't be the same as APP_ADDR + 4, the address is just stored there)
		Start_Boot_func_ptr = Boot_reset_vector_addr;													//we call the local function pointer with the address of the app's reset vector
																									//Note: for the bootloader, this address is an integer. In reality, it will be a function pointer once the app is placed.
		__set_MSP(*(uint32_t*) Boot_Section_Start_Addr);												//we move the stack pointer to the APP address
		Start_Boot_func_ptr();																		//here we call the APP reset function through the local function pointer
	} else {
		printf("Boot not found. \r\n");
	}

}
