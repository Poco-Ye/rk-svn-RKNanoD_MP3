/************************************************************************
 *       Smart Jpeg Decoder
 *
 * File:
 *  hufdec.h
 *
************************************************************************/
#include "image_main.h"
#ifdef JPG_DEC_INCLUDE

#pragma arm section code = "JpgDecCode", rodata = "JpgDecCode", rwdata = "JpgDecData", zidata = "JpgDecBss"
#include "jpgdec_global.h"
#include "jpgdec_decompress.h"
#include "jpgdec_globalvardeclare.h"
// Verifies that all the Huffman tables needed for this scan are available.
int JpgDecCheckHuffTables(void)
{
    int i;

    for (i = 0; i < Jpg_gCompInScan; i++)
    {
        if ((Jpg_gSpectralStart == 0) && (Jpg_pHuffNum[Jpg_gCompDCTable[Jpg_gCompList[i]]] == NULL))
        {
            return(JPGD_UNDEFINED_HUFF_TABLE);
        }

        if ((Jpg_gSpectralEnd > 0) && (Jpg_pHuffNum[Jpg_gCompACTable[Jpg_gCompList[i]]] == NULL))
        {
            return(JPGD_UNDEFINED_HUFF_TABLE);
        }
    }

    for (i = 0; i < JPGD_MAXHUFFTABLES; i++)
        if (Jpg_pHuffNum[i])
        {
            if (!Jpg_gHuffman[i])
                Jpg_gHuffman[i] = (Phuff_tables_t)(gJpg_gHuffman + i*(256*4 + 256 + 512*4));//(Phuff_tables_t)JpgDecAlloc(sizeof(huff_tables_t));

            if (Jpg_gHuffman[i] == NULL)
                return JPGD_TOO_MANY_BLOCKS;

            JpgDecMakeHuffTable(i, Jpg_gHuffman[i]);
        }

    for (i = 0; i < Jpg_gBlocksPerMcu; i++)
    {
        Jpg_gDCHuffSeg[i] = Jpg_gHuffman[Jpg_gCompDCTable[Jpg_gMcuOrg[i]]];
        Jpg_gACHuffSeg[i] = Jpg_gHuffman[Jpg_gCompACTable[Jpg_gMcuOrg[i]]];
        Jpg_pComponent[i]   = &Jpg_gLastDCValue[Jpg_gMcuOrg[i]];
    }

    return 0;
}

/*
// Creates the tables needed for efficient Huffman decoding.
void JpgDecMakeHuffTable(int index, Phuff_tables_t hs)
{
  int p, i, l, si;
  uint8 huffsize[257];
  uint huffcode[257];
  uint code;
  uint subtree;
  int code_size;
  int lastp;
  int nextfreeentry;
  int currententry;

  p = 0;

  for (l = 1; l <= 16; l++)
  {
    for (i = 1; i <= Jpg_pHuffNum[index][l]; i++)
      huffsize[p++] = l;
  }

  huffsize[p] = 0;

  lastp = p;

  code = 0;
  si = huffsize[0];
  p = 0;

  while (huffsize[p])
  {
    while (huffsize[p] == si)
    {
      huffcode[p++] = code;
      code++;
    }

    code <<= 1;
    si++;
  }

  memset(hs->look_up, 0, sizeof(hs->look_up));
  memset(hs->tree, 0, sizeof(hs->tree));
  memset(hs->code_size, 0, sizeof(hs->code_size));

  nextfreeentry = -1;

  p = 0;

  while (p < lastp)
  {
    i = Jpg_pHuffVal[index][p];
    code = huffcode[p];
    code_size = huffsize[p];

    hs->code_size[i] = code_size;

    if (code_size <= 8)
    {
      code <<= (8 - code_size);

      for (l = 1 << (8 - code_size); l > 0; l--)
      {
        hs->look_up[code] = i;
        code++;
      }
    }
    else
    {
      subtree = (code >> (code_size - 8)) & 0xFF;

      currententry = hs->look_up[subtree];

      if (currententry == 0)
      {
        hs->look_up[subtree] = currententry = nextfreeentry;

        nextfreeentry -= 2;
      }

      code <<= (16 - (code_size - 8));

      for (l = code_size; l > 9; l--)
      {
        if ((code & 0x8000) == 0)
          currententry--;

        if (hs->tree[-currententry - 1] == 0)
        {
          hs->tree[-currententry - 1] = nextfreeentry;

          currententry = nextfreeentry;

          nextfreeentry -= 2;
        }
        else
          currententry = hs->tree[-currententry - 1];

        code <<= 1;
      }

      if ((code & 0x8000) == 0)
        currententry--;

      hs->tree[-currententry - 1] = i;
    }

    p++;
  }
}*/
void JpgDecMakeHuffTable(int index, Phuff_tables_t hs)
{
    int p, i, l, si;
    uint8 huffsize[257];
    uint huffcode[257];
    uint code;
    uint subtree;
    int code_size;
    int lastp;
    int nextfreeentry;
    int currententry;

    p = 0;

    for (l = 1; l <= 16; l++)
    {
        for (i = 1; i <= Jpg_pHuffNum[index][l]; i++)
            huffsize[p++] = l;
    }

    huffsize[p] = 0;

    lastp = p;

    code = 0;
    si = huffsize[0];
    p = 0;

    while (huffsize[p])
    {
        while (huffsize[p] == si)
        {
            huffcode[p++] = code;
            code++;
        }

        code <<= 1;
        si++;
    }

    //memset(hs->look_up, 0, sizeof(hs->look_up));
    //memset(hs->tree, 0, sizeof(hs->tree));
    //memset(hs->code_size, 0, sizeof(hs->code_size));
    ImageMemSet(hs->look_up, 0, sizeof(hs->look_up)/(sizeof(uint32) * 8));
    ImageMemSet(hs->tree, 0, sizeof(hs->tree)/(sizeof(uint32) * 8));
    ImageMemSet(hs->code_size, 0, sizeof(hs->code_size)/(sizeof(uint32) * 8));

    nextfreeentry = -1;

    p = 0;

    while (p < lastp)
    {
        //short codebk;
        i = Jpg_pHuffVal[index][p];
        code = huffcode[p];
        code_size = huffsize[p];



        if (code_size <= 8)
        {

            code <<= (8 - code_size);

            for (l = 1 << (8 - code_size); l > 0; l--)
            {
                hs->look_up[code] = code_size;
                hs->code_size[code] = i;
                code++;
            }
        }
        else
        {
            subtree = (code >> (code_size - 8)) & 0xFF;

            currententry = hs->look_up[subtree];

            if (currententry == 0)
            {
                hs->look_up[subtree] = currententry = nextfreeentry;

                nextfreeentry -= 2;
            }

            code <<= (16 - (code_size - 8));

            for (l = code_size; l > 9; l--)
            {
                if ((code & 0x8000) == 0)
                    currententry--;

                if (hs->tree[-currententry - 1] == 0)
                {
                    hs->tree[-currententry - 1] = nextfreeentry;

                    currententry = nextfreeentry;

                    nextfreeentry -= 2;
                }
                else
                    currententry = hs->tree[-currententry - 1];

                code <<= 1;
            }

            if ((code & 0x8000) == 0)
                currententry--;

            hs->tree[-currententry - 1] = i;
        }

        p++;
    }
}
#pragma arm section code
#endif
