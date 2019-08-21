/******************************************************
Copyright(C)2007,Rockchip Co., Ltd. All Rights Reserved.
File:  bmpdecode.c
Desc:  BMP图像解码
Note:
Author:  evan wu
$Log:
Revision :

******************************************************/

//*********************includes*****************************************************
#include "..\ImageInclude\image_main.h"
#ifdef BMP_DEC_INCLUDE
#pragma arm section code = "BmpDecCode", rodata = "BmpDecCode", rwdata = "BmpDecData", zidata = "BmpDecBss"

#include "pBMPDEC.h"
INT32S BMP_ThumbNail_Init(BMPFACTOR *bmpFactor, BMPOUTFACTOR *BmpOutFactor, BMP *bmp, int BufWidth, int BufHeight);
static INT8U gBMPReadBuf[8000*4];
static INT8U gBMPColorBuf[256*4];
static BMPFACTOR bmpFactor;
enum
{
	BMP_Type_2bitRGB,
	BMP_Type_4bitRGB,
	BMP_Type_8bitRGB,
	BMP_Type_16bitRGB,
	BMP_Type_24bitRGB,
	BMP_Type_32bitRGB,
	BMP_Type_RLE8,
	BMP_Type_RLE4,
	BMP_Type_16bitRGB565,
	BMP_Type_32bitRGB888
};

INT16U BMP_CustomOutput_16BitRGB565(IM_PIX_INFO out_info)
{
    int R, G, B;
    INT16U RGB;

    R = out_info.Comp_1;
    G = out_info.Comp_2;
    B = out_info.Comp_3;

    RGB = (R & 0xF8) << 8;
    RGB |= (G & 0xFC) << 3;
    RGB |= (B >> 3);
    return RGB;
}

/******************************************************
Name: BMP_CustomOutput_24BitRGB888
Desc: 将输入的一个像素的R数据，G数据，B数据重新编码成24bitRGB888格式
Param: out_info:一个像素的RGB数据
Return: 24位RGB888编码的像素数据
Global: 无
Note:
Author: evan wu
Log:
******************************************************/
INT32U BMP_CustomOutput_24BitRGB888(IM_PIX_INFO out_info)
{
    int R, G, B;
    INT32U RGB;

    R = out_info.Comp_1;
    G = out_info.Comp_2;
    B = out_info.Comp_3;

    RGB = R << 8;
    RGB |= (G << 16);
    RGB |= (B << 24);

    return RGB;
}
/******************************************************
Name: BMP_CustomOutput_32BitRGB888
Desc: 将输入的一个像素的R数据，G数据，B数据重新编码成32bitRGB888格式
Param: out_info:一个像素的RGB数据
Return: 32位RGB888编码的像素数据
Global: 无
Note:
Author: evan wu
Log:
******************************************************/
INT32U BMP_CustomOutput_32BitRGB888(IM_PIX_INFO out_info)
{
    int R, G, B;
    INT32U RGB = 0;

    R = out_info.Comp_1;
    G = out_info.Comp_2;
    B = out_info.Comp_3;

    RGB = ((R & 0Xff) << 16);
    RGB |= ((G & 0xff) << 8);
    RGB |= (B & 0xff);

    return RGB;
}
INT32S BMP_Decode_init(BMP_CALLBACKS_DECODE BMP_callbacks_decode, BMPOUTFACTOR *BmpOutFactor, BMP *bmp, int BufWidth, int BufHeight)
{

    memset(&bmpFactor, 0, sizeof(BMPFACTOR));
    if (!BMP_ThumbNail_Init(&bmpFactor, BmpOutFactor, bmp, BufWidth, BufHeight))
    {
        return FALSE;
    }
    
    if (bmp->bmih.biHeight > 0)//BMP file is reversed when bmp->bmih.biHeight>0
       //BMP_callbacks_decode.seek_func(BmpOutFactor->BmpFile, (long)((BmpOutFactor->dsty + 1)*(bmpFactor.ReadBufSize)), SEEK_END);
       BMP_callbacks_decode.seek_func(BmpOutFactor->BmpFile, (bmpFactor.ReadBufSize)*(bmpFactor.height - 1) + bmp->bmfh.bfOffBits, SEEK_SET);
    else
       BMP_callbacks_decode.seek_func(BmpOutFactor->BmpFile, bmp->bmfh.bfOffBits, SEEK_SET);
	return TRUE;
}

static INT32S BMP_DecOneRow(int BufWidth,int BufHeight,const int BMP_Type,BMP *bmp,BMPFACTOR *bmpFactor,BMPOUTFACTOR *BmpOutFactor,BMP_CALLBACKS_DECODE* BMP_callbacks_decode)
{
    IM_PIX_INFO out_info;
    long  dstx;
    targbmodel *dstpixels, *pDstLine;
    long  submovedx;
	
	
    submovedx = (BufWidth - bmpFactor->Dst.width) / 2;//x轴的正方向平移量
    //submovedy = (BufHeight - bmpFactor->Dst.height) / 2;//y轴的正方向平移量

    //for (dsty = 0;dsty < bmpFactor->Dst.height;dsty++)
    if(BmpOutFactor->dsty >= (unsigned long)bmpFactor->Dst.height)
		    return FALSE;
    {
        LCD_RGBDATA RGB = 0;
		INT16U BufTemp;
        bmpFactor->x = 0;
#if 0        
        if (bmp->bmih.biHeight < 0)//在循环体外算出该行的首地址
            pDstLine = (targbmodel*)((INT8U*)(bmpFactor->Dst.pdata) + bmpFactor->Dst.byte_width * (dsty + submovedy));    //如果原图是顺序存储，则对应目标图像也顺序存储
        else
            pDstLine = (targbmodel*)((INT8U*)(bmpFactor->Dst.pdata) + bmpFactor->Dst.byte_width * (BufHeight - (dsty + submovedy) - 1)); //如果原图是逆序存储，则对应目标图像也逆序存储
#else
        pDstLine = (targbmodel*)((INT8U*)(bmpFactor->Dst.pdata));
        BMP_callbacks_decode->read_func(bmpFactor->ReadBuf, 1, sizeof(INT8U)*bmpFactor->ReadBufSize, BmpOutFactor->BmpFile);  //读入一行图像数据        
#endif            
        for (dstx = 0;dstx < bmpFactor->Dst.width;dstx++)      //解一行的数据，一个循环解一个像素
        {          
			switch(BMP_Type)
			{
				case BMP_Type_24bitRGB:
				{
//		            BMP_callbacks_decode->read_func(bmpFactor->ReadBuf, 1, 3, BmpOutFactor->BmpFile); //读入一行图像数据
//					out_info.Comp_1 = bmpFactor->ReadBuf[2]; 		 //写入R
//					out_info.Comp_2 = bmpFactor->ReadBuf[1]; 		 //写入G
//					out_info.Comp_3 = bmpFactor->ReadBuf[0]; 		 //写入B	
					out_info.Comp_1 = bmpFactor->ReadBuf[(bmpFactor->x >> 16)*3+2]; 		 //写入R
					out_info.Comp_2 = bmpFactor->ReadBuf[(bmpFactor->x >> 16)*3+1]; 		 //写入G
					out_info.Comp_3 = bmpFactor->ReadBuf[(bmpFactor->x >> 16)*3+0]; 		 //写入B		
							
				}
				break;
				case BMP_Type_32bitRGB888:
				{
//		            BMP_callbacks_decode->read_func(bmpFactor->ReadBuf, 1, 4, BmpOutFactor->BmpFile); //读入一行图像数据
					out_info.Comp_1 = bmpFactor->ReadBuf[(bmpFactor->x >> 16)*4+3]; 		 //写入R
					out_info.Comp_2 = bmpFactor->ReadBuf[(bmpFactor->x >> 16)*4+2]; 		 //写入G
					out_info.Comp_3 = bmpFactor->ReadBuf[(bmpFactor->x >> 16)*4+1]; 		 //写入B
				}
				break;
				case BMP_Type_32bitRGB:
				{
//		            BMP_callbacks_decode->read_func(bmpFactor->ReadBuf, 1, 4, BmpOutFactor->BmpFile); //读入一行图像数据
					out_info.Comp_1 = bmpFactor->ReadBuf[(bmpFactor->x >> 16)*4+2]; 		 //写入R
					out_info.Comp_2 = bmpFactor->ReadBuf[(bmpFactor->x >> 16)*4+1]; 		 //写入G
					out_info.Comp_3 = bmpFactor->ReadBuf[(bmpFactor->x >> 16)*4+0]; 		 //写入B					
				}
				break;				
				case BMP_Type_16bitRGB565:
				{
					
//		            BMP_callbacks_decode->read_func(&BufTemp, 1, 2, BmpOutFactor->BmpFile); //读入一行图像数据
                BufTemp = *((INT16U*)(&bmpFactor->ReadBuf[(bmpFactor->x >> 16)*2])); 
		            out_info.Comp_1 = (BufTemp & RGB565_MARK_R) >> 8;   //读入R
		            out_info.Comp_2 = (BufTemp & RGB565_MARK_G) >> 3;   //读入G
		            out_info.Comp_3 = (BufTemp & RGB565_MARK_B) << 3;   //读入B					
				}
				break;
				case BMP_Type_16bitRGB:
				{
//		            BMP_callbacks_decode->read_func(&BufTemp, 1, 2, BmpOutFactor->BmpFile); //读入一行图像数据
                BufTemp = *((INT16U*)(&bmpFactor->ReadBuf[(bmpFactor->x >> 16)*2])); 
		            out_info.Comp_1 = (BufTemp & RGB555_MARK_R) >> 7;   //读入R
		            out_info.Comp_2 = (BufTemp & RGB555_MARK_G) >> 2;   //读入G
		            out_info.Comp_3 = (BufTemp & RGB555_MARK_B) << 3;   //读入B					
				}	
				break;
				case BMP_Type_8bitRGB:
				{					
//					BMP_callbacks_decode->read_func(&BufTemp, 1, 1, BmpOutFactor->BmpFile);  //读入一行图像数据
                BufTemp = (INT16U)bmpFactor->ReadBuf[(bmpFactor->x >> 16)];
		            out_info.Comp_1 = bmp->bmiColors[BufTemp].rgbRed;    //读入R
		            out_info.Comp_2 = bmp->bmiColors[BufTemp].rgbGreen;    //读入G
		            out_info.Comp_3 = bmp->bmiColors[BufTemp].rgbBlue;    //读入B
				}
				break;
				case BMP_Type_4bitRGB:
				{
            INT32S i = bmpFactor->x >> 16;
            INT8U PixelInByte = i % 2;
            BufTemp = bmpFactor->ReadBuf[(INT32S)(i*bmpFactor->BytesPerPixs)];                                       //读入一个像素的图像数据，一个像素8bit
            switch (PixelInByte)
            {
                case 0:              //高4位为第一个像素
                    BufTemp = (BufTemp >> 4) & 0x0F;
                    PixelInByte = 1;
                    break;
                case 1:              //第四位为第二个像素
                    BufTemp &= 0x0F;
                    PixelInByte = 0;
                    break;
                default:
                    break;
            }
            out_info.Comp_1 = bmp->bmiColors[BufTemp].rgbRed;    //读入R
            out_info.Comp_2 = bmp->bmiColors[BufTemp].rgbGreen;    //读入G
            out_info.Comp_3 = bmp->bmiColors[BufTemp].rgbBlue;    //读入B            					
				}
				break;
				case BMP_Type_2bitRGB:
				{
			      INT32S i = bmpFactor->x >> 16;
            INT8U PixelInByte = i % 8;
            BufTemp = bmpFactor->ReadBuf[(INT32S)(i*bmpFactor->BytesPerPixs)];                                       //读入一个像素的图像数据，一个像素8bit
            switch (PixelInByte)
            {
                case 0:
                    BufTemp = (BufTemp >> 7) & 0x01;                        //取得第一个像素
                    PixelInByte++;
                    break;
                case 1:
                    BufTemp = (BufTemp >> 6) & 0x01;                        //取得第二个像素
                    PixelInByte++;
                    break;
                case 2:
                    BufTemp = (BufTemp >> 5) & 0x01;                        //取得第三像素
                    PixelInByte++;
                    break;
                case 3:
                    BufTemp = (BufTemp >> 4) & 0x01;                        //取得第四像素
                    PixelInByte++;
                    break;
                case 4:
                    BufTemp = (BufTemp >> 3) & 0x01;                        //取得第五像素
                    PixelInByte++;
                    break;
                case 5:
                    BufTemp = (BufTemp >> 2) & 0x01;                        //取得第六像素
                    PixelInByte++;
                    break;
                case 6:
                    BufTemp = (BufTemp >> 1) & 0x01;                        //取得第七像素
                    PixelInByte++;
                    break;
                case 7:
                    BufTemp &= 0x01;                                        //取得第八个像素
                    PixelInByte = 0;
                    break;
                default:
                    break;
            }
            out_info.Comp_1 = bmp->bmiColors[BufTemp].rgbRed;               //读入R
            out_info.Comp_2 = bmp->bmiColors[BufTemp].rgbGreen;             //读入G
            out_info.Comp_3 = bmp->bmiColors[BufTemp].rgbBlue; 
          }
          break;
				
			}
			
            RGB = BMP_callbacks_decode->write_func(out_info);    //转换成LCD所需的编码格式
            dstpixels = (targbmodel*)pDstLine + dstx + submovedx;      //取得该像素在缓冲区中的位置
            *dstpixels = *((targbmodel*) & RGB);       //写入缓冲区
            bmpFactor->x += bmpFactor->dstxInt;            //改成加法，提高效率
//            i = ((bmpFactor->x >> 16) - ((bmpFactor->x - bmpFactor->dstxInt) >> 16) - 1) * (INT32S)bmpFactor->BytesPerPixs;
//            BMP_callbacks_decode->seek_func(BmpOutFactor->BmpFile, i, SEEK_CUR); //SEEK到下一次要读入的位置
        }
        bmpFactor->y += bmpFactor->dstyInt;             //改成加法，提高效率
        bmp->FileOffset = bmpFactor->ReadBufSize * ((bmpFactor->y >> 16) - ((bmpFactor->y - bmpFactor->dstyInt) >> 16) - 1);
        bmp->FileOffset += bmpFactor->ReadBufSize - (unsigned int)(bmp->bmih.biWidth * bmpFactor->BytesPerPixs);
        if(bmp->bmih.biHeight > 0)
           BMP_callbacks_decode->seek_func(BmpOutFactor->BmpFile, bmp->bmfh.bfOffBits + (bmpFactor->height - (bmpFactor->y >> 16))*bmpFactor->ReadBufSize, SEEK_SET);
        else
           //BMP_callbacks_decode->seek_func(BmpOutFactor->BmpFile, bmp->FileOffset, SEEK_CUR); //SEEK到下一次要读入的位置
           BMP_callbacks_decode->seek_func(BmpOutFactor->BmpFile, bmp->bmfh.bfOffBits + (bmpFactor->y >> 16)*bmpFactor->ReadBufSize, SEEK_SET); //SEEK到下一次要读入的位置
    }	
	return TRUE;
}

INT32S BMP_ReadTab(BMP_CALLBACKS_DECODE BMP_callbacks_decode, BMPOUTFACTOR *BmpOutFactor, BMP *bmp)
{
    bmp->bmnumcolors = (1 << bmp->bmih.biBitCount);       //确定有多少个调色板
    bmp->bmiColors = (RGBQUAD *)gBMPColorBuf;//MALLOC(bmp->bmnumcolors * sizeof(RGBQUAD)); //

	while(bmp->bmnumcolors > 256);
    memset(bmp->bmiColors, 0, bmp->bmnumcolors*sizeof(RGBQUAD));
    BMP_callbacks_decode.read_func(bmp->bmiColors, sizeof(RGBQUAD), bmp->bmnumcolors, BmpOutFactor->BmpFile);//读调色板
    return TRUE;
}
INT32S BMP_ThumbNail_Init(BMPFACTOR *bmpFactor, BMPOUTFACTOR *BmpOutFactor, BMP *bmp, int BufWidth, int BufHeight)
{
    bmpFactor->height = abs(bmp->bmih.biHeight);          //取读入图像高度的绝对值
    bmpFactor->BytesPerPixs = (float)bmp->bmih.biBitCount / 8;       //计算每个像素有几个byte
    // Allocate memory for picture


    bmpFactor->ReadBufSize = (INT32S)(bmp->bmih.biWidth * bmpFactor->BytesPerPixs + 3.99) & (~3); //读入图像的字节宽度，一行字节宽度按4对齐;
    bmpFactor->ReadBuf = gBMPReadBuf;//(INT8U *)MALLOC(bmpFactor->ReadBufSize * sizeof(INT8U));     //开读入一行图像数据的Buffer
//    if (!bmpFactor->ReadBuf)
//    {
//        BMP_free(bmp);
//        return FALSE;
//    }
    memset(bmpFactor->ReadBuf, 0, bmpFactor->ReadBufSize);

    if ((bmp->bmih.biWidth < BufWidth) && (bmpFactor->height < BufHeight))//图像宽高小于缓冲区的宽高则不进行缩放
    {
        bmpFactor->Dst.width = bmp->bmih.biWidth;
        bmpFactor->Dst.height = bmpFactor->height;
    }
    else                 //大于则缩小到缓冲区大小
    {
        bmpFactor->zoomy = bmpFactor->zoomx = MIN((float)BufWidth / bmp->bmih.biWidth, (float)BufHeight / bmpFactor->height);
        bmpFactor->Dst.width = (long)(bmp->bmih.biWidth * bmpFactor->zoomx);
        bmpFactor->Dst.height = (long)(bmpFactor->height * bmpFactor->zoomy);
    }
    /* 当图片高度为一个像素，出现除0现象，这里作下规避， 081016， add by phc */
    if (bmpFactor->Dst.width == 0)
        bmpFactor->Dst.width = 1;
    if (bmpFactor->Dst.height == 0)
        bmpFactor->Dst.height = 1;
    //bmpFactor->Dst.byte_width = (bmpFactor->Dst.width * sizeof(LCD_RGBDATA)+3) & ~3;    //存在缓冲区中的图像数据的一行字节宽度
    bmpFactor->Dst.byte_width = BufWidth * sizeof(LCD_RGBDATA);    //存在缓冲区中的图像数据的一行字节宽度
    BmpOutFactor->ImageWInBuf = (unsigned long)(bmpFactor->Dst.width);
    BmpOutFactor->ImageHInBuf = (unsigned long)(bmpFactor->Dst.height);
    bmpFactor->Dst.pdata = (targbmodel*)(BmpOutFactor->ImageBufAddr);            //缓冲区Buffer的头地址
    bmpFactor->dstxInt = (bmp->bmih.biWidth << 16) / bmpFactor->Dst.width + 1;  //预先计算出比值
    bmpFactor->dstyInt = (bmpFactor->height << 16) / bmpFactor->Dst.height + 1;    //预先计算出比值
    //bmp->FileOffset = ((bmpFactor->dstyInt>>16)-1)*bmpFactor->ReadBufSize;
    bmpFactor->y = 0;
    return TRUE;
}



/******************************************************
Name: BMP_Convert_2bitRGB_To_Custom_Format
Desc: 将输入图像文件，解码并重新编码成LCD指定的格式，输出到ImageBufAddr为首地址的Buffer中
Param: 无
Return: 解码成功返回TRUE，解码失败则返回FALSE
Global: bmp，BmpFile，ImageWInBuf，ImageHInBuf，ImageBufAddr，BMP_callbacks_decode，outfile
Note:
Author: evan wu
Log:
******************************************************/
INT32S BMP_ThumbNail_Decode_2bitRGB(BMP_CALLBACKS_DECODE BMP_callbacks_decode, BMPOUTFACTOR *BmpOutFactor, BMP *bmp, int BufWidth, int BufHeight)
{
//    IM_PIX_INFO out_info;
//    long        dstx, dsty;
//    targbmodel *dstpixels, *pDstLine;
//    //BMPFACTOR bmpFactor;
//    long  submovedx, submovedy;
	INT32S ret;

#if 0
    submovedx = (BufWidth - bmpFactor.Dst.width) / 2;//x轴的正方向平移量
    submovedy = (BufHeight - bmpFactor.Dst.height) / 2;//y轴的正方向平移量

    //for (dsty = 0;dsty < bmpFactor.Dst.height;dsty++)
    if(BmpOutFactor->dsty >= bmpFactor.Dst.height)
		    return FALSE;
    {
        LCD_RGBDATA RGB = 0;
        INT8U   BufTemp = 0;
        INT8U   PixelInByte = 0;
        BMP_callbacks_decode.read_func(bmpFactor.ReadBuf, 1, sizeof(INT8U)*bmpFactor.ReadBufSize, BmpOutFactor->BmpFile);   //读入一行图像数据
        bmpFactor.x = 0;
#if 0		
        if (bmp->bmih.biHeight < 0)//在循环体外算出该行的首地址
            pDstLine = (targbmodel*)((INT8U*)(bmpFactor.Dst.pdata) + bmpFactor.Dst.byte_width * (dsty + submovedy));        //如果原图是顺序存储，则对应目标图像也顺序存储
        else
            pDstLine = (targbmodel*)((INT8U*)(bmpFactor.Dst.pdata) + bmpFactor.Dst.byte_width * (BufHeight - (dsty + submovedy) - 1)); //如果原图是逆序存储，则对应目标图像也逆序存储
#else
pDstLine = (targbmodel*)((INT8U*)(bmpFactor.Dst.pdata));

#endif
        for (dstx = 0;dstx < bmpFactor.Dst.width;dstx++)                                //解一行的数据，一个循环解一个像素
        {
            INT32S i = bmpFactor.x >> 16;
            PixelInByte = i % 8;
            BufTemp = bmpFactor.ReadBuf[(INT32S)(i*bmpFactor.BytesPerPixs)];                                       //读入一个像素的图像数据，一个像素8bit
            switch (PixelInByte)
            {
                case 0:
                    BufTemp = (BufTemp >> 7) & 0x01;                        //取得第一个像素
                    PixelInByte++;
                    break;
                case 1:
                    BufTemp = (BufTemp >> 6) & 0x01;                        //取得第二个像素
                    PixelInByte++;
                    break;
                case 2:
                    BufTemp = (BufTemp >> 5) & 0x01;                        //取得第三像素
                    PixelInByte++;
                    break;
                case 3:
                    BufTemp = (BufTemp >> 4) & 0x01;                        //取得第四像素
                    PixelInByte++;
                    break;
                case 4:
                    BufTemp = (BufTemp >> 3) & 0x01;                        //取得第五像素
                    PixelInByte++;
                    break;
                case 5:
                    BufTemp = (BufTemp >> 2) & 0x01;                        //取得第六像素
                    PixelInByte++;
                    break;
                case 6:
                    BufTemp = (BufTemp >> 1) & 0x01;                        //取得第七像素
                    PixelInByte++;
                    break;
                case 7:
                    BufTemp &= 0x01;                                        //取得第八个像素
                    PixelInByte = 0;
                    break;
                default:
                    break;
            }
            out_info.Comp_1 = bmp->bmiColors[BufTemp].rgbRed;               //读入R
            out_info.Comp_2 = bmp->bmiColors[BufTemp].rgbGreen;             //读入G
            out_info.Comp_3 = bmp->bmiColors[BufTemp].rgbBlue;              //读入B
            RGB = BMP_callbacks_decode.write_func(out_info);                //转换成LCD所需的编码格式
            dstpixels = (targbmodel*)pDstLine + dstx + submovedx;                     //取得该像素在缓冲区中的位置
            *dstpixels = *((targbmodel*) & RGB);                            //写入缓冲区
            // if(!PixelInByte)                                                //改成加法，提高效率
            bmpFactor.x += bmpFactor.dstxInt;
        }
        bmpFactor.y += bmpFactor.dstyInt;                                                       //改成加法，提高效率
        bmp->FileOffset = bmpFactor.ReadBufSize * ((bmpFactor.y >> 16) - ((bmpFactor.y - bmpFactor.dstyInt) >> 16) - 1);
        BMP_callbacks_decode.seek_func(BmpOutFactor->BmpFile, bmp->FileOffset, SEEK_CUR); //SEEK到下一次要读入的位置
    }
		 
    return TRUE;
#else
        ret = BMP_DecOneRow(BufWidth,BufHeight,BMP_Type_2bitRGB,bmp,&bmpFactor,BmpOutFactor,&BMP_callbacks_decode);
        return ret;
#endif		 


}


/******************************************************
Name: BMP_Convert_4bitRGB_To_Custom_Format
Desc: 将输入图像文件，解码并重新编码成LCD指定的格式，输出到ImageBufAddr为首地址的Buffer中
Param: 无
Return: 解码成功返回TRUE，解码失败则返回FALSE
Global: bmp，BmpFile，ImageWInBuf，ImageHInBuf，ImageBufAddr，BMP_callbacks_decode，outfile
Note:
Author: evan wu
Log:
******************************************************/
INT32S BMP_ThumbNail_Decode_4bitRGB(BMP_CALLBACKS_DECODE BMP_callbacks_decode, BMPOUTFACTOR *BmpOutFactor, BMP *bmp, int BufWidth, int BufHeight)
{
    //IM_PIX_INFO out_info;
    //long  dstx, dsty;
    //targbmodel *dstpixels, *pDstLine;
    //BMPFACTOR bmpFactor;
    //long  submovedx, submovedy;
	INT32S ret;



#if 0
    submovedx = (BufWidth - bmpFactor.Dst.width) / 2;//x轴的正方向平移量
    submovedy = (BufHeight - bmpFactor.Dst.height) / 2;//y轴的正方向平移量
    if(BmpOutFactor->dsty >= bmpFactor.Dst.height)
		    return FALSE;

    //for (dsty = 0;dsty < bmpFactor.Dst.height;dsty++)
    {
        LCD_RGBDATA RGB = 0;
        INT8U BufTemp = 0;
        INT8U PixelInByte = 0;
        BMP_callbacks_decode.read_func(bmpFactor.ReadBuf, 1, sizeof(INT8U)*bmpFactor.ReadBufSize, BmpOutFactor->BmpFile);  //读入一行图像数据
        bmpFactor.x = 0;
#if 0		
        if (bmp->bmih.biHeight < 0)//在循环体外算出该行的首地址
            pDstLine = (targbmodel*)((INT8U*)(bmpFactor.Dst.pdata) + bmpFactor.Dst.byte_width * (dsty + submovedy));        //如果原图是顺序存储，则对应目标图像也顺序存储
        else
            pDstLine = (targbmodel*)((INT8U*)(bmpFactor.Dst.pdata) + bmpFactor.Dst.byte_width * (BufHeight - (dsty + submovedy) - 1)); //如果原图是逆序存储，则对应目标图像也逆序存储
#else
pDstLine = (targbmodel*)((INT8U*)(bmpFactor.Dst.pdata));

#endif
        for (dstx = 0;dstx < bmpFactor.Dst.width;dstx++)      //解一行的数据，一个循环解一个像素
        {
            INT32S i = bmpFactor.x >> 16;
            PixelInByte = i % 2;
            BufTemp = bmpFactor.ReadBuf[(INT32S)(i*bmpFactor.BytesPerPixs)];                                       //读入一个像素的图像数据，一个像素8bit
            switch (PixelInByte)
            {
                case 0:              //高4位为第一个像素
                    BufTemp = (BufTemp >> 4) & 0x0F;
                    PixelInByte = 1;
                    break;
                case 1:              //第四位为第二个像素
                    BufTemp &= 0x0F;
                    PixelInByte = 0;
                    break;
                default:
                    break;
            }
            out_info.Comp_1 = bmp->bmiColors[BufTemp].rgbRed;    //读入R
            out_info.Comp_2 = bmp->bmiColors[BufTemp].rgbGreen;    //读入G
            out_info.Comp_3 = bmp->bmiColors[BufTemp].rgbBlue;    //读入B
            RGB = BMP_callbacks_decode.write_func(out_info);    //转换成LCD所需的编码格式
            dstpixels = (targbmodel*)pDstLine + dstx + submovedx;    //取得该像素在缓冲区中的位置
            *dstpixels = *((targbmodel*) & RGB);       //写入缓冲区
            //if(!PixelInByte)            //改成加法，提高效率
            bmpFactor.x += bmpFactor.dstxInt;
        }
        bmpFactor.y += bmpFactor.dstyInt;             //改成加法，提高效率
        bmp->FileOffset = bmpFactor.ReadBufSize * ((bmpFactor.y >> 16) - ((bmpFactor.y - bmpFactor.dstyInt) >> 16) - 1);
        if(bmp->bmih.biHeight > 0)
           BMP_callbacks_decode.seek_func(BmpOutFactor->BmpFile, -(bmpFactor.y >> 16)*bmpFactor.ReadBufSize, SEEK_END); //SEEK到下一次要读入的位置
        else
        	 BMP_callbacks_decode.seek_func(BmpOutFactor->BmpFile, bmp->FileOffset, SEEK_CUR); //SEEK到下一次要读入的位置        
    }
        return TRUE;
#else
        ret = BMP_DecOneRow(BufWidth,BufHeight,BMP_Type_4bitRGB,bmp,&bmpFactor,BmpOutFactor,&BMP_callbacks_decode);
        return ret;
#endif
}

/******************************************************
Name: BMP_Convert_8bitRGB_To_Custom_Format
Desc: 将输入图像文件，解码并重新编码成LCD指定的格式，输出到ImageBufAddr为首地址的Buffer中
Param: 无
Return: 解码成功返回TRUE，解码失败则返回FALSE
Global: bmp，BmpFile，ImageWInBuf，ImageHInBuf，ImageBufAddr，BMP_callbacks_decode，outfile
Note:
Author: evan wu
Log:
******************************************************/
INT32S BMP_ThumbNail_Decode_8bitRGB(BMP_CALLBACKS_DECODE BMP_callbacks_decode, BMPOUTFACTOR *BmpOutFactor, BMP *bmp, int BufWidth, int BufHeight)
{
//    IM_PIX_INFO out_info;
//    long  dstx, dsty;
//    targbmodel *dstpixels, *pDstLine;
//   // BMPFACTOR bmpFactor;
//    long  submovedx, submovedy;
	INT32S ret;

#if 0
    submovedx = (BufWidth - bmpFactor.Dst.width) / 2;//x轴的正方向平移量
    submovedy = (BufHeight - bmpFactor.Dst.height) / 2;//y轴的正方向平移量

   // for (dsty = 0;dsty < bmpFactor.Dst.height;dsty++)
    if(BmpOutFactor->dsty >= bmpFactor.Dst.height)
		    return FALSE;
    {
        LCD_RGBDATA RGB = 0;
        INT8U BufTemp = 0;
        bmpFactor.x = 0;
#if 0		
        if (bmp->bmih.biHeight < 0)//在循环体外算出该行的首地址
            pDstLine = (targbmodel*)((INT8U*)(bmpFactor.Dst.pdata) + bmpFactor.Dst.byte_width * (dsty + submovedy));    //如果原图是顺序存储，则对应目标图像也顺序存储
        else
            pDstLine = (targbmodel*)((INT8U*)(bmpFactor.Dst.pdata) + bmpFactor.Dst.byte_width * (BufHeight - (dsty + submovedy) - 1)); //如果原图是逆序存储，则对应目标图像也逆序存储
#else
        pDstLine = (targbmodel*)((INT8U*)(bmpFactor.Dst.pdata));
#endif
        for (dstx = 0;dstx < bmpFactor.Dst.width;dstx++)      //解一行的数据，一个循环解一个像素
        {
            int i;
            BMP_callbacks_decode.read_func(&BufTemp, 1, 1, BmpOutFactor->BmpFile);  //读入一行图像数据
            out_info.Comp_1 = bmp->bmiColors[BufTemp].rgbRed;    //读入R
            out_info.Comp_2 = bmp->bmiColors[BufTemp].rgbGreen;    //读入G
            out_info.Comp_3 = bmp->bmiColors[BufTemp].rgbBlue;    //读入B
            RGB = BMP_callbacks_decode.write_func(out_info);    //转换成LCD所需的编码格式
            dstpixels = (targbmodel*)pDstLine + dstx + submovedx;      //取得该像素在缓冲区中的位置
            *dstpixels = *((targbmodel*) & RGB);       //写入缓冲区
            bmpFactor.x += bmpFactor.dstxInt;            //改成加法，提高效率
            i = ((bmpFactor.x >> 16) - ((bmpFactor.x - bmpFactor.dstxInt) >> 16) - 1) * (INT32S)bmpFactor.BytesPerPixs;
            BMP_callbacks_decode.seek_func(BmpOutFactor->BmpFile, i, SEEK_CUR); //SEEK到下一次要读入的位置
        }
        bmpFactor.y += bmpFactor.dstyInt;             //改成加法，提高效率
        bmp->FileOffset = bmpFactor.ReadBufSize * ((bmpFactor.y >> 16) - ((bmpFactor.y - bmpFactor.dstyInt) >> 16) - 1);
        bmp->FileOffset += bmpFactor.ReadBufSize - bmp->bmih.biWidth * bmpFactor.BytesPerPixs;
        BMP_callbacks_decode.seek_func(BmpOutFactor->BmpFile, bmp->FileOffset, SEEK_CUR); //SEEK到下一次要读入的位置
    }
        return TRUE;
#else
ret = BMP_DecOneRow(BufWidth,BufHeight,BMP_Type_8bitRGB,bmp,&bmpFactor,BmpOutFactor,&BMP_callbacks_decode);
    return ret;

#endif


}

/******************************************************
Name: BMP_Convert_16bitRGB_To_Custom_Format
Desc: 将输入图像文件，解码并重新编码成LCD指定的格式，输出到ImageBufAddr为首地址的Buffer中
Param: 无
Return: 解码成功返回TRUE，解码失败则返回FALSE
Global: bmp，BmpFile，ImageWInBuf，ImageHInBuf，ImageBufAddr，BMP_callbacks_decode，outfile
Note:
Author: evan wu
Log:
******************************************************/
INT32S BMP_ThumbNail_Decode_16bitRGB(BMP_CALLBACKS_DECODE BMP_callbacks_decode, BMPOUTFACTOR *BmpOutFactor, BMP *bmp, int BufWidth, int BufHeight)
{
    INT32S ret;
	ret = BMP_DecOneRow(BufWidth,BufHeight,BMP_Type_16bitRGB,bmp,&bmpFactor,BmpOutFactor,&BMP_callbacks_decode);
    return ret;
}

/******************************************************
Name: BMP_Convert_24bitRGB_To_Custom_Format
Desc: 将输入图像文件，解码并重新编码成LCD指定的格式，输出到ImageBufAddr为首地址的Buffer中
Param: 无
Return: 解码成功返回TRUE，解码失败则返回FALSE
Global: bmp，BmpFile，ImageWInBuf，ImageHInBuf，ImageBufAddr，BMP_callbacks_decode，outfile
Note:
Author: evan wu
Log:
******************************************************/
INT32S BMP_ThumbNail_Decode_24bitRGB(BMP_CALLBACKS_DECODE BMP_callbacks_decode, BMPOUTFACTOR *BmpOutFactor, BMP *bmp, int BufWidth, int BufHeight)
{
    INT32S ret;

    ret = BMP_DecOneRow(BufWidth,BufHeight,BMP_Type_24bitRGB,bmp,&bmpFactor,BmpOutFactor,&BMP_callbacks_decode);
     
    return ret;
}

/******************************************************
Name: BMP_Convert_32bitRGB_To_Custom_Format
Desc: 将输入图像文件，解码并重新编码成LCD指定的格式，输出到ImageBufAddr为首地址的Buffer中
Param: 无
Return: 解码成功返回TRUE，解码失败则返回FALSE
Global: bmp，BmpFile，ImageWInBuf，ImageHInBuf，ImageBufAddr，BMP_callbacks_decode，outfile
Note:
Author: evan wu
Log:
******************************************************/
INT32S BMP_ThumbNail_Decode_32bitRGB(BMP_CALLBACKS_DECODE BMP_callbacks_decode, BMPOUTFACTOR *BmpOutFactor, BMP *bmp, int BufWidth, int BufHeight)
{
	INT32S ret;


    ret = BMP_DecOneRow(BufWidth,BufHeight,BMP_Type_32bitRGB,bmp,&bmpFactor,BmpOutFactor,&BMP_callbacks_decode);

    return ret;



}



/******************************************************
Name: BMP_Convert_RLE8_To_Custom_Format
Desc: 将输入图像文件，解码并重新编码成LCD指定的格式，输出到ImageBufAddr为首地址的Buffer中
Param: 无
Return: 解码成功返回TRUE，解码失败则返回FALSE
Global: bmp，BmpFile，ImageWInBuf，ImageHInBuf，ImageBufAddr，BMP_callbacks_decode，outfile
Note:
Author: evan wu
Log:
******************************************************/
#if 0
INT32S BMP_ThumbNail_Decode_RLE8(BMP_CALLBACKS_DECODE BMP_callbacks_decode, BMPOUTFACTOR *BmpOutFactor, BMP *bmp, int BufWidth, int BufHeight)
{
    IM_PIX_INFO out_info;
    INT8U  byte, byte1, byte2, deltax, deltay;
    INT32U  submovedx, submovedy;
    targbmodel *dstpixels, *pDstLine;
    BMPFACTOR  bmpFactor;
    INT8U  *BufAddTemp;
    INT32U  k, temp;

	return FALSE;
	#if 0
    memset(&bmpFactor, 0, sizeof(BMPFACTOR));
    if (!BMP_ReadTab(BMP_callbacks_decode, BmpOutFactor, bmp))
    {
        BMP_free(bmp);
        return FALSE;
    }
    if (!BMP_ThumbNail_Init(&bmpFactor, BmpOutFactor, bmp, BufWidth, BufHeight))
    {
        BMP_free(bmp);
        return FALSE;
    }

    bmpFactor.dstxInt = (bmpFactor.Dst.width << 16) / bmp->bmih.biWidth + 1;
    bmpFactor.dstyInt = (bmpFactor.Dst.height << 16) / bmpFactor.height + 1;
    bmpFactor.ReadBufSize = bmp->bmfh.bfSize - bmp->bmfh.bfOffBits;
    bmpFactor.ReadBuf = gBMPReadBuf;//(INT8U *)MALLOC(bmpFactor.ReadBufSize * sizeof(INT8U)); //开一个能够存储一行输出图像数据的Buffer

    memset(bmpFactor.ReadBuf, 0, bmpFactor.ReadBufSize);
    BMP_callbacks_decode.seek_func(BmpOutFactor->BmpFile, bmp->bmfh.bfOffBits, SEEK_SET);
    BMP_callbacks_decode.read_func(bmpFactor.ReadBuf, sizeof(INT8U), bmpFactor.ReadBufSize, BmpOutFactor->BmpFile);//读入压缩的图像数据，先按每个像素8位的读一行
    bmpFactor.x = 0;
    bmpFactor.y = 0;
    submovedx = (BufWidth - bmpFactor.Dst.width) / 2;//x轴的正方向平移量
    submovedy = (BufHeight - bmpFactor.Dst.height) / 2;//y轴的正方向平移量
    BufAddTemp = bmpFactor.ReadBuf;
    temp = ((bmpFactor.y * bmpFactor.dstyInt) >> 16) + submovedy;
    if (bmp->bmih.biHeight > 0)
        temp = (BufHeight - temp - 1);
    pDstLine = (INT8U*)bmpFactor.Dst.pdata + bmpFactor.Dst.byte_width * temp;
    while ((INT32U)(BufAddTemp - bmpFactor.ReadBuf) < bmpFactor.ReadBufSize)     //解码开始,将图像数据读入到BmpOutBuf，解成8bitRGB存放在ReadBuf，
    {                //转成RGB565存放在ImageBufAddr
        byte1 = *(BufAddTemp);          //读入两个byte
        BufAddTemp++;
        byte2 = *(BufAddTemp);
        BufAddTemp++;
        // Absolute Mode
        if (byte1 == 0 && byte2 >= 0x03 && byte2 <= 0xFF)    //如果byte1是0,byte2大于等于0x03，小于等于0xFF。则为绝对编码
        {
            for (k = 0; k < byte2; k++)         //将byte2后的byte2个字节的数据复制到ReadBuf
            {
                byte = *(BufAddTemp);
                BufAddTemp++;
                temp = ((bmpFactor.x * bmpFactor.dstxInt) >> 16) + submovedx;
                memcpy((INT8U*)&pDstLine[temp], (INT8U*)&bmp->bmiColors[byte], 4);
                bmpFactor.x++;
                if (bmpFactor.x > bmp->bmih.biWidth)
                    goto EndOfLine;
            }
            // Each run must be aligned on a word boundary
            if ((byte2 % 2) != 0)          //16bit对齐
                BufAddTemp++;
        }
        // Encoded Mode
        else               //如果byte1不是0,或者byte2小于0x03，则为编码模式
        {
            // Indicate an escape
            if (byte1 == 0)            //
            {
                switch (byte2)
                {
                        // End of line
                    case 0:            //byte1为0,byte2为0,为一行结束符
                        {
EndOfLine:
                            bmpFactor.x = 0;
                            bmpFactor.y++;
                            temp = ((bmpFactor.y * bmpFactor.dstyInt) >> 16) + submovedy;
                            if ((bmpFactor.y > bmpFactor.height) || ((temp - submovedy) >= bmpFactor.Dst.height))
                                goto EndOfBitmap;
                            if (bmp->bmih.biHeight > 0)
                                temp = (BufHeight - temp - 1);
                            pDstLine = (INT8U*)bmpFactor.Dst.pdata + bmpFactor.Dst.byte_width * temp;
                        }
                        break;
                        // End of bitmap
                    case 1:            //byte1为0,byte2为1,为图像文件结束符
EndOfBitmap:

                        return TRUE;
                        // Delta
                    case 2:            //byte1为0,byte2为2,为跳行符
                        {
                            deltax = *(BufAddTemp);
                            BufAddTemp++;
                            deltay = *(BufAddTemp);
                            BufAddTemp++;
                            bmpFactor.x += deltax;
                            bmpFactor.y += deltay;
                            if (bmpFactor.y > bmpFactor.height)
                                goto EndOfBitmap;
                            if (bmpFactor.x > bmp->bmih.biWidth)
                                goto EndOfLine;
                            temp = ((bmpFactor.y * bmpFactor.dstyInt) >> 16) + submovedy;
                            if (bmp->bmih.biHeight > 0)
                                temp = (BufHeight - temp - 1);
                            pDstLine = (INT8U*)bmpFactor.Dst.pdata + bmpFactor.Dst.byte_width * temp;
                        }
                    default:
                        break;
                }
            }
            else
            {
                for (k = 0; k < byte1; k++)
                {
                    temp = ((bmpFactor.x * bmpFactor.dstxInt) >> 16) + submovedx;
                    memcpy((INT8U*)&pDstLine[temp], (INT8U*)&bmp->bmiColors[byte2], 4);
                    bmpFactor.x++;
                    if (bmpFactor.x > bmp->bmih.biWidth)
                        goto EndOfLine;
                }
            }
        }
    }
    goto EndOfBitmap;
	#endif
}

/******************************************************
Name: BMP_Convert_RLE4_To_Custom_Format
Desc: 将输入图像文件，解码并重新编码成LCD指定的格式，输出到ImageBufAddr为首地址的Buffer中
Param: 无
Return: 解码成功返回TRUE，解码失败则返回FALSE
Global: bmp，BmpFile，ImageWInBuf，ImageHInBuf，ImageBufAddr，BMP_callbacks_decode，outfile
Note:
Author: evan wu
Log:
******************************************************/
INT32S BMP_ThumbNail_Decode_RLE4(BMP_CALLBACKS_DECODE BMP_callbacks_decode, BMPOUTFACTOR *BmpOutFactor, BMP *bmp, int BufWidth, int BufHeight)
{
    IM_PIX_INFO out_info;
    INT8U  byte, byte1, byte2, deltax, deltay;
    INT32U  submovedx, submovedy;
    targbmodel *dstpixels, *pDstLine;
    //BMPFACTOR bmpFactor;
    INT8U  *BufAddTemp;
    INT32U  k, temp;

	return FALSE;
	#if 0

    BMP_callbacks_decode.seek_func(BmpOutFactor->BmpFile, bmp->bmfh.bfOffBits, SEEK_SET);
    BMP_callbacks_decode.read_func(bmpFactor.ReadBuf, sizeof(INT8U), bmpFactor.ReadBufSize, BmpOutFactor->BmpFile);//读入压缩的图像数据，先按每个像素8位的读一行
    bmpFactor.x = 0;
    bmpFactor.y = 0;
    submovedx = (BufWidth - bmpFactor.Dst.width) / 2;//x轴的正方向平移量
    submovedy = (BufHeight - bmpFactor.Dst.height) / 2;//y轴的正方向平移量
    BufAddTemp = bmpFactor.ReadBuf;
    temp = ((bmpFactor.y * bmpFactor.dstyInt) >> 16) + submovedy;
    if (bmp->bmih.biHeight > 0)
        temp = (BufHeight - temp - 1);
    //pDstLine = (INT8U*)bmpFactor.Dst.pdata + bmpFactor.Dst.byte_width * temp;
    pDstLine = (INT8U*)bmpFactor.Dst.pdata;
    //while ((INT32U)(BufAddTemp - bmpFactor.ReadBuf) < bmpFactor.ReadBufSize)     //解码开始,将图像数据读入到BmpOutBuf，解成8bitRGB存放在ReadBuf，
    while(1)
    {    
		if((INT32U)(BufAddTemp - bmpFactor.ReadBuf) >= bmpFactor.ReadBufSize)
		{
			goto EndOfBitmap;
		}
		
        byte1 = *BufAddTemp++;          //读入两个byte

        byte2 = *BufAddTemp++;

        // Absolute Mode
        if (byte1 == 0 && byte2 >= 0x03 && byte2 <= 0xFF)    //如果byte1是0,byte2大于等于0x03，小于等于0xFF。则为绝对编码
        {
            for (k = 0; k < byte2; k++)         //将byte2后的byte2个字节的数据复制到ReadBuf
            {
                temp = ((bmpFactor.x * bmpFactor.dstxInt) >> 16) + submovedx;
                if (!(k % 2))
                {
                    byte = *BufAddTemp++;
                    memcpy((INT8U*)&pDstLine[temp], (INT8U*)&bmp->bmiColors[(byte&0xf0)>>4], 4);
                }
                else
                    memcpy((INT8U*)&pDstLine[temp], (INT8U*)&bmp->bmiColors[byte&0x0f], 4);
                bmpFactor.x++;
                if (bmpFactor.x > bmp->bmih.biWidth)
                    goto EndOfLine;
            }
            // Each run must be aligned on a word boundary
            if ((byte2 % 4) != 0)          //16bit对齐
                BufAddTemp++;
        }
        // Encoded Mode
        else               //如果byte1不是0,或者byte2小于0x03，则为编码模式
        {
            // Indicate an escape
            if (byte1 == 0)            //
            {
                switch (byte2)
                {
                        // End of line
                    case 0:            //byte1为0,byte2为0,为一行结束符
                        {
EndOfLine:
                            bmpFactor.x = 0;
                            bmpFactor.y++;
                            temp = ((bmpFactor.y * bmpFactor.dstyInt) >> 16) + submovedy;
                            if ((bmpFactor.y > bmpFactor.height) || ((temp - submovedy) >= bmpFactor.Dst.height))
                                goto EndOfBitmap;
                            if (bmp->bmih.biHeight > 0)
                                temp = (BufHeight - temp - 1);
                            pDstLine = (INT8U*)bmpFactor.Dst.pdata + bmpFactor.Dst.byte_width * temp;
                        }
                        break;
                        // End of bitmap
                    case 1:            //byte1为0,byte2为1,为图像文件结束符
EndOfBitmap:

                        return TRUE;
                        // Delta
                    case 2:            //byte1为0,byte2为2,为跳行符
                        {
                            deltax = *(BufAddTemp);
                            BufAddTemp++;
                            deltay = *(BufAddTemp);
                            BufAddTemp++;
                            bmpFactor.x += deltax;
                            bmpFactor.y += deltay;
                            if (bmpFactor.y > bmpFactor.height)
                                goto EndOfBitmap;
                            if (bmpFactor.x > bmp->bmih.biWidth)
                                goto EndOfLine;
                            temp = ((bmpFactor.y * bmpFactor.dstyInt) >> 16) + submovedy;
                            if (bmp->bmih.biHeight > 0)
                                temp = (BufHeight - temp - 1);
                            pDstLine = (targbmodel*)((INT8U*)bmpFactor.Dst.pdata + bmpFactor.Dst.byte_width * temp);
                        }
                    default:
                        break;
                }
            }
            else
            {
                for (k = 0; k < byte1; k++)
                {
                    temp = ((bmpFactor.x * bmpFactor.dstxInt) >> 16) + submovedx;
                    if (!(k % 2))
                        memcpy((INT8U*)&pDstLine[temp], (INT8U*)&bmp->bmiColors[(byte2&0xf0)>>4], 4);
                    else
                        memcpy((INT8U*)&pDstLine[temp], (INT8U*)&bmp->bmiColors[(byte2&0x0f)], 4);
                    bmpFactor.x++;
                    if (bmpFactor.x > bmp->bmih.biWidth)
                        goto EndOfLine;
                }
            }
        }
    }
    goto EndOfBitmap;
	#endif
}
#endif

/******************************************************
Name: BMP_Convert_16bitRGB565_To_Custom_Format
Desc: 将输入图像文件，解码并重新编码成LCD指定的格式，输出到ImageBufAddr为首地址的Buffer中
Param: 无
Return: 解码成功返回TRUE，解码失败则返回FALSE
Global: bmp，BmpFile，ImageWInBuf，ImageHInBuf，ImageBufAddr，BMP_callbacks_decode，outfile
Note:
Author: evan wu
Log:
******************************************************/
INT32S BMP_ThumbNail_Decode_16bitRGB565(BMP_CALLBACKS_DECODE BMP_callbacks_decode, BMPOUTFACTOR *BmpOutFactor, BMP *bmp, int BufWidth, int BufHeight)
{
//    IM_PIX_INFO out_info;
	INT32S ret;
	ret = BMP_DecOneRow(BufWidth,BufHeight,BMP_Type_16bitRGB565,bmp,&bmpFactor,BmpOutFactor,&BMP_callbacks_decode);
    return ret;
}


/******************************************************
Name: BMP_Convert_32bitRGB888_To_Custom_Format
Desc: 将输入图像文件，解码并重新编码成LCD指定的格式，输出到ImageBufAddr为首地址的Buffer中
Param: 无
Return: 解码成功返回TRUE，解码失败则返回FALSE
Global: bmp，BmpFile，ImageWInBuf，ImageHInBuf，ImageBufAddr，BMP_callbacks_decode，outfile
Note:
Author: evan wu
Log:
******************************************************/
INT32S BMP_ThumbNail_Decode_32bitRGB888(BMP_CALLBACKS_DECODE BMP_callbacks_decode, BMPOUTFACTOR *BmpOutFactor, BMP *bmp, int BufWidth, int BufHeight)
{
    //IM_PIX_INFO out_info;
    //INT8U  byte1, byte2, byte3, byte4;
    //long  dstx, dsty;
    //targbmodel *dstpixels, *pDstLine;
    //BMPFACTOR bmpFactor;
    //long  submovedx, submovedy;
	INT32S ret;

#if 0	
    submovedx = (BufWidth - bmpFactor.Dst.width) / 2;//x轴的正方向平移量
    submovedy = (BufHeight - bmpFactor.Dst.height) / 2;//y轴的正方向平移量

    for (dsty = 0;dsty < bmpFactor.Dst.height;dsty++)
    {
        LCD_RGBDATA RGB = 0;
        bmpFactor.x = 0;
        if (bmp->bmih.biHeight < 0)//在循环体外算出该行的首地址
            pDstLine = (targbmodel*)((INT8U*)(bmpFactor.Dst.pdata) + bmpFactor.Dst.byte_width * (dsty + submovedy));    //如果原图是顺序存储，则对应目标图像也顺序存储
        else
            pDstLine = (targbmodel*)((INT8U*)(bmpFactor.Dst.pdata) + bmpFactor.Dst.byte_width * (BufHeight - (dsty + submovedy) - 1)); //如果原图是逆序存储，则对应目标图像也逆序存储
        for (dstx = 0;dstx < bmpFactor.Dst.width;dstx++)      //解一行的数据，一个循环解一个像素
        {                //这里一个像素是3byte
            int i;
            BMP_callbacks_decode.read_func(bmpFactor.ReadBuf, 1, 4, BmpOutFactor->BmpFile);  //读入一行图像数据
            out_info.Comp_1 = bmpFactor.ReadBuf[3];          //写入R
            out_info.Comp_2 = bmpFactor.ReadBuf[2];          //写入G
            out_info.Comp_3 = bmpFactor.ReadBuf[1];          //写入B
            RGB = BMP_callbacks_decode.write_func(out_info);    //转换成LCD所需的编码格式
            dstpixels = (targbmodel*)pDstLine + dstx + submovedx;      //取得该像素在缓冲区中的位置
            *dstpixels = *((targbmodel*) & RGB);       //写入缓冲区
            bmpFactor.x += bmpFactor.dstxInt;            //改成加法，提高效率
            i = ((bmpFactor.x >> 16) - ((bmpFactor.x - bmpFactor.dstxInt) >> 16) - 1) * (INT32S)bmpFactor.BytesPerPixs;
            BMP_callbacks_decode.seek_func(BmpOutFactor->BmpFile, i, SEEK_CUR); //SEEK到下一次要读入的位置
        }
        bmpFactor.y += bmpFactor.dstyInt;             //改成加法，提高效率
        bmp->FileOffset = bmpFactor.ReadBufSize * ((bmpFactor.y >> 16) - ((bmpFactor.y - bmpFactor.dstyInt) >> 16) - 1);
        bmp->FileOffset += bmpFactor.ReadBufSize - bmp->bmih.biWidth * bmpFactor.BytesPerPixs;
        BMP_callbacks_decode.seek_func(BmpOutFactor->BmpFile, bmp->FileOffset, SEEK_CUR); //SEEK到下一次要读入的位置
    }
    return TRUE;
#else
	ret = BMP_DecOneRow(BufWidth,BufHeight,BMP_Type_32bitRGB888,bmp,&bmpFactor,BmpOutFactor,&BMP_callbacks_decode);
	    return ret;
	     
#endif

}
/******************************************************
Name: BMP_Decode
Desc: 将输入图像文件，解码并重新编码成LCD指定的格式，输出到ImageBufAddr为首地址的Buffer中
Param: 无
Return: 输出图像数据大小，解码失败则返回FALSE
Global: bmp
Note:
Author: evan wu
Log:
******************************************************/
INT32S BMP_ThumbNail_Decode(BMP_CALLBACKS_DECODE BMP_callbacks_decode, BMPOUTFACTOR *BmpOutFactor, BMP *bmp, int BufWidth, int BufHeight)
{
    INT32S ret;
    switch (bmp->bmih.biCompression)
    {
            // RGB
        case BI_RGB:
            {
                switch (bmp->bmih.biBitCount)
                {
                        // 2 colors
                    case 1:
                        ret = BMP_ThumbNail_Decode_2bitRGB(BMP_callbacks_decode, BmpOutFactor, bmp, BufWidth, BufHeight);
                        break;
                        // 16 colors
                    case 4:
                        ret = BMP_ThumbNail_Decode_4bitRGB(BMP_callbacks_decode, BmpOutFactor, bmp, BufWidth, BufHeight);
                        break;
                        // 256 colors
                    case 8:
                        ret = BMP_ThumbNail_Decode_8bitRGB(BMP_callbacks_decode, BmpOutFactor, bmp, BufWidth, BufHeight);
                        break;
                        // 65536 colors RGB555
                    case 16:
                        ret = BMP_ThumbNail_Decode_16bitRGB(BMP_callbacks_decode, BmpOutFactor, bmp, BufWidth, BufHeight);
                        break;
                        // 16.7M colors
                    case 24:
                        ret = BMP_ThumbNail_Decode_24bitRGB(BMP_callbacks_decode, BmpOutFactor, bmp, BufWidth, BufHeight);
                        break;
                    case 32:
                        ret = BMP_ThumbNail_Decode_32bitRGB(BMP_callbacks_decode, BmpOutFactor, bmp, BufWidth, BufHeight);
                        break;
                    default:
                        ret = FALSE;
                }
            }
            break;
#if 0
            // RLE8
        case BI_RLE8:
            ret = BMP_ThumbNail_Decode_RLE8(BMP_callbacks_decode, BmpOutFactor, bmp, BufWidth, BufHeight);
            break;
            // RLE4
        case BI_RLE4:
            ret = BMP_ThumbNail_Decode_RLE4(BMP_callbacks_decode, BmpOutFactor, bmp, BufWidth, BufHeight);
            break;
#endif
        case BI_BITFIELDS:
            {
                switch (bmp->bmih.biBitCount)
                {
                        // 65536 colors RGB565
                    case 16:
                        ret =  BMP_ThumbNail_Decode_16bitRGB565(BMP_callbacks_decode, BmpOutFactor, bmp, BufWidth, BufHeight);
                        break;
                    case 32:
                        ret = BMP_ThumbNail_Decode_32bitRGB888(BMP_callbacks_decode, BmpOutFactor, bmp, BufWidth, BufHeight);
                        break;
                }
            }
            break;
        default:     // Should not happen,just in case
            ret = FALSE;
    }
    return ret;
}

/******************************************************
Name: BMP_InitHead
Desc: 将输入图像文件，解码并重新编码成LCD指定的格式，输出到ImageBufAddr为首地址的Buffer中
Param: 无
Return: 解文件头成功返回TRUE，解头失败则返回FALSE
Global: bmp
Note:
Author: evan wu
Log:
******************************************************/
INT8U BMP_InitHead(BMP_CALLBACKS_DECODE BMP_callbacks_decode,BMP *bmp,BMPOUTFACTOR *BmpOutFactor,int BufWidth, int BufHeight)
{
	int isHaveTab = 0;
    memset(bmp, 0, (sizeof(BMP)));
    BMP_callbacks_decode.read_func(bmp, 1, (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)), (void *) BmpOutFactor->BmpFile);
    if ((bmp->bmfh.bfType != BMP_TYPE) || (bmp->bmih.biHeight > 0xffff) || (bmp->bmih.biHeight > 0xffff))
        return FALSE;

  //判断调色板是否存在
	if(BI_RGB == bmp->bmih.biCompression)
	{
		if(1 == bmp->bmih.biBitCount||4 == bmp->bmih.biBitCount||8 == bmp->bmih.biBitCount)
		{
            isHaveTab = 1;
		}
	}
	else if(BI_RLE8 == bmp->bmih.biCompression || BI_RLE4 == bmp->bmih.biCompression)
	{
		isHaveTab = 1;
		return FALSE;
	}
	
    if(isHaveTab)
    {
	    if (!BMP_ReadTab(BMP_callbacks_decode, BmpOutFactor, bmp))
	    {
	        return FALSE;
	    }	
    }
#if 0    
    memset(&bmpFactor, 0, sizeof(BMPFACTOR));
    if (!BMP_ThumbNail_Init(&bmpFactor, BmpOutFactor, bmp, BufWidth, BufHeight))
    {
        return FALSE;
    }
    
    if (bmp->bmih.biHeight > 0)
       //BMP_callbacks_decode.seek_func(BmpOutFactor->BmpFile, -(long)((BmpOutFactor->dsty + 1)*(bmpFactor.ReadBufSize)), SEEK_END);
	   BMP_callbacks_decode.seek_func(BmpOutFactor->BmpFile, bmp->bmfh.bfOffBits + (bmpFactor.height - 1)*(bmpFactor.ReadBufSize), SEEK_SET);
    else
    	BMP_callbacks_decode.seek_func(BmpOutFactor->BmpFile, bmp->bmfh.bfOffBits, SEEK_SET);
#endif	  
	  return TRUE;	
}
INT8U BMP_calc_output(BMP_CALLBACKS_DECODE BMP_callbacks_decode,BMP *bmp,BMPOUTFACTOR *BmpOutFactor,int BufWidth, int BufHeight)
{
    memset(&bmpFactor, 0, sizeof(BMPFACTOR));
    if (!BMP_ThumbNail_Init(&bmpFactor, BmpOutFactor, bmp, BufWidth, BufHeight))
    {
        return FALSE;
    }
    
    if (bmp->bmih.biHeight > 0)
       //BMP_callbacks_decode.seek_func(BmpOutFactor->BmpFile, -(long)((BmpOutFactor->dsty + 1)*(bmpFactor.ReadBufSize)), SEEK_END);
	   BMP_callbacks_decode.seek_func(BmpOutFactor->BmpFile, bmp->bmfh.bfOffBits + (bmpFactor.height - 1)*(bmpFactor.ReadBufSize), SEEK_SET);
    else
    	BMP_callbacks_decode.seek_func(BmpOutFactor->BmpFile, bmp->bmfh.bfOffBits, SEEK_SET);
	return TRUE;
}
#pragma arm section code
#endif

