


#include <bcm2835.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>


#define PIN RPI_GPIO_P1_11 /*Pin 11 is GPIO 17 (6th pin down on left)*/
#define BCM2837_ST_BASE 0x3F003000 /*BCM 2837 System Timer */ 
#define TIMERDROP 0x80000000  /*31 bit number, time drop when counter rolls over*/ 
#define WORD_SIZE   2U


volatile unsigned *TIMER_registers; 
long int ntime, pntime, cntime, dntime; /*Time regesters in nano seconds*/
unsigned int TIMER_GetSysTick(); /*Function to get accurate system time*/
void TIMER_Init(); /*Set up timer*/
void SPI_Run();





void main()
{
	//bcm2835_set_debug(1);
	int period = 500000; /*us  (10000 is 10 ms)*/
	unsigned short Data[4096];
	int i;
	for(i=0 ; i<=4096; i++)
	{
		Data[i] = i;
	}
	
	if (!bcm2835_init())
    {
      printf("bcm2835_init failed. Are you running as root??\n");
    }
    else
    {

		if (!bcm2835_spi_begin())
		{
			printf("bcm2835_spi_begin failed. Are you running as root??\n");
		}
		else
		{
			bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
			bcm2835_spi_setDataMode(BCM2835_SPI_MODE1);                   
			bcm2835_spi_setClockDivider(25); 
			bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // The default
			bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the default
	
			///*set number of loops wanted*/
			//loop = ITERCNT; 
			///*Initalise loop increamenter*/
			//n = 0; 
	
			/* Timer setup*/
			TIMER_Init(); 

 
			/* Set the pin to be an output */
			bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_OUTP);  
	 
			/* Get system time */
			pntime = TIMER_GetSysTick(); 
			/* Initalise current time */
			cntime = pntime; 
			
			while(1)
			{
				/*Get current time */
				cntime = TIMER_GetSysTick(); 
				/*Find differnece between current and previous time (in nano seconds) */
				dntime = cntime-pntime; 
			
				if (dntime<=0) /* If counter has rolled over adjust previous time */
				{
					/* Find true time difference */ /* * Untested ! * */
					pntime -= TIMERDROP;  
				}	
			
			
				if (dntime >= period)  /* If time difference is greater than period */
				{
			
					/*set pin to high (3.3v)*/
					bcm2835_gpio_write(PIN, HIGH); 
					/* Set new previous time */
					pntime = cntime; 
					SPI_Run(Data);	
				}
				else
				{
					/* else set pin to low */
					bcm2835_gpio_write(PIN, LOW); 
				}
			}
		}
	}
}

void SPI_Run(unsigned short rData[4096])
{
	char spiOut[2];
	int m;

	for(m=0; m<4096; m++)
	{
		spiOut[0] = (char)(rData[m]>>8);
		spiOut[1] = (char)(rData[m] & 0x00FF);
		bcm2835_spi_transfern(spiOut, WORD_SIZE);
	}
}




unsigned int TIMER_GetSysTick()
{
    return TIMER_registers[1];
}

void TIMER_Init()
{
    /* open /dev/mem */
    int TIMER_memFd;
    if ((TIMER_memFd = open("/dev/mem", O_RDWR /*|O_SYNC*/) ) < 0)
		{
        printf("can't open /dev/mem - need root ?\n");
		}
    else
		{
		/* mmap BCM System Timer */
		void *TIMER_map = mmap(
			NULL,
			4096, /* BLOCK_SIZE */
			PROT_READ /*|PROT_WRITE*/,
			MAP_SHARED,
			TIMER_memFd,
			BCM2837_ST_BASE
		);

		close(TIMER_memFd);
	
		if (TIMER_map == MAP_FAILED)
			{
			printf("mmap error %d\n", (int)TIMER_map);
			}
		else
			{
			TIMER_registers = (volatile unsigned *)TIMER_map;
			}
		}
}


