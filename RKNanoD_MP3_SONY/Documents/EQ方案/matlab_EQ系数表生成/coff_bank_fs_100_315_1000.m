clc;clear all;close all;
% % % % % % % % % % % % % % % % % % % % % % % % % % % %
%   Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd  All rights reserved.
%   FileName:coef_bank_fs_100_315_1000.m
%  Owner: Cherry.chen
%   Date: 2016.8.25
%   Time: 13:51:36
%   Desc: Uart Device Class
%   History:
%   <author>           <date>           <time>     <version>     <Desc>
%   Cherry.chen     2016.8.25          13:51:36   1.0
% % % % % % % % % % % % % % % % % % % % % % % % % % % % % %
re = fopen('C:\Users\rockchip\Desktop\coef.txt','wb');%����ϵ����·�����޸�
fs = [11025,22050,32000,44100,48000];%����Ƶ�ʡ����У�11025����[8000, 11025, 12000]Hz,22050����[16000, 22050, 24000]Hz
f0 = [100,315, 1000, 3150, 10000];%��ξ�������Ƶ��
Q =[1.2, 1.2, 1.2, 1.2, 1.2];%Ʒ������

%����EQϵ����Eq_Table[fs][f0][db]��ʹ��ʱ����������ò��ʽ����ϵ��ѡ��
for j= 1:1:length(fs)
     fprintf(re,'//��%d ��Ƶ�� \r\n',j-1);
        for  i =1:1:length(f0)
              fprintf(re,'//��%d Ƶ�� \r\n',i-1);
              for db=-12:1:12%����   
%                   Q = 1.4;
                   A = sqrt(10^(db/20));
                  Omega = 2*pi*f0(i)/fs(j);
                   s = sin(Omega);
                   c= cos(Omega);
                   alpha = s/(2 * Q(j));

%%%%%%%%�����ͨ�˲���ϵ�� %%%%%%%%
                 b0 = 1+alpha*A;
                 b1 = -2*cos(Omega);
                 b2 = 1-alpha*A;
                 a0 = 1+alpha/A;
                 a1 = -2*cos(Omega);
                 a2 = 1- alpha/A;
 %%%%%%%%�����ͨ�˲���ϵ�� %%%%%%%%
               %b0 = (1-c)/2;%��ͨ
               %b1 = (1-c);
               %b2 = (1-c)/2;
               %a0 = 1+alpha;
               %a1 = -2*c;
              %a2 = 1-alpha;
              a_1 = [a0/a0 a1/a0 a2/a0];
              b_1 = [b0/a0 b1/a0 b2/a0];   
              
 %%%%%%%%H1ΪIIR�˲���Ƶ����Ӧ %%%%%%%%            
             [H1,w1]=freqz(b_1,a_1);
             gain1 = b0/a0;
             b_1 = b_1/gain1;
            %dbH=20*log10((abs(H)+eps)/max(abs(H)));%��Ϊ�ֱ�ֵ
            dbH1=20*log10(abs(H1));
           
         fprintf(re,'%f,%f,%f,%f,%f,\r\n',-a_1(2),-a_1(3),b_1(2),b_1(3),gain1);
             end
       end
end
fclose(re);