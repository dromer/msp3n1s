/*
 * This file is part of msp3n1s
 * Copyright 2011 Emil Renner Berthing
 *
 * msp3n1s is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * msp3n1s is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with msp3n1s.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <watchdog.h>
#include <clock.h>
#include <pins.h>

#define LED1 1.0
#define LED2 1.6
#define S2   1.3

#define SERIAL_PRINTF_D
#include "lib/serial_tx.c"

#define ONEWIRE_PIN 1.5
#define ONEWIRE_INTERNAL_PULLUP
#include "lib/onewire.c"

static void
dtemp__convert(void)
{
	/* transmit convert temperature command */
	onewire_transmit_8bit(0x44);

	/* wait for conversion to complete */
	while (onewire_receive_1bit() == 0);
}

static int
dtemp__scratchpad_read(unsigned char scratchpad[9])
{
	int i;

	/* transmit read scratchpad command */
	onewire_transmit_8bit(0xBE);

	/* read scratchpad bytes */
	for (i = 0; i < 9; i++)
		scratchpad[i] = onewire_receive_8bit();

	return onewire_crc_8bit(scratchpad, 9);
}

static int __attribute__((unused))
dtemp_convert(unsigned char rom[8], unsigned char scratchpad[9])
{
	if (onewire_rom_match(rom))
		return -1;

	dtemp__convert();

	if (onewire_rom_match(rom))
		return -1;

	return dtemp__scratchpad_read(scratchpad);
}

static int __attribute__((unused))
dtemp_convert_single(unsigned char scratchpad[9])
{
	if (onewire_rom_skip())
		return -1;

	dtemp__convert();

	if (onewire_rom_skip())
		return -1;

	return dtemp__scratchpad_read(scratchpad);
}

void
port1_interrupt(void)
{
	pin_interrupt_disable(S2);
	LPM0_EXIT;
}

int
main(void)
{
	watchdog_off();
	clock_init_1MHz();

	/* set all pins to output high */
	port1_direction = 0xFF;
	port1_output = 0xFF;

	pin_low(LED1);

	/* initialize S2 to receive button press interrupt */
	pin_mode_input(S2);
	pin_resistor_enable(S2);
	pin_high(S2);
	pin_interrupt_falling(S2);

	/* initialize onewire */
	onewire_init();

	/* initialize serial output */
	serial_init_clock();
	serial_init_tx();

	/* enable interrupts */
	__eint();

	while (1) {
		unsigned char scratchpad[9];
		int val;

		/* wait for button press */
		pin_low(LED2);
		pin_interrupt_clear(S2);
		pin_interrupt_enable(S2);
		LPM0;
		pin_high(LED2);

		pin_high(LED1);
		val = dtemp_convert_single(scratchpad);
		pin_low(LED1);

		if (val) {
			serial_printf("No slave found.\n");
			continue;
		}

		val = (scratchpad[1] << 8) | scratchpad[0];

		/*
		serial_dump(rom, 8);
		serial_dump(scratchpad, 9);
		*/

		serial_printf("Read %d.%d C\n", val / 2, (val & 1) ? 5 : 0);
		serial_printf("..or %d - 0.25 + %d/%d C\n", val / 2,
		              (int)(scratchpad[7] - scratchpad[6]),
			      (int)scratchpad[7]);
	}
}
