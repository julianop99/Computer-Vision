/*
 *                         X D I S P I M A. H
 *                        --------------------
 *
 *    Version 1.1
 *   ---------------
 *
 *
 *   Luca Iocchi 1998
 *
 *      iocchi@dis.uniroma1.it
 *      iocchi@ai.sri.com
 */


#include "X11/Xlib.h"


/* Shared Memory flag */
#define XD_SHM    1
#define XD_NOSHM  0


/* Color types */
#define XD_GRAY      0
#define XD_RGB15     1  /* 2 Bpp: 0555 */
#define XD_RGB16     2  /* 2 Bpp: 565  */
#define XD_RGB24     3  /* 3 Bpp: R G B */
#define XD_RGB32     4  /* 4 Bpp: B G R _ */


int xInit(Display *d, int shm);

int xSetImageDisplay(int nframe, Window win, int w, int h, GC gc);

int xDisplayImage(unsigned char *data, int nframe, int colortype, 
                  int x, int y, int magx, int magy); 


