#include "SysInclude.h"
#include "audio_main.h"

#ifdef A_CORE_DECODE
#ifdef HIFI_AlAC_DECODE
#include <stdio.h>
#include "audio_file_access.h"
#pragma arm section code = "AlacHDecCode", rodata = "AlacHDecCode", rwdata = "AlacHDecData", zidata = "AlacHDecBss"
#include "audio_globals.h"
#include "Hifi_alac_dec.h"
#include "alac.h"
#include "hifi_alac_MovFile.h"
#include "hifi.h"
#include "Hw_hifi.h"
#include "../hifi_get_bits.h"
#include "codec.h"
extern FILE *alac_file_handle;
//FILE *alac_outfile;
//FILE *TXTFILE1,*TXTFILE2,*TXTFILE3,*TXTFILE;
//FILE *update_file,*update_file1;


unsigned char ALAC_inbuf[in_buf_size + 8];
//unsigned char ALAC_outbuf[out_buf_size];

ALACContext alac_con;
AVPacket alac_avpkt;


extern int MDATA_offset ;
int Alac_decode_init(FILE* alac_raw_file)
{
	int err;
    #ifdef HIFI_ACC
    Hifi_Set_ACC_XFER_Disable(0,0,HIfi_ACC_TYPE_ALAC);//开始传输配置数据和初始化系数(不往fifo送)
    Hifi_Set_ACC_clear(0);//fpga 内部已实现
    Hifi_Set_ACC_Dmacr(0);
    Hifi_Set_ACC_Intcr(0);
    #endif 
	if (MovFileInit_h(alac_raw_file))
	{
		MovFileClose_h();
		return -100;
	}

	if ((err = MovFileParsing_h(&gMovFile_h.movAudBuf)) != 0)
	{
		MovFileClose_h();

		return err;
	}
   // MovSTseek(&gMovFile_h.movAudBuf,MDATA_offset , SEEK_SET); 
     RKFIO_FSeek(MDATA_offset,SEEK_SET,alac_file_handle);
    return 0;
	
}
int Alac_header_parse()
{
	alac_avpkt.data = ALAC_inbuf;
	alac_avpkt.size =in_buf_size;// fread(inbuf, 1, in_buf_size, f);

	alac_decode_init(&alac_con);
	//MovSTread(alac_avpkt.data, 1, alac_avpkt.size , &gMovFile_h.movAudBuf);		
    RKFIO_FRead(alac_avpkt.data,alac_avpkt.size ,alac_file_handle);


}
int Alac_frame_decode(unsigned char*ALAC_outbuf ,int *out_size)
{
	int bytes_read;
	
	int i;
	    gMovFile_h.curAudioSampleNo +=1 ;
		bytes_read = alac_decode_frame(&alac_con,ALAC_outbuf,&alac_avpkt);
		if (bytes_read < 0) 
		{
			return -1 ;
		}
		*out_size = alac_con.nb_samples*4;//16bit *4  24bit *6
       // fwrite(ALAC_outbuf,1, *out_size, outfile);
		alac_avpkt.size -= bytes_read;
		alac_avpkt.data += bytes_read;
		if (alac_avpkt.size < in_buf_size) 
		{

			if (alac_avpkt.size<0)
			{
				alac_avpkt.size = 0;
			}
			MemMov2(ALAC_inbuf, alac_avpkt.data, alac_avpkt.size);
			alac_avpkt.data = ALAC_inbuf;

			//bytes_read = MovSTread(alac_avpkt.data + alac_avpkt.size, 1,in_buf_size - alac_avpkt.size, &gMovFile_h.movAudBuf);
            
            bytes_read = RKFIO_FRead(alac_avpkt.data + alac_avpkt.size,in_buf_size - alac_avpkt.size,alac_file_handle);
			if (bytes_read > 0)
			{
				alac_avpkt.size += bytes_read;
			}
            if(alac_avpkt.size <= 0)
            {
                return 0;
            }
		}	
        return alac_con.nb_samples;

}



/*****************************************************
buffer_in    熵解码后的残差信号
buffer_out   经过线性预测后的信号
lpc系数用倒序,这样数据源就不需倒序乘
*********************************************************/
void  Alac_Decode_Lpc( int32_t *buffer_in,int32_t *buffer_out,
				 int nb_samples, int bps, int32_t *lpc_coefs,int16_t *adapt_coefs,
				 int lpc_order, int lpc_quant,int16_t *delay,int* extra,int version) //int extra for ape = f->avg
{	

//if          adapt_coefs != NULL    ---->APE (ape的delay 可以使用buffer_in，但是注意数据类型)
//else if     buffer_in   == NULL    ---->FLAC
//else                               ---->ALAC

	int i,j;
 
	  for (i=lpc_order+1; i < nb_samples; i++) //前lpc_order+1个输出数据已确定
	  {
		  int val = 0;
		  int error_val = buffer_in[i];
		  int error_sign = sign_only(error_val);// 判断输入数据的正负。正为1；0为0；负为-1;
		  int d = buffer_in[i-lpc_order-1];
		  int temp[32]; 
		  /* LPC prediction */
		 
		  for (j = 0; j < lpc_order; j++)
		  {
			  temp[j] = (buffer_in[i-lpc_order+j] - buffer_in[i-lpc_order-1]);
			  val +=  temp[j]* lpc_coefs[j];           // FIR滤波 = MAC 

			  /* adapt LPC coefficients */
			  if (error_sign && temp[j])                   //输入数据非0时更新LPC系数
			  {
				  if((error_val * error_sign) > 0)//
				  {
					  int sign;
					  int x;
					  x  = -temp[j];
					  sign = sign_only(x) * error_sign;//1或-1
					  lpc_coefs[j] -= sign;             //更新系数 +1或-1
					  x *= sign;
					  error_val -= (x >> lpc_quant) * (j + 1);
				  }
			  }
		  }

		  val = (val + (1 << (lpc_quant - 1))) >> lpc_quant; //滤波结果移位+0.5的误差
		  val += buffer_in[i-lpc_order-1] + buffer_in[i];
		  buffer_in[i] = sign_extend(val, bps);             //只要有效位的数据
		  buffer_out[i] =buffer_in[i];

	  }


	 }
 int MovAudioSeekToFILE_h(MOV_ST *videoFile)
{

   if (MovSTtell(videoFile) != gMovFile_h.audioSample.sampleOffset + gMovFile_h.audioSample.readSize)
    {
        MovSTseek(videoFile,
                 (long)(gMovFile_h.audioSample.sampleOffset + gMovFile_h.audioSample.readSize) - MovSTtell(videoFile), SEEK_CUR);
    }
    alac_avpkt.data = ALAC_inbuf;
	alac_avpkt.size =in_buf_size;
	//MovSTread(alac_avpkt.data, 1, alac_avpkt.size , &gMovFile_h.movAudBuf);	
	RKFIO_FRead(alac_avpkt.data,alac_avpkt.size ,alac_file_handle);
    return 0;
}
    int ALAC_get_more_data(GetBitContext *gb,int bits_needed)
    {   
        int ret;
        int index ;
        int left_byte ;
        int bits_left = get_bits_left(gb);
        if(bits_left < 0)
        {
           Hifi_Alac_Printf("bits_left 负%d\n",bits_left);
           bits_left = 0;
        }
        if(bits_needed > bits_left )
        {
            index = gb->index >>3;
            left_byte = (bits_left+7)>>3;					
            MemMov2(gb->buffer, &gb->buffer[index], left_byte);
            //ret = MovSTread(&gb->buffer[left_byte], 1, in_buf_size-left_byte, &gMovFile_h.movAudBuf);  
            ret = RKFIO_FRead(&gb->buffer[left_byte], in_buf_size-left_byte, alac_file_handle);             
            gb->index  = gb->index%8;
             if(ret == 0)
            {
              return 0;
            }
         
        }
    }
#pragma arm section code
#endif
#endif