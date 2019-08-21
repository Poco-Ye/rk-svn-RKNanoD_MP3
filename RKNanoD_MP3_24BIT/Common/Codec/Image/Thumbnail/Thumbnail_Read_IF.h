/* ********************************************************************************************** */
/*                  TEMP�@�\�t�����g�G���h�w�b�_                                                  */
/* ********************************************************************************************** */
/* 	Programmed by			Yuki.Kishimoto                                      */
/* **************************************************************************** */

#ifndef THUMBNAIL_IF_H
#define THUMBNAIL_IF_H "0.01"


typedef struct _pic_data {
//	char *Pic;
	long  PicStart;			/* Picture Start Postion */
	long  PicSize;			/* Picture Size */
} PIC_DATA;


/* ============================================================================================== */
/* 	�O���֐��̃v���g�^�C�v�錾�F���@�\�ł��g�p����֐��錾                                        */
/* ============================================================================================== */
void Thumbnail_Read_Main(void);											/* Exif �擾 ���C������ */


/* ---------------------------------------------------------------------------------------------- */
/* 	�O��I/F�֐�                                                                                   */
/* ---------------------------------------------------------------------------------------------- */
void Thumbnail_IF_SetFileName(char *buf);
void Thumbnail_IF_SetFilePointer(FILE *fp);

PIC_DATA *Thumbnail_IF_GetPictureData(void);
long Thumbnail_IF_GetThumbnailOffset(void);
long Thumbnail_IF_GetThumbnailSize(void);
int Thumbnail_IF_GetPictureType(void);

#endif
