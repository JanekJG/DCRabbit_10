/*
   Copyright (c) 2015 Digi International Inc.

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
/*
	write_idblock_rcm45xxw.c

	Example of using WriteIdBlock.lib and WriteIdBlock_RCM45xxW.LIB to write a
	system ID Block to an RCM45xxW board.

	To program an RCM45xxW board which lacks an existing correct and valid system
	ID block, add the appropriate set of the following sets of macro=value lines
	to Dynamic C's Project Options' Defines tab. If Dynamic C stops the compile
	because of too many invalid macro redefinition warnings, increase the
	"Max Shown > Warnings:" count to 100 on the Project Options' Compiler tab.

	For an RCM4510W add:
	   _BOARD_TYPE_=RCM4510W
	   _DC_CLK_DBL_=0
	   _DC_BRD_OPT0_=0x20
	   _DC_MD0_=0x1
	   _DC_MD0_ID_=0xBFD7
	   _DC_MD0_TYPE_=0x1
	   _DC_MD0_SIZE_=0x80
	   _DC_MD0_SECSIZE_=0x1000
	   _DC_MD0_SECNUM_=0x80
	   _DC_MD0_SPEED_=0x46
	   _DC_MD0_MBC_=0x80
	   _DC_MD2_=0x1
	   _DC_MD2_ID_=0x0
	   _DC_MD2_TYPE_=0x0
	   _DC_MD2_SIZE_=0x80
	   _DC_MD2_SECSIZE_=0x0
	   _DC_MD2_SECNUM_=0x0
	   _DC_MD2_SPEED_=0x37
	   _DC_MD2_MBC_=0xC5

	To add a _DC_XBEE_ID_ system macro uncomment the XBEE_ID_VALUE macro
	definition line, below. To have the program set the _DC_XBEE_ID_ system macro
	based on the installed XBee module, define the XBEE_ID_VALUE macro's value to
	0xFFFFFFFFul. To skip XBee version detection, either define the XBEE_ID_VALUE
	macro's value to 0 (no XBee installed) or define the XBEE_ID_VALUE macro to a
	known valid value constructed according to the following information:
	   Upper 16 bits -- hardware version
	      0x1941 - XBee Series 2 (ember processor, ZNet 2.5 or ZB firmware)
	   Lower 16 bits -- firmware version
	      0x1141 - ZNet coordinator firmware
	      0x1341 - ZNet router/end device firmware
	      0x2121 - ZB coordinator firmware
	      0x2321 - ZB router firmware
	      0x2921 - ZB end device firmware
	For example, if using a known XBee series 2 module with firmware version
	0x2921, define the XBEE_ID_VALUE macro's value to be 0x19412921ul. Note that
	this setting is only effective when using Dynamic C version 10.44 or later.

	If rewriting the system ID block in order to change the size of the User
	block or the size of the system's persistent data area, the following
	information should be of particular interest.

	The MAX_USERBLOCK_SIZE macro value tells Dynamic C how much flash storage
	space is reserved for the Rabbit board's system ID and User blocks area, as
	described in the following paragraphs. On all parallel flash equipped and on
	some serial boot flash equipped Rabbit boards, this reserved area size is
	excluded from the xmem code memory org in order to prevent overwrite of the
	system ID and User blocks reserved area. On Rabbit boards equipped with small
	sector flash devices, the value of the MAX_USERBLOCK_SIZE macro should be at
	least ((WIB_SYS4KBLKS + WIB_USER4KBLKS * 2ul) * 4096ul). Large or nonuniform
	sector flash devices' sector layout may cause "gaps" between the system ID
	block and / or the User block images, requiring a further increase in the
	MAX_USERBLOCK_SIZE macro's value. In this latter case, a run time exception
	assertion failure may occur in the WriteIdBlock.lib library. To see more
	information about such an exception, define WIB_VERBOSE (see code below),
	recompile and rerun this utility program. Also see the flash device's data
	sheet to find its sector layout, which in turn determines the optimal
	MAX_USERBLOCK_SIZE macro value.

	All stock Rabbit 4000+ based boards have an 8 KByte (less 6 bytes for the
	validity marker) User block installed. A nonstandard User block size is
	possible within the restrictions imposed by the program (primary, parallel
	or serial) flash type. The User assumes all responsibility for any problems
	that may result from making such a change. In all cases the User block size,
	combined with the 6 byte validity marker, must be in the range [0,64] KBytes
	(i.e. in the range [0,16] 4K blocks). Uncomment and edit the WIB_USER4KBLKS
	macro definition below to set a nonstandard User block size.

	On all stock Rabbit 4000+ based boards the system ID block is combined with a
	persistent data area (16 KBytes in total) which contains the system macros
	table and, optionally, extra memory devices specification table, persistent
	constants, etc. The information in the persistent data area may be extremely
	difficult for a User to replace correctly, and is not intended for alteration
	by User code. A nonstandard persistent data area size is possible on some
	boards depending on the program (primary, parallel or serial) flash type. The
	User assumes all responsibility for any problems that may result from making
	such a change. In all cases the minimum persistent data area size, combined
	with the system ID block, is 4 KBytes (i.e. one 4K block). Uncomment and edit
	the WIB_SYS4KBLKS macro definition below to set a nonstandard persistent data
	area size.
*/

// if using Dynamic C version 10.44 or later, to add a _DC_XBEE_ID_ system macro
//  uncomment and possibly edit the following XBEE_ID_VALUE macro definition
//#define XBEE_ID_VALUE 0xFFFFFFFFul

// uncomment and edit the WIB_USER4KBLKS macro definition value to set a
//  nonstandard User block size (4 KByte block units, default is 2)
//#define WIB_USER4KBLKS 2

// uncomment and edit the WIB_SYS4KBLKS macro definition value to set a
//  nonstandard persistent data area size (4 KByte block units, default is 4)
//#define WIB_SYS4KBLKS 4

// uncomment the following line to make WriteIdBlock.lib functions debuggable
//#define WIB_DEBUG
// uncomment the following line to enable WriteIdBlock.lib functions' verbosity
//#define WIB_VERBOSE

// no user-settable options below this point

// the following macro definition is needed before WriteIdBlock.lib is used
#define ENABLE_SYSTEM_ID_BLOCK_OVERWRITE

#use "WriteIdBlock.lib"
#use "WriteIdBlock_RCM45xxW.lib"

#if 0x0A44 <= CC_VER
/*
	This XBee functionality only exists in Dynamic C versions 10.44 and later.
*/
// bring in idBlock-related function for reading XBee module HW and FW versions
#use "WriteIdBlock_XBee.lib"
#endif	// 0x0A44 <= CC_VER

debug
void main(void)
{
	int rc;
	long build_xalloc_size;
	wib_sysIdBlockData far *build;

	// allocate xmem space for the ID block data structure
	build_xalloc_size = sizeof (wib_sysIdBlockData);
	build = (wib_sysIdBlockData far *)_xalloc(&build_xalloc_size, 0, XALLOC_ANY);

	// initialize the ID block build data structure and fill in typical items
	wib_initWriteIdBlockLib(build, _FLASH_SIZE_);
	wib_addAutomaticValues(build);

#if RCM4510W == _BOARD_TYPE_
	// add RCM4510W specific, fixed information
	wib_addRCM4510WInfo(build);
	// if programming an RCM4510W core as a BL4S230 SBC topping,
	//  uncomment the following line to add the necessary system macro
//	wib_addMacro(build, "MB_TYPE", 0x0100);
#else	// RCM4510W == _BOARD_TYPE_
	#fatal "This sample does not support the current _BOARD_TYPE_ definition."
#endif	// RCM4510W == _BOARD_TYPE_

	// build the ID block and associated items, then write them to the primary
	//  (program, parallel or serial) flash
	rc = wib_buildAndWrite(build);
	wib_termWriteIdBlockLib(build, 1);
	if (rc) {
		printf("\nError, wib_buildAndWrite() result is %d!\n", rc);
	} else {
		printf("\nFinished writing system ID block.\n");
	}

	// we can release our xmem space so long as it was the last one allocated
	xrelease((long) build, build_xalloc_size);
}