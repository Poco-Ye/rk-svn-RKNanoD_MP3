#include "SysInclude.h"
#include "audio_main.h"
#include "audio_file_access.h"
#include "ape.h"

#ifdef A_CORE_DECODE
#ifdef HIFI_APE_DECODE
#pragma arm section code = "ApeHDecCode", rodata = "ApeHDecCode", rwdata = "ApeHDecData", zidata = "ApeHDecBss"

extern FILE *ape_file_handle;

u8 get_ape_bits(ByteIOContext *pb,u8 len)
{
	u8 res_data,new_data;

	res_data = 0;
	pb->size_in_bits -= len;

	if(pb->bit_left < len)
	{
		res_data = pb->buffer[pb->bye_index];
		res_data &= ((1<<pb->bit_left)-1);
		len -= pb->bit_left;
		pb->bye_index++;
		pb->bit_left = 8;
		res_data = (res_data << len);

		new_data = pb->buffer[pb->bye_index];
		new_data &= ((1<<pb->bit_left)-1);
		pb->bit_left -= len;
		new_data = (new_data >> pb->bit_left);
		res_data += new_data;
	}
	else
	{
		res_data = pb->buffer[pb->bye_index];
		res_data &= ((1<<pb->bit_left)-1);
		pb->bit_left -= len;
		res_data = (res_data >> pb->bit_left);
	}
	
	return res_data;
}

void fill_buf(ByteIOContext *pb)
{
	RKFIO_FRead(pb->buffer, 512,ape_file_handle);
	pb->bye_index = 0;
	pb->bit_left = 8;
	pb->size_in_bits = 512*8;
}

u8 get_bitbye(ByteIOContext *pb,u8 len)
{
	u8 res_data;
	
	res_data = 0;
	if(pb->size_in_bits < len)
	{
		res_data = get_ape_bits(pb,pb->size_in_bits);
		len -= pb->size_in_bits;
		fill_buf(pb);
		res_data = (res_data<<len);
		res_data += get_ape_bits(pb,len);
	}
	else
	{
		res_data = get_ape_bits(pb,len);
	}

	return res_data;	
}

u16 get_bitshort(ByteIOContext *pb,u8 len)
{
	u16 res_data;

	res_data = 0;
	while(len >= 8)
	{
		res_data = (res_data << 8);
		res_data += get_bitbye(pb,8);
		len -= 8;		
	}
	res_data = (res_data << len);
	res_data += get_bitbye(pb,len);	

	return res_data;
}

u32 get_bitlong(ByteIOContext *pb,u8 len)
{
	u32 res_data;

	res_data = 0;
	while(len >= 8)
	{
		res_data = (res_data << 8);
		res_data += get_bitbye(pb,8);
		len -= 8;
	}
	res_data = (res_data << len);
	res_data += get_bitbye(pb,len);

	return res_data;
}

uint32_t get_le16(ByteIOContext *pb)
{
	u16 res_data,b_s_data;

	res_data = get_bitshort(pb,16);
	
	b_s_data = ((res_data>>8)&0xff)|(((res_data)&0xff)<<8);
	
	res_data = b_s_data;
	return res_data;
}

uint32_t get_le32(ByteIOContext *pb)
{
	u32 res_data,b_s_data;

	res_data = get_bitlong(pb,32);
	
	b_s_data = ((res_data>>24)&0xff)|(((res_data>>16)&0xff)<<8)|(((res_data>>8)&0xff)<<16)|(((res_data)&0xff)<<24);
	
	res_data = b_s_data;
	return res_data;
}




void url_fskip(ByteIOContext *pb,u16 len)
{
	while(len)
	{
		get_bitbye(pb,8);
		len -= 1;
	}
}

void url_fseek(ByteIOContext *pb,u16 len,u8 type)
{
	if(type == SEEK_CUR)
	{
		while(len)
		{
			get_bitbye(pb,8);
			len -= 1;
		}
	}
}

void get_buffer(ByteIOContext *pb,u8 *buf,u8 len)
{
	u8 *ptr = buf;
	while(len--)
	{
		*ptr++ = get_bitbye(pb,8);
	}
}

void freebuf(ByteIOContext *pb)
{
	pb->size_in_bits = 0;
	pb->size_in_bits = 0;
}

u32 fread32(ByteCache *datas)
{
	u32 res_data;
	if(datas->cacheindex == 0)
	{
		RKFIO_FRead((uint8*)&datas->cachedata,4,ape_file_handle);
		res_data = datas->cachedata;
	}
	else
	{
		res_data = (datas->cachedata << (32 - datas->cacheindex));
		RKFIO_FRead((uint8*)&datas->cachedata,4,ape_file_handle);
		res_data += ((datas->cachedata >> datas->cacheindex) & ((1<<(32 - datas->cacheindex))-1));
	}
	return res_data;
}

u16 fread16(ByteCache *datas)
{
	u16 res_data;

	if(datas->cacheindex == 0)
	{
		RKFIO_FRead((uint8*)&datas->cachedata,4,ape_file_handle);
		res_data = ((datas->cachedata >> 16)&0xffff); 
		datas->cacheindex = 16;
	}
	else if(datas->cacheindex == 8)
	{
		res_data = ((datas->cachedata&0xff)<<8);
		datas->cacheindex = 24;
		RKFIO_FRead((uint8*)&datas->cachedata,4,ape_file_handle);
		res_data += ((datas->cachedata>>24)&0xff);
	}
	else
	{
		datas->cacheindex -= 16;
		res_data = ((datas->cachedata >> datas->cacheindex)&0xffff);
	}
	return res_data;
}

u8 fread8(ByteCache *datas)
{
	u8 res_data;
   
	if(datas->cacheindex == 0)
	{
		RKFIO_FRead((uint8*)&datas->cachedata,4,ape_file_handle);
		res_data = ((datas->cachedata >> 24)&0xff); 
		datas->cacheindex = 24;
	}
	else
	{
		datas->cacheindex -= 8;
		res_data = ((datas->cachedata >> datas->cacheindex)&0xff);
	}
	return res_data;
}

#define malloc_size UINT_MAX //ÿ֡32 byte
char malloc_buff[malloc_size];// 
static long malloc_buff_pos = 0;
void  av_free()
{
	malloc_buff_pos = 0;
}
void *av_malloc(int n)
{
   malloc_buff_pos +=n;
   if(malloc_buff_pos >= malloc_size)
   {
     Hifi_Ape_Printf(" malloc buf error %d %d\n",malloc_buff_pos,n);
   }
   return &malloc_buff[malloc_buff_pos-n];
}

u32 Blockout(int32_t *decoded0,int32_t *decoded1,u8 *outbuf,u16 len,int ch,u32 bps)
{
	int i;
    u8 *samples = (u8 *)outbuf;
   
    if(bps ==24)
    {
	for(i = 0; i < len; i++)
	   {
    		*(int32_t *)samples = decoded0[i];
            samples+= 3;
    		if(ch== 2)
    		{
    			*(int32_t *)samples = decoded1[i];
                samples += 3;
            }
			else
			{
		      *(int32_t *)samples = decoded0[i];
                samples += 3;
           	}
    	}
        return len*2*3;
  
           	}
   
    else
    {
    	for (i = 0; i < len; i++)
    	{
    		*(int32_t *)samples = decoded0[i];
            samples+= 2;
    		if(ch == 2)
    		{
    			*(int32_t *)samples = decoded1[i];
                samples += 2;
    		}  
            else
            {
                *(int32_t *)samples = decoded0[i];
                 samples+= 2;
            }
    	}
 
        return len*2*2;
    }    
}
#pragma arm section code
#endif
#endif
