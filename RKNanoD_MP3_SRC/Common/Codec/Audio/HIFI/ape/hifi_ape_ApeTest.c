

#include "ape.h"
#include "VirtualFile.h"

FILE *ape_file_handle;
APEContext apeobj;
ByteIOContext pbobj;

int Ape_test(void)
{
   #if 0
	int i;

	ape_file_handle = fopen("F:\\ApeTest\\192_24_1s.ape", "rb");

    init_ape();
	ape_read_header();

	for (i = 0; i < apeobj.totalframes; i++) 
	{
		int out_size,in_size;

		fseek(ape_file_handle,(u32)apeobj.frames[i].pos , SEEK_SET);
		if(i == (apeobj.totalframes - 1))
			out_size = apeobj.finalframeblocks;
		else
			out_size = apeobj.blocksperframe;
		in_size = apeobj.frames[i].size;
		ape_decode_frame(out_size,in_size,apeobj.frames[i].skip);

		//if(i >= 40)
			//exit(0);
	}
	#endif 
	return 0;
}