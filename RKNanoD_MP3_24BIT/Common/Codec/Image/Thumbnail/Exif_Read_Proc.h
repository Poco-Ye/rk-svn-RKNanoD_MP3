/* ************************************************ */
/*  Exif_Read_Proc.h                                */
/*  2015/4/27                                       */
/*  by Yuki.KIshimoto                               */
/* ************************************************ */

#ifndef EXIF_PROC_H
#define EXIF_PROC_H "0.01"

/* ============================================================================================== */
/*  �O���֐��̃v���g�^�C�v�錾�F�����W���[���ł��g�p����֐��錾                                  */
/* ============================================================================================== */
/* ---------------------------------------------------------------------------------------------- */
/*  �@�\�֐�                                                                                      */
/* ---------------------------------------------------------------------------------------------- */
int Exif_Read_Proc();                      /* Exif �擾���� */

/* ---------------------------------------------------------------------------------------------- */
/*  �O��I/F�֐�                                                                                   */
/* ---------------------------------------------------------------------------------------------- */
void Exif_Proc_SetFileName(char *buf[]);
void Exif_Proc_SetFilePointer(FILE *fp);
int Exif_Proc_SeekFilePointer(FILE *fp, long ofs);
long Exif_Proc_GetImgWidth(void);
long Exif_Proc_GetImgLength(void);
long Exif_Proc_GetOrientation(void);
long Exif_Proc_GetColorSpace(void);
char* Exif_Proc_GetDateTime(void);
long Exif_Proc_GetFormatLength(void);
long Exif_Proc_GetThumbnailOffset(void);
long Exif_Proc_GetFileSize(void);

#endif
