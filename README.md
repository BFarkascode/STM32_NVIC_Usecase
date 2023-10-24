# STM32_NVIC_Usecase
Usecase for the nested vector controller (NVIC)

This is a project to showcase, how/what/where/why nested vector controls (NVIC) are.

It is going to be a simple bootloader prototype that will navigate between a bootloader and an app. The app is created relying on the findings in the NVMDriver project - that is, we will define the app at a certain memory address - so as to put it into a different partition than where the bootloader is. The app's NVICs will be adjusted to work within that memory section only, isolating the app from the bootloader. The NVICs then will be manipulated to do a software-based reboot and to transition from the boot to the app.  

NVIC_Usecase_Blinky is the app made to match the bootloader. It is placed in a memory section separate from the boot (check linker file) and has its vector table moved to match that place (check system file). The app is otherwise just a simple Blinky (with again HAL_Delay as the timer control and bare metal register control to avoid any additional HAL commands).

