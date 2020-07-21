/*
 *                           G R A B B E R. H
 *                          ------------------
 *
 *    Version 1.3
 *   -------------
 *
 *
 *   Luca Iocchi 1998
 *
 *      iocchi@dis.uniroma1.it
 *
 */


#ifdef __cplusplus
extern "C" {
#endif


/* Capture mode */

#define GR_CAP_MASK 0x0F

#define GR_CAP_GRAY          0
#define GR_CAP_SINGLE        0
#define GR_CAP_INTERLACE     1
#define GR_CAP_PAR_INTERLACE 2
#define GR_CAP_DUAL          3
#define GR_CAP_GRAY_PACKED   4

#define GR_CAP_COLOR15       8
#define GR_CAP_COLOR16       9  /* It doesn't work yet */
#define GR_CAP_COLOR24      10
#define GR_CAP_COLOR32      11
#define GR_CAP_RGB15         8
#define GR_CAP_RGB16         9
#define GR_CAP_RGB24        10
#define GR_CAP_RGB32        11

#define GR_CAP_YUV12        12


/* Buffering mode */

#define GR_BUFMODE          0x10


/* Input format */ 

#define GR_NTSC    0x00
#define GR_PAL     0x20





/*
 * mode parameter in grVideoOpen = 
 *   <Capture mode> | <Buffering mode> | <Input format>
 */                                 


int grVideoOpen(int w, int h, int mode);


int grVideoStartContinuous();


void grVideoHaltContinuous();


int grVideoCheckImage();


int grVideoWaitImage(int timeout);


/* rbuf!=NULL in GR_CAP_INTERLACE and GR_CAP_DUAL mode only */
int grVideoGetImages(unsigned char **lbuf, unsigned char **rbuf);

void grVideoClose();

/* lr in grSetDeviceName (for GR_CAP_DUAL mode only) */
#define GR_LEFT   0
#define GR_RIGHT  1

void grSetDeviceName(int lr, char* dn);

int grGetFPS();
int grSetFPS(int f);

int grGetColorControls (int *brig, int *cont, int *hue, int *csat);
int grSetColorControls (int brig, int cont, int hue, int csat);




/********************   Undistort   ********************************/


#define GR_BIL_INT  2  /* Bilinear Interpolation */
#define GR_IMA_EXP  3  /* Image Expansion */

extern int grUndMode;

void grUndistortSetup(int w, int h, float px, float py);

void grUndistortPar(int lr, float k1, float dx, float dy, float ax);

void grUndistortRot(int lr, float deg);

void grUndistortOn();

void grUndistortOff();

void grUndistort (unsigned char *s, unsigned char *sd, int lr);



#ifdef __cplusplus
}
#endif


