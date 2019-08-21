clc; close all; clear all;
% % % % % % % % % % % % % % % % % % % % % % % % % % % %
% 
%   Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd  All rights reserved.
%   FileName: eq_bass_60_120.m
%  Owner: Cherry.chen
%   Date: 2016.8.25
%   Time: 13:51:00
%   History:
%   <author>           <date>           <time>     <version>     <Desc>
%   Cherry.chen     2016.8.25          13:51:36   1.0
% % % % % % % % % % % % % % % % % % % % % % % % % % % % % %
re = fopen('D:\Program Files\MATLAB\R2013a\work\eq\bass_coef.txt','wb');%生成BASS系数表，路径可修改
fs = [11025,22050,32000,44100,48000];%采样率
f0 = [60,120];%中心频率
Q = [0.8,1.2];%品质因子
db = [12,10,8,6,4,2];%增益

for t = 1:length(f0) 
    fprintf(re,'//中心频率 = %dHz\r\n',f0(t));
    for j=  1:length(fs)
         fprintf(re,'//第%d 个频率 \r\n',j-1);
        for i =1:length(db)         
             A = sqrt(10^(db(i)/20));
             Omega = 2*pi*f0(t)/fs(j);
            s = sin(Omega);
            c= cos(Omega);
            alpha = s / (2 * Q(t));

%%%%%%%%%%带通IIR滤波器系数计算%%%%%%%
            b0 = 1 + alpha*A;%带通
            b1 = -2 * cos(Omega);
            b2 = 1-alpha*A;
            a0 = 1+alpha/A;
            a1 = -2*cos(Omega);
            a2 = 1- alpha/A;

%%%%%%%%%%带通IIR滤波器系数%%%%%%%
            a_1 = [a0/a0 a1/a0 a2/a0];
            b_1 = [b0/a0 b1/a0 b2/a0];    
            [H1,w1]=freqz(b_1, a_1);
  
            %dbH=20*log10((abs(H)+eps)/max(abs(H)));%化为分贝值
             dbH1=20*log10(abs(H1));
             fprintf(re,'%f,%f,%f,%f,%f,\r\n',-a_1(2),-a_1(3),b_1(1),b_1(2),b_1(3));
     
        end
    end
end