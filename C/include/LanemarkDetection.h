 /*
 *              Lanemark Detection Algorithm Using BT848 driver for video capture on a PCI bus
 *              Implementation of MSc Dissertation in 2000. Degree obtained from ITA (Technological Institute of Aeronautics, Brazil)
 *              Author: Juliano Cesar Pimentel (https://github.com/julianop99)
 *              Supervisor: Elder Hemerly
 *              Revision: 17/Dec/1999 
 *               ------------------------------------------------- 
 *              Camera view application. It should work with both Matrox Meteor
 *              frame grabber with meteor 1.4c driver and BT848-based cards
 *              with bt848-0.1 driver.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <stdarg.h>
#include <math.h>
#include <asm/io.h>
#include <unistd.h>

#include "X11/Xlib.h"
#include "X11/Xutil.h"
#include "X11/Xos.h"
#include "X11/Intrinsic.h"
#include "X11/IntrinsicP.h"
#include "X11/CoreP.h"
#include "X11/Shell.h"
#include "X11/keysym.h"

#include <Xm/Xm.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Form.h> 
#include <Xm/ArrowB.h>
#include <Xm/MainW.h>
#include <Xm/CascadeB.h>
#include <Xm/DrawingA.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/List.h>
#include <Xm/ToggleB.h>

#include "grabber.h"
#include "xdispima.h"

/* Call back functions Declaration*/

void CBOnOff();//Widget w, XtPointer client_data, XmPushButtonCallbackStruct *cbs);
void CBSnap();

/* GUI Internal functions Declaration */
void create_motif_app();
void process_events();
void display_images();
void display_labels(int);

void createButtons();
void createFrames();
void createLabels();
void createGC();
void set_image_display();
void display_images();
void display_labels();
void process_events();
void prepareimage();
void calculategradient(unsigned char **buf,int HEIGHT, int WIDTH);
void lowpass(unsigned char **buf, int HEIGHT, int WIDTH, int times);
void findline(unsigned char **buf,int HEIGHT, int WIDTH);

/* X variables */
#define FRAMEBASE 10
#define HBUTT     5
#define NLABELS   5
#define GCCOLORS  5


XtAppContext  app;
Visual *visual;
int screen, depth;
Display *display;
Font font;

Widget top_widget, main_widget, form, hbutt[HBUTT], label[NLABELS], 
       frameL, frameR;
Window frameL_win, frameR_win;
GC igc, gcCol[GCCOLORS];

/* GUI variables */
char gcColName[GCCOLORS][20]={"Black","Red","Green","Blue","Yellow"};
char tybutt[HBUTT]={'p','p','p','p','p'};
char vbname[HBUTT][20]={"On/Off","Snap","","",""};

typedef void (*func)();
func hbfun[HBUTT] = { CBOnOff, CBSnap, CBOnOff, CBOnOff, CBOnOff };

/* Internal global variables */
unsigned char *red,*green,*blue,*gray;
int width=W, height=H;        /* image dimension */
unsigned char *dataL=NULL,        /* pixel data */
              *dataR=NULL;     
int CameraOpen=0;                 /* Camera on */
int Cont=0;                       /* Continuous mode */
int Proc=0;                       /* Data to be processed */
int new_image=0;                  /* new_image flag */
float fps;                        /* frame per second */
int Color=1;                      /* If color camera */
int mode=0;                       /* grVideoOpen mode */

/* External global variables */
extern int width, height;
extern unsigned char *dataL, *dataR;
extern float fps;


/* Lanemark Dectection functions Declaration */
void controle(void);
void habilitaDA(void);
void inversemat(float **mat, int order,float **matinv);

int mainloop();
void process_image();

/* Defines */
#define pi 3.14159
#define W 320
#define H 240
#define GRADTH 20
#define thetap 70
#define radiusp 150
#define upper 30
#define lower 30
#define left 30
#define right 30
#define thetamin 5.0*2.0*pi/360.0
#define thetamax 75.0*2.0*pi/360.0
#define radiusmin 50.0
#define radiusmax 350.0
#define BAND 5
//#define aux 5*sqrt(W*W+H*H)
//#define aux1 W

/* External global variables */
unsigned char **criamatriz(int nr,int nc);
float **criamatriz_float(int nr, int nc);
int  **criamatriz_int(int nr,int nc);

unsigned char **matrizgray, **matgrad, **matgrad1;
float **hough,**mat,**matinv,teta[3];
FILE *formatlab,*formatl;
float *X,**M,**MTM,MTX[3],**invMTM,coef[3],coefd[3],coefl[3];
float *Y,*Yhat,*error,**fiT,**fi,**Pfi,**fiTPT,**fiTPfi,**Eye,**Psum,**invfTf,**BC;
int **save;

int savemat=1,savmat=1;
int flag1;
int invstatus=1;

int address=0x210, iosize=100;

/* Internal global variables */
unsigned long int ii;
unsigned char *nossa,*nossa1;

