/*
*********************************************************************************************************
*                                       NANO_OS The Real-Time Kernel
*                                         FUNCTIONS File for V0.X
*
*                                    (c) Copyright 2013, RockChip.Ltd
*                                          All Rights Reserved
*File    : NanoShell.C
*By      : Zhu Zhe
*Version : V0.x
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/
#include "sysinclude.h"
#ifdef _USE_SHELL_
//#include "typedef_rkos.h"
#include "Device.h"
#include "audiocontrol.h"
#include "audio_main.h"
#include "mainmenu.h"
#include "musicwininterface.h"
#include "mediabrowin.h"
#include "rk_bt_Api.h"
#include "bd_addr.h"



/*
*********************************************************************************************************
*                                        Macro Define
*********************************************************************************************************
*/
#define EFFECT_TEST_NUM 0X00000FFF
#define EFFECT_TEST_BASE_NUM 3000

#define SHELL_RX_BUF_MAX_SIZE                128
#define SHELL_CMD_MAX_ITEM                   30
#define SHELL_CMD_MAX_SIZE                   10


typedef rk_err_t(* SHELL_PARASE_FUN)(HDC dev, uint8 *pStrBuff);

typedef struct _SHELL_CMD_ITEM
{
    uint32 useflag;
    uint8 * ShellCmdName;
    uint32 TaskID;
    SHELL_PARASE_FUN ShellCmdParaseFun;

}SHELL_CMD_ITEM;


typedef struct SHELL_TASK_DATA_BLOCK
{
    uint8* pData;
    uint8* pDataEnd;
    uint8* ShellRxBuffer;
    uint32 ShellRxStart;
    HDC hUart;
//    xTimerHandle IdleTime;
    uint32 IdleCnt;
    uint32 SysTick;
    uint16 * pbuf;
    uint32 temp;
    SHELL_CMD_ITEM * pCmdHeadItem;
    uint32 i;

}SHELL_TASK_DATA_BLOCK;

typedef struct _SHELL_CMD_INFO
{
    uint8 * ShellCmdName;
    uint32 TaskID;
    SHELL_PARASE_FUN ShellCmdParaseFun;

}SHELL_CMD_INFO;


/*
*********************************************************************************************************
*                                      Variable Define
*********************************************************************************************************
*/
extern struct l2cap_pcb *sdp_l2cap;
typedef void(*shell_func_t)(int agrc, void *agrv);
void ShellPrintHelpInfo(int agrc, void *agrv);
void avrcp_volumeup(int agrc, void *agrv);
void avrcp_volumedown(int agrc, void *agrv);
void avrcp_disconnect(int agrc, void *agrv);
void l2cap_connect(int agrc, void *agrv);
void l2cap_disconnect(int agrc, void *agrv);
void acl_disconnect(int agrc, void *agrv);
static void l2cap_echo_req(int agrc, void *agrv);
static void l2cap_info_req(int agrc, void *agrv);
static void print_key(int agrc, void *agrv);
static void l2cap_senddata(int agrc, void *agrv);
static void set_debug_level(int agrc, void *agrv);
static void set_debug_components(int agrc, void *agrv);
static void avdtp_set_role_cmd(int agrc, void *agrv);


const char* ShellRootName[] =
{
    "help",
    "volumeup",
    "volumedown",
    "avrcp_disconnect",
    "l2cap_connect",
    "l2cap_disconnect",
    "acl_disconnect",
    "l2cap_echo",
    "l2cap_info",
    "l2cap_senddata",
    "print_key",
    "set_debugL",
    "set_debugC",
    //add lyb
    "avdtp_get_configuration",
    "avdtp_start",
    "avdtp_diconnect",
    "avdtp_abort",
    "avdtp_close",
    "avdtp_get_capa",
    "avdtp_set_con",
    "avdtp_set_role",
    "\b"                          // the end
};

const shell_func_t root_func[] =
{
    ShellPrintHelpInfo,
     avrcp_volumeup,
     avrcp_volumedown,
     avrcp_disconnect,
     l2cap_connect,
     l2cap_disconnect,
     acl_disconnect,
     l2cap_echo_req,
     l2cap_info_req,
     l2cap_senddata,
     print_key,
     set_debug_level,
     set_debug_components,
     //add lyb
     avdtp_get_configuration_cmd,
     avdtp_start_cmd,
     avdtp_disconnect_pts,
     avdtp_abort_cmd,
     avdtp_close_cmd,
     avdtp_get_capabilities_cmd,
     avdtp_set_configuration_cmd_pts,
     avdtp_set_role_cmd,
};


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/


const SHELL_CMD_INFO ShellRegisterName[] =
{
    "\b",NULL, NULL
};

static SHELL_TASK_DATA_BLOCK*   gpstShellTaskDataBlock;
static SHELL_TASK_DATA_BLOCK stShellTaskDataBlock;
static uint8 ShellRxBuffer[SHELL_RX_BUF_MAX_SIZE];
static uint8 CmdHeadItem[sizeof(SHELL_CMD_ITEM) * SHELL_CMD_MAX_ITEM];

static SHELL_PARASE_FUN currentParaseFun = NULL;

struct l2cap_pcb *ptstest_pcb = NULL;


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/
void ShellPrintHelpInfo(int agrc, void *agrv)
{
    rk_printf("\r\n\r\n");

    rk_printf("\r\n================================================================================");
    rk_printf("\r\n Help Infomation                                                                ");
    rk_printf("\r\n volumeup                                                               ");
    rk_printf("\r\n volumedown                                                                    ");
    rk_printf("\r\n avrcp_disconnect                                                                   ");
    rk_printf("\r\n================================================================================");

    rk_printf("\r\n\r\n");
}

static void avrcp_volumeup(int agrc, void *agrv)
{
    ct_volumeup();
}

static void avrcp_volumedown(int agrc, void *agrv)
{
    ct_volumedown();
}

static void avrcp_disconnect(int agrc, void *agrv)
{
    avctp_disconnected_request(NULL);
    avdtp_disconnect_request(NULL,NULL);

}

static err_t l2cap_disconnected_ind(void *arg, struct l2cap_pcb *pcb, err_t err)
{
    err_t ret = ERR_OK;

    if(pcb->psm == SDP_PSM)
    {
        sdp_lp_disconnected(arg, pcb, err);
        return ret;
    }
    l2cap_close(pcb);
    return ret;
}


static err_t l2cap_connected(void *arg, struct l2cap_pcb *l2cappcb, uint16 result, uint16 status)
{

    if(result == L2CAP_CONN_SUCCESS)
    {
        /* Tell L2CAP that we wish to be informed of a disconnection request */
        l2cap_disconnect_ind(l2cappcb, l2cap_disconnected_ind);
    }
    else
    {
        l2cap_close(l2cappcb);
    }
    return ERR_OK;
}


static void l2cap_connect(int agrc, void *agrv)
{
                 //char ptsaddr[6] = {0x00,0x1B,0xDC,0x07,0x33, 0x8B};
    char ptsaddr[6] = {0x8B,0x33,0x07,0xDC,0x1B,0x00};
    ptstest_pcb = l2cap_new() ;
    printf("l2ca_connect_req\n");
    l2ca_connect_req(ptstest_pcb, (struct bd_addr *)ptsaddr, SDP_PSM, HCI_ALLOW_ROLE_SWITCH, l2cap_connected);
}

static void l2cap_disconnect(int agrc, void *agrv)
{
    //char ptsaddr[6] = {0x00,0x1B,0xDC,0x07,0x33, 0x8B};001BDC07338B
    char ptsaddr[6] = {0x8B,0x33,0x07,0xDC,0x1B,0x00};
    if(ptstest_pcb)
    {
        l2ca_disconnect_req(ptstest_pcb, NULL);
        ptstest_pcb = NULL;
    }
    else
    {
        if(sdp_l2cap)
            l2ca_disconnect_req(sdp_l2cap, NULL);
    }

}

static void acl_disconnect(int agrc, void *agrv)
{
    bt_disconnect((struct bd_addr *)&gSysConfig.BtConfig.LastConnectMac);
}

static void l2cap_senddata(int agrc, void *agrv)
{
    err_t ret;
    struct pbuf *p;
    char data[] = {0x06,0x00,0x01,0x00,0x0d,0x35,0x03,
        0x19,0x11,0x0d,0xff,0xff,0x35,0x03,0x09,0x00,0x09,0x00};

    p = pbuf_alloc(PBUF_RAW, sizeof(data),PBUF_RAM);
    memcpy(p->payload,data, sizeof(data));
    l2ca_datawrite(sdp_l2cap, p);
    pbuf_free(p);

    return ;
}

static void l2cap_echo_req(int agrc, void *agrv)
{
    char ptsaddr[6] = {0x8B,0x33,0x07,0xDC,0x1B,0x00};
    struct l2cap_pcb * pcb;
    lp_connect_req((struct bd_addr *)ptsaddr, 1);
    DelayMs(4000);
    pcb = l2cap_new() ;
    bd_addr_set(&pcb->remote_bdaddr,(struct bd_addr *)ptsaddr);
    l2ca_echo_req(pcb);
    l2cap_close(pcb);
}

static void l2cap_info_req(int agrc, void *agrv)
{
    char ptsaddr[6] = {0x8B,0x33,0x07,0xDC,0x1B,0x00};
    struct l2cap_pcb * pcb;
    lp_connect_req((struct bd_addr *)ptsaddr, 1);
    DelayMs(4000);
    pcb = l2cap_new() ;
    bd_addr_set(&pcb->remote_bdaddr,(struct bd_addr *)ptsaddr);
    l2ca_info_req(pcb, L2CAP_INFOTYPE_Ext_FEATURE_MASK);
    l2cap_close(pcb);
}

static void print_key(int agrc, void *agrv)
{
    int i;
    int j;
    printf("bt paire dev num = %d\n",gSysConfig.BtConfig.PairedDevCnt);
    printf("=============================\n") ;
    for(i=0;i<BT_LINK_KEY_MAX_NUM;i++)
    {   printf("<\n");
        printf("remote name =%s\n", gSysConfig.BtConfig.BtLinkKey[i].name);
        printf("link key:");
        //dumpMemoryCharA(gSysConfig.BtConfig.BtLinkKey[i].LinkKey,16);
        for(j=15; j>=0; j--)
        {
            printf("%02x",gSysConfig.BtConfig.BtLinkKey[i].LinkKey[j]);
        }
        printf("\nmac: 0x%02x-0x%02x-0x%02x-0x%02x-0x%02x-0x%02x\n",
            gSysConfig.BtConfig.BtLinkKey[i].BdAddr[0],
            gSysConfig.BtConfig.BtLinkKey[i].BdAddr[1],
            gSysConfig.BtConfig.BtLinkKey[i].BdAddr[2],
            gSysConfig.BtConfig.BtLinkKey[i].BdAddr[3],
            gSysConfig.BtConfig.BtLinkKey[i].BdAddr[4],
            gSysConfig.BtConfig.BtLinkKey[i].BdAddr[5]
        );
        printf(">\n");
    }
}
extern int bt_debug_components;
extern int bt_debug_level;

static void set_debug_level(int agrc, void *agrv)
{
    bt_debug_level = atoi(agrv);
    printf("bt_debug_level = %d\n", bt_debug_level);
}

static void set_debug_components(int agrc, void *agrv)
{
    bt_debug_components = atoi(agrv);
    printf("bt_debug_components = %d\n", bt_debug_components);
}

static void avdtp_set_role_cmd(int agrc, void *agrv)
{
    int role;
    role = atoi(agrv);
    avdtp_set_role(role);
}



#if 0
rk_err_t ShellCmdRemove(HDC dev, uint8 * pItemName)
{
    SHELL_CMD_ITEM * pCmdItem;

    uint32 i = 0, j = 0;

    if (gpstShellTaskDataBlock->pCmdHeadItem == NULL)
    {
        goto CmdRegErrExit;
    }
    else
    {
        pCmdItem = gpstShellTaskDataBlock->pCmdHeadItem;
        for (j = 0; j < SHELL_CMD_MAX_ITEM; j++)
        {
            if (pCmdItem->useflag == 1)
            {
                if (StrCmpA((uint8*)pCmdItem->ShellCmdName, pItemName, 0) == 0)
                {
                    // already register
                    pCmdItem->useflag = 0;
                    rk_printf("cmd remove success");
                    return RK_SUCCESS;
                }
            }
            pCmdItem++;
        }
    }

    rk_printf("not find cmd");

CmdRegErrExit:

    return RK_SUCCESS;
}


rk_err_t ShellCmdRegister(HDC dev, uint8 * pItemName)
{
    SHELL_CMD_ITEM * pCmdItem;

    uint32 i = 0, j = 0;

    if (gpstShellTaskDataBlock->pCmdHeadItem == NULL)
    {
        goto CmdRegErrExit;
    }
    else
    {
        pCmdItem = gpstShellTaskDataBlock->pCmdHeadItem;

        for (j = 0; j < SHELL_CMD_MAX_ITEM; j++)
        {
            if (pCmdItem->useflag == 1)
            {
                if (StrCmpA((uint8*)pCmdItem->ShellCmdName, pItemName, 0) == 0)
                {
                    // already register
                    rk_printf("device already connected");
                    return RK_SUCCESS;
                }
            }
            pCmdItem++;
        }
    }

    while (StrCmpA((uint8*)ShellRegisterName[i].ShellCmdName,pItemName, 0) != 0)
    {
        i++;
        if (*((uint8*)ShellRegisterName[i].ShellCmdName) == '\b')
        {
            rk_printf("device not exist");
            goto CmdRegErrExit;
        }
    }


    pCmdItem = gpstShellTaskDataBlock->pCmdHeadItem;
    for (j = 0; j < SHELL_CMD_MAX_ITEM; j++)
    {
        if (pCmdItem->useflag == 0)
        {
            pCmdItem->ShellCmdParaseFun  = ShellRegisterName[i].ShellCmdParaseFun;
            pCmdItem->ShellCmdName = ShellRegisterName[i].ShellCmdName;
            pCmdItem->TaskID = ShellRegisterName[i].TaskID;
            pCmdItem->useflag = 1;
            rk_printf("device connect success");
            return RK_SUCCESS;
        }
        pCmdItem++;
    }

    rk_printf("cmd full");
CmdRegErrExit:

    return RK_SUCCESS;
}


rk_err_t ShellCmdRegisterALL(void)
{
    SHELL_CMD_ITEM * pCmdItem;
    int j,i;
    uint CmdCnt = sizeof(ShellRegisterName)/sizeof(SHELL_CMD_INFO) -1;

    pCmdItem = gpstShellTaskDataBlock->pCmdHeadItem;

    if (CmdCnt > SHELL_CMD_MAX_ITEM)
    {
        rk_printf("shell cmd can't register");
        return RK_ERROR;
    }
    i = 0;
    for (j = 0; j < CmdCnt; j++)
    {
        if (pCmdItem->useflag == 0)
        {
            pCmdItem->ShellCmdParaseFun  = ShellRegisterName[i].ShellCmdParaseFun;
            pCmdItem->ShellCmdName = ShellRegisterName[i].ShellCmdName;
            pCmdItem->TaskID = ShellRegisterName[i].TaskID;
            pCmdItem->useflag = 1;
            i++;

        }
        pCmdItem++;
    }

    rk_printf("device connect success");
    return RK_SUCCESS;

}
#endif

/*
*********************************************************************************************************
*                                      voidShellTaskDeInit(void)
*
* Description: 根目录元素提取
*
* Argument(s) : void *p_arg
*
* Return(s)   : none
*
* Note(s)     : none.
*********************************************************************************************************
*/

rk_err_t ShellRootParsing(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;
    SHELL_CMD_ITEM * pTempCmdItem = NULL;

    StrCnt = ShellItemExtract(pstr,&pItem);

    if (StrCnt == 0)
    {
        rk_printf("error cmd\r\nrkos://");
        return RK_ERROR;
    }

    pstr[StrCnt] = 0;

    ret = ShellCheckCmd(ShellRootName, pItem, StrCnt);
    if (ret < 0)
    {
        pTempCmdItem = gpstShellTaskDataBlock->pCmdHeadItem;

        if (currentParaseFun)
        {
            ret = currentParaseFun(dev,pstr);
            if (ret == RK_ERROR)
            {
                rk_printf("error cmd");
            }
            else if (ret == RK_EXIT)
            {
                ShellPrintHelpInfo(NULL,NULL);
                currentParaseFun = NULL;
            }

            return ret;
        }
        else
        {
            for (i = 0; i < SHELL_CMD_MAX_ITEM; i++)
            {
                if (pTempCmdItem->useflag == 1)
                {
                    if (StrCmpA(pTempCmdItem->ShellCmdName, pstr, 0) == 0)
                    {
                        if (pTempCmdItem->ShellCmdParaseFun != NULL)
                        {
                            pItem += StrCnt;
                            pItem++;

                            if (currentParaseFun == NULL)
                            {
                                currentParaseFun = pTempCmdItem->ShellCmdParaseFun;

                                if (pTempCmdItem->TaskID != NULL)
                                {
                                    ModuleOverlay(pTempCmdItem->TaskID, MODULE_OVERLAY_ALL);
                                }
                                ret = pTempCmdItem->ShellCmdParaseFun(dev,"I");

                                if (ret == RK_ERROR)
                                {
                                    rk_printf("error cmd");
                                }
                            }
                            else
                            {
                                ret = pTempCmdItem->ShellCmdParaseFun(dev,pItem);

                                if (ret == RK_ERROR)
                                {
                                    rk_printf("error cmd");
                                }
                            }

                            return ret;


                        }
                    }
                }
                pTempCmdItem++;
            }
        }


        rk_printf("error cmd");
        return RK_ERROR;

    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;                                            //remove '.',the point is the useful item

    if(i >= 0)
    {
        if(root_func[i])
            root_func[i](0,pItem);
    }

    if (ret == RK_ERROR)
    {
        rk_printf("error cmd");
    }

    return ret;
}

/*
*********************************************************************************************************
*                                      void ShellTaskInit(void)
*
* Description:  This function is the Timer Task.
*
* Argument(s) : void *p_arg
*
* Return(s)   : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
rk_err_t ShellTaskInit(void *pvParameters)
{
    //RK_TASK_CLASS*   pShellTask = (RK_TASK_CLASS*)pvParameters;
    SHELL_TASK_DATA_BLOCK*  pShellTaskDataBlock;
    SHELL_CMD_ITEM * pShellCmdItem;

    uint32 i;

    pShellTaskDataBlock = &stShellTaskDataBlock;

    memset(pShellTaskDataBlock,NULL,sizeof(SHELL_TASK_DATA_BLOCK));
    pShellTaskDataBlock->ShellRxBuffer = ShellRxBuffer;
    pShellTaskDataBlock->pCmdHeadItem = (SHELL_CMD_ITEM *)CmdHeadItem;
    pShellCmdItem = (void *)pShellTaskDataBlock->pCmdHeadItem;
    for (i = 0; i < SHELL_CMD_MAX_ITEM; i++)
    {
        pShellCmdItem->useflag = 0;
        pShellCmdItem++;
    }

    gpstShellTaskDataBlock = pShellTaskDataBlock;
    //ShellCmdRegisterALL();
    ShellPrintHelpInfo(NULL,NULL);
    {
        uint8* pstr;
        pstr = gpstShellTaskDataBlock->ShellRxBuffer;
        memset(pstr,0,SHELL_RX_BUF_MAX_SIZE);

        pstr[0] = 'r';
        pstr[1] = 'k';
        pstr[2] = 'o';
        pstr[3] = 's';
        pstr[4] = ':';
        pstr[5] = '/';
        pstr[6] = '/';

        gpstShellTaskDataBlock->ShellRxStart = 7;
        gpstShellTaskDataBlock->i = 7;

        rk_printf("\r\n\r\n\r\n");
        rk_printf(pstr);
    }


    return RK_SUCCESS;

exit:
    return RK_ERROR;
}


/*
*********************************************************************************************************
*                                      voidShellTaskDeInit(void)
*
* Description:  This function is the Timer Task.
*
* Argument(s) : void *p_arg
*
* Return(s)   : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
rk_err_t ShellTaskDeInit(void *pvParameters)
{
    return RK_SUCCESS;

exit:

    return RK_ERROR;

}

/*
*********************************************************************************************************
*                                      void ShellTaskDeInit(void)
*
* Description:  This function is the Timer Task.
*
* Argument(s) : void *p_arg
*
* Return(s)   : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
void ShellTask(void)
{
    uint32 i,j;
    uint8* pstr;
    rk_size_t size;

    pstr = gpstShellTaskDataBlock->ShellRxBuffer;

    i = gpstShellTaskDataBlock->i;

    if (i <= SHELL_RX_BUF_MAX_SIZE)
    {
        size = UartDev_Read(gpstShellTaskDataBlock->hUart, pstr + i, 1);
        if (size == 0)
        {
            return;
        }

        if (pstr[i] == 0x0d)
        {
            UartDev_Write(gpstShellTaskDataBlock->hUart,"\r\n",2,SYNC_MODE,NULL);
            goto process;
        }
        else if ((pstr[i] < 32) && (pstr[i] != '\b'))
        {
            return;
        }
        else if ((pstr[i] >= 127))
        {
            return;
        }

        UartDev_Write(gpstShellTaskDataBlock->hUart,pstr+i,1,SYNC_MODE,NULL);

        if (pstr[i] == '\b')
        {
            if (i == gpstShellTaskDataBlock->ShellRxStart)
            {
                if (gpstShellTaskDataBlock->ShellRxStart == 7)
                {
                    UartDev_Write(gpstShellTaskDataBlock->hUart,"/",1,SYNC_MODE,NULL);
                }
                else
                {
                    UartDev_Write(gpstShellTaskDataBlock->hUart,".",1,SYNC_MODE,NULL);
                }
            }
            else
            {
                UartDev_Write(gpstShellTaskDataBlock->hUart," \b", 2, SYNC_MODE, NULL);
                i--;
                gpstShellTaskDataBlock->i = i;
            }

        }
        else
        {
            i++;
            gpstShellTaskDataBlock->i = i;
        }

    }

    return;

process:

    if (pstr[i] == 0x0d)
    {
        if (i == gpstShellTaskDataBlock->ShellRxStart)
        {
            memset(pstr + gpstShellTaskDataBlock->ShellRxStart,0,(SHELL_RX_BUF_MAX_SIZE - gpstShellTaskDataBlock->ShellRxStart));
            rk_printf("\r\n\r\n");
            rk_printf(pstr);

        }
        else if ((pstr[gpstShellTaskDataBlock->ShellRxStart] == 'c')
                 && (pstr[gpstShellTaskDataBlock->ShellRxStart + 1] == 'd')
                 && (pstr[gpstShellTaskDataBlock->ShellRxStart + 2] == 0x0d))
        {
            pstr[gpstShellTaskDataBlock->ShellRxStart] = 0;
        }
        else if ((pstr[gpstShellTaskDataBlock->ShellRxStart] == 'c')
                 && (pstr[gpstShellTaskDataBlock->ShellRxStart + 1] == 'd')
                 && (pstr[gpstShellTaskDataBlock->ShellRxStart + 2] == ' '))
        {


            if (pstr[gpstShellTaskDataBlock->ShellRxStart + 3] == 0x0d)
            {
                pstr[gpstShellTaskDataBlock->ShellRxStart] = 0;
            }
            else if ((pstr[gpstShellTaskDataBlock->ShellRxStart + 3] == '.')
                     && (pstr[gpstShellTaskDataBlock->ShellRxStart + 4] == '.')
                     &&(pstr[gpstShellTaskDataBlock->ShellRxStart + 5] == 0x0d))
            {
                if (gpstShellTaskDataBlock->ShellRxStart > 8)
                {
                    for (j = gpstShellTaskDataBlock->ShellRxStart - 2; j >= 7; j--)
                    {
                        if (pstr[j] == '.')
                        {
                            break;
                        }
                    }
                    gpstShellTaskDataBlock->ShellRxStart = j + 1;
                }
                pstr[gpstShellTaskDataBlock->ShellRxStart] = 0;
            }
            else if ((pstr[gpstShellTaskDataBlock->ShellRxStart + 3] == '.')
                     && (pstr[gpstShellTaskDataBlock->ShellRxStart + 4] == 0x0d))
            {
                pstr[gpstShellTaskDataBlock->ShellRxStart] = 0;
            }
            else if (memcmp(pstr + gpstShellTaskDataBlock->ShellRxStart + 3, "rkos://", 7) == 0)
            {

                pstr[i] = 0;
                for (j = 0; j < strlen(pstr + gpstShellTaskDataBlock->ShellRxStart + 3); j++)
                {
                    pstr[j] = pstr[gpstShellTaskDataBlock->ShellRxStart + 3 + j];
                }

                if (j > 7)
                {
                    pstr[j++] = '.';
                    gpstShellTaskDataBlock->ShellRxStart = strlen(pstr + gpstShellTaskDataBlock->ShellRxStart + 3) + 1;
                }
                else
                {
                    gpstShellTaskDataBlock->ShellRxStart = strlen(pstr + gpstShellTaskDataBlock->ShellRxStart + 3);
                }
                pstr[gpstShellTaskDataBlock->ShellRxStart] = 0;

            }
            else
            {
                pstr[i] = 0;
                for (j = gpstShellTaskDataBlock->ShellRxStart; j <(gpstShellTaskDataBlock->ShellRxStart + strlen(pstr + gpstShellTaskDataBlock->ShellRxStart + 3)); j++)
                {
                    pstr[j] = pstr[3 + j];
                }
                pstr[j++] = '.';

                gpstShellTaskDataBlock->ShellRxStart += strlen(pstr + gpstShellTaskDataBlock->ShellRxStart + 3) + 1;
                pstr[gpstShellTaskDataBlock->ShellRxStart] = 0;
            }


        }
        else
        {
            pstr[i] = 0;
            ShellRootParsing(gpstShellTaskDataBlock->hUart, pstr + 7);
            memset(pstr + gpstShellTaskDataBlock->ShellRxStart,0,(SHELL_RX_BUF_MAX_SIZE - gpstShellTaskDataBlock->ShellRxStart));
            rk_printf("\r\n\r\n");
            rk_printf(pstr);
        }

        gpstShellTaskDataBlock->i = gpstShellTaskDataBlock->ShellRxStart;

    }

}
#endif

