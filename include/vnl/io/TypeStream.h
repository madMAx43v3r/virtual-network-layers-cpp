/*
 * TypeStream.h
 *
 *  Created on: Jun 29, 2016
 *      Author: mad
 */

#ifndef INCLUDE_VNL_IO_TYPESTREAM_H_
#define INCLUDE_VNL_IO_TYPESTREAM_H_



enum {
	VNL_IO_NULL 		= 0x0,
	VNL_IO_BOOL			= 0x1,
	VNL_IO_INTEGER 		= 0x2,
	VNL_IO_REAL 		= 0x3,
	VNL_IO_BINARY 		= 0x4,
	VNL_IO_STRING 		= 0x5,
	VNL_IO_CALL 		= 0x8,
	VNL_IO_CONST_CALL 	= 0x9,
	VNL_IO_ARRAY 		= 0xA,
	VNL_IO_STRUCT 		= 0xB,
	VNL_IO_CLASS 		= 0xC,
	VNL_IO_INTERFACE	= 0xD
};

enum {
	VNL_IO_BYTE			= 0x1,
	VNL_IO_WORD			= 0x2,
	VNL_IO_DWORD		= 0x4,
	VNL_IO_QWORD		= 0x8,
	VNL_IO_EXTENDED 	= 0xF
};

enum {
	VNL_IO_BEGIN		= 0x0,
	VNL_IO_END			= 0x1
};

enum {
	VNL_IO_FALSE		= 0x0,
	VNL_IO_TRUE			= 0x1
};



#endif /* INCLUDE_VNL_IO_TYPESTREAM_H_ */
