#include "../spidy.h"
#include "../hw.h"
#include "../utils.h"
#include "../io.h"
#include "../gpio.h"

int n, flag=0;

int spidy_main(void *unused)
{
	puts("Your friendly neighborhood Spider-Man\n");

	n=0;

	gpio_init();

	return 0;
}

/* In boot.S there is a loop of spidy_step (never ending story) */
void spidy_step(void *unused)
{
	#if 0

	n++;
	if (n > 30000) {
		n%=30000;
		flag=!flag;
	}
	for (i=1; i<=2 ;i++)
		for (j=0; j<=8 ;j++)
			gpio_dir(GPIO_NR(i,j), 1, flag);

	#else

	int i, j;
	int p = 3;	/* port */
	int b = 0;	/* bit */
	int nr = 32;	/* port * 32 + bit */
	int number = 0;	/* use number? */
	for(j=0;j<100;j++) {
	        while (regs[REG_TMR32B1PC] >= 10)
			;

		if (number)
			gpio_dir(nr, 1, 1);
		else
			gpio_dir(GPIO_NR(p,b), 1, 1);

        	while (regs[REG_TMR32B1PC] >= 10 && regs[REG_TMR32B1PC] < 10800)
        		;

		if (number)
			gpio_dir(nr, 1, 0);
		else
			gpio_dir(GPIO_NR(p,b), 1, 0);

        	while (regs[REG_TMR32B1PC] >= 10800)
	        	;

		for(i=0; i<19;i++) {
		        while (regs[REG_TMR32B1PC] >= 0 && regs[REG_TMR32B1PC] < 10)
				;

	        	while (regs[REG_TMR32B1PC] >= 10)
	        		;
		}
	}

	for(j=0;j<100;j++) {
	        while (regs[REG_TMR32B1PC] >= 10)
			;

		if (number)
			gpio_dir(nr, 1, 1);
		else
			gpio_dir(GPIO_NR(p,b), 1, 1);


  	        while (regs[REG_TMR32B1PC] >= 10)
			;


        	while (regs[REG_TMR32B1PC] >= 0 && regs[REG_TMR32B1PC] < 10200)
        		;

		if (number)
			gpio_dir(nr, 1, 0);
		else
			gpio_dir(GPIO_NR(p,b), 1, 0);

		for(i=0; i<18;i++) {
		        while (regs[REG_TMR32B1PC] >= 0 && regs[REG_TMR32B1PC] < 10)
				;

	        	while (regs[REG_TMR32B1PC] >= 10)
	        		;
		}
	}
	#endif
}
