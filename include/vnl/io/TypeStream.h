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
	VNL_IO_INTEGER 		= 0x1,
	VNL_IO_REAL 		= 0x2,
	VNL_IO_BINARY 		= 0x3,
	VNL_IO_STRING 		= 0x4,
	VNL_IO_ARRAY 		= 0xA,
	VNL_IO_TYPE 		= 0xB,
	VNL_IO_CALL 		= 0xC,
	VNL_IO_CONST_CALL 	= 0xD
};

enum {
	VNL_IO_BYTE			= 0x1,
	VNL_IO_WORD			= 0x2,
	VNL_IO_DWORD		= 0x4,
	VNL_IO_QWORD		= 0x8,
	VNL_IO_EXTENDED 	= 0xF
};



#endif /* INCLUDE_VNL_IO_TYPESTREAM_H_ */
