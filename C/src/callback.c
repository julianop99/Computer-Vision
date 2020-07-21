/*
 *    callbacks.c
 *
 *
 * Luca Iocchi 1998
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/file.h>
#include <stdarg.h>
#include <math.h>

#include <Xm/Xm.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Form.h> 
#include <Xm/ArrowB.h>
#include <Xm/MainW.h>
#include <Xm/CascadeB.h>
#include <Xm/DrawingA.h>



#include "grabber.h"

extern int width, height;
extern unsigned char *dataL, *dataR;
extern int Cont, Color;

int first=1;


/* Callback functions */


/*
 *    CBOnOff()
 */

void CBOnOff(Widget w, XtPointer client_data, XmPushButtonCallbackStruct *cbs)
{ 
  int f,b,c,h,cs;

  if (Cont) {
    grVideoHaltContinuous();
    printf("Stop capturing.\n");
  }
  else {
    if (first) {
      set_image_display();
      first=0;
    }
    grVideoStartContinuous();
    printf("Start capturing.\n");
  }
  Cont=1-Cont;

  grGetColorControls(&b,&c,&h,&cs);
  grSetColorControls(b+10,c,h+30,cs+10);
}





/*
 *    CBSnap()
 */

static int sc=0;   /* snap counter */

void CBSnap(Widget w, XtPointer client_data, XmPushButtonCallbackStruct *cbs)
{
  FILE *f; char nf[20]; unsigned short *p, *pend; unsigned char c;

  if (dataL==NULL)
    return;

  if (Color==0) {
    sprintf(nf,"snap%02d.pgm",sc++);

    if ((f=fopen(nf,"w"))==NULL) {
      printf("Error in opening file %s\n",nf);
      return;
    }

    fprintf(f,"P5\n%d %d 255\n",width,height);
    fwrite(dataL,1,width*height,f);
    fclose(f);
  }
  else {
    sprintf(nf,"snap%02d.ppm",sc++);

    if ((f=fopen(nf,"w"))==NULL) {
      printf("Error in opening file %s\n",nf);
      return;
    }

    fprintf(f,"P6\n%d %d 255\n",width,height);

    if (Color==1) {
      /* Converts RGB 15 in PPM format */
      p=(unsigned short *)dataL; pend=p+width*height;
      while (p<pend) {
	c=(*p&0x7C00)>>7;
	fwrite(&c,1,1,f);
	c=(*p&0x03E0)>>2;
	fwrite(&c,1,1,f);
	c=(*p&0x001F)<<3;
	fwrite(&c,1,1,f);
	p++;
      }
    }
    else {

    }

    fclose(f);
  }

  printf("File %s written.\n",nf);
    
}

