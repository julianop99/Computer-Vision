% Computer Vision Lanemark detector for mobile Robots
% Author: Juliano Pimentel (https://github.com/julianop99)

clear all; close all;clc;
H=240; W=320;HEIGHT=240; WIDTH=320;counter=1;BAND=10;

% Define treshold and limits for the hough transform
THRES=30;upper=10;lower=10;left=10;right=10;
THRES=input('Define Threshold for Hough Transform (default 30):  ');
upper=input('Ignore first upper pixels (default 10):  ');
lower=input('Ignore last lower pixels (default 10): ');
left=input('Ignore first left pixels (default 10): ');
right=input('Ignore last right pixels (default 10): ');

% Load the dat file with the image acquired.
load matt6.dat;
%[file,path] = uigetfile('*.dat');
%load teste;

%filename1 = strcat(path,file)

%load(filename1)
%str=whos('-file',filename)
%str={str.name}
%load(filename,str{:})

k=1;
for i=1:240
   for j=1:320
      mat(i,j)=matt6(k);
      k=k+1;
   end;
end;
%mat1=uint8(mat);
%imshow(mat1);
for i=1:240
   for j=1:320
      matgrad(i,j)=255; % Fill with white.
   end;
end;
% Calculate image gradient in the region between upper, lower, left, right.
for i=upper:240-lower
   for j=left:320-right
      gradx=abs(mat(i,j)-mat(i-1,j));
      grady=abs(mat(i,j)-mat(i,j-1));
      alpha=atan(gradx/(grady+0.1)); % The 0.1 is for avoiding div. by zero.
      grad=gradx+grady;
      if grad>255
         grad=255;
      end;
      % REMARK: We only avaluate the gradient bellow the upper ignore region.
      if grad>THRES & alpha>=pi/10 & alpha<=pi/3
         matgrad(i,j)=0;
      else
         matgrad(i,j)=255;
      end;
    end;
end;
%imshow(matgrad);
%title('Image gradient in allowable region');
% We now calculate the Hough transform, in order to find the two lines.
thetaminh=10*(2*pi/360); % In rad
thetamaxh=30*(2*pi/360); % In rad
thetaph=50; % Number of partitions
deltathetah=(thetamaxh-thetaminh)/thetaph;
storethetah=thetaminh:deltathetah:thetamaxh; % In degrees.
radiusminh=50; % Minimum radius depend on image width.
radiusmaxh=350; % Maximum radius depend on image width.
radiusph=150; % Partitions in radius.
for i=1:thetaph
   for j=1:radiusph
      platehoughh(i,j)=0;
   end;
end;

for i=upper:240-lower
   for j=right:230-left
      if matgrad(i,j)==0 % Then we have a border point and then we apply Hough.
         for k=1:thetaph
            thetah=storethetah(k); % Present angle in rad.
          radiush=(240-i)*sin(thetah)+j*cos(thetah);
   %radiush=sqrt((240-i)*sin(thetah)*(240-i)*sin(thetah)+(j*cos(thetah)*j*cos(thetah)));
            tih=k;
            if radiush>=radiusminh & radiush<=radiusmaxh
                auxr=(radiusph/(radiusmaxh-radiusminh))*(radiush-radiusminh);
                rih=ceil(auxr);
                platehoughh(tih,rih)=platehoughh(tih,rih)+1; % Then rih varies from [radiusph/2,radiusph],
                                                           % for radiusminh to radiusmaxh. Points which are at too close or too far from the origin are ignored  in the process of searching for lines. */
            end; % if radiush
        end; % for k=1
     end; % if matgrad==0
  end; % for j
end;% for i
% At this point platehoughh[][] has the Hough transform, with lines 
% parameterized by thetah and radiush. */
 tih=1;
 rih=1;
 maxx=0;
 for i=1:thetaph
         for j=1:radiusph
                if(platehoughh(i,j)>maxx)
                          maxx=platehoughh(i,j);
                           tih=i;
                           rih=j;
                  end;
          end;
  end;
                  
  % At this point we have a maximum of platehoughh in tih and rih. 
thetah=storethetah(tih); % This is the angle corresponding to the present maximum. */
if(thetah>=0.0) 
   radiush=(rih*(radiusmaxh-radiusminh)/radiusph)+radiusminh; % This is the radiush corresponding to the present maximum. */
end;
%if(thetah<0.0)
 %  radiush=(rih/inclinationhinv)-radiusmaxh; % This is the radiush corresponding to the present maximum. */
% end;


% TEST ONLY.
for i=1:240
   for j=1:320
      matgrad1(i,j)=matgrad(i,j); % Auxiliary matrix for superposing line.
      if matgrad1(i,j)==0
         matgrad1(i,j)=200;
      end;
    end;
end;

%thetah=25*2*pi/360;
%radiush=250;
Pj=radiush*cos(thetah);
Pi=radiush*sin(thetah);
%numpixel=0;
for j=1:320
   i=floor(Pi-(Pj/Pi)*(j-Pj));
   i=240-i; % In Ref 2.
   if i>0 & i<=240
      matgrad1(i,j)=0;
      %lasti=i;
      %lastj=j;
   end;
%end; 
%for i=1:255
 %  map(i,1)=i/255;
 %  map(i,2)=i/255;
 %  map(i,3)=i/255;
%end;
%imshow(matgrad1,map); % Map is necessary for displaying more than 2 colors.

  for k=i-BAND:i+BAND 
    if k>0 & k<H 
     if matgrad(k,j)==0
       M(counter,1)=(HEIGHT-k)*(HEIGHT-k);
       M(counter,2)=(HEIGHT-k);
       M(counter,3)=1.0;
       X(counter)=j;         
       counter=counter+1;   
    end;
  end;
 end;
end;

for i=1:3
 for j=1:3
  MTM(i,j)=0;
  for k=1:counter-1
     MTM(i,j)=MTM(i,j)+M(k,i)*M(k,j);
  end;
end;
end;

MTM

%for i=1:255
 % map(i,1)=i/255;
%  map(i,2)=i/255;
%  map(i,3)=i/255;
%end;
%figure(1);
%imshow(matgrad1,map); % Map is necessary for displaying more than 2 colors.
%hold on;

invMTM = inversemat(MTM,3);

for i=1:3
   MTX(i)=0;
   for j=1:counter-1
       MTX(i)=MTX(i)+M(j,i)*X(j);
   end;
end;

for i=1:3
  coefd(i)=0;
  for j=1:3 
     coefd(i)=coefd(i)+invMTM(i,j)*MTX(j);
  end;
end;


  for i=1:HEIGHT
     y=(HEIGHT-i);
     x=coefd(1)*y*y+coefd(2)*y+coefd(3);
     j=round(x);
     if j > 0 & j < WIDTH
        matgrad1(i,j)=0;
     end;
  end;

for i=1:255
   map(i,1)=i/255;
   map(i,2)=i/255;
   map(i,3)=i/255;
end;
figure(1);
imshow(matgrad1,map); % Map is necessary for displaying more than 2 colors.
coefd
 %teta = update(matgrad,matgrad1,coefd);
 %teta
 
 
lambda=1;inv_lambda=1/lambda;
HEIGHT=240; WIDTH=320; H=240; W=320;BAND =5;
 
teta(1)=coefd(1);
teta(2)=coefd(2);
teta(3)=coefd(3);
teta1=teta
V=[abs(teta(1)) abs(teta(2)) abs(teta(3))];
P=10*diag(V)
 
counter=0;
counter1=0;
counter2=0;

for i=1:HEIGHT
     y=(HEIGHT-i);
     x=teta(1)*y*y+teta(2)*y+teta(3);
     j=round(x);
     if j > 0 & j < WIDTH
        matgrad1(i,j)=0;
        counter1=counter1+1;
        salva(1,counter1)=j; % coordenada horizontal da parabola
        salva(2,counter1)=i; % coordenada vertical, de cima p/ baixo
     end;
end;

for l=1:counter1
   average=0;
   pontos=0;
   for k=salva(2,l)-BAND:1:salva(2,l)+BAND
     if k > 0 & k < HEIGHT
        if matgrad(k,salva(1,l)) == 0
           average=average+salva(2,k);
           pontos=pontos+1;
        end;
     end;
  end;  
  %pontos
  %average
   if pontos ~=0 
   average=average/pontos;   
   counter2=counter2+1; 
   if rem(counter2,1)==0
   %counter2=counter2+1;
   %if counter2 <=50
      counter=counter+1;
      fiT(counter,1)=(HEIGHT-average)*(HEIGHT-average);
      fiT(counter,2)=(HEIGHT-average);
      fiT(counter,3)=1; %// fiT na forma {y^2 y 1}
      Y(counter)=salva(1,l);
   end;
   end;

end;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% /* Atualizacao da Predicao */
 

 for i=1:counter
   Yhat(i)=0;
   for j=1:3
      Yhat(i)=Yhat(i)+fiT(i,j)*teta(j);
   end;
end;

 for i=1:counter
     error(i) = (Y(i))' - Yhat(i);
 end;
        
 %/* Atualizacao da Matriz de Covariancia */

 for i=1:3
       for j=1:counter
           fi(i,j)=fiT(j,i);
       end;
     end;


for i=1:3
   for k=1:counter
       Pfi(i,k)=0;
       for j=1:3
         %fi(j,k)=fiT(k,j);
         Pfi(i,k)=Pfi(i,k)+P(i,j)*fi(j,k);
      end;
   end;
end;

 for i=1:counter
        for j=1:3
           fiTPT(i,j)=Pfi(j,i);
        end;
     end;


for i=1:counter
   for k=1:counter            
       fiTPfi(i,k)=0;
       for j=1:3
%           fiTPT(i,j)=Pfi(j,i);
           fiTPfi(i,k)=fiTPfi(i,k)+fiT(i,j)*Pfi(j,k);
       end;
   end;
end;
for i=1:counter
   for j=1:counter
      if i==j
         Eye(i,j)=lambda;
      else
         Eye(i,j)=0;
     end;
     Psum(i,j)=Eye(i,j)+fiTPfi(i,j);
  end;
 end;
 
 %formatlab=fopen('Psum.dat','wb'); %/* For storing image in .dat. */
  % for i=1:counter
   %   for j=1:counter
   %      fprintf(formatlab,'%.4f\n',Psum(i,j));
   %   end;
   %end;
   %fclose(formatlab); 

 tic
    %invfTf=inv(Psum);
    %invfTf = inversemat(Psum,counter); 
    
    
      
   
%/************* FUNCAO PARA INVERSAO DE MATRIZES *****************/

invstatus=0;
zeps1=1.0e-10; %/* For verifying if mat is badly conditioned. */
order=counter;

for i=1:order
   for j=1:order
     aa(i,j)=Psum(i,j);
     zz=eye(i,j);
   end;
end;

for j=1:order
  l=j;
  cond=abs(aa(j,j));
  for i=j+1:order
     if cond < abs(aa(i,j))
        l=i;
        cond=abs(aa(i,j));
     end;
  end;
  cond % mostrar o comportamento de cond
  
  if cond<zeps1
     invstatus=1;
     return;
  end;
   if l~=j
        for k=1:order
           cond=aa(l,k);
           aa(l,k)=aa(j,k);
           aa(j,k)=cond;
           cond=zz(l,k);
           zz(l,k)=zz(j,k);
           zz(j,k)=cond;
        end;
   end;
     for i=1:order
        if aa(i,j) ~= 0 & i ~= j
           cond=-aa(j,j)/aa(i,j);
           for k=1:order
              aa(i,k)=cond*aa(i,k)+aa(j,k);
           end;
           for k=1:order
              zz(i,k)=cond*zz(i,k)+zz(j,k);
           end;
        end;
     end;
  end;
  
for i=1:order
   cond=aa(i,i);
   for j=1:order
      invfTf(i,j)=zz(i,j)/cond;  
   end;
end;

%*******AQUI TERMINA A FUNÇAO INVERSAO DE MATRIZES

%t=toc;
%disp('tempo');
%disp(t);

load InvfTf1.dat

k=1;
for i=1:sqrt(max(size(InvfTf1)))
   for j=1:sqrt(max(size(InvfTf1)))
      Invf(i,j)=InvfTf1(k);
      k=k+1;
   end;
end;


qualidade = invfTf(1:24,1:24) - Invf;


    
  %// ha a multiplicacao de 3 matrizes:
  %// Pfi[3][counter], invfTf[counter][counter], fiTPT[counter][3]

 for i=1:3             																																
     for h=1:3
       PinvfTPT(i,h)=0;
       for j=1:counter
       BC(j,h)=0;
          for k=1:counter
             BC(j,h)=BC(j,h)+invfTf(j,k)*fiTPT(k,h);
          end;
          PinvfTPT(i,h)=PinvfTPT(i,h)+Pfi(i,j)*BC(j,h);
       end;
    end;
 end;
 
for i=1:3
   for j=1:3
       P(i,j)=inv_lambda*(P(i,j) - PinvfTPT(i,j));
   end;
end;
%cond(P)


%/* Atualizacao da Estimativa dos Estados */

%// Primeiramente, calcula-se P[3][3]*fi[3][counter]*error[counter]

for i=1:3
    Pfierror(i)=0;
    for j=1:3
        BC1(j)=0;
        for k=1:counter
            BC1(j)=BC1(j)+fi(j,k)*error(k);
        end;
        Pfierror(i)=Pfierror(i)+P(i,j)*BC1(j);
   end;
end;

%//entao, procede-se atualizacao de teta:
 
 for i=1:3
    teta(i)= teta(i) + Pfierror(i)
 end;
teta2=teta 
 %%%%%%%%%%%%%%%%%%%%%%%%%%%%
 
 for i=1:HEIGHT
     y=(HEIGHT-i);
     x=teta(1)*y*y+teta(2)*y+teta(3);
     j=round(x);
     if j > 0 & j < WIDTH
        matgrad1(i,j)=0;
     end;
 end;
 
 for i=1:255
   map(i,1)=i/255;
   map(i,2)=i/255;
   map(i,3)=i/255;
 end;
 figure(2);
 imshow(matgrad1,map); % Map is necessary for displaying more than 2 colors.
