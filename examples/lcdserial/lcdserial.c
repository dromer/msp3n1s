/*
 * This file is part of msp3n1s
 * Copyright 2011-2012 Emil Renner Berthing
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

#include <stdlib.h>
#include <watchdog.h>
#include <clock.h>
#include <pins.h>

#include "lib/serial_tx_buffered.c"

#define SERIAL_RX_BUFSIZE 32
#include "lib/serial_rx_buffered.c"

/* data pins must be 1.4, 1.5, 1.6 and 1.7,
 * but these can be freely chosen among the rest */
#define LCD_RS 1.0
#define LCD_EN 1.3
#include "lib/lcd.c"


int
main(void)
{
	unsigned int count;

	watchdog_off();
	clock_init_1MHz();

    /* initialize serial cleck, tx and rx parts */
    serial_init_clock();
    serial_init_tx();
    serial_init_rx();


	lcd_init();

	/* see fx.
	 * http://en.wikipedia.org/wiki/Hitachi_HD44780_LCD_controller
	 * for more commands
	 */
	lcd_command(0x28); /* 2 lines, 5x8 dot font */
	lcd_command(0x10); /* move cursor on write */
	lcd_command(0x0C); /* cursor off */
	lcd_command(0x06); /* move cursor right */
	lcd_clear();       /* clear display */
	lcd_cursor_set(0x04);
	lcd_puts("TechInc ftw");

	lcd_command(0x04); /* move cursor left */
	count = 0;

	/* enable interrupts */
	__eint();

	while (1) {
		unsigned int i = count++;

		lcd_cursor_set(0x49);
		do {
			lcd_putchar('0' + i % 10);
			i /= 10;
		} while (i);

    }
}
