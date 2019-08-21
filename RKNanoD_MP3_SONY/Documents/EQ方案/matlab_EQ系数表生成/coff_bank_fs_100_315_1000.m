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
re = fopen('C:\Users\rockchip\Desktop\coef.txt','wb');%生成系数表，路径可修改
fs = [11025,22050,32000,44100,48000];%采样频率。其中，11025包含[8000, 11025, 12000]Hz,22050包含[16000, 22050, 24000]Hz
f0 = [100,315, 1000, 3150, 10000];%五段均衡中心频率
Q =[1.2, 1.2, 1.2, 1.2, 1.2];%品质因子

%生成EQ系数表Eq_Table[fs][f0][db]，使用时根据增益采用查表方式进行系数选择
for j= 1:1:length(fs)
     fprintf(re,'//第%d 个频率 \r\n',j-1);
        for  i =1:1:length(f0)
              fprintf(re,'//第%d 频段 \r\n',i-1);
              for db=-12:1:12%增益   
%                   Q = 1.4;
                   A = sqrt(10^(db/20));
                  Omega = 2*pi*f0(i)/fs(j);
                   s = sin(Omega);
                   c= cos(Omega);
                   alpha = s/(2 * Q(j));

%%%%%%%%计算带通滤波器系数 %%%%%%%%
                 b0 = 1+alpha*A;
                 b1 = -2*cos(Omega);
                 b2 = 1-alpha*A;
                 a0 = 1+alpha/A;
                 a1 = -2*cos(Omega);
                 a2 = 1- alpha/A;
 %%%%%%%%计算低通滤波器系数 %%%%%%%%
               %b0 = (1-c)/2;%低通
               %b1 = (1-c);
               %b2 = (1-c)/2;
               %a0 = 1+alpha;
               %a1 = -2*c;
              %a2 = 1-alpha;
              a_1 = [a0/a0 a1/a0 a2/a0];
              b_1 = [b0/a0 b1/a0 b2/a0];   
              
 %%%%%%%%H1为IIR滤波器频率响应 %%%%%%%%            
             [H1,w1]=freqz(b_1,a_1);
             gain1 = b0/a0;
             b_1 = b_1/gain1;
            %dbH=20*log10((abs(H)+eps)/max(abs(H)));%化为分贝值
            dbH1=20*log10(abs(H1));
           
         fprintf(re,'%f,%f,%f,%f,%f,\r\n',-a_1(2),-a_1(3),b_1(2),b_1(3),gain1);
             end
       end
end
fclose(re);