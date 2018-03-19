
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/pin_map.h"

#include <stdint.h>

#include "drum_samples.h"

volatile const uint8_t *ps = 0;					// Start of sample
volatile const uint8_t *pe = 0;					// End of sample
												//
volatile unsigned timer = 0;					// Sample timer
												//
struct TLOOP {									// Sample loops
	const uint8_t *ps;							//
	unsigned time;								//
} loop[] = {									//
#if 1											// --- Linn LM-1 samples ---
	lm_bas,		250,							// 4096
	lm_cab,		250,							// 2048
	lm_cga,		250,							// 4096
	lm_clp,		250,							// 4096
	lm_cow,		250,							// 2048
	lm_hat,		250,							// 4096
	lm_rim,		250,							// 2048
	lm_sn,		250,							// 4096
	lm_tmb,		250,							// 4096
	lm_tom,		750,							// 4096
#endif											//
#if 0											//
	lm_bas,		250,							//
	ld_bass1,	250,							//
	ld_bass6,	250,							//
#endif											//
#if 0											//
	lm_cow,		250,							//
	ld_cowbel,	500,							//
#endif											//
#if 0											// --- Linndrum (LM-2) samples ---
	ld_bass1,	250,							// 4096
	ld_bass6,	250,							// 4096
	ld_cbsa1,	250,							// 4096
	ld_clap,	250,							// 4096
	ld_cowbel,	250,							// 4096
	ld_claps,	250,							// 2048
	ld_shak,	250,							// 2048
	ld_snar23,	250,							// 4096
	ld_snare,	250,							// 4096
	ld_sstk1,	250,							// 4096
	ld_tamb,	250,							// 4096
	ld_cga,		250,							// 8192
	ld_snap101,	250,							// 8192
	ld_tom1,	250,							// 8192
	ld_tom6,	250,							// 8192
	ld_tom7,	250,							// 8192
	ld_hihat1,	750,							// 16384
	ld_crsh1,	1500,							// 32768
	ld_ride1,	2000,							// 32768
#endif											//
#if 0											// --- Linn 9000 samples ---
	nk_bass01,	250,							// 8192
	nk_caba01,	250,							// 8192
	nk_clap01,	250,							// 8192
	nk_cnga01,	250,							// 8192
	nk_cowb01,	250,							// 8192
	nk_snar01,	250,							// 8192
	nk_sstk01,	250,							// 8192
	nk_tamb01,	250,							// 8192
	nk_tomm01,	250,							// 8192
	nk_hat01,	1000,							// 16384
	nk_ride01,	1500,							// 32768
	nk_bell01,	1500,							// 32768
	nk_crsh01,	2000,							// 49152
#endif											//
#if 0											// ---- Oberhiem DX samples ---
	dx_conga,		250, 						// 8192
	dx_cowbell,		250, 						// 8192
	dx_hats,		250, 						// 8192
	dx_shake,		250, 						// 8192
	dx_tom,			250, 						// 8192
	dx_fatsnare, 	250, 						// 8192
	dx_kick64,		250,						// 8192
	dx_tambrim,		250, 						// 8192
	dx_timbale,		1000, 						// 8192
#endif
#if 0
												// --- Oberheim DX Drum Machine ---
												// dx_other_samples.c
	dx_ot_clik,		400, 						// 2048
	dx_ot_clap,		400, 						// 2048
	dx_ot_cowb,		400, 						// 2048
	dx_ot_clav,		400, 						// 2048
	dx_ot_clave915,	400, 						// 4096
	dx_ot_toma,		400, 						// 4096
	dx_ot_tomb,		400, 						// 4096
	dx_ot_olcnga,	400, 						// 4096
	dx_ot_shake1,	400, 						// 4096
	dx_ot_beatkick,	500, 						// 8192
	dx_ot_beatsnar,	500, 						// 8192
	dx_ot_bones,	500, 						// 8192
	dx_ot_punch,	500, 						// 8192
	dx_ot_scratch,	500, 						// 8192
	dx_ot_elecsnar,	500, 						// 8192
	dx_ot_elkick3,	500, 						// 8192
	dx_ot_elsnar3,	500, 						// 8192
	dx_ot_eltom3,	500, 						// 8192
	dx_ot_shot,		500, 						// 8192
	dx_ot_lnoise,	750, 						// 16384
	dx_ot_longhat,	1000, 						// 16384
#endif
#if 0
												// --- Oberheim DMX Drum Machine ---
	dmx_21kick,		500, 						// 4096
	dmx_hat1a,		500, 						// 4096
	dmx_sfltom2,	500, 						// 4096
	dmx_shake6,		500, 						// 4096
	dmx_snare6,		500, 						// 4096
	dmx_stik,		500, 						// 4096
	dmx_stom5,		500, 						// 4096
	dmx_crash,		500, 						// 16384
	dmx_ride2a,		500, 						// 16384
	dmx_perid5,		500, 						// 32768
	dmx_dxcr1f,		1000, 						// 32768
#endif
	0,		0									//
};												//

void PwmSetup(unsigned long port, unsigned char pin, unsigned long base, unsigned long timer)
{
	if(port == GPIO_PORTF_BASE) {
		switch(pin) {
			case GPIO_PIN_0: GPIOPinConfigure(GPIO_PF0_T0CCP0); break;
			case GPIO_PIN_1: GPIOPinConfigure(GPIO_PF1_T0CCP1); break;
			case GPIO_PIN_2: GPIOPinConfigure(GPIO_PF2_T1CCP0); break;
			case GPIO_PIN_3: GPIOPinConfigure(GPIO_PF3_T1CCP1); break;
			case GPIO_PIN_4: GPIOPinConfigure(GPIO_PF4_T2CCP0); break;
		}
	} else if(port == GPIO_PORTB_BASE) {
		switch(pin) {
			case GPIO_PIN_6: GPIOPinConfigure(GPIO_PB6_T0CCP0); break;
			case GPIO_PIN_7: GPIOPinConfigure(GPIO_PB7_T0CCP1); break;
		}
	}
	GPIOPinTypeTimer(port, pin);
	TimerPrescaleSet(base, timer, 0);
	//TimerLoadSet(base, timer, 2441);	// 32768 Hz  Linndrum & 9000
	TimerLoadSet(base, timer, 3333);	// 24000 Hz  LM-1
	//TimerMatchSet(base, timer, 1024);
	TimerMatchSet(base, timer, 0);
	TimerControlLevel(base, timer, 1);
	TimerEnable(base, timer);
}

void main(void)
{
	SysCtlClockSet(SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ | SYSCTL_USE_PLL | SYSCTL_SYSDIV_2_5);	// 80 MHz

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);	// Enable ports
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);	// Enable timers
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

													// Configure timers for PWM mode
	TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM | TIMER_CFG_B_PWM);
	TimerConfigure(TIMER1_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM | TIMER_CFG_B_PWM);


													// Configure PWM settings
	PwmSetup(GPIO_PORTB_BASE, GPIO_PIN_6, TIMER0_BASE, TIMER_A);		// 3.03 PD0/PB6 (MSP430 P1.6)
	//PwmSetup(GPIO_PORTF_BASE, GPIO_PIN_1, TIMER0_BASE, TIMER_B);		// Red
	//PwmSetup(GPIO_PORTF_BASE, GPIO_PIN_2, TIMER1_BASE, TIMER_A);		// Blue
	PwmSetup(GPIO_PORTF_BASE, GPIO_PIN_3, TIMER1_BASE, TIMER_B);		// Green


													// Enable Timer0A interrupt
	IntEnable(INT_TIMER0A);
	TimerControlEvent(TIMER0_BASE, TIMER_A, TIMER_EVENT_POS_EDGE);
	TimerIntEnable(TIMER0_BASE, TIMER_CAPA_EVENT);

	IntMasterEnable();								// Global interrupt enable

	struct TLOOP *pl = loop;						//
													//
	for(;;) {										// Forever
		if(!timer) {								// Wait for sample to complete
			const uint16_t len = *(uint16_t *)pl->ps; // Length of sample
			timer = pl->time * 24;					// Duration of sample
			ps = pl->ps + 2;						// Start of sample
			pe = ps + len;							// End of sample
			if(!(++pl)->ps) pl = loop;				// Loop samples
		}											//
	}												//
}

const int16_t mu[256] = {							// mu-Law lookup table (Am6070)
	0,		-8,		-16,	-24,	-32,	-40,	-48,	-56,
	-64,	-72,	-80,	-88,	-96,	-104,	-112,	-120,
	-132,	-148,	-164,	-180,	-196,	-212,	-228,	-244,
	-260,	-276,	-292,	-308,	-324,	-340,	-356,	-372,
	-396,	-428,	-460,	-492,	-524,	-556,	-588,	-620,
	-652,	-684,	-716,	-748,	-780,	-812,	-844,	-876,
	-924,	-988,	-1052,	-1116,	-1180,	-1244,	-1308,	-1372,
	-1436,	-1500,	-1564,	-1628,	-1692,	-1756,	-1820,	-1884,
	-1980,	-2108,	-2236,	-2364,	-2492,	-2620,	-2748,	-2876,
	-3004,	-3132,	-3260,	-3388,	-3516,	-3644,	-3772,	-3900,
	-4092,	-4348,	-4604,	-4860,	-5116,	-5372,	-5628,	-5884,
	-6140,	-6396,	-6652,	-6908,	-7164,	-7420,	-7676,	-7932,
	-8316,	-8828,	-9340,	-9852,	-10364,	-10876,	-11388,	-11900,
	-12412,	-12924,	-13436,	-13948,	-14460,	-14972,	-15484,	-15996,
	-16764,	-17788,	-18812,	-19836,	-20860,	-21884,	-22908,	-23932,
	-24956,	-25980,	-27004,	-28028,	-29052,	-30076,	-31100,	-32124,
	0,		8,		16,		24,		32,		40,		48,		56,
	64,		72,		80,		88,		96,		104,	112,	120,
	132,	148,	164,	180,	196,	212,	228,	244,
	260,	276,	292,	308,	324,	340,	356,	372,
	396,	428,	460,	492,	524,	556,	588,	620,
	652,	684,	716,	748,	780,	812,	844,	876,
	924,	988,	1052,	1116,	1180,	1244,	1308,	1372,
	1436,	1500,	1564,	1628,	1692,	1756,	1820,	1884,
	1980,	2108,	2236,	2364,	2492,	2620,	2748,	2876,
	3004,	3132,	3260,	3388,	3516,	3644,	3772,	3900,
	4092,	4348,	4604,	4860,	5116,	5372,	5628,	5884,
	6140,	6396,	6652,	6908,	7164,	7420,	7676,	7932,
	8316,	8828,	9340,	9852,	10364,	10876,	11388,	11900,
	12412,	12924,	13436,	13948,	14460,	14972,	15484,	15996,
	16764,	17788,	18812,	19836,	20860,	21884,	22908,	23932,
	24956,	25980,	27004,	28028,	29052,	30076,	31100,	32124,
};

void Timer0IntHandler(void)
{
	TimerIntClear(TIMER0_BASE, TIMER_CAPA_EVENT);	// Clear interrupt flag
													//
	static int32_t sample = 1024;					// PWM sample
													//
													// Output previous sample
	TimerMatchSet(TIMER0_BASE, TIMER_A, sample); 	// PWM audio
	TimerMatchSet(TIMER1_BASE, TIMER_B, (sample == 1024) ? 0 : sample); // LED
													//
	sample = 0;										// Prepare for new sample
													//
	if(ps < pe) sample += mu[*ps++];				// If within sample, get sample octet and convert to linear PCM
													//
	sample >>= 6;									// Adjust sample for scale
	sample += 1024;									// Adjust sample for bias
													//
	if(timer) --timer;								// Decrement timer
}

void Timer1IntHandler(void)
{
}
