#ifndef __TYPE_H
#define __TYPE_H

#ifndef __LITTLE_ENDIAN
#define __LITTLE_ENDIAN 1234
#endif

#ifndef __BIG_ENDIAN
#define __BIG_ENDIAN 4321
#endif

#define __BYTE_ORDER __LITTLE_ENDIAN

#define	EIO		 5	/* I/O error */
#define	ENOMEM		12	/* Out of memory */

typedef unsigned int  size_t;
typedef long int  ssize_t;

/* Signed.  */
typedef signed char		int8_t;
typedef short int		int16_t;
typedef int			int32_t;

/* Unsigned.  */
typedef unsigned char uint8_t;
typedef unsigned short int	uint16_t;
typedef unsigned int		uint32_t;

#endif
