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

#include "LanemarkDetection.h"


// Main Function
main()
{

  //variables declaration / matrix callout
  int i,b,c,h,cs,aux,aux1;
  nossa=(unsigned char *)malloc(4*H*W);
  red=(unsigned char *)malloc(H*W);
  green=(unsigned char *)malloc(H*W);
  blue=(unsigned char *)malloc(H*W);
  gray=(unsigned char *)malloc(H*W);
  matrizgray=criamatriz(H,W);
  matgrad=criamatriz(H,W);
  matgrad1=criamatriz(H,W);
  hough=criamatriz_float(thetap,radiusp);
 
  aux=5*sqrt(W*W+H*H);
  aux1=H; //(int)2*BAND*W; //sqrt(W*W+H*H);
  mat=criamatriz_float(aux,aux);
  matinv=criamatriz_float(aux,aux);  

  M=criamatriz_float(aux,3);
  X=(float *)malloc(aux);
  MTM=criamatriz_float(3,3);
  invMTM=criamatriz_float(3,3);
  flag1=0;
  //enables data acquisition
  habilitaDA();
  usleep(100);


  fiT=criamatriz_float(aux1,3);
  Y=(float *)malloc(aux1);
  Yhat=(float *)malloc(aux1);
  error=(float *)malloc(aux1);

  fi=criamatriz_float(3,aux1);
  fiTPT=criamatriz_float(aux1,3);   
  Pfi=criamatriz_float(3,aux1);
  fiTPfi=criamatriz_float(aux1,aux1);
  Psum=criamatriz_float(aux1,aux1);
  invfTf=criamatriz_float(aux1,aux1);
  Eye=criamatriz_float(aux1,aux1);
  save=criamatriz_int(2,2*aux1);
  BC=criamatriz_float(aux1,3);

  //create GUI
  create_motif_app();

  mode = GR_CAP_GRAY | GR_NTSC;

  if (Color==1)
  {
    mode &= ~GR_CAP_MASK;
    mode |= GR_CAP_COLOR15;
  }

  grVideoOpen(width,height,mode);

  grGetColorControls(&b,&c,&h,&cs);
  grSetColorControls(b+30,c,h,cs);

  //infinite loop callout
  mainloop();

}

int mainloop()
{ 
  int i=0, oldtime = 0, newtime, quit = -1;

  display_labels(1);

  while (quit == -1) 
   {
    process_events();		/* check for window events */    

    if (Cont>0) 
     {
        if (!grVideoWaitImage(400)) 
        {
	        printf("No image, timed out!\n");
	        new_image=0;
        }
        else 
        {
      	  if (grVideoGetImages(&dataL,NULL))
	          new_image=1;
        }
     }

    if (new_image)                 /* now display results */
     {     
        new_image = 0;
        process_image();
        display_images();
        // usleep(3000);
        //  geracao do controle neste ponto
        if(flag1==0) 
        {
          outw(3000,address+2);
        }
        flag1=flag1+1;
        if(flag1==2)
        {
          outw(0,address+2);
          flag1=0;
        } 

        if (Cont>0) 
        {  
	        if (++i >= 10 ) 
	        {
	          struct timeval t;
	          gettimeofday(&t, NULL);
	          newtime = t.tv_sec*1000 + t.tv_usec/1000;
	          i = newtime - oldtime;
	          oldtime = newtime;
	          fps=(10*1000.0/(double)i);
	          display_labels(1);
	          i = 0;
	        }
       }
     }
  }

  return quit;
}


void prepareimage()   // deve ser chamada apos grVideoGetImages
{
 
    int sc=0;
    unsigned long int counter,i,j; 
    unsigned short *p, *pend; unsigned char c;
    counter=0;

    if (dataL==NULL)
      return; 
 
    /* Converts RGB 15 in P6 format */
    p=(unsigned short *)dataL; pend=p+W*H;
    while (p<pend) {
	    c=(*p&0x7C00)>>7;  // this is  RED
        red[counter]=c;
	    c=(*p&0x03E0)>>2;  // this is GREEN
        green[counter]=c;	
	    c=(*p&0x001F)<<3;
        blue[counter]=c;  // this is BLUE
        counter=counter+1;
	    p++;
    }
 
    for(i=0;i<H*W;i++) gray[i]=(red[i]+green[i]+blue[i])/3;

    xDisplayImage(gray , 0, XD_GRAY,2, 250, 2, 250);

    counter=0;
    for(i=0;i<H;i++)
    {
        for(j=0;j<W;j++) 
        { 
            matrizgray[i][j]=gray[counter];
            counter=counter+1;
        }
    } 

    // AO RETORNAR DESTA FUNCAO TEMOS A INFORMACAO RGB EM red[],green[],blue[] E A IMAGEM EM GRAY-LEVEL EM gray[].
}  // FIM DE prepareimage()


unsigned char **criamatriz(int nr,int nc)
{
 int i;
 unsigned char **m;
 m=(unsigned char **)calloc(nr,sizeof(unsigned char *));
 for(i=0;i<nr;i++) m[i]=(unsigned char *)calloc(nc,sizeof(unsigned char));
 return(m);
}

float  **criamatriz_float(int nr,int nc)
{
 int i;
 float **m;
 m=(float **)calloc(nr,sizeof(float *));
 for(i=0;i<nr;i++) m[i]=(float *)calloc(nc,sizeof(float));
 return(m);
}


int  **criamatriz_int(int nr,int nc)
{
 int i;
 int **m;
 m=(int **)calloc(nr,sizeof(int *));
 for(i=0;i<nr;i++) m[i]=(int *)calloc(nc,sizeof(int));
 return(m);
}


// FUNCAO PARA CALCULAR O GRADIENTE A PARTIR DA IMAGEM EM GRAY-LEVEL

void calculategradient(unsigned char **buf,int HH, int WW)
{
 int i,j;
 unsigned int gradx,grady,grad;

 for(i=1;i<HH;i++) 
   {
    for(j=1;j<WW;j++)
      {
	  grady=buf[i][j]-buf[i-1][j];
          gradx=buf[i][j]-buf[i][j-1];
          grad=abs(grady)+abs(gradx);

	  if(grad >255) grad=255;
	   if(grad > GRADTH) matgrad[i][j]=0; // SET PIXEL TO BLACK
	else matgrad[i][j]=255;           //  SET PIXEL TO WHITE
	 
	   }                                     //  END OF j

   }                                        //  END OF i
}     
                                     //   END OF FUNCTION void calculategradient(void) !



void process_image()

{
  /*
   *  here you can process dataL buffer containing image
   *  in format specified in grVideoOpen().
   */
  unsigned long int pont;
  int i,j;

  //call for function to prepare the image
  prepareimage();

  // NESTE PONTO A IMAGEM DE DIMENSAO H*W EM GRAY-LEVEL ESTA NA MATRIZ matrizgray[][]

  if(savemat==1)
  { 
    formatlab=fopen("matt.dat","wb"); /* For storing image in .dat. */
    for(i=0;i<H;i++)
    {
      for(j=0;j<W;j++) 	fprintf(formatlab,"%d ",matrizgray[i][j]);
    }
    fclose(formatlab);
    savemat=0;
  }

  // Apply low pass filter
  lowpass(matrizgray,H,W,0);  // NO LOW-PASS

  pont=0;
  for(i=0;i<H;i++)
  { 
     for(j=0;j<W;j++) 
     {
       gray[pont]=matrizgray[i][j];
       pont=pont+1;
     }
  }
  xDisplayImage(gray , 0, XD_GRAY,330,0 , 330, 0);

  //calculategradient(matrizgray,H,W);

  //Algorithm find the line 
  findline(matrizgray,H,W);

  // NESTE PONTO matgerad[][] CONTEM O GRADIENTE DA MATRIZ matrizgray
  pont=0;
  for(i=0;i<H;i++)
  { 
     for(j=0;j<W;j++) 
     {
      gray[pont]=matgrad1[i][j];
      pont=pont+1;
     }
  }

  xDisplayImage(gray , 0, XD_GRAY,330,250 , 330,250);
}  // FIM DE process_image()          

      

void lowpass(unsigned char **buffermat, int versize, int horsize, int times) /* For low pass filtering the image in buffermat[][] */
{
int i,j,k;
float auxfilter;
if(times==0) return; /* Perform no lowpass filtering. */
for(k=0;k<times;k++) /* How many times we apply the lowpass filtering. */
{
for(i=1;i<versize-1;i++) /* The first and last pixel will be ignored. */
  {
   for(j=1;j<horsize-1;j++) /* The first and last pixel will be ignored. */
      {
       auxfilter=(float)(buffermat[i+1][j]+buffermat[i-1][j]+buffermat[i][j+1]+buffermat[i][j-1]);
       buffermat[i][j]=(int)(auxfilter/4);
       }
   }
}
} /* End of the function void lowpass(). */



/******************* HOUGH TRANSFORM ***********************/

/* Calculates image gradient in the region between upper, lower, left, right. */


void findline(unsigned char **buf,int HEIGHT, int WIDTH)
{
  int i,j,gradx,grady,grad,ri,ti,k,i1,i2,j1,j2;
  float alpha,x,y;
  float theta, deltatheta;
  float  radius;
  float storetheta[200]; //obs: thetaph < 200
  float auxr,Pj,Pi,maxx;
  unsigned long counter;


  /* variaveis auxiliares para update prediction feita dentro desta funcao TRIAL */
  int h,l;
  float lambda=0.65,inv_lambda=1.0/0.65;
  unsigned long counter1,counter0;
  //int HEIGHT=240, WIDTH=320;
  float average;
  int pontos;
  float P[3][3],Pfierror[3],PinvfTPT[3][3]; 
  float BC1[3], teta[3],tetal[3]; 

  for(i=0;i<HEIGHT;i++)
  {
    for(j=0;j<WIDTH;j++) matgrad[i][j]=255;
  }

  for(i=upper;i<HEIGHT-lower;i++)
  {
    for(j=left;j<WIDTH-right;j++)
    {
      gradx=abs(buf[i][j]-buf[i-1][j]);
      grady=abs(buf[i][j]-buf[i][j-1]);
      alpha=atan((float)gradx/((float)grady+0.1)); // The 0.1 is for avoiding div. by zero.
      grad=gradx+grady;
      if(grad>255)  grad=255;
            // REMARK: We only evaluate the gradient bellow the upper ignore region.
      if(grad>GRADTH && alpha>=pi/10.0 && alpha<=pi/3.0) matgrad[i][j]=0;

    }
  }

  /*  We now calculate the Hough transform, in order to find the right line. */

  deltatheta=(thetamax-thetamin)/thetap;
  for(i=0;i<thetap;i++) storetheta[i]=thetamin+i*deltatheta; // In Rad.

  //  Generates matrix hough[i][j] with null elements
  for(i=0;i<thetap;i++)
  {
    for(j=0;j<radiusp;j++) hough[i][j]=0;
  }

  for(i=upper;i<HEIGHT-lower;i++)
  {
    for(j=right;j<WIDTH-left;j++)
    {
      if(matgrad[i][j]==0)     //Then we have a border point and then we apply Hough.
      {
        for(k=0;k<thetap;k++)
        {
          theta=storetheta[k]; // Present angle in rad.
          radius=(float)(HEIGHT-i)*sin(theta)+(float)j*cos(theta);
          ti=k;
          if(radius>=radiusmin && radius<=radiusmax)
          {
            auxr=((float)radiusp/((float)(radiusmax-radiusmin)))*((float)(radius-radiusmin));
            ri=floor(auxr);
            hough[ti][ri]=hough[ti][ri]+1; /*  Points which are at too close or too far from the origin are
                                            *  ignored  in the process of searching for lines. */
          } /* if radiush */
        }  /* for k=1 */
      }   /* if matgrad==0 */
    }    /* for j */
  }     /* for i */
  /* At this point platehoughh[][] has the Hough transform, with lines
   * parameterized by thetah and radiush. */
  
  ti=0;
  ri=0;
  maxx=0.0;
  
  for(i=0;i<thetap;i++)
  {
    for(j=0;j<radiusp;j++)
    {
      if(hough[i][j] > maxx)
      {
        maxx=hough[i][j];
        ti=i;
        ri=j;
      }
    }
  }
                    
  /* At this point we have a maximum of platehoughh in tih and rih. */

  theta=storetheta[ti]; /* This is the angle corresponding to the present maximum. */

  radius=((float)ri*((float)(radiusmax-radiusmin))/(float)radiusp)+radiusmin;
  /* This is the radiush corresponding to the present maximum. */

  /* TEST ONLY. */

  for(i=0;i<HEIGHT;i++)
  {
    for(j=0;j<WIDTH;j++)
    {
      matgrad1[i][j]=matgrad[i][j]; // Auxiliary matrix for superposing line.
      if(matgrad1[i][j]==0) matgrad1[i][j]=200;
    }
  }

  //theta=60*2*pi/360;
  //radius=50;

  Pj=radius*cos(theta);
  Pi=radius*sin(theta);

  counter=0;

  for(j=0;j<WIDTH;j++)
  {
    i=floor(Pi-((float)Pj/(float)Pi)*(j-Pj));
    i=HEIGHT-i;  // In Ref 2.
    if(i > 0 && i < HEIGHT)  matgrad1[i][j]=0;
    for(k=i-3;k<i+4;k++) 
    {
      if(k>=0 && k<H) 
      {
        if(matgrad[k][j]==0)
        {
          M[counter][0]=(HEIGHT-k)*(HEIGHT-k);
          M[counter][1]=(HEIGHT-k);
          M[counter][2]=1.0;
          X[counter]=j;         
          counter=counter+1;                 		
        }
      }
    }    
  } 

  for(i=0;i<3;i++)
  {
    for(j=0;j<3;j++)
    {
      MTM[i][j]=0.0;
      for(k=0;k<counter;k++) MTM[i][j]=MTM[i][j]+M[k][i]*M[k][j];
    }
  }

  inversemat(MTM,3,invMTM);


  for(i=0;i<3;i++) 
  {
    MTX[i]=0.0;
    for(j=0;j<counter;j++) MTX[i]=MTX[i]+M[j][i]*X[j];
  }

  for(i=0;i<3;i++)
  {
    coefd[i]=0.0;
    for(j=0;j<3;j++) coefd[i]=coefd[i]+invMTM[i][j]*MTX[j];
  } 

  for(i=0;i<HEIGHT;i++)
  {
    y=(HEIGHT-i);
    x=coefd[0]*y*y+coefd[1]*y+coefd[2];
    j=(int)x;
    if(j > 0 && j < WIDTH)  matgrad1[i][j]=0;
  }

  // AQUI ENTRA A ATUALIZACAO POR WRLS

  /********* FUNCAO PARA ATUALIZAR  PREDICAO DE TETA *************/
  teta[0]=coefd[0];
  teta[1]=coefd[1];
  teta[2]=coefd[2];

  for(i=0;i<3;i++) for(j=0;j<3;j++)  P[i][j]=((i==j) ? 1.0*fabs(teta[i]) : 0.0);

  counter0=0;  
  counter=0;
  counter1=0;
  for(i=0;i<HEIGHT;i++)
  {
    y=(HEIGHT-i);
    x=teta[0]*y*y+teta[1]*y+teta[2];
    j=(int)x;
    if(j>0 && j<WIDTH) 
      {
      matgrad1[i][j]=0;
      save[0][counter1]=j;  // coordenada horizontal da parabola
      save[1][counter1]=i;  // coordenada vertical, de cima para baixo 
      counter1++; 
      }
  }

    
  //printf("\n counter1 = %d", counter1);
  //usleep(900000000);
    

  for(l=0;l<counter1;l++)
  { 
    average=0.0;
    pontos=0; 
    for(k=save[1][l]-BAND;k<save[1][l]+BAND;k++)
      {
      if(k>0 && k<HEIGHT)
        {
        if(matgrad[k][save[0][l]]==0)
          {
          average=(float)average+save[1][k];
          pontos++;
          }
        }
      }

      if(pontos != 0)
        {
          average=(float)average/pontos;
          counter0++;
          if(counter0%2 == 0)
            {
              fiT[counter][0]=(float)(HEIGHT-average)*(HEIGHT-average);
              fiT[counter][1]=(float)(HEIGHT-average);
              fiT[counter][2]=1; // fiT na forma {y^2 y 1}
              Y[counter]=save[0][l];
              counter++;
            }
        } 
  }
        

    /* Atualizacao da Predicao */ 
  
  for(i=0;i<counter;i++)
  {
    Yhat[i]=0.0;
    for(j=0;j<3;j++) Yhat[i]=Yhat[i]+fiT[i][j]*teta[j];
  } 
    
    for(i=0;i<counter;i++)  error[i] = Y[i] - Yhat[i];
          
  /* Atualizacao da Matriz de Covariancia */

  // Determinacao de fi (transposta de fiT)
    for(i=0;i<3;i++)  for(j=0;j<counter;j++) fi[i][j]=fiT[j][i];

  for(i=0;i<3;i++) for(k=0;k<counter;k++)  
    {                 
    Pfi[i][k]=0.0;
    for(j=0;j<3;j++) Pfi[i][k]=Pfi[i][k]+P[i][j]*fi[j][k]; 
    }


  // Determinacao de (Pfi)' (transposta de Pfi)
    for(i=0;i<counter;i++)  for(j=0;j<3;j++) fiTPT[i][j]=Pfi[j][i];

  for(i=0;i<counter;i++) for(k=0;k<counter;k++)  
      {                 
      fiTPfi[i][k]=0.0;
      for(j=0;j<3;j++) fiTPfi[i][k]=fiTPfi[i][k]+fiT[i][j]*Pfi[j][k]; 
      } 
      
  for(i=0;i<counter;i++) for(h=0;h<counter;h++)
  {
    Eye[i][h]=((i==h) ? lambda : 0.0);
    Psum[i][h] = Eye[i][h] + fiTPfi[i][h]; 
  }


  inversemat(Psum,counter,invfTf); 

  // ha a multiplicacao de 3 matrizes:
  // Pfi[3][counter], invfTf[counter][counter], fiTPT[counter][3]
  /* as ordens sao Anxm, Bmxr e Crxp */

  for(i=0;i<3;i++)
    {
    for(h=0;h<3;h++)
      {
      PinvfTPT[i][h]=0.0;
      for(j=0;j<counter;j++)
      {
        BC[j][h]=0.0;
        for(k=0;k<counter;k++)
    {
          BC[j][h]=BC[j][h]+invfTf[j][k]*fiTPT[k][h];
          }     
        PinvfTPT[i][h]=PinvfTPT[i][h]+Pfi[i][j]*BC[j][h];
        }
      }
      }
  for(i=0;i<3;i++) for(j=0;j<3;j++) P[i][j]=inv_lambda*(P[i][j] - PinvfTPT[i][j]);

  /* Atualizacao da Estimativa dos Estados */

  // Primeiramente, calcula-se P[3][3]*fi[3][counter]*error[counter]

  for(i=0;i<3;i++)
  {
    Pfierror[i]=0.0;
    for(j=0;j<3;j++)
    {
      BC1[j]=0.0;
      for(k=0;k<counter;k++) BC1[j]=BC1[j]+fi[j][k]*error[k];
      Pfierror[i]=Pfierror[i]+P[i][j]*BC1[j];
    }
  }

  //entao, procede-se atualizacao de teta:
  
  for(i=0;i<3;i++)   teta[i]= teta[i] + Pfierror[i];

  for(i=0;i<HEIGHT;i++)
  {
    y=(HEIGHT-i);
    x=teta[0]*y*y+teta[1]*y+teta[2];
    j=(int)x;
    if(j>0 && j<WIDTH) matgrad1[i][j]=0;
  }


  /* NESTE PONTO TEMOS O SISTEMA M*a=X PARA CALCULAR OS COEFICIENTES DA PARABOLA a2y2+a1y+a0=x, ONDE y=H-i e x=j. 
  * USOU-SE ENTAO, MQ PARA CALCULAR a. A DIMENSAO DE X E counter */



  // We now calculate the Hough transform, in order to find the left line.

  deltatheta=(thetamax-thetamin)/thetap;
  for(i=0;i<thetap;i++) storetheta[i]=thetamin+0.5*pi+i*deltatheta; // In Rad


  //  Generates matrix hough[i][j] with null elements
  for(i=0;i<thetap;i++)
  {
    for(j=0;j<radiusp;j++)
    {
        hough[i][j]=0;
    }
  }

  for(i=upper;i<HEIGHT-lower;i++)
  {
    for(j=right;j<WIDTH-left;j++)
    {
        if(matgrad[i][j]==0)     //Then we have a border point and then we apply Hough.
        {
          for(k=0;k<thetap;k++)
          {
      theta=storetheta[k]; // Present angle in rad.
      radius=(float)(HEIGHT-i)*sin(theta)+(float)j*cos(theta);
            
  ti=k;
              if(radius>=radiusmin && radius<=radiusmax)
              {
              auxr=((float)radiusp/((float)(radiusmax-radiusmin)))*((float)(radius-radiusmin));

                ri=floor(auxr); 
            hough[ti][ri]=hough[ti][ri]+1; // Then rih varies from [radiusph/2,radiusph],
                // for radiusminh to radiusmaxh. Points which are at too close
                // or too far from the origin are ignored  in the process of searching for lines. */
              } // if radiush
            }   // for k=1
            }   // if matgrad==0
          }    // for j
      }        // for i
      // At this point platehoughh[][] has the Hough transform, with lines
      // parameterized by thetah and radiush. */


  ti=0;
  ri=0;
  maxx=0.0;
  for(i=0;i<thetap;i++)
  {
      for(j=0;j<radiusp;j++)
      {
        if(hough[i][j] > maxx)
        {
          maxx=hough[i][j];
          ti=i;
          ri=j;
        }
      }
  }
                    
  // At this point we have a maximum of platehoughh in tih and rih.

  theta=storetheta[ti]; // This is the angle corresponding to the present maximum. */

  radius=((float)ri*((float)(radiusmax-radiusmin))/(float)radiusp)+radiusmin;
    // This is the radiush corresponding to the present maximum.

  //printf("angulo= %f,raio= %f\n",theta,radius);

  // TEST ONLY.

  //theta=15*2*pi/360+0.5*pi;
  //radius=50; //250;

  Pj=radius*cos(theta);
  Pi=radius*sin(theta);

  counter=0;
  for(j=0;j<WIDTH;j++)
  {
    i=floor(Pi-((float)Pj/(float)Pi)*(j-Pj));
    i=HEIGHT-i;  // In Ref 2.
      if(i > 0 && i < HEIGHT)  matgrad1[i][j]=0;
      for(k=i-3;k<i+4;k++) 
      {
      if(k>=0 && k<H) 
      {
      if(matgrad[k][j]==0)
      {
          
        M[counter][0]=(HEIGHT-k)*(HEIGHT-k);
        M[counter][1]=(HEIGHT-k);
        M[counter][2]=1.0;
        X[counter]=j;         
        counter=counter+1;     		
        }
        }
        } 
    
  } 

  for(i=0;i<3;i++)
  {
  for(j=0;j<3;j++)
    {
      MTM[i][j]=0.0;
      for(k=0;k<counter;k++) MTM[i][j]=MTM[i][j]+M[k][i]*M[k][j];
    }
  }

  inversemat(MTM,3,invMTM);

  for(i=0;i<3;i++) 
  {
  MTX[i]=0.0;
  for(j=0;j<counter;j++) MTX[i]=MTX[i]+M[j][i]*X[j];
  }

  for(i=0;i<3;i++)
  {
    coefl[i]=0.0;
    for(j=0;j<3;j++) coefl[i]=coefl[i]+invMTM[i][j]*MTX[j];
  }


  for(i=0;i<HEIGHT;i++)
  {
    y=(HEIGHT-i);
    x=coefl[0]*y*y+coefl[1]*y+coefl[2];
    j=(int)x;
    if(j > 0 && j < WIDTH)  matgrad1[i][j]=0;
  }

  //printf("\ncoefl0=%f coefl1=%f coefl2=%f",coefl[0],coefl[1],coefl[2]);


  // AQUI ENTRA A ATUALIZACAO POR WRLS

  // update(matgrad,matgrad1, coefl,tetal);

  //printf("\ntetal0=%f tetal1=%f tetal2=%f",tetal[0],tetal[1],tetal[2]);


  //for(i=0;i<HEIGHT;i++)
  // {
  //  y=(HEIGHT-i);
  //  x=tetal[0]*y*y+tetal[1]*y+tetal[2];
  // j=(int)x;
  // if(j>0 && j<WIDTH) matgrad1[i][j]=0;
  //}


} // final da funcao findline




/************* FUNCAO PARA INVERSAO DE MATRIZES *****************/

void inversemat(float **mat, int order,float **matinv)
{
extern int invstatus;
//float aa[3][3],zz[3][3];
double aa[order][order],zz[order][order];
int i,j,k,l;
double cond;
double zeps1=1.0e-10; /* For verifying if mat is badly conditioned. */
for(i=0;i<order;i++)  for(j=0;j<order;j++)
      {
      aa[i][j]=mat[i][j];
      zz[i][j]=((i==j) ? 1.0 : 0.0);
      }
for(j=0;j<order;j++)
  {
  l=j;
  cond=fabs(aa[j][j]);
  for(i=j+1;i<order;i++)   if(cond<fabs(aa[i][j]))
{
l=i;
cond=fabs(aa[i][j]);
}
  if(cond<zeps1)
       {
        invstatus=1;
        return;
       }

  if(l!=j)    for(k=0;k<order;k++)
  {
  cond=aa[l][k];
  aa[l][k]=aa[j][k];
  aa[j][k]=cond;
  cond=zz[l][k];
  zz[l][k]=zz[j][k];
  zz[j][k]=cond;
  }

for(i=0;i<order;i++)   if(aa[i][j]!=0.0 && i!=j)
 {
 cond=-aa[j][j]/aa[i][j];
 for(k=0;k<order;k++) aa[i][k]=cond*aa[i][k]+aa[j][k];
 for(k=0;k<order;k++) zz[i][k]=cond*zz[i][k]+zz[j][k];
 }
}
for(i=0;i<order;i++)
{
 cond=aa[i][i];
 // printf("\ncond = %f ",cond);
 //usleep(90000000);
 for(j=0;j<order;j++) matinv[i][j]=zz[i][j]/cond;  
}

}

























