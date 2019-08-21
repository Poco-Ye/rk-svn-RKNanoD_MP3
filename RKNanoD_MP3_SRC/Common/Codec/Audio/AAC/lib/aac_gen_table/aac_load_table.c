
#ifdef AAC_DEC_INCLUDE


#include <stdio.h>
#include <stdlib.h>
#include "aac_table.h"
unsigned char base[1024*1024];
void load_table()
{

	FILE *fp = fopen("aac_gen_table/aac_table.bin","rb");
	int size = 1024*1024;
	int ret;
	
	
	if(!fp)
		{
		DEBUG("open file error \n");
		return ;
		}
	 ret=fread(base,1,size,fp);

	return ;


}
/*
int main()
	{

	load_table();
	return 0;
	

	}
*/
#endif
