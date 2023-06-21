/**
 * Fakulta informa�n�ch technologi� VUT v Brn�
 * IMP 2020/2021 - projekt
 * Autor: Vojt�ch Mimochodek
 * Login: xmimoc01
 */

#include "MK60D10.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint-gcc.h>
#include <stdbool.h>

/**
 * Bitov� posun pro modifikaci
 * Zdroj: https://community.arm.com/developer/ip-products/processors/f/cortex-m-forum/6679/bit-banding-in-sram-region-cortex-m4
 */
#define BITBAND_REGION_BIT(src_byte_addr, bit)  (((((uint32_t)(src_byte_addr) & 0x000fffff) << 5) | ((uint32_t)(src_byte_addr) & 0xfff00000) | 0x02000000) + (((uint32_t)(bit)) << 2))

#define ADDRESS_ARRAY 32

#define OK 1
#define NOT_OK -42

#define MARCH_X 100
#define MARCH_C 200


void initMCU(void)
{
	MCG_C4 |= ( MCG_C4_DMX32_MASK | MCG_C4_DRST_DRS(0x01) );
	SIM_CLKDIV1 |= SIM_CLKDIV1_OUTDIV1(0x00);
	WDOG_STCTRLH &= ~WDOG_STCTRLH_WDOGEN_MASK;
 }


void initPinAndPort(void)
{
	SIM->SCGC1 = SIM_SCGC1_UART5_MASK;
	SIM->SCGC5 = SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTE_MASK;
	SIM->SCGC6 = SIM_SCGC6_RTC_MASK;
	PORTA->PCR[4] = PORT_PCR_MUX(0x01);
	PORTE->PCR[8]  = PORT_PCR_MUX(0x03);
	PORTE->PCR[9]  = PORT_PCR_MUX(0x03);
	PTA->PDDR =  GPIO_PDDR_PDD(0x0010);
	PTB->PDDR =  GPIO_PDDR_PDD(0x3C);
}


void initUART5(void)
{
	UART5->C2 &= ~(UART_C2_TE_MASK | UART_C2_RE_MASK);
	UART5->BDH = 0x00;
	UART5->BDL = 0x1A;
	UART5->C1 = 0x00;
	UART5->C3 = 0x00;
	UART5->C4 = 0x0F;
	UART5->MA1 = 0x00;
	UART5->MA2 = 0x00;
	UART5->S2 |= 0xC0;
	UART5->C2 |= ( UART_C2_TE_MASK | UART_C2_RE_MASK );
}


/**
 * Funkce pro vys�l�n� jednoho charu - c p�es UART. Tato funkce vy�k�v�, dokud nen� vys�lac� buffer pr�zdn�. Pak odes�l�.
 * Inspirov�no z laboratorn� �lohy �. 1.
 */
void sendChar(char c)
{
	while ( ! (UART5->S1 & UART_S1_TDRE_MASK) && !(UART5->S1 & UART_S1_TC_MASK));
	UART5->D = c;
}


/**
 * Funkce pro vys�l�n� stringu zakon�en�ho 0.
 * Odes�l�n� prob�h� pomoc� funkce sendChar pro odesl�n� jednoho znaku.
 * Inspirov�n� z laboratovn� �lohy �. 1.
 */
void sendString(char *s)
{
	int x = 0;
	while (s[x] != 0) {
		sendChar(s[x++]);
	}
}


/**
 * Funkce pro kontrolu, zdali je aktu�ln� bit na 0.
 */
int isBitZero(volatile uint32_t *actualAddressPointer, int i)
{
	if (*((uint32_t*) BITBAND_REGION_BIT(actualAddressPointer, i)) == 0) {

		return 1;
	}

	return 0;
}


/**
 * Funkce pro kontrolu, zdali je aktu�ln� bit na 1.
 */
int isBitOne(volatile uint32_t *actualAddressPointer, int i)
{
	if (*((uint32_t*) BITBAND_REGION_BIT(actualAddressPointer, i)) == 1) {

		return 1;
	}

	return 0;
}


/**
 * Funkce pro nastaven� aktu�ln�ho bitu na 0.
 */
void setBitToZero(volatile uint32_t *actualAddressPointer, int i)
{
	*((uint32_t*) BITBAND_REGION_BIT(actualAddressPointer, i)) = 0;
}


/**
 * Funkce pro nastaven� aktu�ln�ho bitu na 1.
 */
void setBitToOne(volatile uint32_t *actualAddressPointer, int i)
{
	*((uint32_t*) BITBAND_REGION_BIT(actualAddressPointer, i)) = 1;
}


/**
 * Algoritmus - March X
 * EN:
 * STEP 1 - writes all zeros to array; it sets the registers and does a loop to clear each memory address
 * STEP 2 - starting at lowest address, reads zeros, writes ones, increments up array
 * STEP 3 - starting at highest address, reads ones, writes zeros, decrements down array
 * STEP 4 - reads all zeros from array
 * Zdroj: https://www.nxp.com/docs/en/application-note/AN4873.pdf?fbclid=IwAR03dsRtyhNjK3exlkGKjWbWJg5W6PsxhDUF66kIXUglbFC0YkRMpQ4hIOE
 */
int ramMemoryTestMarchX(volatile uint32_t *pMemStart, volatile uint32_t *pMemEnd)
{

	volatile uint32_t *actualAddressPointer;

	/**
	 * 1. Krok
	 * Ve smy�ce se projdou v�echny adresy pam�ti.
	 * Vynulov�n� pole aktu�ln� testovan� adresy.
	 */
	actualAddressPointer = pMemStart;

	while (actualAddressPointer < pMemEnd) {

		for (int i = 0; i < ADDRESS_ARRAY; i++) {

			setBitToZero(actualAddressPointer, i);
		}
		actualAddressPointer += 1;
	}

	/**
	 * 2. Krok
	 * Ve smy�ce se projdou v�echny adresy.
	 * Zkontroluje se, zdali je na dan�ch adres�ch zapsan� 0. N�sledn� se zap�e hodnota 1 do aktu�ln� zpracov�van� adresy.
	 * Proch�zen� prob�h� od nejni��� adresy po nejvy���.
	 */
	actualAddressPointer = pMemStart;

	while (actualAddressPointer < pMemEnd) {

		for (int i = 0; i < ADDRESS_ARRAY; i++) {

			if (isBitOne(actualAddressPointer, i)) {

				sendString("ERROR: MarchX - Chybn� bit KROK 2\r\n");
				return NOT_OK;
			}
			//*((uint32_t*) BITBAND_REGION_BIT(actualAddressPointer, i)) = 1;
			setBitToOne(actualAddressPointer, i);
		}
		actualAddressPointer += 1;
	}

	/**
	 * 3. Krok
	 * Ve smy�ce se projdou v�echny adresy.
	 * Zkontroluje se,zdali je na dan�ch adres�ch zapsan� 1. N�sledn� se zap�e hodnota 0 do aktu�ln� zpracov�van� adresy.
	 * Proch�zen� prob�h� od nejvy��� adresy po nejni���.
	 */
	actualAddressPointer = pMemEnd - 1;

	while (actualAddressPointer >= pMemStart) {

		for (int i = 0; i < ADDRESS_ARRAY; i++) {

			if (isBitZero(actualAddressPointer, i)) {

				sendString("ERROR: MarchX - Chybn� bit KROK 3\r\n");
				return NOT_OK;
			}
			//*((uint32_t*) BITBAND_REGION_BIT(actualAddressPointer, i)) = 0;
			setBitToZero(actualAddressPointer, i);
		}
		actualAddressPointer -= 1;
	}

	/**
	 * 4. Krok
	 * Ve smy�ce se projdou v�echny adresy.
	 * Zkontroluje se, zdali je na dan�ch adres�ch zapsan� 0.
	 * Proch�zen� prob�h� od nejni��� adresy po nejvy���.
	 */
	actualAddressPointer = pMemStart;

	while (actualAddressPointer < pMemEnd) {

		for (int i = 0; i < ADDRESS_ARRAY; i++) {

			if (isBitOne(actualAddressPointer, i)) {

				sendString("ERROR: MarchX - Chybn� bit KROK 4\r\n");
				return NOT_OK;
			}
		}
		actualAddressPointer += 1;
	}

	return OK;
}


/**
 * Algoritmus - March C
 * EN:
 * STEP1 - writes all zeros to array
 * STEP2 - starting at lowest address, reads zeros, writes ones, increments up array
 * STEP3 - starting at lowest address, reads ones, writes zeros, increments up array
 * STEP4 - starting at highest address, reads zeros, writes ones, decrements down array
 * STEP5 - starting at highest address, reads ones, writes zeros, decrements down array
 * STEP6 - reads all zeros from array
 * Zdroj: https://www.nxp.com/docs/en/application-note/AN4873.pdf?fbclid=IwAR03dsRtyhNjK3exlkGKjWbWJg5W6PsxhDUF66kIXUglbFC0YkRMpQ4hIOE
 */
int ramMemoryTestMarchC(volatile uint32_t *pMemStart, volatile uint32_t *pMemEnd)
{
	volatile uint32_t *actualAddressPointer;

	/**
	 * 1. Krok
	 * Ve smy�ce se projdou v�echny adresy pam�ti.
	 * Vynulov�n� pole aktu�ln� testovan� adresy.
	 */
	actualAddressPointer = pMemStart;

	while (actualAddressPointer < pMemEnd) {

		for (int i = 0; i < ADDRESS_ARRAY; i++) {

			setBitToZero(actualAddressPointer, i);
		}
		actualAddressPointer += 1;
	}

	/**
	 * 2. Krok
	 * Ve smy�ce se projdou v�echny adresy.
	 * Zkontroluje se, zdali je na dan�ch adres�ch zapsan� 0. N�sledn� se zap�e 1 do aktu�ln� zpracov�van� adresy.
	 * Proch�zen� prob�h� od nejni��� adresy po nejvy���.
	 */
	actualAddressPointer = pMemStart;

	while (actualAddressPointer < pMemEnd) {

		for (int i = 0; i < ADDRESS_ARRAY; i++) {

			if (isBitOne(actualAddressPointer, i)) {

				sendString("ERROR: MarchC - Chybn� bit KROK 2\r\n");
				return NOT_OK;
			}

			setBitToOne(actualAddressPointer, i);
		}
		actualAddressPointer += 1;
	}

	/**
	 * 3. Krok
	 * Ve smy�ce se projdou v�echny adresy.
	 * Zkontroluje se, zdali je na dan�ch adres�ch zapsan� 1. N�sledn� se zap�e 0 do aktu�ln� zpracov�van� adresy.
	 * Proch�zen� prob�h� od nejni��� adresy po nejvy���.
	 */
	actualAddressPointer = pMemStart;

	while (actualAddressPointer < pMemEnd) {

		for (int i = 0; i < ADDRESS_ARRAY; i++) {

			if (isBitZero(actualAddressPointer, i)) {

				sendString("ERROR: MarchC - Chybn� bit KROK 3\r\n");
				return NOT_OK;
			}

			setBitToZero(actualAddressPointer, i);
		}
		actualAddressPointer += 1;
	}

	/**
	 * 4. Krok
	 * Ve smy�ce se projdou v�echny adresy.
	 * Zkontroluje se, zdali je na dan�ch adres�ch zapsan� 0. N�sledn� se zap�e 1 do aktu�ln� zpracov�van� adresy.
	 * Proch�zen� prob�h� od nejvy��� adresy po nejni���.
	 */
	actualAddressPointer = pMemEnd - 1;

	while (actualAddressPointer >= pMemStart) {

		for (int i = 0; i < ADDRESS_ARRAY; i++) {

			if (isBitOne(actualAddressPointer, i)) {

				sendString("ERROR: MarchC - Chybn� bit KROK 4\r\n");
				return NOT_OK;
			}

			setBitToOne(actualAddressPointer, i);
		}
		actualAddressPointer -= 1;
	}

	/**
	 * 5. Krok
	 * Ve smy�ce se projdou v�echny adresy.
	 * Zkontroluje se, zdali je na dan�ch adres�ch zapsan� 1. N�sledn� se zap�e 0 do aktu�ln� zpracov�van� adresy.
	 * Proch�zen� prob�h� od nejvy��� po nejni���.
	 */
	actualAddressPointer = pMemEnd - 1;

	while (actualAddressPointer >= pMemStart) {

		for (int i = 0; i < ADDRESS_ARRAY; i++) {

			if (isBitZero(actualAddressPointer, i)) {

				sendString("ERROR: MarchC - Chybn� bit KROK 5\r\n");
				return NOT_OK;
			}

			setBitToZero(actualAddressPointer, i);
		}
		actualAddressPointer -= 1;
	}

	/**
	 * 6. Krok
	 * Ve smy�ce se projdou v�echny adresy.
	 * Zkontroluje se, zdali je na dan�ch adres�ch zapsan� 0.
	 * Proch�zen� prob�h� od nejni��� po nejvy���.
	 */
	actualAddressPointer = pMemStart;

	while (actualAddressPointer < pMemEnd) {

		for (int i = 0; i < ADDRESS_ARRAY; i++) {

			if (isBitOne(actualAddressPointer, i)) {

				sendString("ERROR: MarchC - Chybn� bit KROK 6\r\n");
				return NOT_OK;
			}
		}
		actualAddressPointer += 1;
	}

	return OK;
}


void startTest(int type)
{
	if (type == MARCH_X) {

		int resultMarchX = 0;

		sendString("\r\n======================\r\n");
		sendString("MarchX - TEST 1\r\n");

		resultMarchX = ramMemoryTestMarchX((uint32_t*) 0x20000300, (uint32_t*) 0x20000700);

		if (resultMarchX == NOT_OK) {
			sendString("VYSLEDEK: V prubehu 1. testovani algoritmem MarchX nastala chyba.\r\n");
			sendString("======================\r\n");
		}

		if (resultMarchX == OK) {
			sendString("VYSLEDEK: 1. Testovani algoritmem MarchX probehlo v poradku.\r\n");
			sendString("======================\r\n");
		}

		resultMarchX = 0;

		sendString("\r\n======================\r\n");
		sendString("MarchX - TEST 2\r\n");

		resultMarchX = ramMemoryTestMarchX((uint32_t*) 0x2000FB00, (uint32_t*) 0x2000FF00);

		if (resultMarchX == NOT_OK) {
			sendString("VYSLEDEK: V prubehu 2. testovani algoritmem MarchX nastala chyba.\r\n");
			sendString("======================\r\n");
		}

		if (resultMarchX == OK) {
			sendString("VYSLEDEK: 2. Testovani algoritmem MarchX probehlo v poradku.\r\n");
			sendString("======================\r\n");
		}
	}

	if (type == MARCH_C) {

		int resultMarchC = 0;

		sendString("\r\n======================\r\n");
		sendString("MarchC - TEST 1\r\n");

		resultMarchC = ramMemoryTestMarchC((uint32_t*) 0x20000300, (uint32_t*) 0x20000700);

		if (resultMarchC == NOT_OK) {
			sendString("VYSLEDEK: V prubehu 1. testovani algoritmem MarchC nastala chyba.\r\n");
			sendString("======================\r\n");
		}

		if (resultMarchC == OK) {
			sendString("VYSLEDEK: 1. Testovani algoritmem MarchC probehlo v poradku.\r\n");
			sendString("======================\r\n");
		}

		resultMarchC = 0;

		sendString("\r\n======================\r\n");
		sendString("MarchC - TEST 2\r\n");

		resultMarchC = ramMemoryTestMarchC((uint32_t*) 0x2000FB00, (uint32_t*) 0x2000FF00);

		if (resultMarchC == NOT_OK) {
			sendString("VYSLEDEK: V prubehu 2. testovani algoritmem MarchC nastala chyba.\r\n");
			sendString("======================\r\n");
		}

		if (resultMarchC == OK) {
			sendString("VYSLEDEK: 2. Testovani algoritmem MarchC probehlo v poradku.\r\n");
			sendString("======================\r\n");
		}
	}

}


int main(void)
{

	initMCU();
	initPinAndPort();
	initUART5();

	unsigned char firstChar[1] = " ";
	sendString(firstChar);

	startTest(MARCH_X);
	startTest(MARCH_C);

    while(1) {};
    /* Never leave main */
    return 0;
}
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
