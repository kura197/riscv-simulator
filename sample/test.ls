/*
OUTPUT_FORMAT("binary");
*/

ENTRY(main);

SECTIONS {
		. = 0x7c00;
        .text        : {*(.text)}
        .data        : {*(.data)}
}

