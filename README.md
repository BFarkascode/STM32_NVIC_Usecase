# STM32_NVIC_Usecase
Usecase for the nested vector controller (NVIC)

This is a project to showcase, how/what/where/why nested vector controls (NVIC) are.

## General description
The project will consist of two, separate elements:
1) The app (called “NVIC_Usecase_Blinky”), a simple Blinky example created relying on the findings in the NVMDriver project
2) The boot (called “NVIC_Usecase”), a simple bootloader prototype
The two elements will be stand-alone and would need to be executed separately to prepare the mcu for the project. (It is possible to bundle them together, albeit doing so would execute the app after the boot independently to the boot, something we don’t want to do.)
Technically, what we will do is
- define a simple Blinky app at a certain memory address
- define a boot that will execute a precise jump from the executable section of the boot to the executable section of the app.
NVICs of the boot and the app will be used to navigate between a bootloader and the app.

## To read
Findings discussed within the STM32_NVMDriver project (linker file, memory locations) will be used extensively in this project. As such, it is HIGHLY recommended to check that project out before taking a look at this one. For all intents and purposes, I will assume that all the information shared there have already been understood properly.
The absolutely relevant sections in the refman to use NVICs (I am using the refman for the L0x3 here):
-	12.3. Interrupt and exception vectors: a table of the vector table, where the vectors are, if they can be modified and what they do.
That’s it, only two pages.
Frankly, that hasn’t been enough for me to understand, what they are doing, so let me explain by my own words, what NVICs are.

### NVIC
NVICs are called the vector table, though, in reality, they are the addresses of various pointers…or more like pointers to pointers. Imagine the vector table as a “favourites/quick dial” list in your phone book where “phone numbers” are “pointers” to reach certain functions and processes. This favourites list exists to allow you a quick access to certain numbers in your phone book your are using a lot…and those numbers will allow you quick access to the mates you want to reach a lot.

You see, when the mcu is doing its thing, it has no idea, where data or commands are, it must be told to go and find them using pointers. In order to at least give the mcu some level of stability, the vector table holds the pointers for certain crucial actions and functions that are used by the mcu a lot. Such functions are to find the start of the memory stack (called the memory stack pointer, which is put ALWAYS as the very first pointer in the NVIC, technically on app memory base address), the reset (the second pointer in the NVIC, for 32-bit mcus, this will be on  app memory base address + 4 where the +4 is the size of the memory stack pointer) the clock interrupt (on +8) and the hard fault interrupt pointer (on decimal +12 – or in hex, +C). Of note, these three processes are ALLWAYS at the same place and thus are automatically found by the mcu, plus whenever they are called by the mcu, they take priority over everything else.

The rest of the vectors in the NVIC are settable and can be shuffled around if need arises. As the refman table 55 shows, they include such  processes as calling systick, engaging EXTI interrupts or to have the timer interrupt.

As indicated already above, this vector table is assigned as the very first set of memory positions in FLASH, practically drawing a line in memory and saying that everything AFTER these vectors will be part of a code that is supposed to be executed. Of note, the vector table is not, strictly speaking, part of the application code but external to it, while pointing to certain sections within it. This means that every executable app must be generated with a vector table physically attached to the start of the app (1) and that the vector table MUST have its own designated memory location physically placed “before” the app (2).

Assigning (1) is done at a previously not discussed part of the code, namely in the “system_stm32l0xx.c” file. This is our system file, assigning the most basic functions for the mcu when running an app, including the vector table. We need to find the “miscellaneous configuration” section and change it to the following part:
```
/************************* Miscellaneous Configuration ************************/

/* Note: Following vector table addresses must be defined in line with linker
         configuration. */
/*!< Uncomment the following line if you need to relocate the vector table
     anywhere in Flash or Sram, else the vector table is kept at the automatic
     remap of boot address selected */
#define USER_VECT_TAB_ADDRESS

#if defined(USER_VECT_TAB_ADDRESS)
/*!< Uncomment the following line if you need to relocate your vector Table
     in Sram else user remap will be done in Flash. */
/* #define VECT_TAB_SRAM */
#if defined(VECT_TAB_SRAM)
#define VECT_TAB_BASE_ADDRESS   SRAM_BASE       /*!< Vector Table base address field.
                                                     This value must be a multiple of 0x200. */
#define VECT_TAB_OFFSET         0x00000000U     /*!< Vector Table base offset field.
                                                     This value must be a multiple of 0x200. */
#else
#define VECT_TAB_BASE_ADDRESS   FLASH_BASE      /*!< Vector Table base address field.
                                                     This value must be a multiple of 0x200. */
#define VECT_TAB_OFFSET         0x00008000U     /*!< Vector Table base offset field.
                                                     This value must be a multiple of 0x200. */
#endif /* VECT_TAB_SRAM */
#endif /* USER_VECT_TAB_ADDRESS */

/******************************************************************************/
/**
```
As indicated, we have uncommented the “#define USER_VECT_TAB_ADDRESS” line and changed the “#define VECT_TAB_OFFSET” to 0x00008000U. These will tell the app that we wish to generate our vector table at a custom location and that custom location will be 0x8000 worth of addresses shifted. Mind, the “FLASH_BASE” address is the actual physical start address of the FLASH and NOT the app’s base FLASH address. This difference is important to remember, that is that the FLASH memory allocation done within the linker file can be different compared to the actual physical FLASH memory. We must not mix the two and know and which spot which one is being used (to generate the vector table, we use the base physical address, for the isr_vector, we use the linker-defined app base address).

Taking a look into (2), that part we have already seen in the STM32_NVMDriver project when we discussed the linker file. As it goes, the “.isr_vector” memory section define in the linker file is where the linker will allocate the memory for the app to put its vector table. As indicated in that project, this memory section MUST always be placed as the very first memory section relative to all other memory sections…and it must be placed at the same absolute memory location as the one defined in (1).

Failing to either assign the memory location for the vector table properly or to properly generate it at the right place will mean that the app will refuse to execute: the mcu will not be able to speed dial the stack pointer to find the app’s start position, then reset all functions to execute it.

## Particularities

### App

The system file is changed as indicated above, generating the vector table with an offset of 0x8000, or at the absolute position of 0x8008000.

The linker file assigns the FLASH memory section as a 32 kbyte area starting from 0x8008000. Since we don’t do any additional custom memory allocation, the isr_vector section will automatically placed at 0x8008000. The memory area between 0x8000000 and 0x8008000 will be ignored by the app as if it didn’t exist – that’s where our boot will go.

As expected, the absolute memory position for the vector table and the isr_vector will be identical.

### Boot
We modify the linker file to have the boot’s FLASH memory be only 32 kbyte and end at 0x8007fff. This will prevent the boot and the app to accidentally corrupt each other.

From a memory management point of view, the app’s memory addresses would not be existent for the boot and vice versa. This technically means that we are telling the mcu take a “leap of faith” into an unknown memory location whenever we are switching between the two sections. Mind, the mcu WILL totally execute such a leap of faith no matter what and then crash if it does not land on a proper vector. Our job is to ensure that it will land on such a vector.

## User guide


