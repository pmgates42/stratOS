SECTIONS
{
	/*
	 * Raspberry Pi firmware loads kernel8.img at physical address 0x00080000.
	 * Link at this base so absolute addresses embedded in data (e.g. function
	 * pointers in const task tables) match runtime addresses.
	 */
	. = 0x00080000;

	.text.boot : { *(.text.boot) }
	.text : { *(.text) }
	.rodata : { *(.rodata) }
	.data : { *(.data) }
	. = ALIGN(0x8);
	bss_begin = .;
	.bss : { *(.bss*) } 
	bss_end = .;
}