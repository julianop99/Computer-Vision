 /*
 *              Lanemark Detection Algorithm Using BT848 driver for video capture on a PCI bus
 *              Implementation of MSc Dissertation in 2000. Degree obtained from ITA (Technological Institute of Aeronautics, Brazil)
 *              Author: Juliano Cesar Pimentel (https://github.com/julianop99)
 *              Supervisor: Elder Hemerly
 *              Revision: 17/Dec/1999 
 *               ------------------------------------------------- 
 *              Camera view application. It should work with both Matrox Meteor frame grabber with meteor 1.4c driver and BT848-based cards
 *              with bt848-0.1 driver.
 *              --> This module contains the camera GUI application functions.
 *
 */

#include "LanemarkDetection.h"


void create_motif_app()
{
  int progname=1;
  char *window_name[20];

  window_name[0]="View";

  /* create toplevel app */

  top_widget = XtVaAppInitialize(&app,"Camera View",NULL,0
				 ,&progname,window_name,NULL
				 ,XtNinput,(XtArgVal)True
				 ,NULL);

  /* open default display */
  if ((display = XOpenDisplay (0))==0) {
    fprintf(stderr,"Can't open X window\n");
    return;
  }

  if (xInit(display,XD_NOSHM)==0) {
    printf("Cannot init xdisp\n"); exit(0);
  }

  screen = DefaultScreen(display);
  visual = DefaultVisual(display,screen);
  depth = DefaultDepth(display,screen);

  main_widget = XtVaCreateManagedWidget("Camera View",
        xmMainWindowWidgetClass,   top_widget,
        NULL);

  form = XtVaCreateManagedWidget("form", xmFormWidgetClass, main_widget,
	XmNfractionBase,     FRAMEBASE,
        NULL);

  createButtons();
  createFrames();
  createLabels();
  createGC();

  XtRealizeWidget(top_widget); /* display widget hierarchy */
}

void createButtons()
{
  int i;

  /* Push and Toggle Buttons */

  for (i=0; i<HBUTT; i++)
    if (strcmp(vbname[i],"")!=0) {
      switch (tybutt[i]) {
      case 'p':
	hbutt[i] = XtVaCreateManagedWidget(vbname[i],
             xmPushButtonWidgetClass, form,
             XmNtopAttachment,    XmATTACH_POSITION,
	     XmNtopPosition,      0,
             XmNleftAttachment,   XmATTACH_POSITION,
             XmNleftPosition,     i*2,
             XmNbottomAttachment, XmATTACH_POSITION,
             XmNbottomPosition,   1,
             XmNrightAttachment,   XmATTACH_POSITION,
             XmNrightPosition,    (i+1)*2,
             NULL);
	XtAddCallback(hbutt[i],  XmNdisarmCallback, hbfun[i], NULL);
	break;
      case 't':
      case 'T':
	hbutt[i] = XtVaCreateManagedWidget(vbname[i],
             xmToggleButtonWidgetClass, form,
             XmNtopAttachment,    XmATTACH_POSITION,
	     XmNtopPosition,      0,
             XmNleftAttachment,   XmATTACH_POSITION,
             XmNleftPosition,     i*2,
             XmNbottomAttachment, XmATTACH_POSITION,
             XmNbottomPosition,   1,
             XmNrightAttachment,   XmATTACH_POSITION,
             XmNrightPosition,    (i+1)*2,
	     NULL);
	if (tybutt[i]=='T')
	  XmToggleButtonSetState(hbutt[i],True,True);
	XtAddCallback(hbutt[i], XmNvalueChangedCallback, hbfun[i], NULL);
	break;
      }
    }
}

void createFrames() 
{
  int i;

  frameL=XtVaCreateManagedWidget("left image",xmDrawingAreaWidgetClass,form,
      XmNheight,	   height,
      XmNwidth,            width,
      XmNtopAttachment,    XmATTACH_POSITION,
      XmNtopPosition,      1,
      XmNleftAttachment,   XmATTACH_POSITION,
      XmNleftPosition,     0,
      XmNbottomAttachment, XmATTACH_POSITION,
      XmNbottomPosition,   FRAMEBASE-1,
      XmNrightAttachment,   XmATTACH_POSITION,
      XmNrightPosition,    FRAMEBASE,
      NULL);

  XtAddCallback(frameL, XmNexposeCallback, display_images, NULL);
  
#ifdef LRIM
  frameR=XtVaCreateManagedWidget("right image",xmDrawingAreaWidgetClass,form,
      XmNheight,	   height,
      XmNwidth,            width,
      XmNtopAttachment,    XmATTACH_POSITION,
      XmNtopPosition,      1,
      XmNleftAttachment,   XmATTACH_POSITION,
      XmNleftPosition,     4,
      XmNbottomAttachment, XmATTACH_POSITION,
      XmNbottomPosition,   4,
      XmNrightAttachment,   XmATTACH_POSITION,
      XmNrightPosition,    8,
      NULL);

  XtAddCallback(frameR, XmNexposeCallback, display_images, NULL);
#endif
}

void createLabels() 
{
  XmString xstr[NLABELS];
  char str[NLABELS][20] = {"","fps = 00.0","","",""};
  int i;

  for (i=0; i<NLABELS; i++)
    if (strcmp(str[i],"")!=0) {
      xstr[i] = XmStringCreateLtoR(str[i], XmSTRING_DEFAULT_CHARSET);
      if ( xstr[i] == NULL )
	return;

      label[i]=XtVaCreateManagedWidget(str[i],xmLabelWidgetClass,form,
            XmNlabelString,      xstr[i],
            XmNtopAttachment,    XmATTACH_POSITION, 
            XmNtopPosition,      FRAMEBASE-1,
            XmNleftAttachment,   XmATTACH_POSITION,
            XmNleftPosition,     i*2,
            XmNbottomAttachment, XmATTACH_POSITION,
            XmNbottomPosition,   FRAMEBASE,
            XmNrightAttachment,   XmATTACH_POSITION,
            XmNrightPosition,    (i+1)*2,
            NULL);

      XmStringFree(xstr[i]);
    }
}

void createGC()
{
  int i;
  XGCValues gcv;
  GC gc; Screen *screen_ptr;
  
  XColor xcol, xbgcol, spare; /* xlib color struct */
  Colormap cmap;

  screen_ptr = XtScreen(frameL);
  cmap = DefaultColormapOfScreen(screen_ptr);

  for (i=0; i<GCCOLORS; i++) {
    gcCol[i] = XCreateGC(display, RootWindowOfScreen(screen_ptr), 0, NULL);
    if (XAllocNamedColor(display, cmap, gcColName[i], &xcol, &spare) == 0)
      printf("No %s color\n",gcColName[i]);
    XSetForeground(display, gcCol[i], xcol.pixel);
  }
}

void set_image_display()
{
  XSync(display,False);
  process_events();

  frameL_win = XtWindow(frameL);	/* graphics window */

  igc = DefaultGC(display,screen);
  XSetGraphicsExposures(display, igc, False);

  xSetImageDisplay(0,frameL_win,width,height,igc);
}

void process_events(void)
{
  XEvent event;
  while (XtAppPending(app))
    {
      XtAppNextEvent(app,&event);
      if (event.type==33) {
	grVideoHaltContinuous();
      printf("Stop capturing.\n");
      }
      XtDispatchEvent(&event);
    }
}

extern int Color;
void display_images() 
     {
        if (dataL!=NULL)
        xDisplayImage(dataL, 0, Color?XD_RGB15:XD_GRAY, 2, 2, 1, 1);
}

int ChangeWidgetLabel (Widget w, char *label) 
{
  XmString	xmstr;

  /* Convert the char * to a XmString */
  xmstr = XmStringCreateLtoR(label, XmSTRING_DEFAULT_CHARSET);
  /* Exit if couldn't make into a XmString */
  if ( xmstr == NULL )
    return -1;
  /* We need to unmanage the widget so it resizes correctly */
  XtUnmanageChild(w);
  /* Now change the label */
  XtVaSetValues(w, XmNlabelString, xmstr, NULL);
  /* Re-manage the widget */
  XtManageChild(w);
  /* Free up the XmString	 */
  XmStringFree(xmstr);
  return 0;
}

void display_labels(int c)
{
  char ratebuf[20];
  switch (c) {
  case 1:
    sprintf(ratebuf, "fps = %3.1f", fps);
    ChangeWidgetLabel(label[c], ratebuf);
    break;
  }
}

/******************* FUNCAO CONTROLE**************************/

void habilitaDA(void)
{

if(ioperm(address,iosize,1)==-1) printf("Nao 'e root");
outb(0x30,address);
}

void controle(void)
{
int DAvalue;
float u;
u=2.0;
DAvalue=(u+5)*4096/10;
 outw(DAvalue,address+2); //ESCREVE NO D/A 0
outw(DAvalue,address+4); // ESCREVE  NO D/A 1
}

