#define SYSTEM_APPLICATION_TABLE_DEFINE_HERE
#define MAIN_APP_GUI_OBJECT_DEFINE_HERE

#include "gpio_func_sel.h"
#include "app_infra/main_app.h"
#include "gobj_respond_event.h"
#include "app_infra/event.h"
#include "sysapp_table.h"
#include "app_infra/gobj_mgr.h"
#include "common_gobj_tbl.h"
#include "app_infra/sysapp_msg.h"
#include "app_infra/sysapp_if.h"
#include "app_infra/sysapp_infra.h"
#include "app_infra/hdmitx_daemon.h"
#include "app_infra/app_msg_filter.h"
#include "platform/status_reg.h"
#include "platform/pinmux.h"
#include "app_infra/sysapp_timer.h"
#include "app_infra/dev_callback.h"
#include "mw_logo.h"
#include "app_infra/ss_daemon.h"
#include "app_infra/anim_daemon.h"
#include "platform/platform_venc.h"
#include "common_guiobj_mainmenu.h"
#include "plf_gpio_select.h"
#include "drivers/vfont/vfont_if.h"
#if (IC_VER == QAE377)
#include "app_infra/di_daemon.h"
#endif
#ifdef SUPPORT_iPOD_DOCKING_UART
#include "drivers/drv_ipod.h"
#endif
#include "gui_engine.h"
#include "COM_UI_font_ui_config.h"
#include "COM_UI_font_style_define.h"
#include "COM_UI_langname_config.h"
#include "COM_UI_region_config.h"
#include "COM_UI_strid_config.h"
#include "app_infra/cbk_event.h"
//#include "middlewares/ptnmng/ptnmng_if.h"
#include "auddrv.h"
#include "divx_drm_if.h"
#include "middlewares/common/dtv_display/mid_display.h"
#include "common_gui_setupmenu_share.h"
#include "app_show_logo.h"		//wdy add 2009-10-15
#include "app_private_data.h"
#include "user_init.h"
//#include "../container/container_avi_general.h"
#include "middlewares/file/fmw_if.h"
#include "vpp_mixer_sel.h"
#include "iso639_tbl.h"
#include "app_infra/ir_daemon.h"
#include "app_ptn_mnger.h"
#include "app_infra/gpio_daemon.h"
#ifdef SUPPORT_VFD_DRIVER
#include "app_infra/vfd_daemon.h"
#include "drivers/drv_vfd_panel.h"
#include "drivers/drv_common.h"
#endif
#ifdef NET_SUPPORT
#include "app_infra/net_daemon.h"
#include "app_net_instance.h"
#ifdef NET_ET_SUPPORT
#include "net/et/libet.h"
#endif
#ifdef NET_N32_SUPPORT
#include "net/dlmgr/dlmgr_if.h"
#endif
#endif
#include "drivers/drv_display.h"
#include "drivers/drv_iop.h"
#include "drivers/drv_ir.h"
#include "drivers/drv_cec_tx.h"
#include "autodetect.h"

#if (IR_OSD_DBUG == 1)
#include "ir_dbug/ir_dbug.h"
#endif

#if defined(IR_BOT_RANDOM_K)||defined(IR_BOT_SEQ_K)
#include "automation.h"
#endif
#ifdef NET_SUPPORT
#include "net/net_app/net_test.h"
#endif
#include "net/net_app/network_test.h"
#include "isp.h"
#include "./common_api/app_smartupgrade.inc"
#ifdef SUPPORT_PLAYER_STB_DVB_T
#include "pin_allocator.h"
#include "dvb_systime.h"
#include "dvb_timer_utilities.h"
#include "app_infra/wktimer_daemon.h"
#include "middlewares/ptnmng/partitionlist/mid_partition_list.h"
#include "app_fakestandby.h"
#include "dvb_nvm_store.h"
#include "middlewares/dvb/dvb_recorder/mid_recorder.h"

#endif

#define APPNOINLINE __attribute__((noinline))


#if (SUPPORT_BDMV_USB_LOAD==1)
#include "platform/monflg.h"

#endif
/*define main app call back function*/
typedef void (*MainAppCallBack) (MainAppUIEventHandlerInfo_t *);
static MainAppCallBack SysAppTerminateAckCallBack = NULL;	/*for sys app terminate ack event */
static MainAppCallBack SysAppActiveAckCallBack = NULL;	/*for sys app active ack event */
static MainAppCallBack SysAppActiveFailCallBack = NULL;	/*for sys app active fail event */
static MainAppCallBack MainAppDiscInsertCallBack = NULL; /*for data disc parse_one_done callback*/
extern int mount(const char *devname, const char *dir, const char *fsname);

UINT32 ISPStartToUpgrade = FALSE;

static MainAppUIEventHandlerInfo_t stUIEventHandlerInfo;
N_1LevelItemList_t* pstHeadN_1LevelItem=NULL; //the head of the N-1 level item list, for Setup Menu
N_1LevelItemList_t* pstTailN_1LevelItem=NULL; //the tail of the N-1 level item list, for Setup Menu

#if (DEFAULT_TV_SYSTEM==0)
UINT32 dDefaultTVSys = PSREG_SETUP_PAL;
#elif (DEFAULT_TV_SYSTEM==1)
UINT32 dDefaultTVSys = PSREG_SETUP_NTSC;
#elif (DEFAULT_TV_SYSTEM==2)
UINT32 dDefaultTVSys = PSREG_SETUP_TV_System_Auto;
#else
#error "ERROR: DEFAULT_TV_SYSTEM should be PAL or NTSC"
#endif

static UINT32 gdPhisycalLoaderDiscDevNum = 0;
DI_Handle gPhisycalLoaderDIHandle = 0; //Raymond Temp Add

#define MAIN_APP_DEBUG
#ifdef MAIN_APP_DEBUG
#define mainapp_printf(fmt, arg...) diag_printf("[MainApp]"fmt,##arg)
#else
#define mainapp_printf(a,...) do{}while(0)
#endif

#define MAIN_APP_ERROR
#ifdef MAIN_APP_ERROR
#define mainapp_error(fmt, arg...) diag_printf("[MainApp][ERROR][%d]"fmt,__LINE__,##arg)
#else
#define mainapp_error(a,...) do{}while(0)
#endif

#define GUI_FUNC_CALL(funcCall) if(GUI_SUCCESS != (funcCall)) mainapp_printf("[GUI ERROR][LINE]:[%d]\n",__LINE__)

#define OpenState 0				//wdy add
#define CloseState 1

extern GL_Queue_t MainAppQueueHandle;
extern UINT8 bMPBreakCondition;
extern MainApp_t *pMainAppInstance;
extern struct gl_intr_t gl_intrpool[];

#ifdef SAMBA_SUPPORT
static void APPNOINLINE _MainApp_SysAppActiveAckForSMB(MainAppUIEventHandlerInfo_t * pstEventInfo);
static void APPNOINLINE _MainApp_SysAppActiveFailForSMB(MainAppUIEventHandlerInfo_t * pstEventInfo);
#endif
#ifdef CDDA_MULTI_SESSION_CHECK  //guohao 2010-8-6 10:54 add
static void APPNOINLINE _MainApp_DaemonEventCddaDiscInserted(UINT32 dParam, MainAppUIEventHandlerInfo_t * pstEventInfo);
#endif
UINT8 MAINAPP_IsNetworkAP(UINT32 dSysAppIdx);
static void _MainApp_MaintainGobjOnEject(MainAppUIEventHandlerInfo_t * pstEventInfo);
static void _MainApp_DaemonEventScrnSaverOff(UINT32 dParam, MainAppUIEventHandlerInfo_t * pstEventInfo);
static void _MainApp_UIEventOnSource(MainAppUIEventHandlerInfo_t * pstEventInfo);
static void APPNOINLINE _MainApp_DaemonEventStorageDetached(MainAppUIEventHandlerInfo_t * pstEventInfo);
void _MainApp_Finalizing(void);//add by george.chang 2010.09.09
inline void _MainApp_DiscInsertEvent(MainAppUIEventHandlerInfo_t * pstEventInfo);
static void _MainApp_SetupRelatedMenu2MainMenu(MainAppUIEventHandlerInfo_t * pstEventInfo, MAIN_APP_GUI_OBJ_ID eCurrGUIObjID);
#ifdef IC_SUPPORT_IOP_PWR_CTRL
	extern const unsigned char IOPcode_NEC_Standby_Pwr_Ctrl[];
	#define IOP_CODE_CHOOSED IOPcode_NEC_Standby_Pwr_Ctrl
#else
	#ifdef IC_SUPPORT_IOP_HW_RESET	// iris 2010-11-12 11:48 add
		extern const unsigned char IOPcode_NEC_Standby_HW_Reset[];
		#define IOP_CODE_CHOOSED IOPcode_NEC_Standby_HW_Reset
	#else	// iris 2010-11-12 11:48 add
	extern const unsigned char IOPcode_NEC_Standby[];
	#define IOP_CODE_CHOOSED IOPcode_NEC_Standby
	#endif
#endif	// iris 2010-9-28 14:29 add
#include "./common_api/app_file_loader.inc"
//Screen saver related variables.
UINT32 dSSCounterLimit = 300;	//seconds   //modified by wumin for mantis 107641  5m
UINT32 dSSUpdatePeriod = 20;	//millisecnds
UINT32 pSSInvalieKey[] = {
	DMN_EVENT_APP_TIMER,
	DMN_EVENT_SCREEN_SAVER_ON,
	DMN_EVENT_SCREEN_SAVER_OFF
};//Screen saver related variables.

INT32 sdEarlySetItem[] = { //maintained by ted.chen
	L2_ITEM_Skin,
	L2_ITEM_TV_System,
	L2_ITEM_Output_Component,
	L2_ITEM_YCbCrHDMI_Resolution,
	L2_ITEM_VGA_Resolution,
	L2_ITEM_Aspect_Ratio,
	L2_ITEM_HDMI_1080p_24Hz,
	L2_ITEM_Black_Level_Enable,
	L2_ITEM_Bright,
	L2_ITEM_Contrast,
	L2_ITEM_Hue,
	L2_ITEM_Saturation,
};
UINT32 dEarlySetItemNum=sizeof(sdEarlySetItem) /sizeof(sdEarlySetItem[0]);

#if (IC_VER == QAE377)
int gMessHandle = 0;
extern int gMsgHandle;
#ifdef SUPPORT_PLAYER_STB_DVB_T
static UINT32 gdResentMessage = 0;
static UINT32 gdResentParam = 0;
static void mainapp_SetEventInTransition(UINT32 dMessage, UINT32 dParam)
{
	gdResentMessage = dMessage;
	gdResentParam = dParam;
}
static void mainapp_ResendEventInTransition(void)
{
	UINT32 dActiveSysApp = 0;
	if (gdResentMessage > 0)
	{
		switch (MAINAPP_GetActiveSystemAppIndex(&dActiveSysApp))
		{
			case MAIN_APP_IN_TRANSITION:
				mainapp_printf("[MainApp] In Transition State, wait and retry.\n");
				break;
			case MAIN_APP_SUCCESS:
				SYSAPP_IF_SendGlobalEventWithIndex(dActiveSysApp, gdResentMessage, gdResentParam);
			default:
				gdResentMessage = 0;
				gdResentParam = 0;
				break;
		}
	}
}
void MAINAPP_SendEvent4DvbAP(UINT32 dSysAppIdx, UINT32 dMessage, UINT32 dParam)
{
		switch(dSysAppIdx)
		{
			case SYS_APP_DVB:
					SYSAPP_IF_SendGlobalEventWithIndex(dSysAppIdx,dMessage,dParam);
			break;
		}
}

static INT32 _MainApp_ActivateCurSysAppWithDVBAppType(MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	UINT32 dDeviceType = DEVICE_TYPE_MAX;
	SysDriveCtrlInfo_t *pstSysDriveCtrlInfo = &pstEventInfo->stSysDriveCtrlInfo;
	INT32 sdRet = MAIN_APP_SUCCESS;
	/*1st : close main menu */
	if (GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU))
	{
			GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU);
	}
	/*-2nd:stop the loader*/	//xh.song add for stopping loader when come to dvbt 2010-5-26 15:12
	AppPtn_GetDevType(
		pstSysDriveCtrlInfo->stSysCurPtnHnd,
		&dDeviceType);
	if (dDeviceType == DEVICE_TYPE_DISC)
	{
		mainapp_printf("%s call DI_DiscStop !!!! \n",__FUNCTION__);
		DI_DiscStop(gPhisycalLoaderDIHandle);
	}
	/*2nd: switch to dvb app*/
	sdRet = MAINAPP_ActivateSystemApp(SYS_APP_DVB, 0);
	return sdRet;
}

//xh.song add for booking rec not in dvb_app 2010-11-22 15:34
static void _MainAPP_WKHMToDVBT(void)
{
	int iRet;
	UINT32 dCurAppIdx;
	UINT32 dTimeoutCount = 100;
	if(((MAINAPP_GetActiveSystemAppIndex(&dCurAppIdx) == MAIN_APP_SUCCESS)\
			&&(dCurAppIdx != SYS_APP_DVB))
			||(GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU))
			||(GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_SETUPMENU)))
	{
		if (GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU))
		{
			GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU);
		}
		if(GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_SETUPMENU))
		{
			GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_SETUPMENU);
		}
		if(MAINAPP_ActivateSystemApp(SYS_APP_DVB, 0) == MAIN_APP_TERMINATE_CURRENT_APP_FIRST)
			MAINAPP_SwitchSystemApp(SYS_APP_DVB, 0);
		while(dTimeoutCount-- > 0)
		{
			dCurAppIdx = TOTAL_SYS_APP_SIZE;
			iRet = MAINAPP_GetActiveSystemAppIndex(&dCurAppIdx);
			GL_TaskSleep(5);
			if((dCurAppIdx == SYS_APP_DVB)&&(iRet == MAIN_APP_SUCCESS))
				break;
		}
	}
}
#ifdef SUPPORT_PVR
static INT32 MainApp_IsDvbRecording(UINT32 dMessage)
{
	//lr.wu added for DVB,when recording,don't switch DVD directly 11-01-04
	if((MID_Recorder_GetRecMode() == MID_REC_MODE_MANUAL)
		&& (MID_RecorderIsCurRecording() == DRV_SUCCESS))
	{
		mainapp_printf("switch to DVB\n");
		return TRUE;
	}
    return FALSE;
}
#endif
#endif
static void HWInit8600(void)
{
//Temorary Place for Adding Module Initialize Interfaces Under 8600 Config.

	// audio init API
	//extern void AUDIF_Init_Audio(void);
	//extern void AUDIF_Set_CodingMode(UINT32 coding_mode);
	//AUDIF_Init_Audio();
	//AUDIF_Set_CodingMode(1<<6); //mp3

}
#endif
#ifdef NET_SUPPORT
static INT32 NetDaemon_callback(UINT32 dEvent, UINT32 dParam)
{
	MSG_FILTER_DispatchMessage(gMsgHandle, dEvent, dParam);
	return 1;
}

#endif
#ifdef KOK_SUPPORT
void MAINAPP_SendEvent4KOKAP(UINT32 dSysAppIdx, UINT32 dMessage, UINT32 dParam)
{
	switch(dSysAppIdx)
	{
		case SYS_APP_FILE_PLAYER:
				SYSAPP_IF_SendGlobalEventWithIndex(dSysAppIdx,dMessage,dParam);
		break;
	}
}
#endif

int PtnCallbackFunc(PtnMng_CallbackType_e ePtnMngCallbackEventType, UINT32 dparam)
{
	mainapp_printf("[Ptn Callback] Type:[%d]\n", ePtnMngCallbackEventType);
	switch (ePtnMngCallbackEventType)
	{
	case PTNMNG_CALLBACK_UPDATE_FINISH:
		MAINAPP_SendGlobalEvent(CBK_EVENT_PTNMNG_CALLBACK_UPDATE_FINISH, 0);
		break;
	case PTNMNG_CALLBACK_ONE_PTN_PARSE_DONE:
		MAINAPP_SendGlobalEvent(CBK_EVENT_PTNMNG_CALLBACK_ONE_PTN_PARSE_DONE, dparam);
		break;
	case PTNMNG_CALLBACK_LOAD_PROGRESS:
		break;
	case PTNMNG_CALLBACK_MOUNT_DONE:
		MAINAPP_SendGlobalEvent(CBK_EVENT_PTNMNG_CALLBACK_MOUNT_DONE, dparam);
		break;
	case PTNMNG_CALLBACK_MOUNT_FAIL:
		MAINAPP_SendGlobalEvent(CBK_EVENT_PTNMNG_CALLBACK_MOUNT_FAIL, dparam);
		break;
	default:
		break;
	}

	return 0;
}

#ifdef SUPPORT_RESUME
static UINT32 RsmCallBackFunc(void)
{
	if (PSREG_VALUE(PSREG_SETUP_Resume) == PSREG_SETUP_Resume_On)
		return TRUE;
	else if (PSREG_VALUE(PSREG_SETUP_Resume) == PSREG_SETUP_Resume_Off)
		return FALSE;
	else //added by ted.chen. if "Power On Resume" item is removed from Setup Menu, then enable resume by default
		return TRUE;
}
#endif

#ifdef PLFNV_SUPPORT_DIVX_DRM
static int DivXDRM_CallbackGet(BYTE * buf, int offset, int length)
{
#ifdef SUPPORT_NVM_DRV_VER1
	return NVM_Read_Section(NVM_SECT_DRM_ID, offset, buf, length);
#else
	return 0;
#endif
}

static int DivXDRM_CallbackSave(BYTE * buf, int offset, int length)
{
#ifdef SUPPORT_NVM_DRV_VER1
	return NVM_Write_Section(NVM_SECT_DRM_ID, offset, buf, length);
#else
	return 0;
#endif
}

static void DivXDRM_Initialize(void)
{
#ifdef SUPPORT_NVM_DRV_VER1
	void *pDrmInfo = GL_MemAlloc(DRM_INF_SIZE);
	INT32 sdRet = NVM_SUCCESS;

	if (pDrmInfo)
	{
		sdRet = NVM_Init_Section_Params(NVM_SECT_DRM_ID, pDrmInfo, DRM_INF_SIZE);
		if (NVM_SUCCESS == sdRet)
		{
			sdRet = NVM_Init_Section_DMem(NVM_SECT_DRM_ID);
			if (NVM_SUCCESS != sdRet)
				mainapp_error("[%d] nvm init section dmem\n", sdRet);
		}
		else
			mainapp_error("[%d] nvm init section param\n", sdRet);
	}
#endif
	DivXDRM_Register_GetMemCallback(DivXDRM_CallbackGet);
	DivXDRM_Register_SaveMemCallback(DivXDRM_CallbackSave);
}

static void DivXDRM_Finalize(void)
{
#ifdef SUPPORT_NVM_DRV_VER1
	BYTE *pDrmInfo = NULL;
	UINT32 dSize = 0;
	INT32 sdRet = NVM_SUCCESS;
	sdRet = NVM_Get_Sect_Info(NVM_SECT_DRM_ID, &pDrmInfo, &dSize);
	if (NVM_SUCCESS == sdRet)
		GL_MemFree(pDrmInfo);
	else
		mainapp_error("[%d] nvm get section info\n", sdRet);
#endif
}
#endif

int MAINAPP_SetGUIObjList(void)
{
	pMainAppInstance->pGUIObjectTable = MAIN_APP_GUI_Obj_List;
	return MAIN_APP_SUCCESS;
}

static UINT32 MainApp_CheckTVSysItemExisted(UINT32 dTVSys)
{
	UINT32 dItemExisted=0;
	switch(dTVSys)
	{
		case PSREG_SETUP_NTSC:
			if(SetupMenu_FindItem(L3_ITEM_NTSC,0))
				dItemExisted=1;
			break;
		case PSREG_SETUP_PAL:
			if(SetupMenu_FindItem(L3_ITEM_PAL,0))
				dItemExisted=1;
			break;
		case PSREG_SETUP_TV_System_24Hz: //if TV sys was set to this one, Setup Menu must have 24Hz related item
			dItemExisted=1;
			break;
	}

	return dItemExisted;
}

static UINT32 MainApp_CheckResItemExisted(UINT32 dRes)
{
	UINT32 dItemExisted=0;
	switch(dRes)
	{
		case PSREG_SETUP_480i_576i:
			if(SetupMenu_FindItem(L3_ITEM_480i_576i,0))
				dItemExisted=1;
			break;
		case PSREG_SETUP_480p_576p:
			if(SetupMenu_FindItem(L3_ITEM_480p_576p,0))
				dItemExisted=1;
			break;
		case PSREG_SETUP_720p:
			if(SetupMenu_FindItem(L3_ITEM_720p,0))
				dItemExisted=1;
			break;
		case PSREG_SETUP_1080i:
			if(SetupMenu_FindItem(L3_ITEM_1080i,0))
				dItemExisted=1;
			break;
		case PSREG_SETUP_1080p:
			if(SetupMenu_FindItem(L3_ITEM_1080p,0))
				dItemExisted=1;
			break;
	}

	return dItemExisted;
}

void MainApp_AutoSettingsBW(MainAppUIEventHandlerInfo_t * pstEventInfo, INT32 sdTempFormat)
{
	if(!pstEventInfo || !pstEventInfo->dIsBWMode32BitG2DOnly)
	{
		if(pstEventInfo && pstEventInfo->bInETMode)
		{//added by ted.chen
			if (sdTempFormat >= FMT_720P_60)
				GL_AVCSelectBW(BW_MODE_8203D_HD_NET);
			else
				GL_AVCSelectBW(BW_MODE_8203D_SD_NET);	
		}
		else
		{
			if (sdTempFormat >= FMT_720P_60)
				GL_AVCSelectBW(BW_MODE_8203D_HD);// switch to 8203D configuration - > same as 1500, (joeypan, 2009/12/16)
			else
				GL_AVCSelectBW(BW_MODE_8203D_SD);
		}
	}
}

ResolutionFormat_t MainApp_PSR2VIDC(UINT32 dPSRTVSys, UINT32 dPSRRes)
{
	ResolutionFormat_t sdResFormat=-1;
	switch (dPSRTVSys) //only map PSREG_SETUP_XXX to MID_DISP_Format_e
	{
		case PSREG_SETUP_PAL:
			switch (dPSRRes)
			{
			case PSREG_SETUP_480i_576i:
				sdResFormat = VIDC_720x576i_50hz_43;
				break;
			case PSREG_SETUP_480p_576p:
				sdResFormat = VIDC_720x576p_50hz_43;
				break;
			case PSREG_SETUP_720p:
				sdResFormat = VIDC_1280x720p_50hz_169;
				break;
			case PSREG_SETUP_1080i:
				sdResFormat = VIDC_1920x1080i_50hz_169;
				break;
			case PSREG_SETUP_1080p:
				sdResFormat = VIDC_1920x1080p_50hz_169;
				break;
			}
			break;

		case PSREG_SETUP_NTSC:
			switch (dPSRRes)
			{
			case PSREG_SETUP_480i_576i:
				sdResFormat = VIDC_720x480i_60hz_43;
				break;
			case PSREG_SETUP_480p_576p:
				sdResFormat = VIDC_720x480p_60hz_43;
				break;
			case PSREG_SETUP_720p:
				sdResFormat = VIDC_1280x720p_60hz_169;
				break;
			case PSREG_SETUP_1080i:
				sdResFormat = VIDC_1920x1080i_60hz_169;
				break;
			case PSREG_SETUP_1080p:
				sdResFormat = VIDC_1920x1080p_60hz_169;
				break;
			}
			break;

		case PSREG_SETUP_TV_System_24Hz:
			sdResFormat = VIDC_1920x1080p_24hz_169;
			break;
	}
	return sdResFormat;
}

UINT32 MainApp_VIDC2PSRRes(ResolutionFormat_t eVIDC)
{
	UINT32 dPSRRes=-1;
	switch (eVIDC)	//map Video ID code to PSREG_SETUP_XXX
	{
		case VIDC_720x480i_60hz_43:
		case VIDC_720x576i_50hz_43:
			dPSRRes = PSREG_SETUP_480i_576i;
			break;

		case VIDC_720x480p_60hz_43:
		case VIDC_720x576p_50hz_43:
			dPSRRes = PSREG_SETUP_480p_576p;
			break;

		case VIDC_1280x720p_60hz_169:
		case VIDC_1280x720p_50hz_169:
			dPSRRes = PSREG_SETUP_720p;
			break;

		case VIDC_1920x1080i_60hz_169:
		case VIDC_1920x1080i_50hz_169:
			dPSRRes = PSREG_SETUP_1080i;
			break;

		case VIDC_1920x1080p_60hz_169:
		case VIDC_1920x1080p_50hz_169:
			dPSRRes = PSREG_SETUP_1080p;
			break;

		case VIDC_1920x1080p_24hz_169:
			break;

		default:
			mainapp_printf("[%s]: ERROR: all Video ID Code should be mapped, %d not mapped\n", __FUNCTION__, eVIDC);
			break;
	}
	return dPSRRes;
}

void MainApp_SetupAutoSettings(MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	if (PSREG_IS_INVALID(PSREG_SETUP_YCbCrHDMI_Resolution))
	{
		PSREG_SET_VALID(PSREG_SETUP_YCbCrHDMI_Resolution);
		PSREG_SET_VALUE(PSREG_SETUP_YCbCrHDMI_Resolution, PSREG_SETUP_Resolution_Auto);
	}
	if (PSREG_IS_INVALID(PSREG_SETUP_TV_System))
	{
		PSREG_SET_VALID(PSREG_SETUP_TV_System);
		PSREG_SET_VALUE(PSREG_SETUP_TV_System, PSREG_SETUP_TV_System_Auto);
	}
	if (PSREG_IS_INVALID(PSREG_SETUP_HDMI_1080p_24Hz))
	{
		PSREG_SET_VALID(PSREG_SETUP_HDMI_1080p_24Hz);
		PSREG_SET_VALUE(PSREG_SETUP_HDMI_1080p_24Hz, PSREG_SETUP_HDMI_24Hz_Off);
	}

	UINT32 dTempRes = PSREG_VALUE(PSREG_SETUP_YCbCrHDMI_Resolution);
	UINT32 dTempTVSys = PSREG_VALUE(PSREG_SETUP_TV_System);

	ResolutionFormat_t eTempResFormat = 0;
	//if HDMI's not connected, this condition won't be positive
	//Then dTempRes is set to default resolution later
	if (HDMI_IF_Get_PreferResolution(&eTempResFormat) == HDMI_OPERATION_SUCCESS)
	{
		mainapp_printf("[%s]: TV prefer format (ResolutionFormat_t) =%d  \n", __FUNCTION__, eTempResFormat);

		//assuming TV prefer format won't be 24Hz
		if (eTempResFormat <= VIDC_1920x1080p_60hz_169 && dTempTVSys == PSREG_SETUP_TV_System_Auto)
		{//VIDC_1920x1080p_60hz_169 is the last Video ID code of NTSC
			dTempTVSys = PSREG_SETUP_NTSC;
			if(!MainApp_CheckTVSysItemExisted(dTempTVSys)) //Setup Menu didn't have corresponding item
				dTempTVSys=PSREG_SETUP_TV_System_Auto;
		}
		else if (eTempResFormat <= VIDC_1920x1080p_50hz_169 && dTempTVSys == PSREG_SETUP_TV_System_Auto)
		{//VIDC_1920x1080p_50hz_169 is the last Video ID code of PAL
			dTempTVSys = PSREG_SETUP_PAL;
			if(!MainApp_CheckTVSysItemExisted(dTempTVSys)) //Setup Menu didn't have corresponding item
				dTempTVSys=PSREG_SETUP_TV_System_Auto;
		}
	}

	UINT8 bEDIDSupport24Hz = 0;
	ResolutionFormat_t eTemp24HzFormat = VIDC_1920x1080p_24hz_169;
	HDMI_IF_ResolutionChecked(eTemp24HzFormat, &bEDIDSupport24Hz);

	//if HDMI's not connected, this condition will not be positive
	if ((eTempResFormat == VIDC_1920x1080p_60hz_169 || eTempResFormat == VIDC_1920x1080p_50hz_169) &&
		PSREG_VALUE(PSREG_SETUP_HDMI_1080p_24Hz) == PSREG_SETUP_HDMI_24Hz_On && bEDIDSupport24Hz == 1 &&
		(PSREG_VALUE(PSREG_STAT_VideoFrameRate) == PSREG_VIDEO_FRAME_RATE_24 || 
		PSREG_VALUE(PSREG_STAT_VideoFrameRate) == PSREG_VIDEO_FRAME_RATE_23_976))
		dTempTVSys = PSREG_SETUP_TV_System_24Hz;

	if (dTempTVSys == PSREG_SETUP_TV_System_Auto)
	{
		switch(PSREG_VALUE(PSREG_STAT_VideoFrameRate))
		{
			case PSREG_VIDEO_FRAME_RATE_OTHER: //no content in or free frame rate
			case PSREG_VIDEO_FRAME_RATE_23_976:
			case PSREG_VIDEO_FRAME_RATE_24:
				dTempTVSys=dDefaultTVSys;
				break;
				
			case PSREG_VIDEO_FRAME_RATE_25:
			case PSREG_VIDEO_FRAME_RATE_50:
				dTempTVSys = PSREG_SETUP_PAL;
				dDefaultTVSys=dTempTVSys;
				break;

			case PSREG_VIDEO_FRAME_RATE_29_97:
			case PSREG_VIDEO_FRAME_RATE_59_94:
				dTempTVSys = PSREG_SETUP_NTSC;
				dDefaultTVSys=dTempTVSys;
				break;
		}

		if(!MainApp_CheckTVSysItemExisted(dTempTVSys)) //Setup Menu didn't have corresponding item
			dTempTVSys=dDefaultTVSys;
	}

	if (eTempResFormat && dTempRes == PSREG_SETUP_Resolution_Auto)	//if prefer format got
	{
		UINT32 dMapResult=MainApp_VIDC2PSRRes(eTempResFormat);
		if(dMapResult!=-1)
			dTempRes=dMapResult;

		if(!MainApp_CheckResItemExisted(dTempRes)) //Setup Menu didn't have corresponding item
			dTempRes=PSREG_SETUP_Resolution_Auto;
	}

	if (dTempRes == PSREG_SETUP_Resolution_Auto)
		dTempRes = PSREG_SETUP_480p_576p;	//set dTempRes to default resolution

	if(eTempResFormat) //suggest prefer format got = HDMI plugged
	{
		UINT32 sdOriginalVIDC = MainApp_PSR2VIDC(dTempTVSys, dTempRes);
		UINT32 sdTargetVIDC =-1;
		UINT8 bIsFound=0;
		HDMI_IF_SpecificEdidTablechecked(EDID_TYPE_VideoTiming, sdOriginalVIDC, &sdTargetVIDC, &bIsFound);
		if(bIsFound)
			dTempRes=MainApp_VIDC2PSRRes(sdTargetVIDC);
	}

	UINT8 bIsHDRes=0;
	INT32 sdTempFormat = -1;
	switch (dTempTVSys) //only map PSREG_SETUP_XXX to MID_DISP_Format_e
	{
	case PSREG_SETUP_PAL:
		switch (dTempRes)
		{
		case PSREG_SETUP_480i_576i:
			sdTempFormat = FMT_576I_50;
			break;
		case PSREG_SETUP_480p_576p:
			sdTempFormat = FMT_576P_50;
			break;
		case PSREG_SETUP_720p:
			sdTempFormat = FMT_720P_50;
			bIsHDRes=1;
			break;
		case PSREG_SETUP_1080i:
			sdTempFormat = FMT_1080I_50;
			bIsHDRes=1;
			break;
		case PSREG_SETUP_1080p:
			sdTempFormat = FMT_1080P_50;
			bIsHDRes=1;
			break;
		}
		break;

	case PSREG_SETUP_NTSC:
		switch (dTempRes)
		{
		case PSREG_SETUP_480i_576i:
			sdTempFormat = FMT_480I_60;
			break;
		case PSREG_SETUP_480p_576p:
			sdTempFormat = FMT_480P_60;
			break;
		case PSREG_SETUP_720p:
			sdTempFormat = FMT_720P_60;
			bIsHDRes=1;
			break;
		case PSREG_SETUP_1080i:
			sdTempFormat = FMT_1080I_60;
			bIsHDRes=1;
			break;
		case PSREG_SETUP_1080p:
			sdTempFormat = FMT_1080P_60;
			bIsHDRes=1;
			break;
		}
		break;

	case PSREG_SETUP_TV_System_24Hz:
		sdTempFormat = FMT_1080P_24;
		break;
	}
	mainapp_printf("[%s]: Auto MID_DISP_Format_e (Res & TV Sys) =%d  \n", __FUNCTION__, sdTempFormat);

	MainApp_AutoSettingsBW(pstEventInfo, sdTempFormat);

	DISP_Win_t stPanelWin;
	MID_DISP_Format_e eCurrFormat;
	MID_DISP_GetDmixPlane(&eCurrFormat, &stPanelWin);

	SetupConfig_Param_t* pstSetupConfigParam=(SetupConfig_Param_t *)GL_MemAlloc(sizeof(SetupConfig_Param_t));
	if(!pstSetupConfigParam)
		mainapp_printf("[%s]: ERROR: malloc fail \n", __FUNCTION__);

	if (sdTempFormat != eCurrFormat)
	{
		MID_DISP_SpanMode_e sdSpanMode;
		UINT8 bDAR;
		UINT8 bExpMode;
		UINT32 dAspectRatioExt;
		MID_DISP_GetDispSpanMode(&sdSpanMode);
		MID_DISP_GetDispAspect(&bExpMode,&bDAR,&dAspectRatioExt);
		if(bIsHDRes && bDAR==DAR_4_3 && bExpMode==EXP_DAR && dAspectRatioExt==0 &&
			(sdSpanMode==SPAN_PANSCAN || sdSpanMode==SPAN_LETTERBOX))
		{
			SetupMenu_FindItem(L3_ITEM_169_Normal,1); //default select to L3_ITEM_169_Normal
			SetupMenu_SetFuncAspectRatio(PSREG_SETUP_169_Normal, pstSetupConfigParam, SPAN_LETTERBOX, DAR_16_9);

			if(pstEventInfo)
				if(pstEventInfo->dAppEvent==DMN_EVENT_HDMI_EDID_VALID) //if HDMI plugged in and the prefer format is HD
					_MainApp_SetupRelatedMenu2MainMenu(pstEventInfo, MAINAPP_SETUPMENU); 
		}

		pstSetupConfigParam->sdMidDispFormat = sdTempFormat;
		SetDacDisable(DAC_ENABLE,DAC_DISABLE,DAC_DISABLE,DAC_DISABLE); //for avoiding momentary noise display & Mantis 0118663
		SetupConfig_IF_YCbCrHDMIResolution((UINT32) pstSetupConfigParam);
		SetDacDisable(DAC_ENABLE,DAC_ENABLE,DAC_ENABLE,DAC_ENABLE);
	}

	if(pstSetupConfigParam)
	{
		GL_MemFree(pstSetupConfigParam);
		pstSetupConfigParam=NULL;
	}
}

static INT32 _MainApp_GetToBeActivatedSysAppIdx(MainAppUIEventHandlerInfo_t * pstEventInfo, UINT32 * pdSystemIdx)
{
	INT32 sdRet = 0;
	UINT32 dDeviceType = 0;
	MediaType_e eMediaType = 0;
	UINT32 dIsActivateSysApp = 1;
	UINT32 dSystemIdx = 0;

	sdRet |= AppPtn_GetDevType(pstEventInfo->stSysDriveCtrlInfo.stSysCurPtnHnd, &dDeviceType);
	sdRet |= AppPtn_GetMediaType(pstEventInfo->stSysDriveCtrlInfo.stSysCurPtnHnd, &eMediaType);
	if(sdRet != APPPTN_SUCCESSFUL)
	{
		mainapp_error("AppPtn API Error!!! Code:%d\n", sdRet);
		return MAIN_APP_ACTIVATE_FAILED;
	}

	/*1st step: select which sys app should be activated */
	switch (dDeviceType)
	{
	case DEVICE_TYPE_USB:
		/*if usb drive, now it will active file app */
		dSystemIdx = SYS_APP_FILE_PLAYER;
		DI_DiscStop(gPhisycalLoaderDIHandle);//jiangxl for mantis 120536
		break;
	case DEVICE_TYPE_CARD:
		/*if card drive, now it will active file app */
		dSystemIdx = SYS_APP_FILE_PLAYER;
		DI_DiscStop(gPhisycalLoaderDIHandle);
		break;
	case DEVICE_TYPE_HDD:
		/*if hdd drive, now it will active file app */
		dSystemIdx = SYS_APP_FILE_PLAYER;
		DI_DiscStop(gPhisycalLoaderDIHandle);
		break;

	case DEVICE_TYPE_DISC:
		{
			switch (eMediaType)
			{
			case MEDIA_TYPE_DATA_DISC:
				dSystemIdx = SYS_APP_FILE_PLAYER;
				break;
			case MEDIA_TYPE_CDDA:
				dSystemIdx = SYS_APP_FILE_PLAYER;
				break;
			case MEDIA_TYPE_VCD:
				dSystemIdx = SYS_APP_DISC_PLAYER;
				break;
			case MEDIA_TYPE_SVCD:
				dSystemIdx = SYS_APP_DISC_PLAYER;
				break;
			case MEDIA_TYPE_DVD:
				dSystemIdx = SYS_APP_DISC_PLAYER;
				break;
			case MEDIA_TYPE_MINUSVR:
				dSystemIdx = SYS_APP_DISC_PLAYER;
				break;
			case MEDIA_TYPE_PLUSVR:
				dSystemIdx = SYS_APP_DISC_PLAYER;
				break;
			case MEDIA_TYPE_HDMV:
				dSystemIdx = SYS_APP_DISC_PLAYER;
				break;
			case MEDIA_TYPE_BD:
				dSystemIdx = SYS_APP_DISC_PLAYER;
				break;
#ifdef KOK_SUPPORT
			case MEDIA_TYPE_KOK:
				dSystemIdx = SYS_APP_FILE_PLAYER;
				break;
#endif
			default:
				mainapp_error("unknown media type for Disc drive,pls prepare it before activate sys app\n");
				dIsActivateSysApp = 0;
				break;
			}
		}
		break;

	default:
		mainapp_error("pls prepare sys activate drive/program source before activate sys app\n");
		dIsActivateSysApp = 0;
		break;
	}
	if (dIsActivateSysApp)
	{
		mainapp_printf("activate sys app : %d\n", dSystemIdx);
		*pdSystemIdx = dSystemIdx;
		return MAIN_APP_SUCCESS;
	}
	else
		return MAIN_APP_ACTIVATE_FAILED;
}

static INT32 _MainApp_AutoActivateFileApp(MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	FileAppCtrlInfo_t *pstFileAppCtrlInfo = &pstEventInfo->stSysAppCtrlInfo.stFileAppCtrlInfo;
	UINT32 dIsNeedAutoSwSysApp = 0;
	UINT32 eSysCurDrvSupportFilter = 0;
	MediaType_e eMediaType = 0;
	UINT32 dDeviceType = DEVICE_TYPE_MAX;
	INT32 sdRet = 0;

	sdRet |= AppPtn_GetFilter(pstEventInfo->stSysDriveCtrlInfo.stSysCurPtnHnd, &eSysCurDrvSupportFilter);
	sdRet |= AppPtn_GetMediaType(pstEventInfo->stSysDriveCtrlInfo.stSysCurPtnHnd, &eMediaType);
	sdRet |= AppPtn_GetDevType(pstEventInfo->stSysDriveCtrlInfo.stSysCurPtnHnd, &dDeviceType);
	if(sdRet!=APPPTN_SUCCESSFUL)
		return MAIN_APP_ACTIVATE_FAILED;

	// switch servo speed to x3.4, for HD video file in data disc.
	if ((dDeviceType == DEVICE_TYPE_DISC) && (eSysCurDrvSupportFilter & PTNMNG_FILTER_VIDEO))
	{
		#ifdef SERVO_SPEED_AUTO_CHANGE//first strat from 2X. if needed in play will set to 3.4X
		sdRet = DI_ChangeServoSpeed(gPhisycalLoaderDIHandle, DI_ServoNormalSpeed_2X);
		#else
		sdRet = DI_ChangeServoSpeed(gPhisycalLoaderDIHandle, DI_ServoMediumSpeed_3DOT4X);
		#endif
		if(sdRet != DIMON_IF_SUCCESSFUL)
			mainapp_printf("DI_ChangeServoSpeed fail [%d] \n",sdRet);
	}

                #ifdef USB_MODE_POWEROFF_SERVO
		if((dDeviceType == DEVICE_TYPE_USB)||(dDeviceType == DEVICE_TYPE_HDD)||(dDeviceType == DEVICE_TYPE_CARD))		
		{
		pstEventInfo->bDevmode=1;
		AppPtn_DiscEject(DI_Handle_PHYSICAL);
		DI_DiscStandBy(gPhisycalLoaderDIHandle);
		}
                #endif

	switch (eSysCurDrvSupportFilter & ~(PTNMNG_FILTER_ANYTHING|PTNMNG_FILTER_BIN))
	{
	case 0:
		/*if 0, it means nothing found, so do nothing */
		break;
		/*if has one filter auto switch to the sys app */
	case PTNMNG_FILTER_VIDEO:
		ComGObj_IF_Main_SetHili(MAINMENU_FUNC_HOME_MOVIE);
		pstFileAppCtrlInfo->eFileAppType = FILE_APP_VIDEO;
		pstEventInfo->uIsAutoActiveFileApp = 1;
		dIsNeedAutoSwSysApp = 1;
		break;
	case PTNMNG_FILTER_AUDIO:
		ComGObj_IF_Main_SetHili(MAINMENU_FUNC_HOME_MUSIC);
		switch(eMediaType)
		{
			case MEDIA_TYPE_CDDA:
				pstFileAppCtrlInfo->eFileAppType = FILE_APP_CDDA;
				dIsNeedAutoSwSysApp = 1;
				break;
			default:
				pstFileAppCtrlInfo->eFileAppType = FILE_APP_AUDIO;
				dIsNeedAutoSwSysApp = 1;
				break;
		}
		break;
	case PTNMNG_FILTER_PHOTO:
		ComGObj_IF_Main_SetHili(MAINMENU_FUNC_HOME_PHOTO);
		pstFileAppCtrlInfo->eFileAppType = FILE_APP_PHOTO;
		dIsNeedAutoSwSysApp = 1;
		break;
	case PTNMNG_FILTER_TEXT:
		ComGObj_IF_Main_SetHili(MAINMENU_FUNC_HOME_EBOOK);
		pstFileAppCtrlInfo->eFileAppType = FILE_APP_EBOOK;
		dIsNeedAutoSwSysApp = 1;
		break;
#ifdef KOK_SUPPORT
	case PTNMNG_FILTER_KOK:
		ComGObj_IF_Main_SetHili(MAINMENU_FUNC_HOME_KARAOKE);
		pstFileAppCtrlInfo->eFileAppType = FILE_APP_KOK;
		mainapp_printf("eFileAppType : FILE_APP_KOKN32\n");
		dIsNeedAutoSwSysApp = 1;
		break;
#endif
	default:
		break;
	}
	// check bin file
	if (eSysCurDrvSupportFilter & PTNMNG_FILTER_BIN)
	{
		mainapp_printf("to find rom.bin file\n");
		memset(pstEventInfo->stSysAppCtrlInfo.stSmartUpgradeInfo.UpgradePath, 0, DEVICE_PATH_LENGTH);
		sdRet = AppPtn_GetMountPoint(
					pstEventInfo->stSysDriveCtrlInfo.stSysCurPtnHnd,
					pstEventInfo->stSysAppCtrlInfo.stSmartUpgradeInfo.UpgradePath); //get dev path

		if (sdRet == APPPTN_SUCCESSFUL)
		{
			strcat(pstEventInfo->stSysAppCtrlInfo.stSmartUpgradeInfo.UpgradePath, UPGRADE_FILE_NAME);
			mainapp_printf("upgrade bin file path : %s \n",pstEventInfo->stSysAppCtrlInfo.stSmartUpgradeInfo.UpgradePath);

			sdRet = SmartUpgrade_GetRomVerFromUSB(
						pstEventInfo->stSysAppCtrlInfo.stSmartUpgradeInfo.UpgradePath,
						pstEventInfo->stSysAppCtrlInfo.stSmartUpgradeInfo.UpgradeVersion);
			if (sdRet != ISP_SUCCESS)
				mainapp_printf("Open %s fail !!!\n",pstEventInfo->stSysAppCtrlInfo.stSmartUpgradeInfo.UpgradePath);
			else
			{
				if (GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU))
					GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU);

				if (GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_SMART_UPGRADTE))
					GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_SMART_UPGRADTE);

				pstEventInfo->stSysAppCtrlInfo.stSmartUpgradeInfo.stFirst = ISP_IF_NO_ACTIVE_DISCSYS;
				PSREG_SET_VALUE(PSREG_STAT_PlayMode, PSREG_PLAYMODE_PLAY);//for screensaver
				GOBJ_MGR_CreateObject(
					pMainAppInstance->pGUIObjectTable[MAINAPP_SMART_UPGRADTE],
					pMainAppInstance->GUIObjData,
					MAINAPP_SMART_UPGRADTE,
					(UINT32) pstEventInfo);
			    dIsNeedAutoSwSysApp=0;//add by wumin for mantis 104959
			}

			eSysCurDrvSupportFilter &= (~PTNMNG_FILTER_BIN);
			AppPtn_SetFilter(
				pstEventInfo->stSysDriveCtrlInfo.stSysCurPtnHnd,
				eSysCurDrvSupportFilter,
				APPPTN_SET_FILTER_SET);
		}
	}

	if (dIsNeedAutoSwSysApp)
		return MAINAPP_ActivateSystemApp(SYS_APP_FILE_PLAYER, (UINT32) pstEventInfo);
	else
		return MAIN_APP_ACTIVATE_FAILED;
}

static INT32 _MainApp_AutoActivateCurSysApp(MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	UINT32 dSystemIdx = 0;
	INT32 sdRet = MAIN_APP_SUCCESS;
	if (_MainApp_GetToBeActivatedSysAppIdx(pstEventInfo, &dSystemIdx) == MAIN_APP_SUCCESS)
	{
		switch (dSystemIdx)
		{
		case SYS_APP_DISC_PLAYER:
			sdRet = MAINAPP_ActivateSystemApp(SYS_APP_DISC_PLAYER, (UINT32) pstEventInfo);

			SysAppActiveAckCallBack=_MainApp_DiscInsertEvent;//fix mantis 111779,aerliqu 20101118
			break;
		case SYS_APP_FILE_PLAYER:
			sdRet = _MainApp_AutoActivateFileApp(pstEventInfo);
			break;
		default:
			mainapp_error("%s invalidate to be activate sys app index[%d].\n", __FUNCTION__, dSystemIdx);
			break;
		}
	}
	else
	{
		mainapp_error("%s get to be activate sys app index.\n", __FUNCTION__);
		sdRet = MAIN_APP_ACTIVATE_FAILED;
	}

	return sdRet;
}

static INT32 _MainApp_ResumeActivateFileApp(MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	MediaType_e eMediaType = 0;
	PtnMngFilterType_t stCurFilter = 0;
	FileAppCtrlInfo_t *pstFileAppCtrlInfo = &pstEventInfo->stSysAppCtrlInfo.stFileAppCtrlInfo;
	UINT32 dIsNeedResumeActivate = 0;
	AppPtn_GetFilter(pstEventInfo->stSysDriveCtrlInfo.stSysCurPtnHnd, &stCurFilter);

	switch (pstFileAppCtrlInfo->eFileAppType)
	{
	case FILE_APP_VIDEO:
		dIsNeedResumeActivate = stCurFilter&PTNMNG_FILTER_VIDEO;
		break;
	case FILE_APP_AUDIO:
		dIsNeedResumeActivate = stCurFilter&PTNMNG_FILTER_AUDIO;
		break;
	case FILE_APP_PHOTO:
		dIsNeedResumeActivate = stCurFilter&PTNMNG_FILTER_PHOTO;
		break;
	case FILE_APP_EBOOK:
		dIsNeedResumeActivate = stCurFilter&PTNMNG_FILTER_TEXT;
		break;
	#ifdef GAME_SUPPORT
	case FILE_APP_GAME:
		dIsNeedResumeActivate = stCurFilter&(PTNMNG_FILTER_NES|PTNMNG_FILTER_NATIVE32|PTNMNG_FILTER_MD);
		break;
	#endif
	case FILE_APP_CDDA:
		AppPtn_GetMediaType(pstEventInfo->stSysDriveCtrlInfo.stSysCurPtnHnd, &eMediaType);
		if (eMediaType == MEDIA_TYPE_CDDA)
		{
			dIsNeedResumeActivate = 1;
		}
		break;
	default:
		break;
	}

	if (dIsNeedResumeActivate)
		return MAINAPP_ActivateSystemApp(SYS_APP_FILE_PLAYER, (UINT32) pstEventInfo);
	else
		return MAIN_APP_ACTIVATE_FAILED;
}

static INT32 _MainApp_ResumeActivateCurSysApp(MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	UINT32 dSystemIdx = 0;
	INT32 sdRet = MAIN_APP_SUCCESS;

	if (_MainApp_GetToBeActivatedSysAppIdx(pstEventInfo, &dSystemIdx) == MAIN_APP_SUCCESS)
	{
		switch (dSystemIdx)
		{
		case SYS_APP_DISC_PLAYER:
			sdRet = MAINAPP_ActivateSystemApp(SYS_APP_DISC_PLAYER, (UINT32) pstEventInfo);

			SysAppActiveAckCallBack=_MainApp_DiscInsertEvent;//fix mantis 111779,aerliqu 20101118
			break;
		case SYS_APP_FILE_PLAYER:
			sdRet = _MainApp_ResumeActivateFileApp(pstEventInfo);
			break;
		#ifdef SUPPORT_PLAYER_STB_DVB_T //xh.song add for the second click setup not entry the dvb if last sysapp is dvb 2010-4-19 15:11
		case SYS_APP_DVB:
			sdRet = _MainApp_ActivateCurSysAppWithDVBAppType(pstEventInfo);
		#endif
		default:
			mainapp_error("%s invalidate to be activate sys app index[%d].\n", __FUNCTION__, dSystemIdx);
			break;
		}
	}
	else
	{
		mainapp_error("%s get to be activate sys app index.\n", __FUNCTION__);
		sdRet = MAIN_APP_ACTIVATE_FAILED;
	}

	return sdRet;
}
#if (SUPPORT_BDMV_USB_LOAD==1)
	#include <stdio.h>
	#include <dirent.h>
	#define BDMV_PATH_NAME "/BDROM/BDMV"

static INT32 _MainApp_ActivateCurSysAppWithUSBLoadDiscPlayType(MainAppUIEventHandlerInfo_t * pstEventInfo)
{

	INT32 sdRet = MAIN_APP_SUCCESS;

	if(sdRet != APPPTN_SUCCESSFUL)
	{
		mainapp_error("AppPtn API Error!!! Code:%d\n", sdRet);
		return MAIN_APP_ACTIVATE_FAILED;
	}



	char szPath[FILEMW_PARTITION_PATH_MAX_LEN+sizeof(BDMV_PATH_NAME)];
	AppPtn_GetMountPoint(pstEventInfo->stSysDriveCtrlInfo.stSysCurPtnHnd, szPath);
	strcat(szPath, BDMV_PATH_NAME);
		diag_printf("_MainApp_ActivateCurSysAppWithFileAppType\n");

	diag_printf("szPath=%s\n",szPath);
	if(opendir(szPath))
	{
		stUIEventHandlerInfo.bUsb_load_bdmv_enable=1;
	MAINAPP_ActivateSystemApp(SYS_APP_DISC_PLAYER,(UINT32)&stUIEventHandlerInfo);
	sdRet =SYSAPP_IF_SendCriticalGlobalEventWithIndex(SYS_APP_DISC_PLAYER,DMN_EVENT_DISC_INSERTED, 0);//add by george.chang 2010.10.04


	}

	return sdRet;
}
#endif
static INT32 _MainApp_ActivateCurSysAppWithFileAppType(MainAppUIEventHandlerInfo_t * pstEventInfo, FileAppType_e eFileAppType)
{
	UINT32 dSystemIdx = 0;
	INT32 sdRet = MAIN_APP_SUCCESS;
#ifdef GAME_SUPPORT
#ifdef SUPPORT_OSD_GAME
	/*osd game is indepent with sys drive,media filter,so activate file app derectly */
	if (eFileAppType == FILE_APP_GAME)
	{
		pstEventInfo->stSysAppCtrlInfo.stFileAppCtrlInfo.eFileAppType = eFileAppType;
		sdRet = MAINAPP_ActivateSystemApp(SYS_APP_FILE_PLAYER, (UINT32) pstEventInfo);
	}
	else
#endif
#endif

#ifdef SAMBA_SUPPORT
	if(eFileAppType == FILE_APP_SMB)
	{
		pstEventInfo->stSysAppCtrlInfo.stFileAppCtrlInfo.eFileAppType = eFileAppType;
		sdRet = MAINAPP_ActivateSystemApp(SYS_APP_FILE_PLAYER, (UINT32) pstEventInfo);
	}
	else
#endif
	if (_MainApp_GetToBeActivatedSysAppIdx(pstEventInfo, &dSystemIdx) == MAIN_APP_SUCCESS)
	{
		switch (dSystemIdx)
		{
		case SYS_APP_DISC_PLAYER:
			sdRet = MAINAPP_ActivateSystemApp(SYS_APP_DISC_PLAYER, (UINT32) pstEventInfo);
			SYSAPP_IF_SendCriticalGlobalEventWithIndex(SYS_APP_DISC_PLAYER,DMN_EVENT_DISC_INSERTED, 0);//add by george.chang 2010.10.04

			break;
		case SYS_APP_FILE_PLAYER:
			pstEventInfo->stSysAppCtrlInfo.stFileAppCtrlInfo.eFileAppType = eFileAppType;
			sdRet = MAINAPP_ActivateSystemApp(SYS_APP_FILE_PLAYER, (UINT32) pstEventInfo);
			break;
		default:
			mainapp_error("%s invalidate to be activate sys app index[%d].\n", __FUNCTION__, dSystemIdx);
			break;
		}
	}
	else
	{
		mainapp_error("%s get to be activate sys app index.\n", __FUNCTION__);
		sdRet = MAIN_APP_ACTIVATE_FAILED;
	}

	return sdRet;
}

#ifdef NET_ET_SUPPORT
static INT32 _MainApp_ActivateCurSysAppWithETAppType(MainAppUIEventHandlerInfo_t * pstEventInfo, EtAppType_e eETAppType)
{
	INT32 sdRet = MAIN_APP_SUCCESS;
	/*1st : close main menu */
	if (GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU))
		GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU);

	/*2nd : close share reging top R1*/
//	ShareGobj_IF_SetRegHideMode(SHARE_REGION_TOP);
	/*3rd : activate ET app */
	pstEventInfo->stSysAppCtrlInfo.stEtAppCtrlInfo.eEtAppType = eETAppType;
	sdRet = MAINAPP_ActivateSystemApp(SYS_APP_ET_PLAYER, (UINT32) pstEventInfo);
	mainapp_printf("line: %d sdRet: %d\n", __LINE__, sdRet);
	return sdRet;
}
#endif

#ifdef NET_VOD_SUPPORT
static INT32 _MainApp_ActivateCurSysAppWithVodAppType(MainAppUIEventHandlerInfo_t * pstEventInfo, VodAppType_e eVodAppType)
{
	INT32 sdRet = MAIN_APP_SUCCESS;
	/*1st : close main menu */
	if (GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU))
	{
		GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU);
	}
	/*2nd : close share reging top R1*/
	//ShareGobj_IF_SetRegHideMode(SHARE_REGION_TOP);
	/*3rd : activate ET app */
	pstEventInfo->stSysAppCtrlInfo.stVodAppCtrlInfo.eVodAppType = eVodAppType;
	sdRet = MAINAPP_ActivateSystemApp(SYS_APP_VOD_PLAYER, (UINT32) pstEventInfo);
	mainapp_printf("line: %d sdRet: %d\n", __LINE__, sdRet);
	return sdRet;
}
#endif

#ifdef NET_N32_SUPPORT
static INT32 _MainApp_ActivateCurSysAppWithN32AppType(MainAppUIEventHandlerInfo_t * pstEventInfo, N32AppType_e eN32AppType)
{
	INT32 sdRet = MAIN_APP_SUCCESS;
	/*1st : close main menu */
	if (GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU))
		GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU);

	/*2nd : close share reging top R1*/
	ShareGobj_IF_SetRegHideMode(SHARE_REGION_TOP);
	/*3rd : activate ET app */
	pstEventInfo->stSysAppCtrlInfo.stN32AppCtrlInfo.eN32AppType = eN32AppType;
	sdRet = MAINAPP_ActivateSystemApp(SYS_APP_N32_PLAYER, (UINT32) pstEventInfo);
	mainapp_printf("line: %d sdRet: %d\n", __LINE__, sdRet);
	return sdRet;
}
#endif
#ifdef NET_NEWS_SUPPORT
static INT32 _MainApp_ActivateCurSysAppWithNewsAppType(MainAppUIEventHandlerInfo_t * pstEventInfo, UINT32 dAppType)
{
	INT32 sdRet = MAIN_APP_SUCCESS;
	if (GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU))
		GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU);

	ShareGobj_IF_SetRegHideMode(SHARE_REGION_TOP);

	sdRet = MAINAPP_ActivateSystemApp(dAppType, (UINT32) pstEventInfo);
	mainapp_printf("line: %d sdRet: %d\n", __LINE__, sdRet);
	return sdRet;
}
#endif
#ifdef NET_STOCK_SUPPORT
static INT32 _MainApp_ActivateCurSysAppWithStockAppType(MainAppUIEventHandlerInfo_t * pstEventInfo, UINT32 dAppType)
{
	INT32 sdRet = MAIN_APP_SUCCESS;
	if (GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU))
		GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU);

	ShareGobj_IF_SetRegHideMode(SHARE_REGION_TOP);

	sdRet = MAINAPP_ActivateSystemApp(dAppType, 0);
	mainapp_printf("line: %d sdRet: %d\n", __LINE__, sdRet);
	return sdRet;
}
#endif
#ifdef NET_WEATHER_SUPPORT
static INT32 _MainApp_ActivateCurSysAppWithYWAppType(MainAppUIEventHandlerInfo_t * pstEventInfo, UINT32 dAppType)
{
	INT32 sdRet = MAIN_APP_SUCCESS;
	if (GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU))
		GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU);

	ShareGobj_IF_SetRegHideMode(SHARE_REGION_TOP);

	sdRet = MAINAPP_ActivateSystemApp(SYS_APP_YWEATHER,dAppType);
	mainapp_printf("line: %d sdRet: %d\n", __LINE__, sdRet);
	return sdRet;
}
#endif
#ifdef NET_GMAP_SUPPORT
static INT32 _MainApp_ActivateCurSysAppWithGMAppType(MainAppUIEventHandlerInfo_t * pstEventInfo, UINT32 dAppType)
{
	INT32 sdRet = MAIN_APP_SUCCESS;
	if (GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU))
		GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU);

	ShareGobj_IF_SetRegHideMode(SHARE_REGION_TOP);

	sdRet = MAINAPP_ActivateSystemApp(SYS_APP_GMAP,dAppType);
	mainapp_printf("line: %d sdRet: %d\n", __LINE__, sdRet);
	return sdRet;
}
#endif
#ifdef NET_PHOTO_SUPPORT
static INT32 _MainApp_ActivateCurSysAppWithNetPhotoAppType(MainAppUIEventHandlerInfo_t * pstEventInfo, UINT32 dAppType)
{
	INT32 sdRet = MAIN_APP_SUCCESS;
	/*1st : close main menu */
	if (GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU))
		GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU);
	/*2nd : close share reging top R1*/
	ShareGobj_IF_SetRegHideMode(SHARE_REGION_TOP);
	/*3rd : activate NETPHOTO app */
	sdRet = MAINAPP_ActivateSystemApp(SYS_APP_NETPHOTO_PLAYER, (UINT32) pstEventInfo);
	mainapp_printf("line: %d sdRet: %d\n", __LINE__, sdRet);
	return sdRet;
}
#endif
#if defined(NET_RADIO_SUPPORT)||defined(NET_LOCAL_RADIO_SUPPORT)
static INT32 _MainApp_ActivateCurSysAppWithNetRadioAppType(MainAppUIEventHandlerInfo_t * pstEventInfo, UINT32 dAppType)
{
	INT32 sdRet = MAIN_APP_SUCCESS;
	/*1st : close main menu */
	if (GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU))
		GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU);
	/*2nd : close share reging top R1*/
	ShareGobj_IF_SetRegHideMode(SHARE_REGION_TOP);
	/*3rd : activate NETRADIO app */
	sdRet = MAINAPP_ActivateSystemApp(SYS_APP_NETRADIO_PLAYER, (UINT32) pstEventInfo);
	mainapp_printf("line: %d sdRet: %d\n", __LINE__, sdRet);
	return sdRet;
}
#endif

#ifdef NET_BROWSER_SUPPORT
static INT32 _MainApp_ActivateCurSysAppWithBrowserAppType(MainAppUIEventHandlerInfo_t * pstEventInfo, BrowserAppType_e eBrowserdAppType)
{
	INT32 sdRet = MAIN_APP_SUCCESS;

	/*1st : close main menu */
	if (GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU))
	{
		GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU);
	}

	/*2nd : close share reging top R1*/
	ShareGobj_IF_SetRegHideMode(SHARE_REGION_TOP);

        /*3rd : activate NETBROWSER app */
		//fix
	 pstEventInfo->stSysAppCtrlInfo.stBrowserAppCtrlInfo.eBrowserAppType = eBrowserdAppType;
	mainapp_printf("------[%d]-------------\n", eBrowserdAppType);
	sdRet = MAINAPP_ActivateSystemApp(SYS_APP_BROWSER, (UINT32) pstEventInfo);
	mainapp_printf("line: %d sdRet: %d\n", __LINE__, sdRet);
	return sdRet;
}
#endif

#ifdef NET_MUSIC_SUPPORT
static INT32 _MainApp_ActivateCurSysAppWithNetMusicAppType(MainAppUIEventHandlerInfo_t * pstEventInfo, UINT32 dAppType)
{
	INT32 sdRet = MAIN_APP_SUCCESS;
	/*1st : close main menu */
	if (GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU))
		GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU);
	/*2nd : close share reging top R1*/
	//ShareGobj_IF_SetRegHideMode(SHARE_REGION_TOP);
	/*3rd : activate NETRADIO app */
	sdRet = MAINAPP_ActivateSystemApp(SYS_APP_NETMUSIC_PLAYER, (UINT32) pstEventInfo);
	mainapp_printf("line: %d sdRet: %d\n", __LINE__, sdRet);
	return sdRet;
}
#endif

static void _MainApp_AutoSWSysDrive(MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	INT32 sdRet = APPPTN_SUCCESSFUL;
	AppPtn_Handle_t astPtnHnd[APPPTN_MAX_PTN];
	UINT32 dDrvCnt = 0;
	SysDriveCtrlInfo_t *pstSysDriveCtrlInfo = &pstEventInfo->stSysDriveCtrlInfo;

	/*do nothing when tray stay out heng.zhu_c1 added here*/
	if (pstEventInfo->eTrayState == PHYSICAL_TRAY_OUT)
		return;
	/*1st step : check whether cur sys drv is available, if yes, do nothing*/
	if(AppPtn_IsStorageAvailable(pstSysDriveCtrlInfo->stSysCurPtnHnd))
		return;
	/*2nd step : check if some dirve is mounted, if no drv mounted ,set sys drive to null */
	sdRet = AppPtn_GetAllPtnCount(&dDrvCnt);
	if(sdRet!=APPPTN_SUCCESSFUL)
	{
		mainapp_error("%s : get ptn count fail. Code:%d\n", __FUNCTION__, sdRet);
		pstSysDriveCtrlInfo->stSysCurPtnHnd = NULL;
		return;
	}

	if (dDrvCnt == 0)
	{
		//mainapp_printf("no drive is pluged in\n");
		pstSysDriveCtrlInfo->stSysCurPtnHnd = NULL;
		return;
	}

	/*3th step : auto change sys drv, now select the fist validate drv(with validate content) */
	sdRet = AppPtn_GetAllPtnList(astPtnHnd);
	if(sdRet != APPPTN_SUCCESSFUL)
		pstSysDriveCtrlInfo->stSysCurPtnHnd = NULL;
	else
		pstSysDriveCtrlInfo->stSysCurPtnHnd = astPtnHnd[0];
}

static void _MainApp_SWSysDrive(MainAppUIEventHandlerInfo_t * pstEventInfo, UINT32 dDriveType)
{
	AppPtn_Handle_t astPtnHnd[APPPTN_MAX_PTN];
	UINT32 dDrvCnt = 0;
	SysDriveCtrlInfo_t *pstSysDriveCtrlInfo = &pstEventInfo->stSysDriveCtrlInfo;

	if (APPPTN_SUCCESSFUL != AppPtn_GetPtnCountByDeviceType(dDriveType, &dDrvCnt))
	{
		mainapp_error("%s : get ptn count fail.\n", __FUNCTION__);
		return;
	}

	if (!dDrvCnt)
	{
		mainapp_printf("no drive is pluged in\n");
		return;
	}

	if (pstEventInfo->bIsHoldHomeMenu == 1) //for mantis 1178.4,hen.zhu_c1 add here 
	{
		pstEventInfo->bIsHoldHomeMenu = 0;
		return;
	}
	
	/*find 1st validate drive */
	if (APPPTN_SUCCESSFUL == AppPtn_GetPtnListByDeviceType(dDriveType, astPtnHnd))
		pstSysDriveCtrlInfo->stSysCurPtnHnd = astPtnHnd[0];
	else
		mainapp_error("%s : get ptn filter fail.\n", __FUNCTION__);
}

static void _MainApp_DataDiscInsertCallBack(MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	UINT32 dSwitch2Disc = 1;
	UINT32 dMainAppState = 0;
	if(pstEventInfo->stSysDriveCtrlInfo.stSysCurPtnHnd != NULL)
	{
		MAINAPP_GetMainAppState(&dMainAppState);
		if (dMainAppState == MAIN_APP_NORMAL)
		{
			UINT32 dSysAppIdx = TOTAL_SYS_APP_SIZE;
			INT32 sdRet = MAIN_APP_SUCCESS;
			sdRet = MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
			switch (sdRet)
			{
			case MAIN_APP_NO_ACTIVE_SYSTEM_APP:
				if (GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_SOURCE)
					||GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_SETUPMENU)
					||GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_SETUPMENUSPKR)
					||GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_SUPERSETUPMENU)
					||GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_REGIONMENU)
					||GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_CUSTOMERMENU)
#if defined(NET_WIFI_SUPPORT)
					||GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_SETUPMENUNETWORK)
#endif
					||GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU))
				{
					dSwitch2Disc = 0;
				}
				break;

			case MAIN_APP_SUCCESS:
				dSwitch2Disc = 0;
				break;

			default:
				break;
			}
		}
	}
	if (dSwitch2Disc)
		_MainApp_SWSysDrive(pstEventInfo, DEVICE_TYPE_DISC);

	MainAppDiscInsertCallBack = NULL;
}

static void _MainApp_DataDiscInsertCBK_Stop(MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	mainapp_printf("%s call DI_DiscStop !!!! \n",__FUNCTION__);
	DI_DiscStop(gPhisycalLoaderDIHandle);

	MainAppDiscInsertCallBack = NULL;
}

#if (SUPPORT_iPOD_DOCKING==1)
static int  _MainApp_ActivateiPodSysapp(MainAppUIEventHandlerInfo_t * pstEventInfo)
{
    int sdRet=0;
    diag_printf("%s called\n",__FUNCTION__);
	UINT32 dMainAppState = MAIN_APP_NORMAL;
	MAINAPP_GetMainAppState(&dMainAppState);
	if (dMainAppState == MAIN_APP_NORMAL)
	{
		UINT32 dSysAppIdx = TOTAL_SYS_APP_SIZE;
		INT32 sdRet = MAIN_APP_SUCCESS;
		/*check sys app active state. */
		sdRet = MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
		switch (sdRet)
		{
		case MAIN_APP_NO_ACTIVE_SYSTEM_APP:
			mainapp_printf("%s on NO_ACTIVE_SYSTEM_APP.\n", __FUNCTION__);
			sdRet=MAINAPP_ActivateSystemApp(SYS_APP_IPOD_PLAYER,0);
			if(sdRet==MAIN_APP_SUCCESS)
			{}
			break;

		case MAIN_APP_SUCCESS:
		    sdRet= MAINAPP_SwitchSystemApp(SYS_APP_IPOD_PLAYER,0);
		    break;

		default:
			mainapp_error("%s on %d condition.\n", __FUNCTION__, sdRet);
			break;
		}
	}
	else
		mainapp_printf("%s on main app state[%d].\n", __FUNCTION__, dMainAppState);

    if(sdRet==0)
		PSREG_SET_VALUE(PSREG_STAT_PlayMode, PSREG_PLAYMODE_PLAY);

	return sdRet;
}
#endif

static void _MainApp_GUIFBEventHander4GobjHomeMenu(UINT32 dMessage, MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	MediaType_e eMediaType = 0;
	switch (dMessage)
	{
	case GUI_RESPOND_HOMEMENU_CLICK_MOVIE:
		mainapp_printf("[HOME GOBJ RESPONSE MSG] GUI_RESPOND_HOMEMENU_CLICK_MOVIE\n");
#if (SUPPORT_BDMV_USB_LOAD==1)
		if(monflg==MONFLG_LOAD_USB_BDMV)//Load BDMB file from USB add by george.chang2011.01.11
		{
			_MainApp_ActivateCurSysAppWithUSBLoadDiscPlayType(pstEventInfo);
			break;
		}
		else
			stUIEventHandlerInfo.bUsb_load_bdmv_enable=0;
#endif
		_MainApp_ActivateCurSysAppWithFileAppType(pstEventInfo, FILE_APP_VIDEO);
		break;

	case GUI_RESPOND_HOMEMENU_CLICK_PHOTO:
		mainapp_printf("[HOME GOBJ RESPONSE MSG] GUI_RESPOND_HOMEMENU_CLICK_PHOTO\n");
		_MainApp_ActivateCurSysAppWithFileAppType(pstEventInfo, FILE_APP_PHOTO);
		break;
	case GUI_RESPOND_HOMEMENU_CLICK_MUSIC:
		mainapp_printf("[HOME GOBJ RESPONSE MSG] GUI_RESPOND_HOMEMENU_CLICK_MUSIC\n");
		AppPtn_GetMediaType(pstEventInfo->stSysDriveCtrlInfo.stSysCurPtnHnd, &eMediaType);
		switch(eMediaType)
		{
			case MEDIA_TYPE_CDDA:
				_MainApp_ActivateCurSysAppWithFileAppType(pstEventInfo, FILE_APP_CDDA);
				break;
			default:
				_MainApp_ActivateCurSysAppWithFileAppType(pstEventInfo, FILE_APP_AUDIO);
				break;
		}
		break;
	case GUI_RESPOND_HOMEMENU_CLICK_EBOOK:
		mainapp_printf("[HOME GOBJ RESPONSE MSG] GUI_RESPOND_HOMEMENU_CLICK_EBOOK\n");
		_MainApp_ActivateCurSysAppWithFileAppType(pstEventInfo, FILE_APP_EBOOK);
		break;

	#ifdef GAME_SUPPORT
	case GUI_RESPOND_HOMEMENU_CLICK_GAME:
		mainapp_printf("[HOME GOBJ RESPONSE MSG] GUI_RESPOND_HOMEMENU_CLICK_GAME\n");
		_MainApp_ActivateCurSysAppWithFileAppType(pstEventInfo, FILE_APP_GAME);
		break;
    #endif
	//xy.zhu add for kok
	#ifdef KOK_SUPPORT
	case GUI_RESPOND_HOMEMENU_CLICk_KARAOKE:
		mainapp_printf("[HOME GOBJ RESPONSE MSG] GUI_RESPOND_HOMEMENU_CLICk_KARAOKE\n");
		_MainApp_ActivateCurSysAppWithFileAppType(pstEventInfo, FILE_APP_KOK);
	break;
	#endif
#ifdef SAMBA_SUPPORT
	case GUI_RESPOND_HOMEMENU_CLICK_SMB:
		mainapp_printf("[HOME GOBJ RESPONSE MSG] GUI_RESPOND_HOMEMENU_CLICK_SMB\n");
		pstEventInfo->stSysDriveCtrlInfo.stSysPrePtnHnd = pstEventInfo->stSysDriveCtrlInfo.stSysCurPtnHnd;
		AppPtn_GetSMBPtnHandle(&pstEventInfo->stSysDriveCtrlInfo.stSysCurPtnHnd);
		if(pstEventInfo->stSysDriveCtrlInfo.stSysPrePtnHnd)
		{
			PtnMngFilterType_t eSupportFilter = 0;
			AppPtn_GetFilter(pstEventInfo->stSysDriveCtrlInfo.stSysPrePtnHnd,&eSupportFilter);
			AppPtn_SetFilter(pstEventInfo->stSysDriveCtrlInfo.stSysCurPtnHnd,eSupportFilter,APPPTN_SET_FILTER_ADD);
		}
		SysAppActiveFailCallBack = _MainApp_SysAppActiveFailForSMB;
		SysAppActiveAckCallBack = _MainApp_SysAppActiveAckForSMB;
		if(MAIN_APP_SUCCESS != _MainApp_ActivateCurSysAppWithFileAppType(pstEventInfo, FILE_APP_SMB))
		{
			mainapp_error("Fail to active system app\n");
			pstEventInfo->stSysDriveCtrlInfo.stSysCurPtnHnd = pstEventInfo->stSysDriveCtrlInfo.stSysPrePtnHnd;
			_MainApp_SysAppActiveFailForSMB(pstEventInfo);
		}
		break;
	case GUI_RESPOND_AUTO_SWITCH_SYS_DRIVE: //for mantis bug 95456 zw.cao at 2010.07.19
		_MainApp_AutoSWSysDrive(pstEventInfo);
		break;
#endif

	case GUI_RESPOND_HOMEMENU_CLICK_SETUP:
		mainapp_printf("[HOME GOBJ RESPONSE MSG] GUI_RESPOND_HOMEMENU_CLICK_SETUP\n");
		/*1st : close main menu */
		if (GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU))
			GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU);
		/*2nd : open setup menu */
		if (!GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_SETUPMENU))
		{
			ShareGobj_IF_SetRegHideMode(SHARE_REGION_TOP);
			GOBJ_MGR_CreateObject(pMainAppInstance->pGUIObjectTable[MAINAPP_SETUPMENU], pMainAppInstance->GUIObjData, MAINAPP_SETUPMENU, (UINT32) pstEventInfo);
		}
		break;
#if(SUPPORT_iPOD_DOCKING==1)
    case GUI_RESPOND_HOMEMENU_CLICK_IPOD:
    _MainApp_ActivateiPodSysapp(pstEventInfo);
    break;
#endif

#ifdef SUPPORT_PLAYER_STB_DVB_T
	case GUI_RESPOND_HOMEMENU_CLICK_DVBT:
		mainapp_printf("[HOME GOBJ RESPONSE MSG] GUI_RESPOND_HOMEMENU_CLICK_DVBT\n");
		_MainApp_ActivateCurSysAppWithDVBAppType(pstEventInfo);
		break;
#endif

	case GUI_RESPOND_HOMEMENU_RESUME:
		mainapp_printf("[HOME GOBJ RESPONSE MSG] GUI_RESPOND_HOMEMENU_RESUME\n");
		/*close home menu */
		if (GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU))
		{
			GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU);
		}
		if(MAIN_APP_SUCCESS !=_MainApp_ResumeActivateCurSysApp(pstEventInfo))
		{
			/*open home menu */
			if (!GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU))
			{
				GOBJ_MGR_CreateObject(pMainAppInstance->pGUIObjectTable[MAINAPP_GUIOBJ_MAINMENU], pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU,
					  (UINT32) pstEventInfo);
			}
		}
		break;

#ifdef NET_ET_SUPPORT
	case GUI_RESPOND_ETMENU_CLICK_MOVIE:
		mainapp_printf("[ET MAIN MENU GOBJ RESPONSE MSG] GUI_RESPOND_ETMENU_CLICK_MOVIE\n");
		break;

	case GUI_RESPOND_ETMENU_CLICK_DLMGR:
		mainapp_printf("[ET MAIN MENU GOBJ RESPONSE MSG] GUI_RESPOND_ETMENU_CLICK_DLMGR\n");
		_MainApp_ActivateCurSysAppWithETAppType(pstEventInfo, ET_APP_DOWNLOAD_MGR);
		break;

	case GUI_RESPOND_ETMENU_CLICK_DL:
		mainapp_printf("[ET MAIN MENU GOBJ RESPONSE MSG] GUI_RESPOND_ETMENU_CLICK_DL\n");
		_MainApp_ActivateCurSysAppWithETAppType(pstEventInfo, ET_APP_DOWNLOAD);
		break;

	case GUI_RESPOND_ETMENU_CLICK_SETUP:
		mainapp_printf("[ET MAIN MENU GOBJ RESPONSE MSG] GUI_RESPOND_ETMENU_CLICK_SETUP\n");
		_MainApp_ActivateCurSysAppWithETAppType(pstEventInfo, ET_APP_SETUP);
		break;
	#ifdef ET_SUPPORT_VOD
	case GUI_RESPOND_ETMENU_CLICK_VOD:
		mainapp_printf("[ET MAIN MENU GOBJ RESPONSE MSG] GUI_RESPOND_ETMENU_CLICK_VOD\n");
		_MainApp_ActivateCurSysAppWithETAppType(pstEventInfo, ET_APP_VOD);
		break;
	#endif
#endif

#ifdef NET_VOD_SUPPORT
        #ifdef VOD_GLOBAL_SUPPORT
        case GUI_RESPOND_VODMENU_CLICK_GLOBAL:
			mainapp_printf("GUI_RESPOND_VODMENU_CLICK_GLOBAL");
            _MainApp_ActivateCurSysAppWithVodAppType(pstEventInfo, VOD_APP_GLOBAL);
            break;
        #endif
	#ifdef VOD_LOCAL_SUPPORT
	#ifdef VOD_SINA_SUPPORT
	case GUI_RESPOND_VODMENU_CLICK_SINA:
		_MainApp_ActivateCurSysAppWithVodAppType(pstEventInfo, VOD_APP_SINA);
		break;
	#endif
	#ifdef VOD_TUDOU_SUPPORT
	case GUI_RESPOND_VODMENU_CLICK_TUDOU:
		_MainApp_ActivateCurSysAppWithVodAppType(pstEventInfo, VOD_APP_TUDOU);
		break;
	#endif
	#endif
	#if defined(NET_ET_SUPPORT) && defined(ET_SUPPORT_VOD)
	case GUI_RESPOND_VODMENU_CLICK_ET:
		_MainApp_ActivateCurSysAppWithETAppType(pstEventInfo, ET_APP_VOD);
		break;
	#endif
#endif

#ifdef NET_N32_SUPPORT
	case GUI_RESPOND_N32MENU_CLICK_DLMGR:
		mainapp_printf("[N32 MAIN MENU GOBJ RESPONSE MSG] GUI_RESPOND_N32MENU_CLICK_DLMGR\n");
		_MainApp_ActivateCurSysAppWithN32AppType(pstEventInfo, N32_APP_DOWNLOAD_MGR);
		break;

	case GUI_RESPOND_N32MENU_CLICK_DL:
		mainapp_printf("[N32 MAIN MENU GOBJ RESPONSE MSG] GUI_RESPOND_N32MENU_CLICK_DL\n");
		_MainApp_ActivateCurSysAppWithN32AppType(pstEventInfo, N32_APP_DOWNLOAD);
		break;

	case GUI_RESPOND_N32MENU_CLICK_SETUP:
		mainapp_printf("[N32 MAIN MENU GOBJ RESPONSE MSG] GUI_RESPOND_N32MENU_CLICK_SETUP\n");
		_MainApp_ActivateCurSysAppWithN32AppType(pstEventInfo, N32_APP_SETUP);
		break;
#endif
#ifdef NET_NEWS_SUPPORT
	case GUI_RESPOND_NETMENU_CLICK_NEWS:
		_MainApp_ActivateCurSysAppWithNewsAppType(pstEventInfo, SYS_APP_NEWS_PLAYER);
		break;
#endif
#ifdef NET_STOCK_SUPPORT
	case GUI_RESPOND_NETMENU_CLICK_STOCK:
		_MainApp_ActivateCurSysAppWithStockAppType(pstEventInfo, SYS_APP_STOCK_PLAYER);
		break;
#endif
#ifdef NET_WEATHER_SUPPORT
	case GUI_RESPOND_YWEATHER_CLICK_VIEW:
		{
			mainapp_printf("active yeather app ...\n");
			_MainApp_ActivateCurSysAppWithYWAppType(pstEventInfo, 0);
			break;
		}
#endif
#ifdef NET_GMAP_SUPPORT
	case GUI_RESPOND_GOOGLEMAP_CLICK_VIEW:
		{
			mainapp_printf("active google map  app ...\n");
			_MainApp_ActivateCurSysAppWithGMAppType(pstEventInfo, 0);
			break;
		}
#endif
#ifdef NET_PHOTO_SUPPORT
	case GUI_RESPOND_NETMENU_CLICK_PHOTO:
		{
			mainapp_printf("active NETPHOTO app ...\n");
			_MainApp_ActivateCurSysAppWithNetPhotoAppType(pstEventInfo, 0);
			break;
		}
#endif
#if defined(NET_RADIO_SUPPORT)||defined(NET_LOCAL_RADIO_SUPPORT)
	case GUI_RESPOND_NETMENU_CLICK_RADIO:
		{
			mainapp_printf("active NETRadio app ...\n");
			_MainApp_ActivateCurSysAppWithNetRadioAppType(pstEventInfo, 0);
			break;
		}
#endif

#ifdef NET_BROWSER_SUPPORT
	case GUI_RESPOND_NETMENU_CLICK_BROWSER:
		{
			mainapp_printf("active NET Browser app ...\n");
            		_MainApp_ActivateCurSysAppWithBrowserAppType(pstEventInfo, BROWSER_APP_DEFAULT);
			break;
        }
	case GUI_RESPOND_BROWSERMENU_CLICK_YOUKU:
		{
			mainapp_printf("active BROWSERMENU_CLICK_YOUKU ...\n");
            		_MainApp_ActivateCurSysAppWithBrowserAppType(pstEventInfo, BROWSER_APP_YOUKU);
			break;
			
		}
	case GUI_RESPOND_BROWSERMENU_CLICK_TUDOU:
		{

			mainapp_printf("active BROWSERMENU_CLICK_TUDOU app ...\n");
            		_MainApp_ActivateCurSysAppWithBrowserAppType(pstEventInfo, BROWSER_APP_TUDOU);
			break;
		}
	case GUI_RESPOND_BROWSERMENU_CLICK_QIYI:
		{

			mainapp_printf("active BROWSERMENU_CLICK_QIYI app ...\n");
            		_MainApp_ActivateCurSysAppWithBrowserAppType(pstEventInfo, BROWSER_APP_QIYI);
			break;
		}
	case GUI_RESPOND_BROWSERMENU_CLICK_SINA:
		{

			mainapp_printf("active BROWSERMENU_CLICK_SINA app ...\n");
            		_MainApp_ActivateCurSysAppWithBrowserAppType(pstEventInfo, BROWSEr_APP_SINA);
			break;
		}
	case GUI_RESPOND_BROWSERMENU_CLICK_YOUMI:
		{

			mainapp_printf("active BROWSERMENU_CLICK_YOUMI app...\n");
            		_MainApp_ActivateCurSysAppWithBrowserAppType(pstEventInfo, BROWSER_APP_YOUMI);
			break;
		}
	case GUI_RESPOND_BROWSERMENU_CLICK_YOUTUBE:
		{

			mainapp_printf("active BROWSERMENU_CLICK_YOUTUBE app ...\n");
            		_MainApp_ActivateCurSysAppWithBrowserAppType(pstEventInfo, BROWSER_APP_YOUTUBE);
			break;
		}
#endif

#ifdef NET_MUSIC_SUPPORT
	case GUI_RESPOND_NETMENU_CLICK_MUSIC:
		{
			mainapp_printf("active net music app ...\n");
			_MainApp_ActivateCurSysAppWithNetMusicAppType(pstEventInfo, 0);
			break;
		}
#endif

	default:
		mainapp_printf("Unknown GUI feedback message received for home gobj.\n");
		break;
	}
}

static void _MainApp_GUIFBEventHander4GobjSourceMenu(UINT32 dMessage, MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	switch (dMessage)
	{
	case GUI_RESPOND_SYS_DRIVE_CHG:
		mainapp_printf("[SOURCE GOBJ RESPONSE MSG] GUI_RESPOND_SYS_DRIVE_CHG\n");
		/*close source menu */
		if (GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_SOURCE))
		{
			GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_SOURCE);
		}
		/*active sysapp at first, if failed, open home menu*/
		if(MAIN_APP_SUCCESS!=_MainApp_AutoActivateCurSysApp(pstEventInfo))
		{	/*open home menu */
			pstEventInfo->stComGobj.stMainGobj.eMenuType = MAIN_MENU_TYPE_HOME;
			if (!GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU)
				&& !GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_SMART_UPGRADTE))
			{
				GOBJ_MGR_CreateObject(pMainAppInstance->pGUIObjectTable[MAINAPP_GUIOBJ_MAINMENU], pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU,
								  (UINT32) pstEventInfo);
			}
		}
		break;
	case GUI_RESPOND_SOURCEMENU_RESUME:
		mainapp_printf("[SOURCE GOBJ RESPONSE MSG] GUI_RESPOND_SOURCEMENU_RESUME\n");
		/*close source menu */
		if (GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_SOURCE))
		{
			GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_SOURCE);
		}
		if(MAIN_APP_SUCCESS !=_MainApp_ResumeActivateCurSysApp(pstEventInfo))
		{	
			if(MAIN_APP_SUCCESS!=_MainApp_AutoActivateCurSysApp(pstEventInfo))
			{
				/*open home menu */
				pstEventInfo->stComGobj.stMainGobj.eMenuType = MAIN_MENU_TYPE_HOME;
				if (!GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU))
				{
					GOBJ_MGR_CreateObject(pMainAppInstance->pGUIObjectTable[MAINAPP_GUIOBJ_MAINMENU], pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU,
						(UINT32) pstEventInfo);
				}
			}
		}
		break;
	default:
		mainapp_printf("Unknown GUI feedback message received for source gobj.\n");
		break;
	}
}

static void _MainApp_GUIFBEventHander4GobjSetupMenu(UINT32 dMessage, MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	char *DevicePath = NULL;
	switch (dMessage)
	{
	case GUI_RESPOND_SETUPMENUSW2MAINMENU:
		mainapp_printf("[SETUP GOBJ RESPONSE MSG] GUI_RESPOND_SETUPMENUSW2MAINMENU\n");
		/*1st : close setup menu */
		if (GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_SETUPMENU))
		{
			GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_SETUPMENU);
			ShareGobj_IF_SetRegNormalMode(SHARE_REGION_TOP);
		}
		/*2nd : open main menu */
		if (!GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU))
			GOBJ_MGR_CreateObject(pMainAppInstance->pGUIObjectTable[MAINAPP_GUIOBJ_MAINMENU], pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU,
								  (UINT32) pstEventInfo);
		break;
	case GUI_RESPOND_SETUPMENUSW2SMARTUPGRADE:
		ISP_SetUpgradeType(
			ISP_FROM_SOURCE_1,
			DevicePath);
		break;
	default:
		mainapp_printf("Unknown GUI feedback message received for setup gobj.\n");
		break;
	}
}

static void _MainApp_GUIFBEventHander4GobjSmartUpgrade(UINT32 dMessage, MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	#ifdef NET_ISP_SUPPORT
	char *DevicePath = NULL;
	#endif

	switch (dMessage)
	{
	case GUI_RESPOND_SMARTUPGRADE_YES:
		mainapp_printf("GUI_RESPOND_SMARTUPGRADE_YES \n");
		mainapp_printf("UpgradePath %s \n",pstEventInfo->stSysAppCtrlInfo.stSmartUpgradeInfo.UpgradePath);
		if (pstEventInfo->stSysAppCtrlInfo.stSmartUpgradeInfo.UpgradeType == SMARTUPGRADE_FROM_LOCAL)
		{
			ISP_GetBinFileFromDevice(
				ISP_FROM_SOURCE_1,
				pstEventInfo->stSysAppCtrlInfo.stSmartUpgradeInfo.UpgradePath);
		}
		#ifdef NET_ISP_SUPPORT
		else if (pstEventInfo->stSysAppCtrlInfo.stSmartUpgradeInfo.UpgradeType == SMARTUPGRADE_FROM_NET)
			ISP_GetBinFileFromDevice(ISP_FROM_SOURCE_2, DevicePath);
		#endif
		break;
	case GUI_RESPOND_SMARTUPGRADE_NO:
		PSREG_SET_VALUE(PSREG_STAT_PlayMode, PSREG_PLAYMODE_IDLE);//for screensaver
		if (GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_SMART_UPGRADTE))
		{
		        GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_SMART_UPGRADTE);
			  //ShareGobj_IF_SetRegNormalMode(SHARE_REGION_TOP);
		}
		//added by wumin for auto play the disc or file
		if(pstEventInfo->eDiscState == DISC_LOAD_DOWN)
		 {
			if(MAIN_APP_SUCCESS!=_MainApp_AutoActivateCurSysApp(pstEventInfo))
			{
		if (!GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU))
		{
			MCF_SwitchMode(MEMMAP_MODE_HOMEMENU);
			ShareGobj_IF_SetRegNormalMode(SHARE_REGION_TOP);//add by wumin for mantis 126600
			pstEventInfo->stComGobj.stMainGobj.eMenuType = MAIN_MENU_TYPE_HOME;
			GOBJ_MGR_CreateObject(
				pMainAppInstance->pGUIObjectTable[MAINAPP_GUIOBJ_MAINMENU],
				pMainAppInstance->GUIObjData,
				MAINAPP_GUIOBJ_MAINMENU,
				(UINT32) pstEventInfo);
		}
			}
		}
		break;
	case CBK_EVENT_ISP_GET_BIN_DONE:
		mainapp_printf("CBK_EVENT_ISP_GET_BIN_DONE !!! \n");
		if (DI_TrayOut(gPhisycalLoaderDIHandle) == DIMON_IF_SUCCESSFUL)
			ISPStartToUpgrade = TRUE;
		else
		{
			mainapp_printf("loader tray out fail !!! \n");
			ISP_Start(); // add for error handle.
		}
		break;
	case CBK_EVENT_ISP_FINISH:
		mainapp_printf("\n CBK_EVENT_ISP_FINISH \n");
		//MAINAPP_Finalize();
		//bMPBreakCondition = 1;
		break;
	default:
		mainapp_printf("Unknown GUI feedback message received for setup gobj.\n");
		break;
	}
}

#if defined(NET_ET_SUPPORT) || defined(NET_N32_SUPPORT)
static void _MainApp_GUIFBEvent4GobjDaig(UINT32 dMessage, MainAppUIEventHandlerInfo_t *pstEventInfo)
{
	UINT32 dGobjId = MAINAPP_GUIOBJ_MAX;
	UINT32 dNeedOpenMenu = 0;
	INT32 sdRet = 0;
	switch(dMessage)
	{
	case GUI_RESPOND_CONFIRM_YES:
		{
	#if defined(NET_ET_SUPPORT) && defined(ET_UNSUPPORT_DAEMON_DOWNLOAD)
		UINT32 dIsOnRun = 0;
		if(pstEventInfo->stComGobj.eDiagType == COMMON_GOBJ_DIAG_ETEXIT)
		{
			sdRet = ET_StopTaskAll();
			do
			{
				sdRet = ET_CheckHasTaskOnRun(&dIsOnRun);
				if(dIsOnRun)
					GL_TaskSleep(5);
				else
					break;
			}while(1);
		}
		#ifdef ET_SUPPORT_VOD
		else if(pstEventInfo->stComGobj.eDiagType == COMMON_GOBJ_DIAG_ETDL2VOD)
		{
			sdRet = ET_StopTaskAll();
			do
			{
				sdRet = ET_CheckHasTaskOnRun(&dIsOnRun);
				if(dIsOnRun)
					GL_TaskSleep(5);
				else
					break;
			}while(1);
		}
		#endif
		else
	#endif
	#if defined(NET_N32_SUPPORT) && defined(N32_UNSUPPORT_DAEMON_DOWNLOAD)
		if(pstEventInfo->stComGobj.eDiagType == COMMON_GOBJ_DIAG_N32EXIT)
			sdRet = DLMGR_IF_StopAllTasks();
	#endif
		if(sdRet != 0)
		{
			diag_printf("%s[%d] occured error at return ET or N32\n", __FUNCTION__, __LINE__);
			break;
		}

		if (GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_DIAG))
 	       	 GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_DIAG);

		switch(pstEventInfo->dAppEvent)
		{
		case UI_EVENT_SOURCE:
			dGobjId = MAINAPP_GUIOBJ_SOURCE;
			dNeedOpenMenu = 1;
			break;
		case UI_EVENT_EJECT:
			break;
		case UI_EVENT_SETUP:
		default:
		#if defined(NET_ET_SUPPORT) && defined(ET_UNSUPPORT_DAEMON_DOWNLOAD)
			if(pstEventInfo->stComGobj.eDiagType == COMMON_GOBJ_DIAG_ETEXIT)
			{
				if(pstEventInfo->stComGobj.eChgType == HOME_CHANGE_CONTENT2NETWORK)
					pstEventInfo->stComGobj.stMainGobj.eMenuType = MAIN_MENU_TYPE_NETWORK;
				else
					pstEventInfo->stComGobj.stMainGobj.eMenuType = MAIN_MENU_TYPE_HOME;

				dGobjId = MAINAPP_GUIOBJ_MAINMENU;
				dNeedOpenMenu = 1;
			}
			#ifdef ET_SUPPORT_VOD
			else if(pstEventInfo->stComGobj.eDiagType == COMMON_GOBJ_DIAG_ETDL2VOD)
				_MainApp_ActivateCurSysAppWithETAppType(pstEventInfo, ET_APP_VOD);
			#endif
			else
		#endif
		#if defined(NET_N32_SUPPORT) && defined(N32_UNSUPPORT_DAEMON_DOWNLOAD)
			if(pstEventInfo->stComGobj.eDiagType == COMMON_GOBJ_DIAG_N32EXIT)
			{
				if(pstEventInfo->stComGobj.eChgType == HOME_CHANGE_CONTENT2NETWORK)
					pstEventInfo->stComGobj.stMainGobj.eMenuType = MAIN_MENU_TYPE_NETWORK;
				else
					pstEventInfo->stComGobj.stMainGobj.eMenuType = MAIN_MENU_TYPE_HOME;

				dGobjId = MAINAPP_GUIOBJ_MAINMENU;
				dNeedOpenMenu = 1;
			}
		#endif
			break;
		}
		if(dNeedOpenMenu)
		{
			if (!GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, dGobjId))
			{
				GOBJ_MGR_CreateObject(
				pMainAppInstance->pGUIObjectTable[dGobjId],
				pMainAppInstance->GUIObjData,
				dGobjId,
				(UINT32) pstEventInfo);
			}
		}

		if(pstEventInfo->dAppEvent == UI_EVENT_EJECT)
			MAINAPP_SendGlobalEvent(UI_EVENT_EJECT, 0);
		}
		break;
	case GUI_RESPOND_CONFIRM_NO:
		#if defined(NET_ET_SUPPORT) && defined(ET_UNSUPPORT_DAEMON_DOWNLOAD)
		if(pstEventInfo->stComGobj.eDiagType == COMMON_GOBJ_DIAG_ETEXIT)
			pstEventInfo->stComGobj.stMainGobj.eMenuType = MAIN_MENU_TYPE_NET_ET;
		#endif
		#if defined(NET_N32_SUPPORT) && defined(N32_UNSUPPORT_DAEMON_DOWNLOAD)
		if(pstEventInfo->stComGobj.eDiagType == COMMON_GOBJ_DIAG_N32EXIT)
			pstEventInfo->stComGobj.stMainGobj.eMenuType = MAIN_MENU_TYPE_NET_N32;
		#endif
		if (GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_DIAG))
 	       	 GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_DIAG);

		if (!GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU))
		{
			GOBJ_MGR_CreateObject(
			pMainAppInstance->pGUIObjectTable[MAINAPP_GUIOBJ_MAINMENU],
			pMainAppInstance->GUIObjData,
			MAINAPP_GUIOBJ_MAINMENU,
			(UINT32) pstEventInfo);
		}
		break;
	default:
		break;
	}
}
#endif


void MAINAPP_GUIFeedbackEventHandler(UINT32 dMessage, UINT32 dParam)
{
	MainAppUIEventHandlerInfo_t *pstEventInfo = &stUIEventHandlerInfo;
	UINT32 dMainAppState = MAIN_APP_NORMAL;
	//mainapp_printf("GUIOBJ: %d : responce message : 0x%x received.\n", dParam, dMessage);
	/*check main app state,now just normal state do something */
	MAINAPP_GetMainAppState(&dMainAppState);
	if (dMainAppState == MAIN_APP_NORMAL)
	{
		UINT32 dSysAppIdx = TOTAL_SYS_APP_SIZE;
		INT32 sdRet = MAIN_APP_SUCCESS;
		/*check sys app active state. */
		sdRet = MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
		switch (sdRet)
		{
		case MAIN_APP_NO_ACTIVE_SYSTEM_APP:
			//mainapp_printf("%s on NO_ACTIVE_SYSTEM_APP.\n", __FUNCTION__);
			switch (dParam)
			{
			case MAINAPP_GUIOBJ_MAINMENU:
				_MainApp_GUIFBEventHander4GobjHomeMenu(dMessage, pstEventInfo);
				break;
				
			case MAINAPP_GUIOBJ_SOURCE:
				_MainApp_GUIFBEventHander4GobjSourceMenu(dMessage, pstEventInfo);
				break;

			case MAINAPP_SETUPMENU:
				_MainApp_GUIFBEventHander4GobjSetupMenu(dMessage, pstEventInfo);
				break;

			case MAINAPP_SMART_UPGRADTE:
				_MainApp_GUIFBEventHander4GobjSmartUpgrade(dMessage, pstEventInfo);
				break;
		#if defined(NET_ET_SUPPORT) || defined(NET_N32_SUPPORT)
			case MAINAPP_GUIOBJ_DIAG:
				_MainApp_GUIFBEvent4GobjDaig(dMessage, pstEventInfo);
				break;
		#endif
			default:
				mainapp_printf("this gobj feed back event handler is not implement .\n");
				break;
			}
			break;

		case MAIN_APP_SUCCESS:
			switch (dSysAppIdx)
			{
			case SYS_APP_DISC_PLAYER:
				break;
			case SYS_APP_FILE_PLAYER:
				break;
			case TOTAL_SYS_APP_SIZE:
				break;
			default:
				break;
			}
			break;

		default:
			mainapp_error("%s on %d condition.\n", __FUNCTION__, sdRet);
			break;
		}
	}
	else
		mainapp_printf("%s on main app state[%d].\n", __FUNCTION__, dMainAppState);
}

static void _MainApp_ActivateSysApp(MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	UINT32 dGobjTotalCnt = 0;
	INT32 sdRet = GUI_OBJMGR_SUCCESS;
	UINT32 dSystemIdx = 0;
	UINT32 dIsNeedActvateSysApp = 1;
	MediaType_e eMediaType = 0;

	sdRet = GOBJ_MGR_GetTotalCreatedObjects(pMainAppInstance->GUIObjData, &dGobjTotalCnt);
	if (sdRet == GUI_OBJMGR_SUCCESS)
	{
		UINT32 i = 0;
		UINT32 dGobjId = 0;
		for (i = 0; i < dGobjTotalCnt; i++)
		{
			sdRet = GOBJ_MGR_GetObjectIDByIndex(pMainAppInstance->GUIObjData, i, &dGobjId);
			if (sdRet == GUI_OBJMGR_SUCCESS)
			{
				mainapp_printf("%s on gobj[%d].\n", __FUNCTION__, dGobjId);
				switch (dGobjId)
				{
				case MAINAPP_GUIOBJ_SOURCE:
				case MAINAPP_SETUPMENU:
				case MAINAPP_SETUPMENUSPKR:
				case MAINAPP_SUPERSETUPMENU:
				case MAINAPP_REGIONMENU:
				case MAINAPP_CUSTOMERMENU:
				case MAINAPP_SMART_UPGRADTE:    //added by wumin
					dIsNeedActvateSysApp = 0;
					mainapp_printf("%s do nothing under gobj[%d].\n", __FUNCTION__, dGobjId);
					break;
				case MAINAPP_SCREENSAVER:
					mainapp_printf("MAINAPP_SCREENSAVER gobj exist !!\n");
					 _MainApp_DaemonEventScrnSaverOff(0,pstEventInfo);
					break;

				default:

					break;
				}
			}
			else
			{
				mainapp_error("%s get obj id ret [%d] index[%d].\n", __FUNCTION__, sdRet, i);
				break;
			}
		}
	}
	else
		mainapp_error("%s get total obj cnt [%d] .\n", __FUNCTION__, sdRet);

#ifdef NET_ISP_SUPPORT
	if((pstEventInfo->stSysAppCtrlInfo.stSmartUpgradeInfo.UpgradeType == SMARTUPGRADE_FROM_NET)//add by wumin
		&&(pstEventInfo->stSysAppCtrlInfo.stSmartUpgradeInfo.stFirst == ISP_IF_NO_ACTIVE_DISCSYS)
			&&(PSREG_VALUE(PSREG_SMART_UPGRADE) ==PSREG_SMART_UPGRADE_ON)
				&& (pstEventInfo->stNetDriveCtrlInfo.eState == NET_DRIVE_STATE_BOOTUP))
		dIsNeedActvateSysApp = 0;
#endif

	if((pstEventInfo->stSysAppCtrlInfo.stSmartUpgradeInfo.UpgradeType == SMARTUPGRADE_FROM_LOCAL)  //add by wumin
		&&(pstEventInfo->stSysAppCtrlInfo.stSmartUpgradeInfo.stFirst == ISP_IF_NO_ACTIVE_DISCSYS))//lgz 2010/07/07 add for when in upgrade UI not active any sys app
		dIsNeedActvateSysApp = 0;

	if (dIsNeedActvateSysApp)
	{
		if (_MainApp_GetToBeActivatedSysAppIdx(pstEventInfo, &dSystemIdx) == MAIN_APP_SUCCESS)
		{
			switch (dSystemIdx)
			{
			case SYS_APP_DISC_PLAYER:
				if (GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU))
					GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU);

				if(pstEventInfo->stSysAppCtrlInfo.stSmartUpgradeInfo.stFirst != ISP_IF_NO_ACTIVE_DISCSYS)
					sdRet = MAINAPP_ActivateSystemApp(SYS_APP_DISC_PLAYER, (UINT32) pstEventInfo);
				else	{}
				break;
			case SYS_APP_FILE_PLAYER:
				AppPtn_GetMediaType( pstEventInfo->stSysDriveCtrlInfo.stSysCurPtnHnd, &eMediaType);
				switch(eMediaType)
				{
				case MEDIA_TYPE_CDDA:
					sdRet = _MainApp_AutoActivateFileApp(pstEventInfo);
					break;
				#if 0//def KOK_SUPPORT
				case MEDIA_TYPE_KOK://xy.zhu add for kok disc
					sdRet = _MainApp_AutoActivateFileApp(pstEventInfo);
					break;
#endif
				default:				
					if(pstEventInfo->eDiscState == DISC_LOAD_DOWN)
					{
						sdRet = _MainApp_AutoActivateFileApp(pstEventInfo);
						if((sdRet == MAIN_APP_ACTIVATE_FAILED)
							|| (sdRet == MAIN_APP_INDEX_ERROR)
							|| (sdRet == MAIN_APP_INSTANCE_NOT_INITIALIZED))
						{

							if (!GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU)
								&& !GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_SMART_UPGRADTE))
							{
										PSREG_SET_VALUE(PSREG_STAT_PlayMode, PSREG_PLAYMODE_PLAY);
										pstEventInfo->stComGobj.stMainGobj.eMenuType = MAIN_MENU_TYPE_HOME;
										GOBJ_MGR_CreateObject(
										pMainAppInstance->pGUIObjectTable[MAINAPP_GUIOBJ_MAINMENU],
										pMainAppInstance->GUIObjData,
										MAINAPP_GUIOBJ_MAINMENU,
										(UINT32) pstEventInfo);
							}
						}
					}
				}
				break;
			default:
				mainapp_error("%s invalidate to be activate sys app index[%d].\n", __FUNCTION__, dSystemIdx);
				break;
			}
		}
		else
			mainapp_error("%s get to be activate sys app index.\n", __FUNCTION__);
	}
}
static void _MainApp_PtnMngMountDone(MainAppUIEventHandlerInfo_t * pstEventInfo, UINT32 dParam)
{
	INT32 sdRet = 0;
	UINT32 dDeviceType = DEVICE_TYPE_NULL;
	AppPtn_Handle_t stMountPtn = NULL;
	sdRet = AppPtn_AddPtn(dParam, &stMountPtn);/*add partition*/
	if((APPPTN_SUCCESSFUL==sdRet)||(APPPTN_WARNING_EXIST==sdRet))
	{
		/*update partition list */
		if (AppPtn_RefreshPtnByDevNum(dParam) != APPPTN_SUCCESSFUL)
			mainapp_error("ptn mng update current list.\n");

		AppPtn_GetDevType(stMountPtn,&dDeviceType);
		switch(dDeviceType)
		{
			case DEVICE_TYPE_CARD:
				ShareGobj_IF_ShowHintMsg(HINT_MSG_CARD_PLUG_IN, HINT_MSG_TIMEOUT, MSG_PARAM_NULL);
				PSREG_SET_VALUE(PSREG_STAT_MainAppStatus, PSREG_MAINAPPSTATUS_CARD_ATTACH);
				break;
			case DEVICE_TYPE_HDD:
				ShareGobj_IF_ShowHintMsg(HINT_MSG_USB_PLUG_IN, HINT_MSG_TIMEOUT, MSG_PARAM_NULL);
				PSREG_SET_VALUE(PSREG_STAT_MainAppStatus, PSREG_MAINAPPSTATUS_USB_ATTACH);
				break;
			case DEVICE_TYPE_USB:
				ShareGobj_IF_ShowHintMsg(HINT_MSG_USB_PLUG_IN, HINT_MSG_TIMEOUT, MSG_PARAM_NULL);
				PSREG_SET_VALUE(PSREG_STAT_MainAppStatus, PSREG_MAINAPPSTATUS_USB_ATTACH);
				break;
			default:
				break;
		}
	}
}

static void _MainApp_PtnMngParseOneDone4Storage(MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	AppPtn_Handle_t stCurPtnHnd = NULL;
	SysDriveCtrlInfo_t *pstSysDriveCtrlInfo = &pstEventInfo->stSysDriveCtrlInfo;
	/*bup up current sys actived drive */
	stCurPtnHnd = pstSysDriveCtrlInfo->stSysCurPtnHnd;

	/*1st : if update finish, sys may need change sys activate drive. */
	_MainApp_AutoSWSysDrive(pstEventInfo);
	if(MainAppDiscInsertCallBack)
		MainAppDiscInsertCallBack(pstEventInfo);

	/*2nd : handle main app control flow. */
	UINT32 dMainAppState = MAIN_APP_NORMAL;

	/*check main app state,now just normal state do something */
	MAINAPP_GetMainAppState(&dMainAppState);
	if (dMainAppState == MAIN_APP_NORMAL)
	{
		UINT32 dSysAppIdx = TOTAL_SYS_APP_SIZE;
		INT32 sdRet = MAIN_APP_SUCCESS;
		/*check sys app active state. */
		sdRet = MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
		switch (sdRet)
		{
		case MAIN_APP_NO_ACTIVE_SYSTEM_APP:
			mainapp_printf("%s on NO_ACTIVE_SYSTEM_APP.\n", __FUNCTION__);
			ShareGobj_IF_ShowHintMsg(HINT_MSG_FILE_LOADING, HINT_MSG_TIMEOUT_INFINITE, MSG_PARAM_CLEAR_HINT_MSG);
			if (pstSysDriveCtrlInfo->stSysCurPtnHnd != stCurPtnHnd)
				_MainApp_ActivateSysApp(pstEventInfo);
			else
				mainapp_printf("%s sys drive [handle:0x%08X] is not changed.\n", __FUNCTION__, (UINT32)stCurPtnHnd);
			break;

		case MAIN_APP_SUCCESS:
		{
			switch (dSysAppIdx)
			{
				case SYS_APP_DISC_PLAYER:
				case SYS_APP_FILE_PLAYER:
					if (pstSysDriveCtrlInfo->stSysCurPtnHnd != stCurPtnHnd)
					{
						sdRet = MAINAPP_TerminateSystemApp(dSysAppIdx);
						if (sdRet == SYSTEM_APP_SUCCESS)
						{
							/*ZW.Cao:what does it mean? who marked it?*/
							//SysAppTerminateAckCallBack = _MainApp_UICBKEventPtnMngUpdateFinish;
						}
						else
							mainapp_error(" Terminate System App[%d]\n", sdRet);
					}
					break;

				case TOTAL_SYS_APP_SIZE:
					mainapp_printf("%s on TOTAL_SYS_APP_SIZE.\n", __FUNCTION__);
					break;
				default:
					break;
			}
			break;
		}
		default:
			mainapp_error("%s on %d condition.\n", __FUNCTION__, sdRet);
			break;
		}
	}
	else
		mainapp_printf("%s on main app state[%d].\n", __FUNCTION__, dMainAppState);
}
#ifdef NET_ET_SUPPORT
static void APPNOINLINE _MainApp_EtMountHandle(MainAppUIEventHandlerInfo_t *pstEventInfo,UINT32 dDevNum)
{
	INT32 sdReturnValue = ET_IF_SUCCESSFUL;
	AppPtn_Handle_t  stPtnHandle = NULL;
	char* pszMountPonit = NULL;

	if(!pstEventInfo)
	{
		mainapp_error("%s\n", __FUNCTION__);
		return;
	}

	pszMountPonit = GL_MemAlloc(ET_PATH_NAME_MAX_LENTH);
	if(!pszMountPonit)
	{
		mainapp_error("alloc et new path name\n");
		return;
	}
	memset(pszMountPonit,0,ET_PATH_NAME_MAX_LENTH);
	do
	{
		sdReturnValue = AppPtn_GetPtnHandle(&stPtnHandle,dDevNum);
		if(sdReturnValue != APPPTN_SUCCESSFUL)
		{
			mainapp_error("get ptn handle\n");
			break;
		}
		if(!stPtnHandle)
		{
			mainapp_error("invalidate ptn handle\n");
			break;
		}
		sdReturnValue = AppPtn_GetMountPoint(stPtnHandle,pszMountPonit);
		if(sdReturnValue != APPPTN_SUCCESSFUL)
		{
			mainapp_error("get ptn mount point\n");
			break;
		}
		sdReturnValue = APP_NET_InstCtrl(APP_NET_INST_ET,APP_NET_INST_OPS_MOUNT,(UINT32)pszMountPonit);
		if(sdReturnValue < APP_NET_IF_SUCCESSFUL)
		{
			mainapp_error("boot up check et download\n");
			break;
		}
		pstEventInfo->stSysAppCtrlInfo.stEtAppCtrlInfo.stDlPtnHdl = stPtnHandle;

	}while(0);
	GL_MemFree(pszMountPonit);
}

static void APPNOINLINE _MainApp_EtUnmountHandle(MainAppUIEventHandlerInfo_t *pstEventInfo,UINT32 dDevNum)
{
	INT32 sdReturnValue = ET_IF_SUCCESSFUL;
	AppPtn_Handle_t  stPtnHandle = NULL;
	char* pszMountPonit = NULL;

	if(!pstEventInfo)
	{
		mainapp_error("%s\n", __FUNCTION__);
		return;
	}

	pszMountPonit = GL_MemAlloc(ET_PATH_NAME_MAX_LENTH);
	if(!pszMountPonit)
	{
		mainapp_error("alloc et new path name\n");
		return;
	}
	memset(pszMountPonit,0,ET_PATH_NAME_MAX_LENTH);
	do
	{
		sdReturnValue = AppPtn_GetPtnHandle(&stPtnHandle,dDevNum);
		if(sdReturnValue != APPPTN_SUCCESSFUL)
		{
			mainapp_error("get ptn handle\n");
			break;
		}
		if(!stPtnHandle)
		{
			mainapp_error("invalidate ptn handle\n");
			break;
		}
		sdReturnValue = AppPtn_GetMountPoint(stPtnHandle,pszMountPonit);
		if(sdReturnValue != APPPTN_SUCCESSFUL)
		{
			mainapp_error("get ptn mount point\n");
			break;
		}
		sdReturnValue = APP_NET_InstCtrl(APP_NET_INST_ET,APP_NET_INST_OPS_UNMOUNT,(UINT32)pszMountPonit);
		if(sdReturnValue < APP_NET_IF_SUCCESSFUL)
		{
			mainapp_error("boot up check et download\n");
			break;
		}
		pstEventInfo->stSysAppCtrlInfo.stEtAppCtrlInfo.stDlPtnHdl = NULL;
	}while(0);
	GL_MemFree(pszMountPonit);
}
#endif

#ifdef NET_N32_SUPPORT

static void APPNOINLINE _MainApp_N32MountHandle(MainAppUIEventHandlerInfo_t *pstEventInfo,UINT32 dDevNum)
{
	INT32 sdReturnValue = DLMGR_IF_SUCCESSFUL;
	AppPtn_Handle_t  stPtnHandle = NULL;
	char* pszMountPonit = NULL;

	if(!pstEventInfo)
	{
		mainapp_error("%s\n", __FUNCTION__);
		return;
	}

	pszMountPonit = GL_MemAlloc(DLMGR_IF_FILEPATH_MAX_LEN);
	if(!pszMountPonit)
	{
		mainapp_error("alloc et new path name\n");
		return;
	}
	memset(pszMountPonit,0,DLMGR_IF_FILEPATH_MAX_LEN);
	do
	{
		sdReturnValue = AppPtn_GetPtnHandle(&stPtnHandle,dDevNum);
		if(sdReturnValue != APPPTN_SUCCESSFUL)
		{
			mainapp_error("get ptn handle\n");
			break;
		}
		if(!stPtnHandle)
		{
			mainapp_error("invalidate ptn handle\n");
			break;
		}
		sdReturnValue = AppPtn_GetMountPoint(stPtnHandle,pszMountPonit);
		if(sdReturnValue != APPPTN_SUCCESSFUL)
		{
			mainapp_error("get ptn mount point\n");
			break;
		}
		sdReturnValue = APP_NET_InstCtrl(APP_NET_INST_N32,APP_NET_INST_OPS_MOUNT,(UINT32)pszMountPonit);
		if(sdReturnValue < APP_NET_IF_SUCCESSFUL)
		{
			mainapp_error("boot up check et download\n");
			break;
		}

	}while(0);
	GL_MemFree(pszMountPonit);
}

static void APPNOINLINE _MainApp_N32UnmountHandle(MainAppUIEventHandlerInfo_t *pstEventInfo,UINT32 dDevNum)
{
	INT32 sdReturnValue = DLMGR_IF_SUCCESSFUL;
	AppPtn_Handle_t  stPtnHandle = NULL;
	char* pszMountPonit = NULL;

	if(!pstEventInfo)
	{
		mainapp_error("%s\n", __FUNCTION__);
		return;
	}

	pszMountPonit = GL_MemAlloc(DLMGR_IF_FILEPATH_MAX_LEN);
	if(!pszMountPonit)
	{
		mainapp_error("alloc et new path name\n");
		return;
	}
	memset(pszMountPonit,0,DLMGR_IF_FILEPATH_MAX_LEN);
	do
	{
		sdReturnValue = AppPtn_GetPtnHandle(&stPtnHandle,dDevNum);
		if(sdReturnValue != APPPTN_SUCCESSFUL)
		{
			mainapp_error("get ptn handle\n");
			break;
		}
		if(!stPtnHandle)
		{
			mainapp_error("invalidate ptn handle\n");
			break;
		}
		sdReturnValue = AppPtn_GetMountPoint(stPtnHandle,pszMountPonit);
		if(sdReturnValue != APPPTN_SUCCESSFUL)
		{
			mainapp_error("get ptn mount point\n");
			break;
		}
		sdReturnValue = APP_NET_InstCtrl(APP_NET_INST_N32,APP_NET_INST_OPS_UNMOUNT,(UINT32)pszMountPonit);
		if(sdReturnValue < APP_NET_IF_SUCCESSFUL)
		{
			mainapp_error("boot up check et download\n");
			break;
		}

	}while(0);
	GL_MemFree(pszMountPonit);
}

#endif
#ifdef CDDA_MULTI_SESSION_CHECK  //guohao 2010-8-6 10:54 add
static INT32 _MainApp_PtnMngInvalidDataCheckCDDA(MainAppUIEventHandlerInfo_t * pstEventInfo,UINT32 *pdParam)
{
	/*1st : back current drive,if fail return drive ptr. */
	AppPtn_Handle_t stCurPtnHndBack = NULL;
	static MainAppCallBack MainAppDiscInsertCallBackBackup = NULL; /*for data disc parse_one_done callback function backup*/
	SysDriveCtrlInfo_t *pstSysDriveCtrlInfo = &pstEventInfo->stSysDriveCtrlInfo;
	stCurPtnHndBack = pstSysDriveCtrlInfo->stSysCurPtnHnd;
	MainAppDiscInsertCallBackBackup = MainAppDiscInsertCallBack;
	/*2st : sys may need change sys activate drive. */
	_MainApp_AutoSWSysDrive(pstEventInfo);
	if(MainAppDiscInsertCallBack)
		MainAppDiscInsertCallBack(pstEventInfo);

	UINT32 eSysCurDrvSupportFilter = 0;
	INT32 sdRet = 0;
	sdRet = AppPtn_GetFilter(pstEventInfo->stSysDriveCtrlInfo.stSysCurPtnHnd, &eSysCurDrvSupportFilter);
	if(sdRet!=APPPTN_SUCCESSFUL)
	{//recover drive ptr. if the chack is fail.
		pstSysDriveCtrlInfo->stSysCurPtnHnd = stCurPtnHndBack;
		return MAIN_APP_ACTIVATE_FAILED;
	}
	/*3st : check bin file. because at b4 do add ptn, filter updata bin,we will check is it really have bin file*/
	if (eSysCurDrvSupportFilter ==  (PTNMNG_FILTER_ANYTHING|PTNMNG_FILTER_BIN))//means after add ptn,only have bin filter
	{
		memset(pstEventInfo->stSysAppCtrlInfo.stSmartUpgradeInfo.UpgradePath, 0, DEVICE_PATH_LENGTH);
		sdRet = AppPtn_GetMountPoint(
					pstEventInfo->stSysDriveCtrlInfo.stSysCurPtnHnd,
					pstEventInfo->stSysAppCtrlInfo.stSmartUpgradeInfo.UpgradePath); //get dev path

		if (sdRet == APPPTN_SUCCESSFUL)
		{
			strcat(pstEventInfo->stSysAppCtrlInfo.stSmartUpgradeInfo.UpgradePath, "/rom.bin\0");
			//mainapp_printf("upgrade bin file path : %s \n",pstEventInfo->stSysAppCtrlInfo.stSmartUpgradeInfo.UpgradePath);
			int fd;
			fd = open(pstEventInfo->stSysAppCtrlInfo.stSmartUpgradeInfo.UpgradePath, O_RDONLY);
			if(fd < 0)
			{
				//mainapp_printf("Open %s fail !!!\n",pstEventInfo->stSysAppCtrlInfo.stSmartUpgradeInfo.UpgradePath);
				eSysCurDrvSupportFilter &= (~PTNMNG_FILTER_BIN);
				AppPtn_SetFilter(
					pstEventInfo->stSysDriveCtrlInfo.stSysCurPtnHnd,
					eSysCurDrvSupportFilter,
					APPPTN_SET_FILTER_SET);
			}
			else//do nothing
				close(fd);
		}
	}

	sdRet = AppPtn_GetFilter(pstEventInfo->stSysDriveCtrlInfo.stSysCurPtnHnd, &eSysCurDrvSupportFilter);
	if(sdRet!=APPPTN_SUCCESSFUL)
	{//recover drive ptr. if the chack is fail.
		pstSysDriveCtrlInfo->stSysCurPtnHnd = stCurPtnHndBack;
		return MAIN_APP_ACTIVATE_FAILED;
	}
	/*4st : invalid data disc, then check the CDDA info.*/
	if(eSysCurDrvSupportFilter == PTNMNG_FILTER_ANYTHING)
	{
		//mainapp_printf("!!!--- not support filter file--- for test run CDDA\n");
		UINT32 dSysAppIdx = TOTAL_SYS_APP_SIZE;
		INT32 sdRet = MAIN_APP_SUCCESS;
		sdRet = MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
		//mainapp_printf("!!!--- sdRet=%x ,dSysAppIdx=%x\n",sdRet,dSysAppIdx);

		if((sdRet == MAIN_APP_NO_ACTIVE_SYSTEM_APP))
			if(DI_CheckCDDAInfo(pstEventInfo->DIHandle[DI_Handle_PHYSICAL],pdParam) == DIMON_IF_SUCCESSFUL)
				return MAIN_APP_SUCCESS;
	}
	//recover drive ptr. if the chack is fail.
	pstSysDriveCtrlInfo->stSysCurPtnHnd = stCurPtnHndBack;
	MainAppDiscInsertCallBack = MainAppDiscInsertCallBackBackup;//if MAIN_APP_ACTIVATE_FAILED,MainAppDiscInsertCallBack will used in _MainApp_PtnMngParseOneDone4Storage

	return MAIN_APP_ACTIVATE_FAILED;
}
#endif
//add by george.chang 2010.0909
static INT32 _MainApp_ExecuteDiscEject(UINT32 dParam,MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	diag_printf("[Main App]ExecuteDiscEject: %d\n",dParam);
	//add by george.chang 2010.09.29
	UINT32 dDeviceType = 0;
	INT32 sdRet = MAIN_APP_SUCCESS;
	UINT32 dDiscLoaderId = DI_Handle_PHYSICAL;

	if(dParam & 0x01)  //Terminate Disc Player
		MAINAPP_TerminateSystemApp(SYS_APP_DISC_PLAYER);

	if(dParam & 0x02)  //Execute Tray out
	{
						
		APP_Show_Logo(LOGO_UI,LOGO_WITH_WORD);//jiangxl 20101220
		ShareGobj_IF_ShowHintMsg(HINT_MSG_DISC_LOADING, HINT_MSG_TIMEOUT_INFINITE, MSG_PARAM_CLEAR_HINT_MSG);
		if(dParam & 0x04)//when region code err,show region code err icon,swjiang,2010.12.10
			ShareGobj_IF_ShowHintMsg(HINT_MSG_REGION_ERROR, HINT_MSG_TIMEOUT_5S, MSG_PARAM_RESET_BG_MSG); 
		ShareGobj_IF_ShowHintMsg(HINT_MSG_OPEN, HINT_MSG_TIMEOUT, MSG_PARAM_RESET_BG_MSG);

		PSREG_SET_VALUE(PSREG_STAT_MainAppStatus, PSREG_MAINAPPSTATUS_TRAYOUT);
		//DI_TrayOut(gPhisycalLoaderDIHandle);
		sdRet = DI_TrayOut(gPhisycalLoaderDIHandle);//add by wumin for mantis 111536
		if (sdRet == DIMON_IF_SUCCESSFUL)
		{
			AppPtn_GetDevType( pstEventInfo->stSysDriveCtrlInfo.stSysCurPtnHnd, &dDeviceType);
			AppPtn_DiscEject(dDiscLoaderId);
			pstEventInfo->eTrayState = PHYSICAL_TRAY_OUT;
			if(DEVICE_TYPE_DISC==dDeviceType)
				pstEventInfo->stSysDriveCtrlInfo.stSysCurPtnHnd = NULL;
		}
	}

	diag_printf("[Main App]ToDoItem after ExecuteDiscEject: \n");
	stUIEventHandlerInfo.bIsDiscEjecting=FALSE;

	if(stUIEventHandlerInfo.bEjectPendingAction==UI_EVENT_POWER)//high priority and skip all todo item
	{
		diag_printf("[Main App]PowerOff: \n");

		StandByMode eMode = MAINAPP_AWAKE;
		MAINAPP_IsEnteringStandby(&eMode);

		if(eMode != MAINAPP_STANDBY)
			_MainApp_Finalizing();
	}
	stUIEventHandlerInfo.bEjectPendingAction=0;

	return MAIN_APP_SUCCESS;
}

#ifdef NET_ISP_SUPPORT
static void _MainApp_ShowUpgradeMenu(MainAppUIEventHandlerInfo_t *pstEventInfo)
{
	UINT32 dMainAppState = MAIN_APP_NORMAL;
	UINT32 dIsNeedOpenUpgradeMenu = 1;

	MAINAPP_GetMainAppState(&dMainAppState);
	if (dMainAppState == MAIN_APP_NORMAL)
	{
		UINT32 dSysAppIdx = TOTAL_SYS_APP_SIZE;
		INT32 sdRet = MAIN_APP_SUCCESS;

		sdRet = MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
		switch (sdRet)
		{
			case MAIN_APP_NO_ACTIVE_SYSTEM_APP:
				dIsNeedOpenUpgradeMenu = 1;
				mainapp_printf("%s on NO_ACTIVE_SYSTEM_APP.\n", __FUNCTION__);
				break;
			case MAIN_APP_SUCCESS:
				dIsNeedOpenUpgradeMenu = 0;
				break;
			default:
				dIsNeedOpenUpgradeMenu = 0;
				break;
		}
	}

	if(1==dIsNeedOpenUpgradeMenu)
	{
		if(GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU))
			GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU);
		if(GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_SETUPMENU))
			GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_SETUPMENU);

		if(!GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_SMART_UPGRADTE))
		{
			GOBJ_MGR_CreateObject(
				pMainAppInstance->pGUIObjectTable[MAINAPP_SMART_UPGRADTE],
				pMainAppInstance->GUIObjData,
				MAINAPP_SMART_UPGRADTE,
				(UINT32) pstEventInfo);
		}
	}
}
#endif

#ifdef SERVO_SPEED_AUTO_CHANGE
static INT32 _MainApp_ChangeSrvSpeed(UINT32 dParam)//1:means speed down, 2:means speed up //guohao add for ctrl srv speed
{
	INT32 sdRet = MAIN_APP_SUCCESS;

	switch(dParam)
	{
	case 1:
		sdRet = DI_ChangeServoSpeed(gPhisycalLoaderDIHandle, DI_ServoNormalSpeed_2X);
		break;

	case 2:
		sdRet = DI_ChangeServoSpeed(gPhisycalLoaderDIHandle, DI_ServoMediumSpeed_3DOT4X);
		break;

	default:
		mainapp_printf(" not need change speed.\n");
		break;
	}

	if(sdRet != DIMON_IF_SUCCESSFUL)
		mainapp_printf(" MainApp_ChangeSrvSpeed fail [%d] \n",sdRet);

	return sdRet;
}
#endif
void MAINAPP_CBKEventHandler(UINT32 dMessage, UINT32 dParam)
{
	MainAppUIEventHandlerInfo_t *pstEventInfo = &stUIEventHandlerInfo;
	mainapp_printf("MAINAPP_CBKEventHandler dMessage:0x%x, dParam:0x%x\n",dMessage, dParam); // mainapp_printf
	switch (dMessage)
	{
	case CBK_EVENT_DISCMW_DISC_EJECT_ACK://add by george.chang 2010.0909
		_MainApp_ExecuteDiscEject(dParam,pstEventInfo);
		break;

	case CBK_EVENT_PTNMNG_CALLBACK_UPDATE_FINISH:
		/*cbk event partition mamager updata list finish */
		pstEventInfo->dAppEvent = dMessage;
		break;

	case CBK_EVENT_PTNMNG_CALLBACK_MOUNT_DONE:
		_MainApp_PtnMngMountDone(pstEventInfo, dParam);
		#ifdef NET_SUPPORT
		_MainApp_SetupMenuDLPosDetect();
		#endif
		break;
	case CBK_EVENT_PTNMNG_CALLBACK_MOUNT_FAIL:
		mainapp_error("CBK_EVENT_PTNMNG_CALLBACK_MOUNT_FAIL !!! \n");
		ShareGobj_IF_ShowHintMsg(HINT_MSG_FILE_LOADING, HINT_MSG_TIMEOUT_INFINITE, MSG_PARAM_CLEAR_HINT_MSG);
		break;
#ifdef NET_ET_SUPPORT
	case CBK_EVENT_NET_ET_DOWNLOAD_FINISH:
		{
			#define		PATH_NAME_MAX_LENTH	256
			char pEtDlPoint[PATH_NAME_MAX_LENTH] ;
			memset(pEtDlPoint, 0, PATH_NAME_MAX_LENTH);
			strcpy(pEtDlPoint,(const char*)dParam);
			/*move ET_DL_ROOT_FOLDER "/sunplus" */
			char *pPos = NULL;
			pPos = strchr((pEtDlPoint+1),'/');
			if(pPos != NULL)
			{
				*pPos = 0;
			}
			if(AppPtn_GetPtnHandleByMountPoint(&pstEventInfo->stSysAppCtrlInfo.stEtAppCtrlInfo.stDlPtnHdl,(const char*)pEtDlPoint) == APPPTN_SUCCESSFUL)
			{
			/*update partition list */
			if (APPPTN_SUCCESSFUL != AppPtn_RefreshPtn(pstEventInfo->stSysAppCtrlInfo.stEtAppCtrlInfo.stDlPtnHdl))
				mainapp_error("ptn mng update current list.\n");
			}
		}
		break;
#endif
#ifdef NET_N32_SUPPORT
	case CNK_EVENT_NET_N32_DOWNLOAD_FINISH:
		{
			char* pMountPoint = (char*)dParam;
			if (pMountPoint != NULL)
			{
				AppPtn_Handle_t ptnHdl = NULL;
				if (APPPTN_SUCCESSFUL == AppPtn_GetPtnHandleByMountPoint(&ptnHdl, pMountPoint))
				{
					//diag_printf("AppPtn_SetFilter [%s][%d]", pMountPoint, ptnHdl);
					AppPtn_SetFilter(ptnHdl, PTNMNG_FILTER_NATIVE32, APPPTN_SET_FILTER_ADD);
				}
			}
		}
		break;
#endif

	case CBK_EVENT_PTNMNG_CALLBACK_ONE_PTN_PARSE_DONE:
		AppPtn_AddPtn(dParam, NULL);/*Update Filter*/
#ifdef NET_ET_SUPPORT
		_MainApp_EtMountHandle(pstEventInfo,dParam);
#endif
#ifdef NET_N32_SUPPORT
		_MainApp_N32MountHandle(pstEventInfo,dParam);
#endif
		pstEventInfo->dAppEvent = dMessage;
		#ifdef CDDA_MULTI_SESSION_CHECK  //guohao 2010-8-6 10:54 add
		if(_MainApp_PtnMngInvalidDataCheckCDDA(pstEventInfo, &dParam) == MAIN_APP_SUCCESS)
		{
			mainapp_printf("CDDA_DISC_INSERTED received.\n");
			_MainApp_DaemonEventCddaDiscInserted(dParam, pstEventInfo);
		}
		else
		#endif
		_MainApp_PtnMngParseOneDone4Storage(pstEventInfo);
		break;
#ifdef NET_ISP_SUPPORT
	case CBK_EVENT_NETISP_DOWNLOAD_FAIL:
		pstEventInfo->stSysAppCtrlInfo.stSmartUpgradeInfo.UpgradeType = SMARTUPGRADE_DOWNLOAD_ERROR;
		_MainApp_ShowUpgradeMenu(pstEventInfo);
		break;
	case CBK_EVENT_NETISP_UPGRADE:
		PSREG_SET_VALUE(PSREG_STAT_PlayMode, PSREG_PLAYMODE_PLAY);
		pstEventInfo->stSysAppCtrlInfo.stSmartUpgradeInfo.UpgradeType = SMARTUPGRADE_FROM_NET;
		_MainApp_ShowUpgradeMenu(pstEventInfo);
		break;
#endif
	case CBK_EVENT_SYS_APP_IN_NORMAL_MODE:
	case CBK_EVENT_SYS_APP_IN_COPY_MODE:
	case CBK_EVENT_SYS_APP_IN_DELETE_MODE:
		mainapp_printf("CBK_EVENT_SYS_APP_IN_ %d \n",dParam);
		pstEventInfo->stSysAppCtrlInfo.stFileAppCtrlInfo.eFileAppStatus = dParam;
		break;
	#ifdef SUPPORT_ISO_IMAGE
	case CBK_EVENT_SYS_APP_FILE_LOADER_OPEN:
		fldr_printf("CBK_EVENT_SYS_APP_FILE_LOADER_OPEN %d \n",dParam);
		if ((PSREG_VALUE(PSREG_FILE_LOADER) == PSREG_FILE_LOADER_INIT)
			|| (PSREG_VALUE(PSREG_FILE_LOADER) == PSREG_FILE_LOADER_CLOSE)
		)
		{
			pstEventInfo->stSysDriveCtrlInfo.stSysPrePtnHnd	= pstEventInfo->stSysDriveCtrlInfo.stSysCurPtnHnd;
			PSREG_SET_VALUE(PSREG_FILE_LOADER, PSREG_FILE_LOADER_OPEN);
		}
		break;

	case CBK_EVENT_SYS_APP_FILE_LOADER_STOP:
		fldr_printf("CBK_EVENT_SYS_APP_FILE_LOADER_STOP %d \n",dParam);
		if (PSREG_VALUE(PSREG_FILE_LOADER) == PSREG_FILE_LOADER_PLAY)
		{
			fldr_printf("CBK_EVENT_SYS_APP_FILE_LOADER_STOP \n");
			PSREG_SET_VALUE(PSREG_FILE_LOADER, PSREG_FILE_LOADER_RESUME);
			UINT32 dSysAppIdx = 0;
			int sdRet = 0;
			/*check sys app active state. */
			sdRet = MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
			if ((sdRet == MAIN_APP_SUCCESS) && (dSysAppIdx == SYS_APP_DISC_PLAYER))
			{
				sdRet = MAINAPP_TerminateSystemApp(dSysAppIdx);
				if (sdRet == SYSTEM_APP_SUCCESS)
					SysAppTerminateAckCallBack = _MainApp_FileLoaderReturn2FileApp;
				else
					fldr_printf(" Terminate System App[%d]\n", sdRet);
			}
		}
		break;
	#endif // SUPPORT_ISO_IMAGE
	case CBK_EVENT_SYS_APP_4_STOP_LOADER: //lgz 2010/07/01
		mainapp_printf("CBK_EVENT_SYS_APP_4_STOP_LOADER %d \n",dParam);
		if(PSREG_VALUE(PSREG_STAT_MainAppStatus) == PSREG_MAINAPPSTATUS_DISCLOADING)
			ShareGobj_IF_ShowHintMsg(HINT_MSG_DISC_LOADING, HINT_MSG_TIMEOUT_INFINITE, MSG_PARAM_CLEAR_HINT_MSG);

		DI_DiscStop(gPhisycalLoaderDIHandle);
		break;
	#ifdef SERVO_SPEED_AUTO_CHANGE
	case CBK_EVENT_SYS_APP_NEED_CHANGE_SRV_SPEED://guohao add for ctrl srv speed
		{
			//check cur mount dev is loader
			UINT32 dDriveType = 0;
			if(AppPtn_GetDevType(pstEventInfo->stSysDriveCtrlInfo.stSysCurPtnHnd, &dDriveType)==APPPTN_SUCCESSFUL)
			{
				mainapp_printf("---dDriveType=%d\n",dDriveType); //guohao 2010-12-2 19:58
				if(dDriveType == DEVICE_TYPE_DISC)
				{
					mainapp_printf("CBK_EVENT_SYS_APP_NEED_CHANGE_SRV_SPEED %d \n",dParam);
					_MainApp_ChangeSrvSpeed(dParam);
				}
			}
		}
		break;
	#endif
	default:
		break;
	}
}

#if defined(NET_ET_SUPPORT) || defined(NET_N32_SUPPORT)
static void _MainApp_ActiveDiagMenu(MainAppUIEventHandlerInfo_t * pstEventInfo, UINT32 *dIsNeedOpenUIMenu)
{
#if defined(NET_ET_SUPPORT) && defined(ET_UNSUPPORT_DAEMON_DOWNLOAD)
	if(pstEventInfo->stComGobj.stMainGobj.eMenuType == MAIN_MENU_TYPE_NET_ET)
	{
		UINT32 dIsOnRun = 0;
		ET_CheckHasTaskOnRun(&dIsOnRun);
		if(dIsOnRun)
		{
			*dIsNeedOpenUIMenu = 0;
			pstEventInfo->stComGobj.eDiagType = COMMON_GOBJ_DIAG_ETEXIT;
			if (!GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_DIAG))
				GOBJ_MGR_CreateObject(pMainAppInstance->pGUIObjectTable[MAINAPP_GUIOBJ_DIAG],
									pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_DIAG,
								  	(UINT32) pstEventInfo);
		}
	}
#endif
#if defined(NET_N32_SUPPORT) && defined(N32_UNSUPPORT_DAEMON_DOWNLOAD)
	if(pstEventInfo->stComGobj.stMainGobj.eMenuType == MAIN_MENU_TYPE_NET_N32)
	{
		if(DLMGR_IF_IsTaskOnRun())
		{
			*dIsNeedOpenUIMenu = 0;
			pstEventInfo->stComGobj.eDiagType = COMMON_GOBJ_DIAG_N32EXIT;
			if (!GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_DIAG))
				GOBJ_MGR_CreateObject(pMainAppInstance->pGUIObjectTable[MAINAPP_GUIOBJ_DIAG],
									pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_DIAG,
								  	(UINT32) pstEventInfo);
		}
	}
#endif
}
#endif

static void _MainApp_MaintainGobjOnSource(MainAppUIEventHandlerInfo_t * pstEventInfo,UINT8 bIsKeepSourceMenu)
{
	UINT32 dGobjTotalCnt = 0;
	INT32 sdRet = GUI_OBJMGR_SUCCESS;
	sdRet = GOBJ_MGR_GetTotalCreatedObjects(pMainAppInstance->GUIObjData, &dGobjTotalCnt);
	if (sdRet == GUI_OBJMGR_SUCCESS)
	{
		UINT32 i = 0;
		UINT32 dGobjId = 0;
		UINT32 dIsNeedOpenSourceMenu = 1;
		for (i = 0; i < dGobjTotalCnt; i++)
		{
			sdRet = GOBJ_MGR_GetObjectIDByIndex(pMainAppInstance->GUIObjData, i, &dGobjId);
			if (sdRet == GUI_OBJMGR_SUCCESS)
			{
				mainapp_printf("%s on gobj[%d].\n", __FUNCTION__, dGobjId);
				switch (dGobjId)
				{
				case MAINAPP_GUIOBJ_SOURCE:
					if (!bIsKeepSourceMenu)
					{
						GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_SOURCE);
						dIsNeedOpenSourceMenu = 0;
					}
					break;
				case MAINAPP_SETUPMENU:
				case MAINAPP_SETUPMENUSPKR:
				case MAINAPP_SUPERSETUPMENU:
				case MAINAPP_REGIONMENU:
				case MAINAPP_CUSTOMERMENU:
#if defined(NET_WIFI_SUPPORT)
                                case MAINAPP_SETUPMENUNETWORK:
#endif
					GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, dGobjId);
					ShareGobj_IF_SetRegNormalMode(SHARE_REGION_TOP);
					break;
				case MAINAPP_GUIOBJ_MAINMENU:
					GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, dGobjId);
				#if defined(NET_ET_SUPPORT) || defined(NET_N32_SUPPORT)
					_MainApp_ActiveDiagMenu(pstEventInfo, &dIsNeedOpenSourceMenu);
				#endif
					break;
				default:
					break;
				}
			}
			else
			{
				mainapp_error("%s get obj id ret [%d] index[%d].\n", __FUNCTION__, sdRet, i);
				break;
			}
		}
		if (dIsNeedOpenSourceMenu)
			GOBJ_MGR_CreateObject(pMainAppInstance->pGUIObjectTable[MAINAPP_GUIOBJ_SOURCE], pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_SOURCE,
								  (UINT32) pstEventInfo);
	}
	else
		mainapp_error("%s get total obj cnt [%d] .\n", __FUNCTION__, sdRet);
}

static void _MainApp_UIEventOnSource(MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	UINT32 dMainAppState = MAIN_APP_NORMAL;
	UINT32 dTotalDrvCnt = 0;
	UINT32 i = 0;
#ifdef ET_UNSUPPORT_DAEMON_DOWNLOAD
	UINT32 dIsDLOnRun = 0;
#endif
	//check mounted drv count
	for (i = DEVICE_TYPE_USB; i < DEVICE_TYPE_MAX; i++)
	{
		UINT32 dDrvCnt = 0;
		if (APPPTN_SUCCESSFUL != AppPtn_GetPtnCountByDeviceType(i, &dDrvCnt))
		{
			//if PTNMNG_ERROR_MAINTHREAD_BUSY, also break out.
			mainapp_error("get ptn count fail.\n");
			return;
		}
		dTotalDrvCnt += dDrvCnt;
	}
	//just platform has mounted 1 more drv, can popup source menu
	if(pstEventInfo->stComGobj.stSourceGobj.dMenuTimeOut == 0)
	{
		if (dTotalDrvCnt == 0)
		{
			ShareGobj_IF_ShowHintMsg(HINT_MSG_INVALIDATE_KEY, HINT_MSG_TIMEOUT, MSG_PARAM_NULL); //for mantis 84488
			return;
		}
	}
	else if(pstEventInfo->stComGobj.stSourceGobj.dMenuTimeOut > 0)
	{
		if (dTotalDrvCnt == 0)
		{
			//to home menu
			pstEventInfo->stComGobj.stSourceGobj.dMenuTimeOut = 0;
			MAINAPP_SendGlobalEvent(UI_EVENT_SETUP, MAIN_MENU_TYPE_HOME);
			return;
		}
	}
	/*check main app state,now just normal state do something */
	MAINAPP_GetMainAppState(&dMainAppState);
	if (dMainAppState == MAIN_APP_NORMAL)
	{
		UINT32 dSysAppIdx = TOTAL_SYS_APP_SIZE;
		INT32 sdRet = MAIN_APP_SUCCESS;
		UINT32 dDeviceType = DEVICE_TYPE_MAX;
		SysDriveCtrlInfo_t *pstSysDriveCtrlInfo = &pstEventInfo->stSysDriveCtrlInfo;

		AppPtn_GetDevType(
			pstSysDriveCtrlInfo->stSysCurPtnHnd,
			&dDeviceType);

		if (dDeviceType == DEVICE_TYPE_DISC)
			MainAppDiscInsertCallBack = _MainApp_DataDiscInsertCBK_Stop;

		#ifdef SUPPORT_ISO_IMAGE
		if (PSREG_VALUE(PSREG_FILE_LOADER) == PSREG_FILE_LOADER_PLAY)
			MainAppDiscInsertCallBack = MainApp_DiscFileLoaderCBK_Stop;
		#endif
		/*check sys app active state. */
		sdRet = MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
		switch (sdRet)
		{
		case MAIN_APP_NO_ACTIVE_SYSTEM_APP:
			mainapp_printf("%s on NO_ACTIVE_SYSTEM_APP.\n", __FUNCTION__);
			if (MainAppDiscInsertCallBack)
				MainAppDiscInsertCallBack(pstEventInfo);
				_MainApp_MaintainGobjOnSource(pstEventInfo,0);
			break;

		case MAIN_APP_SUCCESS:
			switch (dSysAppIdx)
			{
			case SYS_APP_DISC_PLAYER:
				mainapp_printf("%s on SYS_APP_DISC_PLAYER.\n", __FUNCTION__);
				break;
			case SYS_APP_FILE_PLAYER:
				mainapp_printf("%s on SYS_APP_FILE_PLAYER.\n", __FUNCTION__);
				if (pstEventInfo->stSysAppCtrlInfo.stFileAppCtrlInfo.eFileAppStatus != FILE_APP_NORMAL)
				{
					mainapp_printf("not in file app normal state\n");
					ShareGobj_IF_ShowHintMsg(HINT_MSG_INVALIDATE_KEY, HINT_MSG_TIMEOUT, MSG_PARAM_NULL);
					return;
				}
				break;
			#ifdef ET_UNSUPPORT_DAEMON_DOWNLOAD
			case SYS_APP_ET_PLAYER:
				ET_CheckHasTaskOnRun(&dIsDLOnRun);
				if(dIsDLOnRun)
				{
					MSG_FILTER_DispatchMessageEX(ETEXIT_WITH_DAEMON_DOWNLOAD, pstEventInfo->dAppEvent);
					return;
				}
				break;
			#endif
			#ifdef N32_UNSUPPORT_DAEMON_DOWNLOAD
			case SYS_APP_N32_PLAYER:
				if(DLMGR_IF_IsTaskOnRun())
				{
					MSG_FILTER_DispatchMessageEX(N32EXIT_WITH_DAEMON_DOWNLOAD, pstEventInfo->dAppEvent);
					return;
				}
				break;
			#endif

			case TOTAL_SYS_APP_SIZE:
				mainapp_printf("%s on TOTAL_SYS_APP_SIZE.\n", __FUNCTION__);
				break;
			default:
				break;
			}

			sdRet = MAINAPP_TerminateSystemApp(dSysAppIdx);
			if (sdRet == SYSTEM_APP_SUCCESS)
				SysAppTerminateAckCallBack = _MainApp_UIEventOnSource;
			else
				mainapp_error(" Terminate System App[%d]\n", sdRet);
			break;

		default:
			mainapp_error("%s on %d condition.\n", __FUNCTION__, sdRet);
			break;
		}
	}
	else
		mainapp_printf("%s on main app state[%d].\n", __FUNCTION__, dMainAppState);
}

static void _MainApp_MaintainGobjOnSetup(MainAppUIEventHandlerInfo_t * pstEventInfo, UINT32 bIsKeepHomeMenu)
{
	UINT32 dGobjTotalCnt = 0;
	INT32 sdRet = GUI_OBJMGR_SUCCESS;
	sdRet = GOBJ_MGR_GetTotalCreatedObjects(pMainAppInstance->GUIObjData, &dGobjTotalCnt);
	if (sdRet == GUI_OBJMGR_SUCCESS)
	{
		UINT32 i = 0;
		UINT32 dGobjId = 0;
		UINT32 dIsNeedOpenHomeMenu = 1;
		for (i = 0; i < dGobjTotalCnt; i++)
		{
			sdRet = GOBJ_MGR_GetObjectIDByIndex(pMainAppInstance->GUIObjData, i, &dGobjId);
			if (sdRet == GUI_OBJMGR_SUCCESS)
			{
				mainapp_printf("%s on gobj[%d].\n", __FUNCTION__, dGobjId);
				switch (dGobjId)
				{
				case MAINAPP_GUIOBJ_MAINMENU:
				case MAINAPP_SMART_UPGRADTE://when upgrading,can not create any obj  for mantis 112466
					if(!bIsKeepHomeMenu)
					{
						GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU);
						dIsNeedOpenHomeMenu = 0;
					}
					break;
				case MAINAPP_SETUPMENU:
				case MAINAPP_SETUPMENUSPKR:
				case MAINAPP_SUPERSETUPMENU:
				case MAINAPP_REGIONMENU:
				case MAINAPP_CUSTOMERMENU:
#if defined(NET_WIFI_SUPPORT)
                                case MAINAPP_SETUPMENUNETWORK:
#endif
					GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, dGobjId);
					ShareGobj_IF_SetRegNormalMode(SHARE_REGION_TOP);
					break;
				case MAINAPP_GUIOBJ_SOURCE:
					GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, dGobjId);
					break;
			#if defined(ET_UNSUPPORT_DAEMON_DOWNLOAD) || defined(N32_UNSUPPORT_DAEMON_DOWNLOAD)
				case MAINAPP_GUIOBJ_DIAG:
					dIsNeedOpenHomeMenu = 0;
					break;
			#endif
				default:
					break;
				}
			}
			else
			{
				mainapp_error("%s get obj id ret [%d] index[%d].\n", __FUNCTION__, sdRet, i);
				break;
			}
		}
		if (dIsNeedOpenHomeMenu)
		{
			GOBJ_MGR_CreateObject(
				pMainAppInstance->pGUIObjectTable[MAINAPP_GUIOBJ_MAINMENU],
				pMainAppInstance->GUIObjData,
				MAINAPP_GUIOBJ_MAINMENU,
				(UINT32) pstEventInfo);
		}
	}
	else
		mainapp_error("%s get total obj cnt [%d] .\n", __FUNCTION__, sdRet);
}

static void _MainApp_MaintainGobjOnNoDisc(MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	UINT32 dGobjTotalCnt = 0;
	INT32 sdRet = GUI_OBJMGR_SUCCESS;
	sdRet = GOBJ_MGR_GetTotalCreatedObjects(pMainAppInstance->GUIObjData, &dGobjTotalCnt);
	if (sdRet == GUI_OBJMGR_SUCCESS)
	{
		UINT32 i = 0;
		UINT32 dGobjId = 0;
		UINT32 dIsNeedOpenHomeMenu = 1;
		for (i = 0; i < dGobjTotalCnt; i++)
		{
			sdRet = GOBJ_MGR_GetObjectIDByIndex(pMainAppInstance->GUIObjData, i, &dGobjId);
			if (sdRet == GUI_OBJMGR_SUCCESS)
			{
				mainapp_printf("%s on gobj[%d].\n", __FUNCTION__, dGobjId);
				switch (dGobjId)
				{
			#if defined(ET_UNSUPPORT_DAEMON_DOWNLOAD) || defined(N32_UNSUPPORT_DAEMON_DOWNLOAD)
				case MAINAPP_GUIOBJ_DIAG:
			#endif
				case MAINAPP_GUIOBJ_MAINMENU:
				case MAINAPP_SETUPMENU:
				case MAINAPP_SMART_UPGRADTE://add by wumin for mantis 126601
					dIsNeedOpenHomeMenu = 0;
					break;
				case MAINAPP_SETUPMENUSPKR:
				case MAINAPP_SUPERSETUPMENU:
				case MAINAPP_REGIONMENU:
				case MAINAPP_CUSTOMERMENU:
#if defined(NET_WIFI_SUPPORT)
                                case MAINAPP_SETUPMENUNETWORK:
#endif
					GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, dGobjId);
					ShareGobj_IF_SetRegNormalMode(SHARE_REGION_TOP);
					break;
				case MAINAPP_GUIOBJ_SOURCE:
					GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, dGobjId);
					break;
				default:
					break;
				}
			}
			else
			{
				mainapp_error("%s get obj id ret [%d] index[%d].\n", __FUNCTION__, sdRet, i);
				break;
			}
		}
#ifdef NET_ISP_SUPPORT
	if((pstEventInfo->stSysAppCtrlInfo.stSmartUpgradeInfo.UpgradeType == SMARTUPGRADE_FROM_NET)
		&&(pstEventInfo->stSysAppCtrlInfo.stSmartUpgradeInfo.stFirst == ISP_IF_NO_ACTIVE_DISCSYS)
		   &&(PSREG_VALUE(PSREG_SMART_UPGRADE) ==PSREG_SMART_UPGRADE_ON)
		        && (pstEventInfo->stNetDriveCtrlInfo.eState == NET_DRIVE_STATE_BOOTUP)) //isp upgrade   //modified by wumin for mantis 108896
			dIsNeedOpenHomeMenu = 0;
#endif

	if((pstEventInfo->stSysAppCtrlInfo.stSmartUpgradeInfo.UpgradeType == SMARTUPGRADE_FROM_LOCAL)
		&&(pstEventInfo->stSysAppCtrlInfo.stSmartUpgradeInfo.stFirst == ISP_IF_NO_ACTIVE_DISCSYS))//lgz 2010/07/07 add for when in upgrade UI not active any sys app//usb upgrade
		dIsNeedOpenHomeMenu = 0;

		if (dIsNeedOpenHomeMenu)
		{
			pstEventInfo->stComGobj.stMainGobj.eMenuType = MAIN_MENU_TYPE_HOME;
			GOBJ_MGR_CreateObject(
				pMainAppInstance->pGUIObjectTable[MAINAPP_GUIOBJ_MAINMENU],
				pMainAppInstance->GUIObjData,
				MAINAPP_GUIOBJ_MAINMENU,
				(UINT32) pstEventInfo);
		}
	}
	else
		mainapp_error("%s get total obj cnt [%d] .\n", __FUNCTION__, sdRet);
}

static void _MainApp_UIEventOnSetup(MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	UINT32 dMainAppState = MAIN_APP_NORMAL;
	/*check main app state,now just normal state do something */
#ifdef ET_UNSUPPORT_DAEMON_DOWNLOAD
	UINT32 dIsOnRun = 0;
#endif
	MAINAPP_GetMainAppState(&dMainAppState);
	if (dMainAppState == MAIN_APP_NORMAL)
	{
		UINT32 dSysAppIdx = TOTAL_SYS_APP_SIZE;
		INT32 sdRet = MAIN_APP_SUCCESS;
		/*check sys app active state. */
		sdRet = MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
		switch (sdRet)
		{
		case MAIN_APP_NO_ACTIVE_SYSTEM_APP:
			mainapp_printf("%s on NO_ACTIVE_SYSTEM_APP.\n", __FUNCTION__);
			_MainApp_MaintainGobjOnSetup(pstEventInfo,0);
			break;

		case MAIN_APP_SUCCESS:
			switch (dSysAppIdx)
			{
			case SYS_APP_DISC_PLAYER:
				mainapp_printf("%s on SYS_APP_DISC_PLAYER.\n", __FUNCTION__);
				/*TODO :  */
				break;
			case SYS_APP_FILE_PLAYER:
				mainapp_printf("%s on SYS_APP_FILE_PLAYER.\n", __FUNCTION__);
				if (pstEventInfo->stSysAppCtrlInfo.stFileAppCtrlInfo.eFileAppStatus != FILE_APP_NORMAL)
				{
					mainapp_printf("not in file app normal state\n");
					ShareGobj_IF_ShowHintMsg(HINT_MSG_INVALIDATE_KEY, HINT_MSG_TIMEOUT, MSG_PARAM_NULL);
					return;
				}
				
				#if 0//def KOK_SUPPORT //xy.zhu add for kok disc 
				if(pstEventInfo->stSysAppCtrlInfo.stFileAppCtrlInfo.eFileAppType == FILE_APP_KOKN32)
				{
					mainapp_printf("Now is kok disc\n");
					ShareGobj_IF_ShowHintMsg(HINT_MSG_INVALIDATE_KEY, HINT_MSG_TIMEOUT, MSG_PARAM_NULL);
					return;
				}
			    #endif
			    
				break;
			#ifdef ET_UNSUPPORT_DAEMON_DOWNLOAD
			case SYS_APP_ET_PLAYER:
				sdRet = ET_CheckHasTaskOnRun(&dIsOnRun);
				if(dIsOnRun && pstEventInfo->stComGobj.stMainGobj.eMenuType == MAIN_MENU_TYPE_HOME)
				{
					MSG_FILTER_DispatchMessageEX(ETEXIT_WITH_DAEMON_DOWNLOAD, pstEventInfo->dAppEvent);
					return;
				}
				break;
			#endif
			#ifdef N32_UNSUPPORT_DAEMON_DOWNLOAD
			case SYS_APP_N32_PLAYER:
				if(DLMGR_IF_IsTaskOnRun() && pstEventInfo->stComGobj.stMainGobj.eMenuType == MAIN_MENU_TYPE_HOME)
				{
					MSG_FILTER_DispatchMessageEX(N32EXIT_WITH_DAEMON_DOWNLOAD, pstEventInfo->dAppEvent);
					return;
				}
				break;
			#endif

			case TOTAL_SYS_APP_SIZE:
				mainapp_printf("%s on TOTAL_SYS_APP_SIZE.\n", __FUNCTION__);
				break;
			default:
				break;
			}

			sdRet = MAINAPP_TerminateSystemApp(dSysAppIdx);
			if (sdRet == SYSTEM_APP_SUCCESS)
				SysAppTerminateAckCallBack = _MainApp_UIEventOnSetup;
			else
				mainapp_error(" Terminate System App[%d]\n", sdRet);
			break;

		default:
			mainapp_error("%s on %d condition.\n", __FUNCTION__, sdRet);
			break;
		}
	}
	else
		mainapp_printf("%s on main app state[%d].\n", __FUNCTION__, dMainAppState);
}

inline int checkState(void)
{
	return CloseState;
}

static void _MainApp_MaintainGobjOnEject(MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	UINT32 dGobjTotalCnt = 0;
	INT32 sdRet = GUI_OBJMGR_SUCCESS;

	//app_set_DisplayMixerMode(MIXER_UI_720P);
	//APP_Show_Logo(LOGO_UI,LOGO_WITH_WORD);//jiangxl 20101220
	
	sdRet = GOBJ_MGR_GetTotalCreatedObjects(pMainAppInstance->GUIObjData, &dGobjTotalCnt);
	if (sdRet == GUI_OBJMGR_SUCCESS)
	{
		UINT32 i = 0;
		UINT32 dGobjId = 0;
		for (i = 0; i < dGobjTotalCnt; i++)
		{
			sdRet = GOBJ_MGR_GetObjectIDByIndex(pMainAppInstance->GUIObjData, i, &dGobjId);
			if (sdRet == GUI_OBJMGR_SUCCESS)
			{
				mainapp_printf("%s on gobj[%d].\n", __FUNCTION__, dGobjId);
				switch (dGobjId)
				{
				case MAINAPP_GUIOBJ_SOURCE:
					GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_SOURCE);
					break;
				case MAINAPP_SETUPMENU:
				case MAINAPP_SETUPMENUSPKR:
				case MAINAPP_SUPERSETUPMENU:
				case MAINAPP_REGIONMENU:
				case MAINAPP_CUSTOMERMENU:
#if defined(NET_WIFI_SUPPORT)
                                case MAINAPP_SETUPMENUNETWORK:
#endif
					GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, dGobjId);
					ShareGobj_IF_SetRegNormalMode(SHARE_REGION_TOP);
					break;
				case MAINAPP_GUIOBJ_MAINMENU:
					GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU);
					break;
				default:
					break;
				}
			}
			else
			{
				mainapp_error("%s get obj id ret [%d] index[%d].\n", __FUNCTION__, sdRet, i);
				break;
			}
		}
	}
	else
		mainapp_error("%s get total obj cnt [%d] .\n", __FUNCTION__, sdRet);
}
static void _MainApp_UIEventOnEject(MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	UINT32* dSuperPW=pstEventInfo->stComGobj.stSuperSetupGobj.dSuperPW;
	int i;
	UINT32 dDeviceType = 0;
#ifdef ET_UNSUPPORT_DAEMON_DOWNLOAD
	UINT32 dIsOnRun = 0;
#endif
	UINT32 dMainAppState = MAIN_APP_NORMAL;
	UINT32 dDiscLoaderId = DI_Handle_PHYSICAL;
       #ifdef USB_MODE_POWEROFF_SERVO
	if (pstEventInfo->bDevmode == 1)
       	{
	DI_DiscResume(gPhisycalLoaderDIHandle);
	pstEventInfo->bDevmode = 0;
       	}
       #endif
	/*check main app state,now just normal state do something */
	MAINAPP_GetMainAppState(&dMainAppState);
	if (dMainAppState == MAIN_APP_NORMAL)
	{
		UINT32 dSysAppIdx = TOTAL_SYS_APP_SIZE;
		INT32 sdRet = MAIN_APP_SUCCESS;
		/*check sys app active state. */
		sdRet = MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
		switch (sdRet)
		{
		case MAIN_APP_NO_ACTIVE_SYSTEM_APP:
		#ifdef ET_UNSUPPORT_DAEMON_DOWNLOAD
			if(pstEventInfo->stComGobj.stMainGobj.eMenuType == MAIN_MENU_TYPE_NET_ET)
			{
				ET_CheckHasTaskOnRun(&dIsOnRun);
				if(dIsOnRun)
				{
					if(GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU))
						GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU);

					pstEventInfo->stComGobj.eDiagType = COMMON_GOBJ_DIAG_ETEXIT;
					if (!GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_DIAG))
						GOBJ_MGR_CreateObject(pMainAppInstance->pGUIObjectTable[MAINAPP_GUIOBJ_DIAG],
											pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_DIAG,
										  	(UINT32) pstEventInfo);
					return;
				}
			}
		#endif
		#ifdef N32_UNSUPPORT_DAEMON_DOWNLOAD
			if(pstEventInfo->stComGobj.stMainGobj.eMenuType == MAIN_MENU_TYPE_NET_N32)
			{
				if(DLMGR_IF_IsTaskOnRun())
				{
					if(GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU))
						GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU);

					pstEventInfo->stComGobj.eDiagType = COMMON_GOBJ_DIAG_N32EXIT;
					if (!GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_DIAG))
						GOBJ_MGR_CreateObject(pMainAppInstance->pGUIObjectTable[MAINAPP_GUIOBJ_DIAG],
											pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_DIAG,
										  	(UINT32) pstEventInfo);
					return;
				}
			}
		#endif
		    #ifdef SUPPORT_ISO_IMAGE  //lgz 2011-3-1 18:53 add for mantis 0126131
		    if(PSREG_VALUE(PSREG_FILE_LOADER) == PSREG_FILE_LOADER_PLAY)
		    {
		    	if(MainAppDiscInsertCallBack)
					MainAppDiscInsertCallBack(pstEventInfo);
		    }		   
		    #endif
			mainapp_printf("%s on NO_ACTIVE_SYSTEM_APP.\n", __FUNCTION__);
			_MainApp_MaintainGobjOnEject(pstEventInfo);
#ifdef TESTBED_USE_LOADER
			sdRet = DI_TrayIn(gPhisycalLoaderDIHandle);
			if (sdRet == DIMON_IF_INVALIDCTL)
			{
				mainapp_printf("Tray Out\n");
				sdRet = DI_TrayOut(gPhisycalLoaderDIHandle);
				if (sdRet == DIMON_IF_SUCCESSFUL)
				{
					AppPtn_GetDevType( pstEventInfo->stSysDriveCtrlInfo.stSysCurPtnHnd, &dDeviceType);
					AppPtn_DiscEject(dDiscLoaderId);  //for mantis 100821
					pstEventInfo->eTrayState = PHYSICAL_TRAY_OUT;
					pstEventInfo->stSysDriveCtrlInfo.stSysCurPtnHnd = NULL;
					pstEventInfo->bIsHoldHomeMenu = 0;
					switch (dDeviceType)
					{
					case DEVICE_TYPE_DISC:
						//_MainApp_AutoSWSysDrive(pstEventInfo);
						break;
					case DEVICE_TYPE_USB:
						mainapp_printf("[attention] Tray Out, on DEVICE_TYPE_USB\n");
						break;
					case DEVICE_TYPE_CARD:
						mainapp_printf("[attention] Tray Out, on DEVICE_TYPE_CARD\n");
						break;
					case DEVICE_TYPE_HDD:
						mainapp_printf("[attention] Tray Out, on DEVICE_TYPE_HDD\n");
						break;
					default:
						mainapp_printf("[attention] Tray Out, on unknown device type\n");
						break;
					}

					APP_Show_Logo(LOGO_UI, LOGO_WITH_WORD);//jiangxl 
					ShareGobj_IF_ShowHintMsg(HINT_MSG_DISC_LOADING, HINT_MSG_TIMEOUT_INFINITE, MSG_PARAM_CLEAR_HINT_MSG);
					ShareGobj_IF_ShowHintMsg(HINT_MSG_OPEN, HINT_MSG_TIMEOUT, MSG_PARAM_RESET_BG_MSG);
					PSREG_SET_VALUE(PSREG_STAT_MainAppStatus, PSREG_MAINAPPSTATUS_TRAYOUT);
					for(i=0;i<SUPER_PASSWORD_LEN;i++) //clear Super Setup Menu PW
						dSuperPW[i]=0;
				}
				else
					mainapp_error("Tray Out\n");
			}
			else
			{
				pstEventInfo->eTrayState = PHYSICAL_TRAY_IN;
				mainapp_printf("Tray In\n");
				ShareGobj_IF_ShowHintMsg(HINT_MSG_CLOSE, HINT_MSG_TIMEOUT, MSG_PARAM_RESET_BG_MSG);
				PSREG_SET_VALUE(PSREG_STAT_MainAppStatus, PSREG_MAINAPPSTATUS_TRAYIN);
				for(i=0;i<SUPER_PASSWORD_LEN;i++) //clear Super Setup Menu PW
					dSuperPW[i]=0;
			}
#else
			if (checkState() == OpenState)	//open param
			{
				usb_disk_atapi_closetry((UINT8 *) "/dev/sda/0");
				ShareGobj_IF_ShowHintMsg(HINT_MSG_CLOSE, HINT_MSG_TIMEOUT, MSG_PARAM_RESET_BG_MSG);
			}
			else
			{
				umount((char *)"/dev/sda/0");
				usb_disk_atapi_opentry((UINT8 *) "/dev/sda/0");
				ShareGobj_IF_ShowHintMsg(HINT_MSG_OPEN, HINT_MSG_TIMEOUT, MSG_PARAM_RESET_BG_MSG);
			}
#endif
			break;

		case MAIN_APP_SUCCESS:
			switch (dSysAppIdx)
			{
			case SYS_APP_DISC_PLAYER:
				mainapp_printf("%s on SYS_APP_DISC_PLAYER.\n", __FUNCTION__);
				/*TODO :  */
			    #ifdef SUPPORT_ISO_IMAGE  //lgz 2011-3-1 18:53 add for mantis 0126131
			    if(PSREG_VALUE(PSREG_FILE_LOADER) == PSREG_FILE_LOADER_PLAY)
			    {
			    	MainAppDiscInsertCallBack=MainApp_FileLoaderReturn2Home;
			    }
				else
			    #endif				
			    {
       			sdRet = DI_TrayIn(gPhisycalLoaderDIHandle);
				if (sdRet == DIMON_IF_INVALIDCTL)
				{
					mainapp_printf("Tray Out\n");

					stUIEventHandlerInfo.bIsDiscEjecting=TRUE;//add by george.chang 2010.09.29
					SYSAPP_IF_SendCriticalGlobalEventWithIndex(SYS_APP_DISC_PLAYER, UI_EVENT_EJECT,0);
					SysAppTerminateAckCallBack = NULL;


				}
				else
				{
					mainapp_printf("Tray In\n");
					pstEventInfo->eTrayState = PHYSICAL_TRAY_IN;
					ShareGobj_IF_ShowHintMsg(HINT_MSG_CLOSE, HINT_MSG_TIMEOUT, MSG_PARAM_RESET_BG_MSG);
					PSREG_SET_VALUE(PSREG_STAT_MainAppStatus, PSREG_MAINAPPSTATUS_TRAYIN);
				}

				_MainApp_MaintainGobjOnEject(pstEventInfo);
				return;
			    }

				break;
			case SYS_APP_FILE_PLAYER:
				mainapp_printf("%s on SYS_APP_FILE_PLAYER.\n", __FUNCTION__);
				if (pstEventInfo->stSysAppCtrlInfo.stFileAppCtrlInfo.eFileAppStatus != FILE_APP_NORMAL)
				{
					mainapp_printf("not in file app normal state\n");
					ShareGobj_IF_ShowHintMsg(HINT_MSG_INVALIDATE_KEY, HINT_MSG_TIMEOUT, MSG_PARAM_NULL);
					return;
				}
				break;
			#ifdef ET_UNSUPPORT_DAEMON_DOWNLOAD
			case SYS_APP_ET_PLAYER:
				sdRet = ET_CheckHasTaskOnRun(&dIsOnRun);
				if(dIsOnRun)
				{
					MSG_FILTER_DispatchMessageEX(ETEXIT_WITH_DAEMON_DOWNLOAD, pstEventInfo->dAppEvent);
					return;
				}
				break;
			#endif
			#ifdef N32_UNSUPPORT_DAEMON_DOWNLOAD
			case SYS_APP_N32_PLAYER:
				if(DLMGR_IF_IsTaskOnRun())
				{
					MSG_FILTER_DispatchMessageEX(N32EXIT_WITH_DAEMON_DOWNLOAD, pstEventInfo->dAppEvent);
					return;
				}
				break;
			#endif
			case TOTAL_SYS_APP_SIZE:
				mainapp_printf("%s on TOTAL_SYS_APP_SIZE.\n", __FUNCTION__);
				break;
			default:
				break;
			}

			sdRet = MAINAPP_TerminateSystemApp(dSysAppIdx);
			if (sdRet == SYSTEM_APP_SUCCESS)
				SysAppTerminateAckCallBack = _MainApp_UIEventOnEject;
			else
				mainapp_error("Terminate System App[%d]\n", sdRet);

			if(MAINAPP_IsNetworkAP(dSysAppIdx)) //added by ted.chen for Mantis 0106522 & 0107204
			{
				MCF_SwitchToOriginalMode();
				if(pstEventInfo->bInETMode)
				{
					DISP_Win_t stPanelWin;
					MID_DISP_Format_e eCurrFormat;
					MID_DISP_GetDmixPlane(&eCurrFormat, &stPanelWin);
					if (eCurrFormat >= FMT_720P_60)
						GL_AVCSelectBW(BW_MODE_8203D_HD);
					else
						GL_AVCSelectBW(BW_MODE_8203D_SD);
					APP_Show_Logo(LOGO_UI,LOGO_WITH_WORD);
					DI_DiscResume(gPhisycalLoaderDIHandle);
					pstEventInfo->bInETMode=0;
				}
			}
			break;

		default:
			mainapp_error("%s on %d condition.\n", __FUNCTION__, sdRet);
			break;
		}
	}
	else
		mainapp_printf("%s on main app state[%d].\n", __FUNCTION__, dMainAppState);
}

static void _MainApp_UIEventOnPlay(MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	UINT32 dMainAppState = MAIN_APP_NORMAL;
	/*check main app state,now just normal state do something */
	MAINAPP_GetMainAppState(&dMainAppState);
	if (dMainAppState == MAIN_APP_NORMAL)
	{
		UINT32 dSysAppIdx = TOTAL_SYS_APP_SIZE;
		INT32 sdRet = MAIN_APP_SUCCESS;
		/*check sys app active state. */
		sdRet = MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
		switch (sdRet)
		{
			case MAIN_APP_NO_ACTIVE_SYSTEM_APP:
				mainapp_printf("UI_EVENT_PLAY received on NO_ACTIVE_SYSTEM_APP.\n");
			#ifdef TESTBED_USE_LOADER
				sdRet = DI_TrayIn(gPhisycalLoaderDIHandle);
				if (sdRet == DIMON_IF_INVALIDCTL)
				{
					mainapp_printf("Tray Out\n");
					return;
				}
				else
				{
					pstEventInfo->eTrayState = PHYSICAL_TRAY_IN;
					mainapp_printf("Tray In\n");
					ShareGobj_IF_ShowHintMsg(HINT_MSG_CLOSE, HINT_MSG_TIMEOUT, MSG_PARAM_RESET_BG_MSG);
					PSREG_SET_VALUE(PSREG_STAT_MainAppStatus, PSREG_MAINAPPSTATUS_TRAYIN);
				}
			#else
				if (checkState() == OpenState)	//open param
				{
					usb_disk_atapi_closetry((UINT8 *) "/dev/sda/0");
					ShareGobj_IF_ShowHintMsg(HINT_MSG_CLOSE, HINT_MSG_TIMEOUT, MSG_PARAM_RESET_BG_MSG);
				}
				else
					return;
			#endif
				_MainApp_MaintainGobjOnEject(pstEventInfo);//Same action as eject tray in.
				break;

			default:
				mainapp_printf("UI_EVENT_SETUP received on error : %d condition.\n", sdRet);
				break;
		}
	}
	else
		mainapp_printf("UI_EVENT_PLAY received not on MAIN_APP_NORMAL.\n");
}

static void _MainApp_ShowResolutionHintMsg(void)
{
	DISP_Win_t stPanelWin;
	MID_DISP_Format_e eCurrFormat;
	UINT32 bCurRes;

	MID_DISP_GetDmixPlane(&eCurrFormat, &stPanelWin);

	switch(eCurrFormat)
			{
			case FMT_480P_60:
			case FMT_576P_50:
				bCurRes = HINT_MSG_RESOLUTION_480P_576P;
				break;
			case FMT_480I_60:
			case FMT_576I_50:
				bCurRes = HINT_MSG_RESOLUTION_480I_576I;
				break;
			case FMT_720P_60:
			case FMT_720P_50:
			case FMT_720P_24:
				bCurRes = HINT_MSG_RESOLUTION_720P;
				break;
			case FMT_1080I_60:
			case FMT_1080I_50:
				bCurRes = HINT_MSG_RESOLUTION_1080I;
				break;
			case FMT_1080P_60:
			case FMT_1080P_50:
			case FMT_1080P_24:
				bCurRes = HINT_MSG_RESOLUTION_1080P;
				break;
			default:
				bCurRes = HINT_MSG_INVALIDATE_KEY;
				break;
			}

	ShareGobj_IF_ShowHintMsg(bCurRes, HINT_MSG_TIMEOUT_7S, MSG_PARAM_NULL);
}

static void _MainApp_CloseSetupRelatedMenu(void)
{//close all menus on R4 except Setup Menu & Speaker Menu
	UINT32 dGobjTotalCnt = 0;
	INT32 sdRet = GUI_OBJMGR_SUCCESS;
	sdRet = GOBJ_MGR_GetTotalCreatedObjects(pMainAppInstance->GUIObjData, &dGobjTotalCnt);
	if (sdRet == GUI_OBJMGR_SUCCESS)
	{
		UINT32 i = 0;
		UINT32 dGobjId = 0;
		for (i = 0; i < dGobjTotalCnt; i++)
		{
			sdRet = GOBJ_MGR_GetObjectIDByIndex(pMainAppInstance->GUIObjData, i, &dGobjId);
			if (sdRet == GUI_OBJMGR_SUCCESS)
			{
				switch(dGobjId)
				{
				case MAINAPP_SUPERSETUPMENU:
				case MAINAPP_REGIONMENU:
				case MAINAPP_CUSTOMERMENU:
#if defined(NET_WIFI_SUPPORT)
                                case MAINAPP_SETUPMENUNETWORK:
#endif
					GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, dGobjId);
					ShareGobj_IF_SetRegNormalMode(SHARE_REGION_TOP);
					break;
				default:
					break;
				}
			}
		}
	}
}

int _MainApp_FinalizeDone(void)
{
	//Berfore do this function, Active System App should be terminated!
	MAINAPP_Finalize();
	//bMPBreakCondition = 1;
	return MAIN_APP_SUCCESS;
}

void _MainApp_Finalizing(void)
{
	UINT32 dActSysAppIndex = 0;
	INT32 sdRet = 0;
	//------------------------------------------------------------ (1)
	//Disable Output Signal first. (Video DAC, Audio DAC, HDMI)
	//HDMI_IF_Set_TMDSOff();
	HDMI_IF_Set_Power(HDMI_POWER_OFF);
	AUDIF_PowerDown_DAC();
	SetAvpdEnable(SEL_OFF);
	SetDacDisable(DAC_DISABLE, DAC_DISABLE, DAC_DISABLE, DAC_DISABLE);
	//Un-do CEC, and send <inactive source> command.
	DRV_CEC_Finalize();
	//???
	DEVCBK_Finalize();
	//------------------------------------------------------------ (2)
	sdRet = MAINAPP_GetActiveSystemAppIndex(&dActSysAppIndex);

	if((sdRet == MAIN_APP_SUCCESS)||(sdRet == MAIN_APP_IN_TRANSITION))
	{
		//Note: if MAIN_APP_IN_TRANSITION, will get terminated ACK soon.
		MAINAPP_SetStandbyTimeOutProcedure( _MainApp_FinalizeDone );
		MAINAPP_SetStandbyMode(MAINAPP_STANDBY);

		sdRet = MAINAPP_TerminateSystemApp(dActSysAppIndex);

		if ((sdRet == MAIN_APP_SUCCESS) || (sdRet == MAIN_APP_IN_TRANSITION))
		{
			//Before doing _MainApp_FinalizeDone, Active App must be finalized.
			//It will be done in Terminated ACK.

		}
		else
			_MainApp_FinalizeDone();
	}
	else //MAIN_APP_NO_ACTIVE_SYSTEM_APP or etc.
		_MainApp_FinalizeDone();
}

UINT8 MAINAPP_IsNetworkAP(UINT32 dSysAppIdx)
{
	UINT8 bIsNetworkAP=0;
	switch(dSysAppIdx)
	{
#ifdef NET_ET_SUPPORT
		case SYS_APP_ET_PLAYER:
#endif
#ifdef NET_N32_SUPPORT
		case SYS_APP_N32_PLAYER:
#endif
#ifdef NET_NEWS_SUPPORT
		case SYS_APP_NEWS_PLAYER:
#endif
#ifdef NET_STOCK_SUPPORT
		case SYS_APP_STOCK_PLAYER:
#endif
#ifdef NET_WEATHER_SUPPORT
		case SYS_APP_YWEATHER:
#endif
#ifdef NET_PHOTO_SUPPORT
		case SYS_APP_NETPHOTO_PLAYER:
#endif
#ifdef NET_GMAP_SUPPORT
		case SYS_APP_GMAP:
#endif
#if defined(NET_RADIO_SUPPORT) || defined(NET_LOCAL_RADIO_SUPPORT)
		case SYS_APP_NETRADIO_PLAYER:
#endif
#ifdef NET_BROWSER_SUPPORT
		case SYS_APP_BROWSER:
#endif
#ifdef NET_VOD_SUPPORT
                  case SYS_APP_VOD_PLAYER:
#endif 
		bIsNetworkAP=1;
		break;
	}
	return bIsNetworkAP;
}

static void _MAINAPP_SendEvent4NetworkAPs(UINT32 dSysAppIdx, UINT32 dMessage, UINT32 dParam)
{//this func is used for the events, which are list in MainAppMessageDefaultTable[], needed by Network APs
	if(MAINAPP_IsNetworkAP(dSysAppIdx))
		SYSAPP_IF_SendGlobalEventWithIndex(dSysAppIdx,dMessage,dParam);
}

static void _MainApp_SetupRelatedMenu2MainMenu(MainAppUIEventHandlerInfo_t * pstEventInfo, MAIN_APP_GUI_OBJ_ID eCurrGUIObjID)
{
	if (GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, eCurrGUIObjID))
	{
		GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, eCurrGUIObjID);
		ShareGobj_IF_SetRegNormalMode(SHARE_REGION_TOP);

		GOBJ_MGR_CreateObject(pMainAppInstance->pGUIObjectTable[MAINAPP_GUIOBJ_MAINMENU], pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU,
							  (UINT32) pstEventInfo);
	}
}

static void _MainApp_UIEventOnResolution(UINT32 dParam, MainAppUIEventHandlerInfo_t *pstEventInfo)
{
	if(PSREG_VALUE(PSREG_SETUP_Output_Component) != PSREG_SETUP_YCbCr)
		return;

	_MainApp_CloseSetupRelatedMenu();
	_MainApp_SetupRelatedMenu2MainMenu(pstEventInfo, MAINAPP_SETUPMENU); 
	_MainApp_SetupRelatedMenu2MainMenu(pstEventInfo, MAINAPP_SETUPMENUSPKR); 

	if(PSREG_IS_INVALID(PSREG_SETUP_YCbCrHDMI_Resolution))
		PSREG_SET_VALID(PSREG_SETUP_YCbCrHDMI_Resolution);

	UINT32 dYCbCrHDMI_Resolution = PSREG_VALUE(PSREG_SETUP_YCbCrHDMI_Resolution);

	UINT8 bFindItmeCount=0;
	UINT8 bTotalRes=5; //5 is the total kinds of resolution
	setupmenu_item_id_t eItemID=-1;
	ShareGobjHintMsg_e eHintMsg_R1 = HINT_MSG_NULL;
	ShareGobjHintMsgTimeOut_e eHintMsg_TimeOut_R1 = HINT_MSG_TIMEOUT_NULL;
	ShareGobj_IF_GetA1HintMsg(&eHintMsg_R1, &eHintMsg_TimeOut_R1);
	diag_printf("[%d]The get eHintMsg_R1 value is : %d \n",__LINE__,eHintMsg_R1);
	if((eHintMsg_R1 == HINT_MSG_RESOLUTION_480I_576I)||(eHintMsg_R1 == HINT_MSG_RESOLUTION_480P_576P)\
		||(eHintMsg_R1 == HINT_MSG_RESOLUTION_720P)||(eHintMsg_R1 == HINT_MSG_RESOLUTION_1080I)||(eHintMsg_R1 == HINT_MSG_RESOLUTION_1080P))
	{
	do
	{
		switch(dYCbCrHDMI_Resolution) //map PSR values to item IDs & orderly chg resolution
		{//following cases list all resolutions that system supported but they are >= Setup Menu customization ones
		case PSREG_SETUP_Resolution_Auto:
			dYCbCrHDMI_Resolution = PSREG_SETUP_480i_576i;
			eItemID=L3_ITEM_480i_576i;
			break;
		case PSREG_SETUP_480i_576i:
			dYCbCrHDMI_Resolution=PSREG_SETUP_480p_576p;
			eItemID=L3_ITEM_480p_576p;
			break;
		case PSREG_SETUP_480p_576p:
			dYCbCrHDMI_Resolution=PSREG_SETUP_720p;
			eItemID=L3_ITEM_720p;
			break;
		case PSREG_SETUP_720p:
			dYCbCrHDMI_Resolution=PSREG_SETUP_1080i;
			eItemID=L3_ITEM_1080i;
			break;
		case PSREG_SETUP_1080i:
			dYCbCrHDMI_Resolution=PSREG_SETUP_1080p;
			eItemID=L3_ITEM_1080p;
			break;
		case PSREG_SETUP_1080p:
			dYCbCrHDMI_Resolution=PSREG_SETUP_480i_576i;
			eItemID=L3_ITEM_480i_576i;
			break;
		}
		bFindItmeCount++;
	}
	while(!SetupMenu_FindItem(eItemID,1) && bFindItmeCount<bTotalRes);
	}
	if(bFindItmeCount>=bTotalRes)
		dYCbCrHDMI_Resolution=PSREG_SETUP_Resolution_Auto;
	PSREG_SET_VALUE(PSREG_SETUP_YCbCrHDMI_Resolution, dYCbCrHDMI_Resolution);
	MainApp_SetupAutoSettings(pstEventInfo);
	_MainApp_ShowResolutionHintMsg();

	#ifdef SUPPORT_VFD_DRIVER
	DRV_Status_t eDrvStatus = DRV_SUCCESS;
	eDrvStatus = DRV_VfdUpdateData(VFD_EVENT_ResolutionChg,0);
	if(eDrvStatus != DRV_SUCCESS)
		mainapp_printf("%s is called at line %d return value is %d.\n", __FUNCTION__, __LINE__, eDrvStatus);
	#endif
}

static void _MainApp_ShowPNSwitchHintMsg(UINT32 dCurTVSystem)
{
	switch(dCurTVSystem)
	{
	case PSREG_SETUP_PAL:
		ShareGobj_IF_ShowHintMsg(HINT_MSG_TVSYSTEM_PAL, HINT_MSG_TIMEOUT_7S, MSG_PARAM_NULL);
		break;
	case PSREG_SETUP_NTSC:
		ShareGobj_IF_ShowHintMsg(HINT_MSG_TVSYSTEM_NTSC, HINT_MSG_TIMEOUT_7S, MSG_PARAM_NULL);
		break;
	case PSREG_SETUP_TV_System_Auto:
		ShareGobj_IF_ShowHintMsg(HINT_MSG_TVSYSTEM_AUTO, HINT_MSG_TIMEOUT_7S, MSG_PARAM_NULL);
		break;
	default:
		break;
	}
}

static void _MainApp_UIEventOnPNSwitch(UINT32 dParam, MainAppUIEventHandlerInfo_t *pstEventInfo)
{
	UINT32 dTVSystem = PSREG_VALUE(PSREG_SETUP_TV_System);
	if(PSREG_IS_INVALID(PSREG_SETUP_TV_System))
		PSREG_SET_VALID(PSREG_SETUP_TV_System);

	_MainApp_CloseSetupRelatedMenu();
	_MainApp_SetupRelatedMenu2MainMenu(pstEventInfo, MAINAPP_SETUPMENU); 
	_MainApp_SetupRelatedMenu2MainMenu(pstEventInfo, MAINAPP_SETUPMENUSPKR); 

	setupmenu_item_id_t eItemID=-1;
	do
	{
		switch(dTVSystem)
		{
		case PSREG_SETUP_PAL:
			dTVSystem = PSREG_SETUP_NTSC;
			eItemID = L3_ITEM_NTSC;
			break;
		case PSREG_SETUP_NTSC:
			dTVSystem = PSREG_SETUP_TV_System_Auto;
			eItemID = L3_ITEM_TV_System_Auto;
			break;
		case PSREG_SETUP_TV_System_Auto:
			dTVSystem = PSREG_SETUP_PAL;
			eItemID = L3_ITEM_PAL;
			break;
		}
	}
	while(!SetupMenu_FindItem(eItemID,1));
	PSREG_SET_VALUE(PSREG_SETUP_TV_System, dTVSystem);
	MainApp_SetupAutoSettings(pstEventInfo);

	//UINT32 dSysAppIdx = TOTAL_SYS_APP_SIZE;
	//MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
	//_MAINAPP_SendEvent4NetworkAPs(dSysAppIdx, pstEventInfo->dAppEvent, dParam);
	_MainApp_ShowPNSwitchHintMsg(dTVSystem);

	#ifdef SUPPORT_VFD_DRIVER
	DRV_Status_t eDrvStatus = DRV_SUCCESS;
	eDrvStatus = DRV_VfdUpdateData(VFD_EVENT_PNSwitch,0);
	if(eDrvStatus != DRV_SUCCESS)
		mainapp_printf("%s is called at line %d return value is %d.\n", __FUNCTION__, __LINE__, eDrvStatus);
	#endif
}


static void _MainApp_MaintainGobjOnSuperMenu(MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	UINT32 dGobjTotalCnt = 0;
	INT32 sdRet = GUI_OBJMGR_SUCCESS;
	sdRet = GOBJ_MGR_GetTotalCreatedObjects(pMainAppInstance->GUIObjData, &dGobjTotalCnt);
	if (sdRet == GUI_OBJMGR_SUCCESS)
	{
		UINT32 i = 0;
		UINT32 dGobjId = 0;
		UINT32 dIsNeedOpenSuperMenu = 1;
		for (i = 0; i < dGobjTotalCnt; i++)
		{
			sdRet = GOBJ_MGR_GetObjectIDByIndex(pMainAppInstance->GUIObjData, i, &dGobjId);
			if (sdRet == GUI_OBJMGR_SUCCESS)
			{
				mainapp_printf("%s on gobj[%d].\n", __FUNCTION__, dGobjId);
				switch (dGobjId)
				{
				case MAINAPP_GUIOBJ_SOURCE:
					GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_SOURCE);
					break;
				case MAINAPP_SUPERSETUPMENU:
					GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, dGobjId);
					ShareGobj_IF_SetRegNormalMode(SHARE_REGION_TOP);
					dIsNeedOpenSuperMenu = 0;
					break;
				case MAINAPP_SETUPMENU:
				case MAINAPP_SETUPMENUSPKR:
				case MAINAPP_REGIONMENU:
				case MAINAPP_CUSTOMERMENU:
#if defined(NET_WIFI_SUPPORT)
                                case MAINAPP_SETUPMENUNETWORK:
#endif
					GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, dGobjId);
					ShareGobj_IF_SetRegNormalMode(SHARE_REGION_TOP);
					break;
				case MAINAPP_GUIOBJ_MAINMENU:
					GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, dGobjId);
					break;
				default:
					break;
				}
			}
			else
			{
				mainapp_error("%s get obj id ret [%d] index[%d].\n", __FUNCTION__, sdRet, i);
				break;
			}
		}
		if (dIsNeedOpenSuperMenu)
		{
			ShareGobj_IF_SetRegHideMode(SHARE_REGION_TOP);
			GOBJ_MGR_CreateObject(pMainAppInstance->pGUIObjectTable[MAINAPP_SUPERSETUPMENU], pMainAppInstance->GUIObjData, MAINAPP_SUPERSETUPMENU, 0);
		}
	}
	else
		mainapp_error("%s get total obj cnt [%d] .\n", __FUNCTION__, sdRet);
}
#if defined(SUPPORT_REGION_MENU)
static void _MainApp_MaintainGobjOnRegionMenu(MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	UINT32 dGobjTotalCnt = 0;
	INT32 sdRet = GUI_OBJMGR_SUCCESS;
	sdRet = GOBJ_MGR_GetTotalCreatedObjects(pMainAppInstance->GUIObjData, &dGobjTotalCnt);
	if (sdRet == GUI_OBJMGR_SUCCESS)
	{
		UINT32 i = 0;
		UINT32 dGobjId = 0;
		UINT32 dIsNeedOpenRegionMenu = 1;
		for (i = 0; i < dGobjTotalCnt; i++)
		{
			sdRet = GOBJ_MGR_GetObjectIDByIndex(pMainAppInstance->GUIObjData, i, &dGobjId);
			if (sdRet == GUI_OBJMGR_SUCCESS)
			{
				mainapp_printf("%s on gobj[%d].\n", __FUNCTION__, dGobjId);
				switch (dGobjId)
				{
				case MAINAPP_GUIOBJ_SOURCE:
					GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_SOURCE);
					break;
				case MAINAPP_REGIONMENU:
					GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, dGobjId);
					ShareGobj_IF_SetRegNormalMode(SHARE_REGION_TOP);
					dIsNeedOpenRegionMenu = 0;
					break;
				case MAINAPP_SETUPMENU:
				case MAINAPP_SETUPMENUSPKR:
				case MAINAPP_SUPERSETUPMENU:
				case MAINAPP_CUSTOMERMENU:
#if defined(NET_WIFI_SUPPORT)
                                case MAINAPP_SETUPMENUNETWORK:
#endif
					GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, dGobjId);
					ShareGobj_IF_SetRegNormalMode(SHARE_REGION_TOP);
					break;
				case MAINAPP_GUIOBJ_MAINMENU:
					GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, dGobjId);
					break;
				default:
					break;
				}
			}
			else
			{
				mainapp_error("%s get obj id ret [%d] index[%d].\n", __FUNCTION__, sdRet, i);
				break;
			}
		}
		if (dIsNeedOpenRegionMenu)
		{
			ShareGobj_IF_SetRegHideMode(SHARE_REGION_TOP);
			GOBJ_MGR_CreateObject(pMainAppInstance->pGUIObjectTable[MAINAPP_REGIONMENU], pMainAppInstance->GUIObjData, MAINAPP_REGIONMENU, 0);
		}
	}
	else
		mainapp_error("%s get total obj cnt [%d] .\n", __FUNCTION__, sdRet);
}
#endif
static void _MainApp_MaintainGobjOnCustomerMenu(MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	UINT32 dGobjTotalCnt = 0;
	INT32 sdRet = GUI_OBJMGR_SUCCESS;
	sdRet = GOBJ_MGR_GetTotalCreatedObjects(pMainAppInstance->GUIObjData, &dGobjTotalCnt);
	if (sdRet == GUI_OBJMGR_SUCCESS)
	{
		UINT32 i = 0;
		UINT32 dGobjId = 0;
		UINT32 dIsNeedOpenCustMenu = 1;
		for (i = 0; i < dGobjTotalCnt; i++)
		{
			sdRet = GOBJ_MGR_GetObjectIDByIndex(pMainAppInstance->GUIObjData, i, &dGobjId);
			if (sdRet == GUI_OBJMGR_SUCCESS)
			{
				mainapp_printf("%s on gobj[%d].\n", __FUNCTION__, dGobjId);
				switch (dGobjId)
				{
				case MAINAPP_GUIOBJ_SOURCE:
					GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_SOURCE);
					break;
				case MAINAPP_CUSTOMERMENU:
					GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, dGobjId);
					ShareGobj_IF_SetRegNormalMode(SHARE_REGION_TOP);
					dIsNeedOpenCustMenu = 0;
					break;
				case MAINAPP_SETUPMENU:
				case MAINAPP_SETUPMENUSPKR:
				case MAINAPP_SUPERSETUPMENU:
				case MAINAPP_REGIONMENU:
#if defined(NET_WIFI_SUPPORT)
                                case MAINAPP_SETUPMENUNETWORK:
#endif
					GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, dGobjId);
					ShareGobj_IF_SetRegNormalMode(SHARE_REGION_TOP);
					break;
				case MAINAPP_GUIOBJ_MAINMENU:
					GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, dGobjId);
					break;
				default:
					break;
				}
			}
			else
			{
				mainapp_error("%s get obj id ret [%d] index[%d].\n", __FUNCTION__, sdRet, i);
				break;
			}
		}
		if (dIsNeedOpenCustMenu)
		{
			ShareGobj_IF_SetRegHideMode(SHARE_REGION_TOP);
			GOBJ_MGR_CreateObject(pMainAppInstance->pGUIObjectTable[MAINAPP_CUSTOMERMENU], pMainAppInstance->GUIObjData, MAINAPP_CUSTOMERMENU, 0);
		}
	}
	else
		mainapp_error("%s get total obj cnt [%d] .\n", __FUNCTION__, sdRet);
}

int MAINAPP_UIEventHandler(UINT32 dMessage, UINT32 dParam)
{
	MainAppUIEventHandlerInfo_t *pstEventInfo = &stUIEventHandlerInfo;
	UINT32* dSuperPW=pstEventInfo->stComGobj.stSuperSetupGobj.dSuperPW;
	UINT32 dSysAppIdx = TOTAL_SYS_APP_SIZE;
#ifdef PLAY_DISC_FROM_HD
	DIMON_COMMON_Struct_t *pDiInfo = NULL;
#endif
	UINT32 dMainAppState = MAIN_APP_NORMAL;
#ifdef SUPPORT_FAKE_STANDBY
	if(MainApp_GetStandbyStatus()==TRUE)
	{
		if(dMessage < UI_EVENT_NULL&&dMessage != UI_EVENT_POWER)
			return MAIN_APP_SUCCESS;
	}
#endif
	/*For mantis 108084*/
	MAINAPP_GetMainAppState(&dMainAppState);
	if (dMainAppState == MAIN_APP_SYSTEM_APP_ON_TRANSITION)
	{
		switch (dMessage)
		{
			case UI_EVENT_POWER:
			case UI_EVENT_EJECT:
			case UI_EVENT_RESOLUTION:
				break;
			default:
				return MAIN_APP_SUCCESS;
				break;
		}
	}

	int i;
	switch (dMessage)
	{
	case UI_EVENT_POWER:
		mainapp_printf("UI_EVENT_POWER received.\n");
		if(stUIEventHandlerInfo.bIsDiscEjecting)		//add by george.chang 2010.10.08
		{
			stUIEventHandlerInfo.bEjectPendingAction=UI_EVENT_POWER;
			break;
		}
#ifdef SUPPORT_FAKE_STANDBY
		if(DVB_Timer_CheckNearestRemainingRecorderTime()==TRUE)
		{
			if(MAINAPP_GetStandbyBusyStatus()==FALSE)
				MainApp_FakePowerOnOff();
		}
		else

#endif
	{
		StandByMode eMode = MAINAPP_AWAKE;
		MAINAPP_IsEnteringStandby(&eMode);

		if(eMode != MAINAPP_STANDBY)
			_MainApp_Finalizing();
	}
		break;
	case UI_EVENT_SOURCE:
		//add by george.chang 2010.10.08
		if(stUIEventHandlerInfo.bIsDiscEjecting)
			break;
        #ifdef SUPPORT_PLAYER_STB_DVB_T
	#ifdef SUPPORT_PVR
		if(MainApp_IsDvbRecording(dMessage))
		{
			MAINAPP_SendEvent4DvbAP(SYS_APP_DVB, dMessage, dParam);
			break;
		}
        #endif
        #endif
		mainapp_printf("UI_EVENT_SOURCE Event received.\n");
		pstEventInfo->dAppEvent = dMessage;
		pstEventInfo->stComGobj.stSourceGobj.dMenuTimeOut = dParam*1000;/*second to millisecond*/
		if (pstEventInfo->eDiscState == DISC_LOADING)	
		{
			pstEventInfo->stSysDriveCtrlInfo.stSysCurPtnHnd = NULL;
			pstEventInfo->bDiscResumeFlag = 1;
		}
		_MainApp_UIEventOnSource(pstEventInfo);
		break;
	case UI_EVENT_SETUP:
		//add by george.chang 2010.10.08
		if(stUIEventHandlerInfo.bIsDiscEjecting)
		break;
		#ifdef SUPPORT_PLAYER_STB_DVB_T
		#ifdef SUPPORT_PVR
		if(MainApp_IsDvbRecording(dMessage))
		{
			MAINAPP_SendEvent4DvbAP(SYS_APP_DVB, dMessage, dParam);
			break;
		}
		#endif
		#endif
		mainapp_printf("UI_EVENT_SETUP Event received.\n");
		pstEventInfo->dAppEvent = dMessage;
		if(dParam>=MAIN_MENU_TYPE_MAX)
			dParam = MAIN_MENU_TYPE_HOME;

		pstEventInfo->stComGobj.stMainGobj.eMenuType = dParam;
		if (pstEventInfo->eDiscState == DISC_LOADING)	
		{
			if (pstEventInfo->bIsHoldHomeMenu != 1)
			{
				pstEventInfo->stSysDriveCtrlInfo.stSysCurPtnHnd = NULL;
			}
			pstEventInfo->bDiscResumeFlag = 1;
		}
		_MainApp_UIEventOnSetup(pstEventInfo);
		break;
	case UI_EVENT_EJECT:
		mainapp_printf("UI_EVENT_EJECT Event received.\n");
       #ifdef SUPPORT_PLAYER_STB_DVB_T
       #ifdef SUPPORT_PVR
		if(MainApp_IsDvbRecording(dMessage))
		{
			MAINAPP_SendEvent4DvbAP(SYS_APP_DVB, dMessage, dParam);
			break;
		}
       #endif
       #endif
		pstEventInfo->bDiscResumeFlag=0; //added by ted.chen

		pstEventInfo->dAppEvent = dMessage;
		_MainApp_UIEventOnEject(pstEventInfo);
		#if 0//swjiang move it to _MainApp_ExecuteDiscEject,2010.12.10
		if(dParam == CBK_EVENT_DISCMW_REGION_ERR)//when region code error,we will auto eject and show msg,for mantis:97298
			ShareGobj_IF_ShowHintMsg(HINT_MSG_REGION_ERROR, HINT_MSG_TIMEOUT_5S, MSG_PARAM_RESET_BG_MSG); //guohao move it,then eject show
		#endif
		break;
	case UI_EVENT_PLAY:
		mainapp_printf("UI_EVENT_PLAY Event received.\n");
		pstEventInfo->dAppEvent = UI_EVENT_EJECT;
		_MainApp_UIEventOnPlay(pstEventInfo);
		break;
	case UI_EVENT_RESOLUTION:
		mainapp_printf("UI_EVENT_RESOLUTION Event received.\n");
		pstEventInfo->dAppEvent = dMessage;
		_MainApp_UIEventOnResolution(dParam, pstEventInfo);

		MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
        #ifdef SUPPORT_PLAYER_STB_DVB_T
		 MAINAPP_SendEvent4DvbAP(dSysAppIdx, dMessage | PASS_TO_SYSAPP, dParam);
        #endif
		_MAINAPP_SendEvent4NetworkAPs(dSysAppIdx, dMessage, dParam);
		break;
	case UI_EVENT_PN:
		pstEventInfo->dAppEvent = dMessage;
		_MainApp_UIEventOnPNSwitch(dParam, pstEventInfo);

		MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
       #ifdef SUPPORT_PLAYER_STB_DVB_T
		MAINAPP_SendEvent4DvbAP(dSysAppIdx, dMessage | PASS_TO_SYSAPP, dParam);
       #endif
		_MAINAPP_SendEvent4NetworkAPs(dSysAppIdx, dMessage, dParam);
		break;

	case UI_EVENT_VOL_UP:  //for network APs cleaning mute hint
	case UI_EVENT_VOL_DN:  //for network APs cleaning mute hint
	case UI_EVENT_MUTE:
		MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
#ifdef SUPPORT_PLAYER_STB_DVB_T
		MAINAPP_SendEvent4DvbAP(dSysAppIdx, dMessage | PASS_TO_SYSAPP, dParam);
#endif
		_MAINAPP_SendEvent4NetworkAPs(dSysAppIdx, dMessage, dParam);
		#ifdef KOK_SUPPORT
		if(pstEventInfo->stSysAppCtrlInfo.stFileAppCtrlInfo.eFileAppType == FILE_APP_KOK)
		{
			MAINAPP_SendEvent4KOKAP(dSysAppIdx, dMessage, dParam);
		}
		#endif
		break;

#ifdef PLAY_DISC_FROM_HD
	case UI_EVENT_0:
		pDiInfo = GL_MemAlloc(sizeof(DIMON_COMMON_Struct_t));
		memset(pDiInfo, 0, sizeof(DIMON_COMMON_Struct_t));
		MSG_FILTER_DispatchMessage(gMsgHandle, DMN_EVENT_VCD_DISC_INSERTED, (UINT32) pDiInfo);
		break;
	case UI_EVENT_2:
		pDiInfo = GL_MemAlloc(sizeof(DIMON_COMMON_Struct_t));
		memset(pDiInfo, 0, sizeof(DIMON_COMMON_Struct_t));
		MSG_FILTER_DispatchMessage(gMsgHandle, DMN_EVENT_BD_VIDEO_DISC_INSERTED, (UINT32) pDiInfo);
		break;
	case UI_EVENT_3:
		pDiInfo = GL_MemAlloc(sizeof(DIMON_COMMON_Struct_t));
		memset(pDiInfo, 0, sizeof(DIMON_COMMON_Struct_t));
		MSG_FILTER_DispatchMessage(gMsgHandle, DMN_EVENT_AVCHD_DISC_INSERTED, (UINT32) pDiInfo);
		break;
	case UI_EVENT_4:
		pDiInfo = GL_MemAlloc(sizeof(DIMON_COMMON_Struct_t));
		memset(pDiInfo, 0, sizeof(DIMON_COMMON_Struct_t));
		MSG_FILTER_DispatchMessage(gMsgHandle, DMN_EVENT_DVD_VIDEO_DISC_INSERTED, (UINT32) pDiInfo);
		break;
#endif

	default:
		mainapp_printf("Un-handled UI Eventid: 0x%x.\n", dMessage);
		break;
	}

	if(PSREG_VALUE(PSREG_STAT_MainAppStatus)==PSREG_MAINAPPSTATUS_TRAYOUT)
	{
		for(i=0;i<SUPER_PASSWORD_LEN-1;i++)
			dSuperPW[i]=dSuperPW[i+1];
		dSuperPW[SUPER_PASSWORD_LEN-1]=dMessage;

		if(dSuperPW[0]==SUPER_PW_DIGIT1 && dSuperPW[1] ==SUPER_PW_DIGIT2 &&
			dSuperPW[2]==SUPER_PW_DIGIT3 && dSuperPW[3]==SUPER_PW_DIGIT4)
                       {
			APP_Show_Logo(LOGO_UI,LOGO_ONLY_BACKGRUND);//cai.w for super menu background 010730
			_MainApp_MaintainGobjOnSuperMenu(pstEventInfo);
                        }
		else if(dSuperPW[0]==CUSTMOER_PW_DIGIT1 && dSuperPW[1] ==CUSTMOER_PW_DIGIT2 &&
			dSuperPW[2]==CUSTMOER_PW_DIGIT3 && dSuperPW[3]==CUSTMOER_PW_DIGIT4)
			_MainApp_MaintainGobjOnCustomerMenu(pstEventInfo);
#if defined(SUPPORT_REGION_MENU)
		else if(dSuperPW[0]==REGION_PW_DIGIT1 && dSuperPW[1] ==REGION_PW_DIGIT2 &&
			dSuperPW[2]==REGION_PW_DIGIT3 && dSuperPW[3]==REGION_PW_DIGIT4)
                          {
                         APP_Show_Logo(LOGO_UI,LOGO_ONLY_BACKGRUND);//cai.w for super menu background 010730
			_MainApp_MaintainGobjOnRegionMenu(pstEventInfo);
                          }
#endif
	}

	return MAIN_APP_SUCCESS;
}

static void APPNOINLINE _MainApp_DaemonEventBegainIdentifyDisc(UINT32 dParam, MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	UINT32 dMainAppState = MAIN_APP_NORMAL;
	/*check main app state,now just normal state do something */
	MAINAPP_GetMainAppState(&dMainAppState);
	if (dMainAppState == MAIN_APP_NORMAL)
	{
		UINT32 dSysAppIdx = TOTAL_SYS_APP_SIZE;
		INT32 sdRet = MAIN_APP_SUCCESS;
		/*check sys app active state. */
		sdRet = MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
		switch (sdRet)
		{
		case MAIN_APP_NO_ACTIVE_SYSTEM_APP:
			mainapp_printf("%s on NO_ACTIVE_SYSTEM_APP.\n", __FUNCTION__);
			if (PSREG_VALUE(PSREG_STAT_MainAppStatus) != PSREG_MAINAPPSTATUS_TRAYOUT)
				ShareGobj_IF_ShowHintMsg(HINT_MSG_DISC_LOADING, HINT_MSG_TIMEOUT_INFINITE, MSG_PARAM_NULL);
			PSREG_SET_VALUE(PSREG_STAT_MainAppStatus, PSREG_MAINAPPSTATUS_DISCLOADING);
			break;
		case MAIN_APP_SUCCESS:
			switch (dSysAppIdx)
			{
			case SYS_APP_DISC_PLAYER:
				mainapp_printf("%s on SYS_APP_DISC_PLAYER.\n", __FUNCTION__);
				/*TODO :  */
				break;
			case SYS_APP_FILE_PLAYER:
				mainapp_printf("%s on SYS_APP_FILE_PLAYER.\n", __FUNCTION__);
				/*TODO :  */
				break;

			case TOTAL_SYS_APP_SIZE:
				mainapp_printf("%s on TOTAL_SYS_APP_SIZE.\n", __FUNCTION__);
				break;
			default:
				break;
			}
			break;

		default:
			mainapp_error("%s on %d condition.\n", __FUNCTION__, sdRet);
			break;
		}
	}
	else
		mainapp_printf("%s on main app state[%d].\n", __FUNCTION__, dMainAppState);
}

static void APPNOINLINE _MainApp_DaemonEventDataDiscInserted(UINT32 dParam, MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	UINT32 dMainAppState = MAIN_APP_NORMAL;
	DIMON_COMMON_Struct_t *pDiInfo = NULL;
	UINT32 dDiscLoaderId = DI_Handle_PHYSICAL;
	pDiInfo = (DIMON_COMMON_Struct_t *) dParam;
	IS_FILE_LOADER_HANDLE(pDiInfo->DIHandle, dDiscLoaderId); //wc-iso

	AppPtn_Mount(gdPhisycalLoaderDiscDevNum, pDiInfo->eDiscFileSystem, DEVICE_TYPE_DISC);
	AppPtn_DiscInsert(MEDIA_TYPE_DATA_DISC, pDiInfo->dDiskId, dDiscLoaderId); //wc-iso
	/*check main app state,now just normal state do something */
	MAINAPP_GetMainAppState(&dMainAppState);
	if (dMainAppState == MAIN_APP_NORMAL)
	{
		UINT32 dSysAppIdx = TOTAL_SYS_APP_SIZE;
		INT32 sdRet = MAIN_APP_SUCCESS;
		/*check sys app active state. */
		sdRet = MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
		switch (sdRet)
		{
		case MAIN_APP_NO_ACTIVE_SYSTEM_APP:
			mainapp_printf("%s on NO_ACTIVE_SYSTEM_APP.\n", __FUNCTION__);
			MainAppDiscInsertCallBack = _MainApp_DataDiscInsertCallBack;
			ShareGobj_IF_ShowHintMsg(HINT_MSG_DISC_TYPE_DATA, HINT_MSG_TIMEOUT, MSG_PARAM_RESET_BG_MSG);
			ShareGobj_IF_ShowHintMsg(HINT_MSG_FILE_LOADING, HINT_MSG_TIMEOUT_INFINITE, MSG_PARAM_SHOW_BG_MSG);
			PSREG_SET_VALUE(PSREG_STAT_MainAppStatus, PSREG_MAINAPPSTATUS_DATADISCIndentify);
			break;

		case MAIN_APP_SUCCESS:
			switch (dSysAppIdx)
			{
			case SYS_APP_DISC_PLAYER:
				break;
			case SYS_APP_FILE_PLAYER:
				break;
			case TOTAL_SYS_APP_SIZE:
				break;
			default:
				break;
			}

			MainAppDiscInsertCallBack = _MainApp_DataDiscInsertCBK_Stop;
			break;

		default:
			break;
		}
	}
	else
		mainapp_printf("%s on main app state[%d].\n", __FUNCTION__, dMainAppState);
}
//add by george.chang 2010.10.08
inline void _MainApp_DiscInsertEvent(MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	SYSAPP_IF_SendCriticalGlobalEventWithIndex(SYS_APP_DISC_PLAYER, DMN_EVENT_DISC_INSERTED, 0);//add by george.chang 2010.09.09
}
//add by george.chang redo disc DiscInsert 2010.10.08
 void _MainApp_ActiveDiscInsert(MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	UINT32 dMainAppState = MAIN_APP_NORMAL;
	/*check main app state,now just normal state do something */
	MAINAPP_GetMainAppState(&dMainAppState);
	if (dMainAppState == MAIN_APP_NORMAL)
	{
		UINT32 dSysAppIdx = TOTAL_SYS_APP_SIZE;
		INT32 sdRet = MAIN_APP_SUCCESS;
		/*check sys app active state. */
		sdRet = MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
		switch (sdRet)
		{
		case MAIN_APP_NO_ACTIVE_SYSTEM_APP:
			mainapp_printf("%s on NO_ACTIVE_SYSTEM_APP.\n", __FUNCTION__);
			_MainApp_ActivateSysApp(pstEventInfo);
			SysAppActiveAckCallBack=_MainApp_DiscInsertEvent;//add by george.chang 2010.10.08
			break;

			if ((PSREG_VALUE(PSREG_STAT_MainAppStatus) == PSREG_MAINAPPSTATUS_USB_ATTACH)
			  	|| (PSREG_VALUE(PSREG_STAT_MainAppStatus) == PSREG_MAINAPPSTATUS_CARD_ATTACH))
			{
				mainapp_printf("%s call DI_DiscStop !!!! \n",__FUNCTION__);
				DI_DiscStop(gPhisycalLoaderDIHandle);
			}
			break;

		default:
			break;
		}
	}
	else
		mainapp_printf("%s on main app state[%d].\n", __FUNCTION__, dMainAppState);
}

static void APPNOINLINE _MainApp_DaemonEventDvdDiscInserted(UINT32 dParam, MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	UINT32 dMainAppState = MAIN_APP_NORMAL;
	DIMON_COMMON_Struct_t *pDiInfo = NULL;
	UINT32 dSysAppIdx = TOTAL_SYS_APP_SIZE;
	INT32 sdRet = MAIN_APP_SUCCESS;
	UINT32 dDiscLoaderId = DI_Handle_PHYSICAL;

	pDiInfo = (DIMON_COMMON_Struct_t *) dParam;

	IS_FILE_LOADER_HANDLE(pDiInfo->DIHandle, dDiscLoaderId); //wc-iso

	AppPtn_DiscInsert(MEDIA_TYPE_DVD, pDiInfo->dDiskId, dDiscLoaderId); //wc-iso

    #ifdef SUPPORT_ISO_IMAGE //lgz 2011-1-21 19:51 add for mantis 0122998
    if(PSREG_VALUE(PSREG_FILE_LOADER) == PSREG_FILE_LOADER_IDENTIFY_DISC)
    {
	    fldr_printf("ISO file,do not change stSysCurPtnHnd!\n");	
    }
    else
    #endif
	_MainApp_SWSysDrive(pstEventInfo, DEVICE_TYPE_DISC);
	/*check main app state,now just normal state do something */
	MAINAPP_GetMainAppState(&dMainAppState);
	if (dMainAppState == MAIN_APP_NORMAL)
	{
		/*check sys app active state. */
		sdRet = MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
		switch (sdRet)
		{
		case MAIN_APP_NO_ACTIVE_SYSTEM_APP:
			mainapp_printf("%s on NO_ACTIVE_SYSTEM_APP.\n", __FUNCTION__);
			ShareGobj_IF_ShowHintMsg(HINT_MSG_DISC_TYPE_DVD, HINT_MSG_TIMEOUT, MSG_PARAM_RESET_BG_MSG);
			PSREG_SET_VALUE(PSREG_STAT_MainAppStatus, PSREG_MAINAPPSTATUS_DVDIndentify);
			if(!pstEventInfo->bDiscResumeFlag)
			{
				_MainApp_ActivateSysApp(pstEventInfo);
			}
			SysAppActiveAckCallBack=_MainApp_DiscInsertEvent;//add by george.chang 2010.10.08
			break;

		case MAIN_APP_SUCCESS:
			switch (dSysAppIdx)
			{
			case SYS_APP_DISC_PLAYER:
				//add by george.chang 2010.10.04
				sdRet = MAINAPP_TerminateSystemApp(dSysAppIdx);
				if (sdRet == SYSTEM_APP_SUCCESS)
				{
					ShareGobj_IF_ShowHintMsg(HINT_MSG_DISC_TYPE_DVD, HINT_MSG_TIMEOUT, MSG_PARAM_RESET_BG_MSG);
					PSREG_SET_VALUE(PSREG_STAT_MainAppStatus, PSREG_MAINAPPSTATUS_DVDIndentify);
					SysAppTerminateAckCallBack = _MainApp_ActiveDiscInsert;
				}
				else
					mainapp_printf("Terminate System App fail [%d]\n", sdRet);
				break;
			case SYS_APP_FILE_PLAYER:
				break;
			case TOTAL_SYS_APP_SIZE:
				break;
			default:
				break;
			}

			if ((PSREG_VALUE(PSREG_STAT_MainAppStatus) == PSREG_MAINAPPSTATUS_USB_ATTACH)
			  	|| (PSREG_VALUE(PSREG_STAT_MainAppStatus) == PSREG_MAINAPPSTATUS_CARD_ATTACH))
			{
				diag_printf("%s call DI_DiscStop !!!! \n",__FUNCTION__);
				DI_DiscStop(gPhisycalLoaderDIHandle);
			}

			#ifdef SUPPORT_ISO_IMAGE
			if (PSREG_VALUE(PSREG_FILE_LOADER) == PSREG_FILE_LOADER_IDENTIFY_DISC)
			{
  				sdRet = MAINAPP_TerminateSystemApp(dSysAppIdx);
				if (sdRet == SYSTEM_APP_SUCCESS)
					SysAppTerminateAckCallBack = MainApp_DVDInsertCBK;
				else
					fldr_printf("Terminate System App fail [%d]\n", sdRet);
			}
			#endif // SUPPORT_ISO_IMAGE
			break;

		default:
			break;
		}
	}
	else
		mainapp_printf("%s on main app state[%d].\n", __FUNCTION__, dMainAppState);
}

static void APPNOINLINE _MainApp_DaemonEventPlusvrDiscInserted(UINT32 dParam, MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	UINT32 dMainAppState = MAIN_APP_NORMAL;
	DIMON_COMMON_Struct_t *pDiInfo = NULL;
	UINT32 dDiscLoaderId = DI_Handle_PHYSICAL;

	pDiInfo = (DIMON_COMMON_Struct_t *) dParam;

	IS_FILE_LOADER_HANDLE(pDiInfo->DIHandle, dDiscLoaderId); //wc-iso

	AppPtn_DiscInsert(MEDIA_TYPE_PLUSVR, pDiInfo->dDiskId, dDiscLoaderId);

	_MainApp_SWSysDrive(pstEventInfo, DEVICE_TYPE_DISC);
	/*check main app state,now just normal state do something */
	MAINAPP_GetMainAppState(&dMainAppState);
	if (dMainAppState == MAIN_APP_NORMAL)
	{
		UINT32 dSysAppIdx = TOTAL_SYS_APP_SIZE;
		INT32 sdRet = MAIN_APP_SUCCESS;
		/*check sys app active state. */
		sdRet = MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
		switch (sdRet)
		{
		case MAIN_APP_NO_ACTIVE_SYSTEM_APP:
			mainapp_printf("%s on NO_ACTIVE_SYSTEM_APP.\n", __FUNCTION__);
			ShareGobj_IF_ShowHintMsg(HINT_MSG_DISC_TYPE_DVD /*temp for plusvr */ , HINT_MSG_TIMEOUT, MSG_PARAM_RESET_BG_MSG);
			PSREG_SET_VALUE(PSREG_STAT_MainAppStatus, PSREG_MAINAPPSTATUS_VRIndentify);
			if(!pstEventInfo->bDiscResumeFlag)
				_MainApp_ActivateSysApp(pstEventInfo);

			SysAppActiveAckCallBack=_MainApp_DiscInsertEvent;//add by george.chang 2010.10.08
			break;

		case MAIN_APP_SUCCESS:
			switch (dSysAppIdx)
			{
			case SYS_APP_DISC_PLAYER:
			//add by george.chang 2010.10.04
			sdRet = MAINAPP_TerminateSystemApp(dSysAppIdx);
			if (sdRet == SYSTEM_APP_SUCCESS)
			{
				ShareGobj_IF_ShowHintMsg(HINT_MSG_DISC_TYPE_DVD /*temp for plusvr */ , HINT_MSG_TIMEOUT, MSG_PARAM_RESET_BG_MSG);
				PSREG_SET_VALUE(PSREG_STAT_MainAppStatus, PSREG_MAINAPPSTATUS_VRIndentify);
				SysAppTerminateAckCallBack = _MainApp_ActiveDiscInsert;
			}
			else
				mainapp_printf("Terminate System App fail [%d]\n", sdRet);
				break;
			case SYS_APP_FILE_PLAYER:
				break;
			case TOTAL_SYS_APP_SIZE:
				break;
			default:
				break;
			}

			if ((PSREG_VALUE(PSREG_STAT_MainAppStatus) == PSREG_MAINAPPSTATUS_USB_ATTACH)
			  	|| (PSREG_VALUE(PSREG_STAT_MainAppStatus) == PSREG_MAINAPPSTATUS_CARD_ATTACH))
			{
				mainapp_printf("%s call DI_DiscStop !!!! \n",__FUNCTION__);
				DI_DiscStop(gPhisycalLoaderDIHandle);
			}
			break;

		default:
			break;
		}
	}
	else
		mainapp_printf("%s on main app state[%d].\n", __FUNCTION__, dMainAppState);
}

static void APPNOINLINE _MainApp_DaemonEventMinusvrDiscInserted(UINT32 dParam, MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	UINT32 dMainAppState = MAIN_APP_NORMAL;
	DIMON_COMMON_Struct_t *pDiInfo = NULL;
	UINT32 dDiscLoaderId = DI_Handle_PHYSICAL;
	pDiInfo = (DIMON_COMMON_Struct_t *) dParam;
	IS_FILE_LOADER_HANDLE(pDiInfo->DIHandle, dDiscLoaderId); //wc-iso

	AppPtn_DiscInsert(MEDIA_TYPE_MINUSVR, pDiInfo->dDiskId, dDiscLoaderId); //wc-iso

	_MainApp_SWSysDrive(pstEventInfo, DEVICE_TYPE_DISC);
	/*check main app state,now just normal state do something */
	MAINAPP_GetMainAppState(&dMainAppState);
	if (dMainAppState == MAIN_APP_NORMAL)
	{
		UINT32 dSysAppIdx = TOTAL_SYS_APP_SIZE;
		INT32 sdRet = MAIN_APP_SUCCESS;
		/*check sys app active state. */
		sdRet = MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
		switch (sdRet)
		{
		case MAIN_APP_NO_ACTIVE_SYSTEM_APP:
			mainapp_printf("%s on NO_ACTIVE_SYSTEM_APP.\n", __FUNCTION__);
			ShareGobj_IF_ShowHintMsg(HINT_MSG_DISC_TYPE_DVD /*temp for minusvr */ , HINT_MSG_TIMEOUT, MSG_PARAM_RESET_BG_MSG);
			PSREG_SET_VALUE(PSREG_STAT_MainAppStatus, PSREG_MAINAPPSTATUS_VRIndentify);
			if(!pstEventInfo->bDiscResumeFlag)
				_MainApp_ActivateSysApp(pstEventInfo);
			
			SysAppActiveAckCallBack=_MainApp_DiscInsertEvent;
			break;

		case MAIN_APP_SUCCESS:
			switch (dSysAppIdx)
			{
			case SYS_APP_DISC_PLAYER:
				//add by george.chang 2010.10.04
				sdRet = MAINAPP_TerminateSystemApp(dSysAppIdx);
				if (sdRet == SYSTEM_APP_SUCCESS)
				{
					ShareGobj_IF_ShowHintMsg(HINT_MSG_DISC_TYPE_DVD /*temp for minusvr */ , HINT_MSG_TIMEOUT, MSG_PARAM_RESET_BG_MSG);
					PSREG_SET_VALUE(PSREG_STAT_MainAppStatus, PSREG_MAINAPPSTATUS_VRIndentify);
					SysAppTerminateAckCallBack = _MainApp_ActiveDiscInsert;
				}
				else
					mainapp_printf("Terminate System App fail [%d]\n", sdRet);
				break;
			case SYS_APP_FILE_PLAYER:
				break;
			case TOTAL_SYS_APP_SIZE:
				break;
			default:
				break;
			}

			if ((PSREG_VALUE(PSREG_STAT_MainAppStatus) == PSREG_MAINAPPSTATUS_USB_ATTACH)
			  	|| (PSREG_VALUE(PSREG_STAT_MainAppStatus) == PSREG_MAINAPPSTATUS_CARD_ATTACH))
			{
				mainapp_printf("%s call DI_DiscStop !!!! \n",__FUNCTION__);
				DI_DiscStop(gPhisycalLoaderDIHandle);
			}
			break;

		default:
			break;
		}
	}
	else
		mainapp_printf("%s on main app state[%d].\n", __FUNCTION__, dMainAppState);
}

static void APPNOINLINE _MainApp_DaemonEventVcdDiscInserted(UINT32 dParam, MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	UINT32 dMainAppState = MAIN_APP_NORMAL;
	UINT32 dDiscLoaderId = DI_Handle_PHYSICAL;
	DIMON_VCD_Struct_t *pDiInfo = (DIMON_VCD_Struct_t *) dParam;
	IS_FILE_LOADER_HANDLE(pDiInfo->stComInfo.DIHandle, dDiscLoaderId); //wc-iso

	AppPtn_DiscInsert(MEDIA_TYPE_VCD, pDiInfo->stComInfo.dDiskId, dDiscLoaderId); //wc-iso

	_MainApp_SWSysDrive(pstEventInfo, DEVICE_TYPE_DISC);
	/*check main app state,now just normal state do something */
	MAINAPP_GetMainAppState(&dMainAppState);
	if (dMainAppState == MAIN_APP_NORMAL)
	{
		UINT32 dSysAppIdx = TOTAL_SYS_APP_SIZE;
		INT32 sdRet = MAIN_APP_SUCCESS;
		/*check sys app active state. */
		sdRet = MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
		switch (sdRet)
		{
		case MAIN_APP_NO_ACTIVE_SYSTEM_APP:
			mainapp_printf("%s on NO_ACTIVE_SYSTEM_APP.\n", __FUNCTION__);
#ifdef TESTBED_USE_LOADER
			pstEventInfo->dDevIdx=pDiInfo->stComInfo.dDevIndex;
			memcpy(&pstEventInfo->stPlayerInfo, &pDiInfo->stToc, sizeof(TOC_INFO));
#endif
			ShareGobj_IF_ShowHintMsg(HINT_MSG_DISC_TYPE_VCD, HINT_MSG_TIMEOUT, MSG_PARAM_RESET_BG_MSG);
			PSREG_SET_VALUE(PSREG_STAT_MainAppStatus, PSREG_MAINAPPSTATUS_VCDIndentify);
			if(!pstEventInfo->bDiscResumeFlag)
				_MainApp_ActivateSysApp(pstEventInfo);
			
			SysAppActiveAckCallBack=_MainApp_DiscInsertEvent;//add by george.chang 2010.10.08
			break;

		case MAIN_APP_SUCCESS:
			switch (dSysAppIdx)
			{
			case SYS_APP_DISC_PLAYER:
				//add by george.chang 2010.10.04
				sdRet = MAINAPP_TerminateSystemApp(dSysAppIdx);
				if (sdRet == SYSTEM_APP_SUCCESS)
				{
					pstEventInfo->dDevIdx=pDiInfo->stComInfo.dDevIndex;
					memcpy(&pstEventInfo->stPlayerInfo, &pDiInfo->stToc, sizeof(TOC_INFO));
					ShareGobj_IF_ShowHintMsg(HINT_MSG_DISC_TYPE_VCD, HINT_MSG_TIMEOUT, MSG_PARAM_RESET_BG_MSG);
					PSREG_SET_VALUE(PSREG_STAT_MainAppStatus, PSREG_MAINAPPSTATUS_VCDIndentify);
					SysAppTerminateAckCallBack = _MainApp_ActiveDiscInsert;
				}
				else
					mainapp_printf("Terminate System App fail [%d]\n", sdRet);
				break;
			case SYS_APP_FILE_PLAYER:
				break;
			case TOTAL_SYS_APP_SIZE:
				break;
			default:
				break;
			}

			if ((PSREG_VALUE(PSREG_STAT_MainAppStatus) == PSREG_MAINAPPSTATUS_USB_ATTACH)
			  	|| (PSREG_VALUE(PSREG_STAT_MainAppStatus) == PSREG_MAINAPPSTATUS_CARD_ATTACH))
			{
				mainapp_printf("%s call DI_DiscStop !!!! \n",__FUNCTION__);
				DI_DiscStop(gPhisycalLoaderDIHandle);
			}
			break;

		default:
			break;
		}
	}
	else
		mainapp_printf("%s on main app state[%d].\n", __FUNCTION__, dMainAppState);
}

static void APPNOINLINE _MainApp_DaemonEventSvcdDiscInserted(UINT32 dParam, MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	UINT32 dMainAppState = MAIN_APP_NORMAL;
	UINT32 dDiscLoaderId = DI_Handle_PHYSICAL;
	DIMON_VCD_Struct_t *pDiInfo = (DIMON_VCD_Struct_t *) dParam;
	IS_FILE_LOADER_HANDLE(pDiInfo->stComInfo.DIHandle, dDiscLoaderId); //wc-iso

	AppPtn_DiscInsert(MEDIA_TYPE_SVCD, pDiInfo->stComInfo.dDiskId, dDiscLoaderId); //wc-iso

	_MainApp_SWSysDrive(pstEventInfo, DEVICE_TYPE_DISC);
	/*check main app state,now just normal state do something */
	MAINAPP_GetMainAppState(&dMainAppState);
	if (dMainAppState == MAIN_APP_NORMAL)
	{
		UINT32 dSysAppIdx = TOTAL_SYS_APP_SIZE;
		INT32 sdRet = MAIN_APP_SUCCESS;
		/*check sys app active state. */
		sdRet = MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
		switch (sdRet)
		{
		case MAIN_APP_NO_ACTIVE_SYSTEM_APP:
			mainapp_printf("%s on NO_ACTIVE_SYSTEM_APP.\n", __FUNCTION__);
#ifdef TESTBED_USE_LOADER
			pstEventInfo->dDevIdx=pDiInfo->stComInfo.dDevIndex;
			memcpy(&pstEventInfo->stPlayerInfo, &pDiInfo->stToc, sizeof(TOC_INFO));
#endif
			ShareGobj_IF_ShowHintMsg(HINT_MSG_DISC_TYPE_SVCD, HINT_MSG_TIMEOUT, MSG_PARAM_RESET_BG_MSG);
			PSREG_SET_VALUE(PSREG_STAT_MainAppStatus, PSREG_MAINAPPSTATUS_VCDIndentify);
			if(!pstEventInfo->bDiscResumeFlag)
				_MainApp_ActivateSysApp(pstEventInfo);
			
			SysAppActiveAckCallBack=_MainApp_DiscInsertEvent;//add by george.chang 2010.10.08
			break;

		case MAIN_APP_SUCCESS:
			switch (dSysAppIdx)
			{
			case SYS_APP_DISC_PLAYER:
				//add by george.chang 2010.10.04
				sdRet = MAINAPP_TerminateSystemApp(dSysAppIdx);
				if (sdRet == SYSTEM_APP_SUCCESS)
				{
					pstEventInfo->dDevIdx=pDiInfo->stComInfo.dDevIndex;
					memcpy(&pstEventInfo->stPlayerInfo, &pDiInfo->stToc, sizeof(TOC_INFO));
					ShareGobj_IF_ShowHintMsg(HINT_MSG_DISC_TYPE_SVCD, HINT_MSG_TIMEOUT, MSG_PARAM_RESET_BG_MSG);
					PSREG_SET_VALUE(PSREG_STAT_MainAppStatus, PSREG_MAINAPPSTATUS_VCDIndentify);
					SysAppTerminateAckCallBack = _MainApp_ActiveDiscInsert;
				}
				else
					mainapp_printf("Terminate System App fail [%d]\n", sdRet);
				break;
			case SYS_APP_FILE_PLAYER:
				break;
			case TOTAL_SYS_APP_SIZE:
				break;
			default:
				break;
			}

			if ((PSREG_VALUE(PSREG_STAT_MainAppStatus) == PSREG_MAINAPPSTATUS_USB_ATTACH)
			  	|| (PSREG_VALUE(PSREG_STAT_MainAppStatus) == PSREG_MAINAPPSTATUS_CARD_ATTACH))
			{
				mainapp_printf("%s call DI_DiscStop !!!! \n",__FUNCTION__);
				DI_DiscStop(gPhisycalLoaderDIHandle);
			}
			break;

		default:
			break;
		}
	}
	else
		mainapp_printf("%s on main app state[%d].\n", __FUNCTION__, dMainAppState);
}

static void APPNOINLINE _MainApp_DaemonEventCddaDiscInserted(UINT32 dParam, MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	UINT32 dMainAppState = MAIN_APP_NORMAL;
	UINT32 dDiscLoaderId = DI_Handle_PHYSICAL;
	DIMON_VCD_Struct_t *pDiInfo = (DIMON_VCD_Struct_t *) dParam;
	IS_FILE_LOADER_HANDLE(pDiInfo->stComInfo.DIHandle, dDiscLoaderId); //wc-iso

	AppPtn_DiscInsert(MEDIA_TYPE_CDDA, pDiInfo->stComInfo.dDiskId, dDiscLoaderId); //wc-iso

	_MainApp_SWSysDrive(pstEventInfo, DEVICE_TYPE_DISC);

	/*check main app state,now just normal state do something */
	MAINAPP_GetMainAppState(&dMainAppState);
	if (dMainAppState == MAIN_APP_NORMAL)
	{
		UINT32 dSysAppIdx = TOTAL_SYS_APP_SIZE;
		INT32 sdRet = MAIN_APP_SUCCESS;
		/*check sys app active state. */
		sdRet = MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
		switch (sdRet)
		{
		case MAIN_APP_NO_ACTIVE_SYSTEM_APP:
			mainapp_printf("%s on NO_ACTIVE_SYSTEM_APP.\n", __FUNCTION__);
#ifdef TESTBED_USE_LOADER
			memcpy(&pstEventInfo->stSysAppCtrlInfo.stFileAppCtrlInfo.stCDDATrkInfo, &pDiInfo->stToc, sizeof(TOC_INFO));
#endif //TESTBED_USE_LOADER
			ShareGobj_IF_ShowHintMsg(HINT_MSG_DISC_TYPE_AUDIO, HINT_MSG_TIMEOUT, MSG_PARAM_RESET_BG_MSG);
			PSREG_SET_VALUE(PSREG_STAT_MainAppStatus, PSREG_MAINAPPSTATUS_CDIndentify);
			if(!pstEventInfo->bDiscResumeFlag)
				_MainApp_ActivateSysApp(pstEventInfo);
			break;

		case MAIN_APP_SUCCESS:
			switch (dSysAppIdx)
			{
			case SYS_APP_DISC_PLAYER:
				break;
			case SYS_APP_FILE_PLAYER:
				break;
			case TOTAL_SYS_APP_SIZE:
				break;
			default:
				break;
			}

			if ((PSREG_VALUE(PSREG_STAT_MainAppStatus) == PSREG_MAINAPPSTATUS_USB_ATTACH)
			  	|| (PSREG_VALUE(PSREG_STAT_MainAppStatus) == PSREG_MAINAPPSTATUS_CARD_ATTACH))
			{
				mainapp_printf("%s call DI_DiscStop !!!! \n",__FUNCTION__);
				DI_DiscStop(gPhisycalLoaderDIHandle);
			}
			break;

		default:
			break;
		}
	}
	else
		mainapp_printf("%s on main app state[%d].\n", __FUNCTION__, dMainAppState);
}

static void APPNOINLINE _MainApp_DaemonEventAVChdDiscInserted(UINT32 dParam, MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	UINT32 dMainAppState = MAIN_APP_NORMAL;
	DIMON_COMMON_Struct_t *pDiInfo = NULL;
	UINT32 dDiscLoaderId = DI_Handle_PHYSICAL;
	pDiInfo = (DIMON_COMMON_Struct_t *) dParam;
	IS_FILE_LOADER_HANDLE(pDiInfo->DIHandle, dDiscLoaderId); //wc-iso

	AppPtn_DiscInsert(MEDIA_TYPE_HDMV, pDiInfo->dDiskId, dDiscLoaderId); //wc-iso

    #ifdef SUPPORT_ISO_IMAGE //lgz 2011-1-21 19:51 add for mantis 0122998
    if(PSREG_VALUE(PSREG_FILE_LOADER) == PSREG_FILE_LOADER_IDENTIFY_DISC)
    {
	    fldr_printf("ISO file,do not change stSysCurPtnHnd!\n");	
    }
    else
    #endif
	_MainApp_SWSysDrive(pstEventInfo, DEVICE_TYPE_DISC);
	/*check main app state,now just normal state do something */
	MAINAPP_GetMainAppState(&dMainAppState);
	if (dMainAppState == MAIN_APP_NORMAL)
	{
		UINT32 dSysAppIdx = TOTAL_SYS_APP_SIZE;
		INT32 sdRet = MAIN_APP_SUCCESS;
		/*check sys app active state. */
		sdRet = MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
		switch (sdRet)
		{
		case MAIN_APP_NO_ACTIVE_SYSTEM_APP:
			mainapp_printf("%s on NO_ACTIVE_SYSTEM_APP.\n", __FUNCTION__);
			ShareGobj_IF_ShowHintMsg(HINT_MSG_DISC_TYPE_AVCHD , HINT_MSG_TIMEOUT, MSG_PARAM_RESET_BG_MSG);
			PSREG_SET_VALUE(PSREG_STAT_MainAppStatus, PSREG_MAINAPPSTATUS_BDIndentify);
			if(!pstEventInfo->bDiscResumeFlag)
			{
				_MainApp_ActivateSysApp(pstEventInfo);
			}

			SysAppActiveAckCallBack=_MainApp_DiscInsertEvent;//add by george.chang 2010.10.08
			break;

		case MAIN_APP_SUCCESS:
			switch (dSysAppIdx)
			{
			case SYS_APP_DISC_PLAYER:
           			SYSAPP_IF_SendCriticalGlobalEventWithIndex(SYS_APP_DISC_PLAYER, DMN_EVENT_DISC_INSERTED, 0);//add by george.chang 2010.09.09
				break;
			case SYS_APP_FILE_PLAYER:
				break;
			case TOTAL_SYS_APP_SIZE:
				break;
			default:
				break;
			}

			if ((PSREG_VALUE(PSREG_STAT_MainAppStatus) == PSREG_MAINAPPSTATUS_USB_ATTACH)
			  	|| (PSREG_VALUE(PSREG_STAT_MainAppStatus) == PSREG_MAINAPPSTATUS_CARD_ATTACH))
			{
				mainapp_printf("%s call DI_DiscStop !!!! \n",__FUNCTION__);
				DI_DiscStop(gPhisycalLoaderDIHandle);
			}
			#ifdef SUPPORT_ISO_IMAGE
			if (PSREG_VALUE(PSREG_FILE_LOADER) == PSREG_FILE_LOADER_IDENTIFY_DISC)
			{
  				sdRet = MAINAPP_TerminateSystemApp(dSysAppIdx);
				if (sdRet == SYSTEM_APP_SUCCESS)
					SysAppTerminateAckCallBack = MainApp_AVChdInsertCBK;
				else
					fldr_printf("Terminate System App fail [%d]\n", sdRet);
			}
			#endif // SUPPORT_ISO_IMAGE
			break;

		default:
			break;
		}
	}
	else
		mainapp_printf("%s on main app state[%d].\n", __FUNCTION__, dMainAppState);
}

#ifdef KOK_SUPPORT
static void APPNOINLINE _MainApp_DaemonEventKokDiscInserted(UINT32 dParam, MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	UINT32 dMainAppState = MAIN_APP_NORMAL;
	DIMON_COMMON_Struct_t *pDiInfo = NULL;
	UINT32 dDiscLoaderId = DI_Handle_PHYSICAL;

	pDiInfo = (DIMON_COMMON_Struct_t *) dParam;
	IS_FILE_LOADER_HANDLE(pDiInfo->DIHandle, dDiscLoaderId); //wc-iso

	AppPtn_Mount(gdPhisycalLoaderDiscDevNum, pDiInfo->eDiscFileSystem, DEVICE_TYPE_DISC);
	AppPtn_DiscInsert(MEDIA_TYPE_KOK, pDiInfo->dDiskId, dDiscLoaderId); //wc-iso
	/*check main app state,now just normal state do something */
	MAINAPP_GetMainAppState(&dMainAppState);
	if (dMainAppState == MAIN_APP_NORMAL)
	{
		UINT32 dSysAppIdx = TOTAL_SYS_APP_SIZE;
		INT32 sdRet = MAIN_APP_SUCCESS;

		/*check sys app active state. */
		sdRet = MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);

		switch (sdRet)
		{
		case MAIN_APP_NO_ACTIVE_SYSTEM_APP:
			mainapp_printf("%s on NO_ACTIVE_SYSTEM_APP.\n", __FUNCTION__);
			MainAppDiscInsertCallBack = _MainApp_DataDiscInsertCallBack;
			ShareGobj_IF_ShowHintMsg(HINT_MSG_DISC_TYPE_DATA, HINT_MSG_TIMEOUT, MSG_PARAM_RESET_BG_MSG);
			ShareGobj_IF_ShowHintMsg(HINT_MSG_FILE_LOADING, HINT_MSG_TIMEOUT_INFINITE, MSG_PARAM_SHOW_BG_MSG);
			PSREG_SET_VALUE(PSREG_STAT_MainAppStatus, PSREG_MAINAPPSTATUS_DATADISCIndentify);
			break;

		case MAIN_APP_SUCCESS:
			switch (dSysAppIdx)
			{
			case SYS_APP_DISC_PLAYER:
				break;
			case SYS_APP_FILE_PLAYER:
				break;
			case TOTAL_SYS_APP_SIZE:
				break;
			default:
				break;
			}

			MainAppDiscInsertCallBack = _MainApp_DataDiscInsertCBK_Stop;
			break;

		default:
			mainapp_error("%s on %d condition.\n", __FUNCTION__, sdRet);
			break;
		}
	}
	else
		mainapp_printf("%s on main app state[%d].\n", __FUNCTION__, dMainAppState);

	PSREG_SET_VALUE(PSREG_STAT_ShowScreenSaver, PSREG_SCRN_SAVER_ON);
	PSREG_SET_VALID(PSREG_STAT_ScrnSaverOn);
}
#endif

static void APPNOINLINE _MainApp_DaemonEventBDDiscInserted(UINT32 dParam, MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	UINT32 dMainAppState = MAIN_APP_NORMAL;
	DIMON_COMMON_Struct_t *pDiInfo = NULL;
	UINT32 dDiscLoaderId = DI_Handle_PHYSICAL;
	pDiInfo = (DIMON_COMMON_Struct_t *) dParam;
	IS_FILE_LOADER_HANDLE(pDiInfo->DIHandle, dDiscLoaderId); //wc-iso

	AppPtn_DiscInsert(MEDIA_TYPE_BD, pDiInfo->dDiskId, dDiscLoaderId); //wc-iso

    #ifdef SUPPORT_ISO_IMAGE //lgz 2011-1-21 19:51 add for mantis 0122998
    if(PSREG_VALUE(PSREG_FILE_LOADER) == PSREG_FILE_LOADER_IDENTIFY_DISC)
    {
	    fldr_printf("ISO file,do not change stSysCurPtnHnd!\n");	
    }
    else
    #endif
	_MainApp_SWSysDrive(pstEventInfo, DEVICE_TYPE_DISC);
	/*check main app state,now just normal state do something */
	MAINAPP_GetMainAppState(&dMainAppState);
	if (dMainAppState == MAIN_APP_NORMAL)
	{
		UINT32 dSysAppIdx = TOTAL_SYS_APP_SIZE;
		INT32 sdRet = MAIN_APP_SUCCESS;
		/*check sys app active state. */
		sdRet = MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
		switch (sdRet)
		{
		case MAIN_APP_NO_ACTIVE_SYSTEM_APP:
			mainapp_printf("%s on NO_ACTIVE_SYSTEM_APP.\n", __FUNCTION__);
			ShareGobj_IF_ShowHintMsg(HINT_MSG_DISC_TYPE_BD, HINT_MSG_TIMEOUT, MSG_PARAM_RESET_BG_MSG);
			PSREG_SET_VALUE(PSREG_STAT_MainAppStatus, PSREG_MAINAPPSTATUS_BDIndentify);
			if(!pstEventInfo->bDiscResumeFlag)
			{
				_MainApp_ActivateSysApp(pstEventInfo);
			}
			SysAppActiveAckCallBack=_MainApp_DiscInsertEvent;//add by george.chang 2010.10.08
			break;

		case MAIN_APP_SUCCESS:
			switch (dSysAppIdx)
			{
			case SYS_APP_DISC_PLAYER:
            			SYSAPP_IF_SendCriticalGlobalEventWithIndex(SYS_APP_DISC_PLAYER, DMN_EVENT_DISC_INSERTED, 0);//add by george.chang 2010.09.09
				break;
			case SYS_APP_FILE_PLAYER:
				break;
			case TOTAL_SYS_APP_SIZE:
				break;
			default:
				break;
			}

			if ((PSREG_VALUE(PSREG_STAT_MainAppStatus) == PSREG_MAINAPPSTATUS_USB_ATTACH)
			  	|| (PSREG_VALUE(PSREG_STAT_MainAppStatus) == PSREG_MAINAPPSTATUS_CARD_ATTACH))
			{
				mainapp_printf("%s call DI_DiscStop !!!! \n",__FUNCTION__);
				DI_DiscStop(gPhisycalLoaderDIHandle);
			}

			#ifdef SUPPORT_ISO_IMAGE
			if (PSREG_VALUE(PSREG_FILE_LOADER) == PSREG_FILE_LOADER_IDENTIFY_DISC)
			{
  				sdRet = MAINAPP_TerminateSystemApp(dSysAppIdx);
				if (sdRet == SYSTEM_APP_SUCCESS)
					SysAppTerminateAckCallBack = MainApp_BDInsertCBK;
				else
					fldr_printf("Terminate System App fail [%d]\n", sdRet);
			}
			#endif // SUPPORT_ISO_IMAGE

			break;

		default:
			break;
		}
	}
	else
		mainapp_printf("%s on main app state[%d].\n", __FUNCTION__, dMainAppState);
}

static void APPNOINLINE _MainApp_DaemonEventUnknownDisc(UINT32 dParam, MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	UINT32 dMainAppState = MAIN_APP_NORMAL;
	MAINAPP_GetMainAppState(&dMainAppState);
	if (dMainAppState == MAIN_APP_NORMAL)
	{
		UINT32 dSysAppIdx = TOTAL_SYS_APP_SIZE;
		INT32 sdRet = MAIN_APP_SUCCESS;
		sdRet = MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
		switch (sdRet)
		{
		case MAIN_APP_NO_ACTIVE_SYSTEM_APP:
			mainapp_printf("%s on NO_ACTIVE_SYSTEM_APP.\n", __FUNCTION__);
			_MainApp_MaintainGobjOnNoDisc(pstEventInfo);
			ShareGobj_IF_ShowHintMsg(HINT_MSG_NO_DISC, HINT_MSG_TIMEOUT_5S, MSG_PARAM_RESET_BG_MSG);
			break;

		case MAIN_APP_SUCCESS:
			switch (dSysAppIdx)
			{
			case SYS_APP_DISC_PLAYER:
				ShareGobj_IF_ShowHintMsg(HINT_MSG_NO_DISC, HINT_MSG_TIMEOUT_5S, MSG_PARAM_RESET_BG_MSG);
				break;
			case SYS_APP_FILE_PLAYER:
				ShareGobj_IF_ShowHintMsg(HINT_MSG_NO_DISC, HINT_MSG_TIMEOUT_5S, MSG_PARAM_NULL);
				break;
			case TOTAL_SYS_APP_SIZE:
				ShareGobj_IF_ShowHintMsg(HINT_MSG_NO_DISC, HINT_MSG_TIMEOUT_5S, MSG_PARAM_RESET_BG_MSG);
				break;
			default:
				ShareGobj_IF_ShowHintMsg(HINT_MSG_NO_DISC, HINT_MSG_TIMEOUT_5S, MSG_PARAM_RESET_BG_MSG);
				break;
			}
			break;

		default:
			break;
		}
	}
	else
		mainapp_printf("%s on main app state[%d].\n", __FUNCTION__, dMainAppState);

	PSREG_SET_VALUE(PSREG_STAT_MainAppStatus, PSREG_MAINAPPSTATUS_NODISC);
}

static void APPNOINLINE _MainApp_DaemonEventErrorDisc(UINT32 dParam, MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	UINT32 dMainAppState = MAIN_APP_NORMAL;
	MAINAPP_GetMainAppState(&dMainAppState);
	if (dMainAppState == MAIN_APP_NORMAL)
	{
		UINT32 dSysAppIdx = TOTAL_SYS_APP_SIZE;
		INT32 sdRet = MAIN_APP_SUCCESS;
		sdRet = MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
		switch (sdRet)
		{
		case MAIN_APP_NO_ACTIVE_SYSTEM_APP:
			mainapp_printf("%s on NO_ACTIVE_SYSTEM_APP.\n", __FUNCTION__);
			_MainApp_MaintainGobjOnNoDisc(pstEventInfo);
			break;

		case MAIN_APP_SUCCESS:
			switch (dSysAppIdx)
			{
			case SYS_APP_DISC_PLAYER:
				break;
			case SYS_APP_FILE_PLAYER:
				#ifdef SUPPORT_ISO_IMAGE
				if (PSREG_VALUE(PSREG_FILE_LOADER) == PSREG_FILE_LOADER_IDENTIFY_DISC)
					PSREG_SET_VALUE(PSREG_FILE_LOADER, PSREG_FILE_LOADER_RESUME);
				#endif // SUPPORT_ISO_IMAGE
				break;
			case TOTAL_SYS_APP_SIZE:
				break;
			default:
				break;
			}

			mainapp_printf("%s call DI_DiscStop !!!! \n",__FUNCTION__);
			DI_DiscStop(gPhisycalLoaderDIHandle);
			break;

		default:
			break;
		}
	}
	else
		mainapp_printf("%s on main app state[%d].\n", __FUNCTION__, dMainAppState);

	PSREG_SET_VALUE(PSREG_STAT_MainAppStatus, PSREG_MAINAPPSTATUS_DISC_ERROR);
	switch(PSREG_VALUE(PSREG_STAT_PlayMode))
	{
		case PSREG_PLAYMODE_PLAY	:
		case PSREG_PLAYMODE_PAUSE:
			ShareGobj_IF_ShowHintMsg(HINT_MSG_BG_DISC_ERROR, HINT_MSG_TIMEOUT_5S, MSG_PARAM_RESET_BG_MSG);
			break;
		case  PSREG_PLAYMODE_IDLE	:
		case PSREG_PLAYMODE_STOP	:
		case PSREG_PLAYMODE_BROWSE:
			ShareGobj_IF_ShowHintMsg(HINT_MSG_DISC_ERROR, HINT_MSG_TIMEOUT_5S, MSG_PARAM_RESET_BG_MSG);
			break;
		default:
			mainapp_printf("Unknown play mode.\n");
			break;
	}

	mainapp_printf("%s call DI_DiscStop !!!! \n",__FUNCTION__);
	DI_DiscStop(gPhisycalLoaderDIHandle);
}

static void APPNOINLINE _MainApp_DaemonEventStorageAttached(UINT32 dParam, MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	UINT32 dDeviceType = 0;
	UINT32 dMainAppState = MAIN_APP_NORMAL;

	switch(pstEventInfo->dAppEvent)
	{
		case DMN_EVENT_CARD_DEV_ATTACHED:
			dDeviceType = DEVICE_TYPE_CARD;
			break;
		case DMN_EVENT_IDE_HDD_ATTACHED:
			dDeviceType = DEVICE_TYPE_HDD;
			break;
		case DMN_EVENT_USB_HDD_ATTACHED:
			dDeviceType = DEVICE_TYPE_USB;
			break;
		default:
			break;
	}

	if(APPPTN_SUCCESSFUL != AppPtn_Mount(dParam, FILE_SYSTEM_FAT, dDeviceType))
	{
		mainapp_error("mount error!\n");
		return;
	}
	/*check main app state,now just normal state do something */
	MAINAPP_GetMainAppState(&dMainAppState);
	if (dMainAppState == MAIN_APP_NORMAL)
	{
		UINT32 dSysAppIdx = TOTAL_SYS_APP_SIZE;
		INT32 sdRet = MAIN_APP_SUCCESS;
		/*check sys app active state. */
		sdRet = MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
		switch (sdRet)
		{
			case MAIN_APP_NO_ACTIVE_SYSTEM_APP:
                                #ifdef USB_MODE_POWEROFF_SERVO
				if (PSREG_VALUE(PSREG_STAT_MainAppStatus) == PSREG_MAINAPPSTATUS_NODISC)
					{
					pstEventInfo->bDevmode=1;
					AppPtn_DiscEject(DI_Handle_PHYSICAL);
				         DI_DiscStandBy(gPhisycalLoaderDIHandle);
					}
                                #endif
				mainapp_printf("%s on NO_ACTIVE_SYSTEM_APP.\n", __FUNCTION__);
				ShareGobj_IF_ShowHintMsg(HINT_MSG_FILE_LOADING, HINT_MSG_TIMEOUT_INFINITE, MSG_PARAM_SHOW_BG_MSG);
				break;

			case MAIN_APP_SUCCESS:
			{
				switch (dSysAppIdx)
				{
					case SYS_APP_DISC_PLAYER:
						break;
					case SYS_APP_FILE_PLAYER:
						break;
					#ifdef SUPPORT_PLAYER_STB_DVB_T
					case SYS_APP_DVB:
						MAINAPP_SendEvent4DvbAP(dSysAppIdx, pstEventInfo->dAppEvent | PASS_TO_SYSAPP, dParam);
						break;
					#endif
					case TOTAL_SYS_APP_SIZE:
						break;
					default:
						_MAINAPP_SendEvent4NetworkAPs(dSysAppIdx, pstEventInfo->dAppEvent, dParam);
						break;
				}
				break;
			}

			default:
				break;
		}
	}
	else
		mainapp_printf("%s on main app state[%d].\n", __FUNCTION__, dMainAppState);
}

static void APPNOINLINE _MainApp_DaemonEventStorageDetached(MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	int sdRet = MAIN_APP_SUCCESS;
	UINT32 dSysAppIdx = TOTAL_SYS_APP_SIZE;
	UINT32 dMainAppState = MAIN_APP_NORMAL;
	SysDriveCtrlInfo_t *pstSysDriveCtrlInfo = &pstEventInfo->stSysDriveCtrlInfo;
	UINT32 dDetachDevIdx;
	AppPtn_Handle_t stPtnHnd = NULL;
	UINT32 dDrvCnt =0;

	mainapp_printf(" %s  dev:[%d]\n",__FUNCTION__, pstEventInfo->dDetachDevIdx);
	if (pstEventInfo->dDetachDevIdx != INVALID_VALUE)
		dDetachDevIdx = pstEventInfo->dDetachDevIdx;
	else
		dDetachDevIdx = pstEventInfo->dCbkDetachDevIdx;

#ifdef NET_ET_SUPPORT
	_MainApp_EtUnmountHandle(pstEventInfo,dDetachDevIdx);
#endif
#ifdef NET_N32_SUPPORT
	_MainApp_N32UnmountHandle(pstEventInfo,dDetachDevIdx);
#endif
	sdRet = AppPtn_GetPtnHandle(&stPtnHnd, dDetachDevIdx);
	if(sdRet != APPPTN_SUCCESSFUL)
	{
		mainapp_printf(" detach dev not in ptn list \n");
		return;
	}
	if(!stPtnHnd)
	{
		mainapp_error("invalidate ptn handle\n");
		// Mantis 96634, Unmount "/udska1" fail issue. RC: Need to use dCbkDetachDevIdx to remove "/udska1"
		pstEventInfo->dDetachDevIdx = INVALID_VALUE;
		return;
	}

	do {
#ifdef SUPPORT_ISO_IMAGE
		if ((PSREG_VALUE(PSREG_FILE_LOADER) == PSREG_FILE_LOADER_PLAY)
			&& (stPtnHnd == pstSysDriveCtrlInfo->stSysPrePtnHnd))
		{
			PSREG_SET_VALUE(PSREG_FILE_LOADER, PSREG_FILE_LOADER_DEV_REMOVE);

			/*check sys app active state. */
			sdRet = MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
			if ((sdRet == MAIN_APP_SUCCESS) && (dSysAppIdx == SYS_APP_DISC_PLAYER))
			{
				sdRet = MAINAPP_TerminateSystemApp(dSysAppIdx);
				if (sdRet == SYSTEM_APP_SUCCESS)
				{
					SysAppTerminateAckCallBack = _MainApp_FileLoaderRemoveDev;
					pstEventInfo->dDetachDevIdx = INVALID_VALUE;    //lgz 2011-3-2 17:33 add for mantis 0126533
					pstEventInfo->dCbkDetachDevIdx = dDetachDevIdx;
				}					
				else
					mainapp_error(" Terminate System App[%d]\n", sdRet);
				return;
			}
		}
#endif // SUPPORT_ISO_IMAGE
		if (pstSysDriveCtrlInfo->stSysCurPtnHnd  != stPtnHnd)
		{
			mainapp_printf(" not detach current partition \n");
			AppPtn_RemovePtn(dDetachDevIdx);
			AppPtn_Umount(dDetachDevIdx);
			#ifdef NET_SUPPORT
			_MainApp_SetupMenuDLPosDetect();
			#endif
			pstEventInfo->dDetachDevIdx = INVALID_VALUE;
			break;
		}
                #ifdef USB_MODE_POWEROFF_SERVO
                if (pstEventInfo->bDevmode == 1)
       	          {
	                  DI_DiscResume(gPhisycalLoaderDIHandle);
                          pstEventInfo->bDevmode = 0;
       	           }
                 #endif
		MAINAPP_GetMainAppState(&dMainAppState);
		if (dMainAppState == MAIN_APP_NORMAL)
		{
			sdRet = MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);

			switch (sdRet)
			{
			case MAIN_APP_NO_ACTIVE_SYSTEM_APP:
				mainapp_printf("%s on NO_ACTIVE_SYSTEM_APP.\n", __FUNCTION__);
				/*pay attention:AppPtn_RemovePtn() should be called before AppPtn_Umount()*/
				AppPtn_RemovePtn(dDetachDevIdx);
				AppPtn_Umount(dDetachDevIdx);
				#ifdef NET_SUPPORT
				_MainApp_SetupMenuDLPosDetect();
				#endif
				pstEventInfo->dDetachDevIdx = INVALID_VALUE;
    			pstEventInfo->dCbkDetachDevIdx = INVALID_VALUE;
				/*auto change sys actived drive */
				//_MainApp_AutoSWSysDrive(pstEventInfo);
				AppPtn_GetAllPtnCount(&dDrvCnt);

				APP_Show_Logo(CAPTURE_LOGO,LOGO_WITH_WORD); //jiangxl 
				if (dDrvCnt >= 1)
				{
					/* open source menu*/
					if(pstEventInfo->stComGobj.stSourceGobj.dMenuTimeOut == 0)
					{
						pstEventInfo->stComGobj.stSourceGobj.dMenuTimeOut = 5*1000;/*5s*/
						_MainApp_MaintainGobjOnSource(pstEventInfo,1);
					}
				}
				else
				{
					/* open home menu*/
					_MainApp_MaintainGobjOnSetup(pstEventInfo,1);
				}

				break;

			case MAIN_APP_SUCCESS:
				switch (dSysAppIdx)
				{
				case SYS_APP_DISC_PLAYER:
					mainapp_printf("%s on SYS_APP_DISC_PLAYER.\n", __FUNCTION__);
					//TODO : clear the media type icon
					/*usb drive is under file app, so under disc player just need
					   mention user usb plug out */
					#if (SUPPORT_BDMV_USB_LOAD==1)
					if(monflg==MONFLG_LOAD_USB_BDMV)// Load BDMB file from USB add by george.chang2011.01.11
					{
						sdRet = MAINAPP_TerminateSystemApp(dSysAppIdx);
						if (sdRet == SYSTEM_APP_SUCCESS)
						{
							SysAppTerminateAckCallBack = _MainApp_DaemonEventStorageDetached;
							pstEventInfo->dDetachDevIdx = INVALID_VALUE;
							pstEventInfo->dCbkDetachDevIdx = dDetachDevIdx;

						}
					else
						mainapp_error(" Terminate System App[%d] error \n", sdRet);
					}
					#endif
					break;
				case SYS_APP_FILE_PLAYER:

					mainapp_printf("%s on SYS_APP_FILE_PLAYER.\n", __FUNCTION__);
					/*check current sys active drive is changed or not.
					   if changed, need terminate the sys app, or switch to anthor sys
					   app, waiting to decide! */
					mainapp_printf("terminate file app \n");
    				sdRet = MAINAPP_TerminateSystemApp(dSysAppIdx);
					if (sdRet == SYSTEM_APP_SUCCESS)
					{
						SysAppTerminateAckCallBack = _MainApp_DaemonEventStorageDetached;
						pstEventInfo->dDetachDevIdx = INVALID_VALUE;
						pstEventInfo->dCbkDetachDevIdx = dDetachDevIdx;
					}
					else
						mainapp_error(" Terminate System App[%d] error \n", sdRet);
					break;

		#ifdef SUPPORT_PLAYER_STB_DVB_T
				case SYS_APP_DVB:
					mainapp_printf("%s on DVB SYS_APP_PLAYER.\n", __FUNCTION__);
					/*pay attention:AppPtn_RemovePtn() should be called before AppPtn_Umount()*/
					AppPtn_RemovePtn(dDetachDevIdx);
					AppPtn_Umount(dDetachDevIdx);
					#ifdef NET_SUPPORT
					_MainApp_SetupMenuDLPosDetect();
					#endif
					/*bup up current sys actived drive */
					//stCurPtnHnd = pstSysDriveCtrlInfo->stSysCurPtnHnd;
					pstEventInfo->dDetachDevIdx = INVALID_VALUE;
    				        pstEventInfo->dCbkDetachDevIdx = INVALID_VALUE;
					/*auto change sys actived drive */
					_MainApp_AutoSWSysDrive(pstEventInfo);	
					//MAINAPP_SendEvent4DvbAP(dSysAppIdx, pstEventInfo->dAppEvent | PASS_TO_SYSAPP, dDetachDevIdx);
					break;
		#endif
				case TOTAL_SYS_APP_SIZE:
					mainapp_printf("%s on TOTAL_SYS_APP_SIZE.\n", __FUNCTION__);
					break;

		#ifdef NET_SUPPORT
			#ifdef NET_ET_SUPPORT
				case SYS_APP_ET_PLAYER:
			#endif
			#ifdef NET_N32_SUPPORT
				case SYS_APP_N32_PLAYER:
			#endif
                        #ifdef NET_PHOTO_SUPPORT
                                case SYS_APP_NETPHOTO_PLAYER:
                        #endif
			#ifdef NET_NEWS_SUPPORT
               		case SYS_APP_NEWS_PLAYER:
                 	#endif
			#ifdef NET_STOCK_SUPPORT
               		case SYS_APP_STOCK_PLAYER:
                 	#endif
			#ifdef NET_WEATHER_SUPPORT
               		case SYS_APP_YWEATHER:
                 	#endif
			#ifdef NET_GMAP_SUPPORT
               		case SYS_APP_GMAP:
                 	#endif
			#if defined(NET_RADIO_SUPPORT) || defined(NET_LOCAL_RADIO_SUPPORT)
               		case SYS_APP_NETRADIO_PLAYER:
                 	#endif
			#ifdef NET_BROWSER_SUPPORT
               		case SYS_APP_BROWSER:
                 	#endif			
			#if defined(NET_ET_SUPPORT) || defined(NET_N32_SUPPORT)||defined(NET_PHOTO_SUPPORT)||defined(NET_NEWS_SUPPORT)||defined(NET_STOCK_SUPPORT)||defined(NET_WEATHER_SUPPORT)||defined(NET_GMAP_SUPPORT)||defined(NET_RADIO_SUPPORT)||defined(NET_LOCAL_RADIO_SUPPORT)||defined(NET_BROWSER_SUPPORT)
					mainapp_printf("%s on Net SYS_APP_PLAYER.\n", __FUNCTION__);
					/*pay attention:AppPtn_RemovePtn() should be called before AppPtn_Umount()*/
					AppPtn_RemovePtn(dDetachDevIdx);
					AppPtn_Umount(dDetachDevIdx);
					#ifdef NET_SUPPORT
					_MainApp_SetupMenuDLPosDetect();
					#endif
					/*bup up current sys actived drive */
					//stCurPtnHnd = pstSysDriveCtrlInfo->stSysCurPtnHnd;
					pstEventInfo->dDetachDevIdx = INVALID_VALUE;
    					pstEventInfo->dCbkDetachDevIdx = INVALID_VALUE;
					/*auto change sys actived drive */
					_MainApp_AutoSWSysDrive(pstEventInfo);

					_MAINAPP_SendEvent4NetworkAPs(dSysAppIdx, pstEventInfo->dAppEvent, dDetachDevIdx);
					break;
			#endif
		#endif
				default:
						_MAINAPP_SendEvent4NetworkAPs(dSysAppIdx, pstEventInfo->dAppEvent, dDetachDevIdx);
					break;
				}
				break;

			default:
				mainapp_error("%s on %d condition.\n", __FUNCTION__, sdRet);
				break;
			}
		}
		else
			mainapp_printf("%s on main app state[%d].\n", __FUNCTION__, dMainAppState);
	} while(0);
	/*Show hint message*/
	switch(pstEventInfo->dAppEvent)
	{
		case DMN_EVENT_CARD_DEV_DETACHED:
			ShareGobj_IF_ShowHintMsg(HINT_MSG_CARD_PLUG_OUT, HINT_MSG_TIMEOUT, MSG_PARAM_NULL);
			PSREG_SET_VALUE(PSREG_STAT_MainAppStatus, PSREG_MAINAPPSTATUS_CARD_DETACH);
			break;
		case DMN_EVENT_IDE_HDD_DETACHED:
			ShareGobj_IF_ShowHintMsg(HINT_MSG_USB_PLUG_OUT, HINT_MSG_TIMEOUT, MSG_PARAM_NULL);
			PSREG_SET_VALUE(PSREG_STAT_MainAppStatus, PSREG_MAINAPPSTATUS_USB_DETACH);
			break;
		case DMN_EVENT_USB_HDD_DETACHED:
			ShareGobj_IF_ShowHintMsg(HINT_MSG_USB_PLUG_OUT, HINT_MSG_TIMEOUT, MSG_PARAM_NULL);
			PSREG_SET_VALUE(PSREG_STAT_MainAppStatus, PSREG_MAINAPPSTATUS_USB_DETACH);
			break;
		default:
			break;
	}
}

static void APPNOINLINE _MainApp_DaemonEventScrnSaverOn(UINT32 dParam, MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	UINT32 dMainAppState = MAIN_APP_NORMAL;
	MAINAPP_GetMainAppState(&dMainAppState);
	if (dMainAppState == MAIN_APP_NORMAL)
	{
		UINT32 dSysAppIdx = TOTAL_SYS_APP_SIZE;
		INT32 sdRet = MAIN_APP_SUCCESS;
		sdRet = MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
		switch (sdRet)
		{
		case MAIN_APP_NO_ACTIVE_SYSTEM_APP:
			break;

		case MAIN_APP_SUCCESS:
			switch (dSysAppIdx)
			{
			case SYS_APP_DISC_PLAYER:
				break;
			case SYS_APP_FILE_PLAYER:
				break;
			case TOTAL_SYS_APP_SIZE:
				break;
			default:
				break;
			}
			break;

		default:
			break;
		}
	}
	else
		mainapp_printf("%s on main app state[%d].\n", __FUNCTION__, dMainAppState);

	GUI_FUNC_CALL(GEL_HideRegions());

	app_DisplayOSDPlaneOnly();
	int i;
	if (!GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_SCREENSAVER))
	{
		GOBJ_MGR_CreateObject(pMainAppInstance->pGUIObjectTable[MAINAPP_SCREENSAVER], pMainAppInstance->GUIObjData, MAINAPP_SCREENSAVER,
							  (UINT32) pstEventInfo);

		for(i=UI_EVENT_0;i<UI_EVENT_NULL;i++)
			MSG_FILTER_InsertMsgToTable(i);
	}
	else
	{
		for(i=UI_EVENT_0;i<UI_EVENT_NULL;i++)
			MSG_FILTER_RemoveMsgFromTable(i);

		GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_SCREENSAVER);
	}
}

static void APPNOINLINE _MainApp_DaemonEventScrnSaverOff(UINT32 dParam, MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	UINT32 dMainAppState = MAIN_APP_NORMAL;
	MAINAPP_GetMainAppState(&dMainAppState);
	if (dMainAppState == MAIN_APP_NORMAL)
	{
		UINT32 dSysAppIdx = TOTAL_SYS_APP_SIZE;
		INT32 sdRet = MAIN_APP_SUCCESS;
		sdRet = MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
		switch (sdRet)
		{
		case MAIN_APP_NO_ACTIVE_SYSTEM_APP:
			break;

		case MAIN_APP_SUCCESS:
			switch (dSysAppIdx)
			{
			case SYS_APP_DISC_PLAYER:
				break;
			case SYS_APP_FILE_PLAYER:
				break;
			case TOTAL_SYS_APP_SIZE:
				break;
			default:
				break;
			}
			break;

		default:
			break;
		}
	}
	else
		mainapp_printf("%s on main app state[%d].\n", __FUNCTION__, dMainAppState);

	int i;
	for(i=UI_EVENT_0;i<UI_EVENT_NULL;i++)
			MSG_FILTER_RemoveMsgFromTable(i);
	GOBJ_MGR_DestroyObject(pMainAppInstance->GUIObjData, MAINAPP_SCREENSAVER);

	app_RestoreAllPlane();

	GUI_FUNC_CALL(GEL_RestoreRegions());
}

static void APPNOINLINE _MainApp_DaemonEventHdmiEdidValid(UINT32 dParam, MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	//following handle HDMI Color Setting
	if (PSREG_IS_INVALID(PSREG_SETUP_HDMI_Color_Setting))
	{
		PSREG_SET_VALID(PSREG_SETUP_HDMI_Color_Setting);
		PSREG_SET_VALUE(PSREG_SETUP_HDMI_Color_Setting, PSREG_SETUP_HDMI_Color_Auto);
	}

	ColorSpaceFormat_t eCurrColorSpace;
	HDMI_IF_Get_ColorSpace(&eCurrColorSpace);

	ColorSpaceFormat_t eTempColorSpace;
	if (PSREG_VALUE(PSREG_SETUP_HDMI_Color_Setting) == PSREG_SETUP_HDMI_Color_Auto)
	{
		HDMI_IF_Get_PreferColorSpace(&eTempColorSpace);

		if (eTempColorSpace != eCurrColorSpace)
			SetupConfig_IF_HDMIColorSetting(eTempColorSpace);
	}
	else
	{
		switch (PSREG_VALUE(PSREG_SETUP_HDMI_Color_Setting))
		{
		case PSREG_SETUP_HDMI_Color_YCbCr:
			eTempColorSpace = HDMI_CSF_YC444;
			break;
		case PSREG_SETUP_RGB:
			eTempColorSpace = HDMI_CSF_RGB;
			break;
		case PSREG_SETUP_xv_Color:
			eTempColorSpace = HDMI_CSF_XVCOLOR;
			break;
		}

		if (eTempColorSpace != eCurrColorSpace)
			SetupConfig_IF_HDMIColorSetting(eTempColorSpace);
	}

	//following handle HDMI Deep Color
	if (PSREG_IS_INVALID(PSREG_SETUP_HDMI_Deep_Color))
	{
		PSREG_SET_VALID(PSREG_SETUP_HDMI_Deep_Color);
		PSREG_SET_VALUE(PSREG_SETUP_HDMI_Deep_Color, PSREG_SETUP_HDMI_Deep_Color_Auto);
	}

	DeepColorFormat_t eCurrDeepColor;
	HDMI_IF_Get_DeepColor(&eCurrDeepColor);

	DeepColorFormat_t eTempDeepColor;
	if (PSREG_VALUE(PSREG_SETUP_HDMI_Deep_Color) == PSREG_SETUP_HDMI_Deep_Color_Auto)
	{
		HDMI_IF_Get_PreferDeepColor(&eTempDeepColor);

		if (eTempDeepColor != eCurrDeepColor)
			SetupConfig_IF_HDMIDeepColor(eTempDeepColor);
	}
	else
	{
		switch (PSREG_VALUE(PSREG_SETUP_HDMI_Deep_Color))
		{
		case PSREG_SETUP_8bit:
			eTempDeepColor = HDMI_DCF_8_BITS;
			break;
		case PSREG_SETUP_10bit:
			eTempDeepColor = HDMI_DCF_10_BITS;
			break;
		case PSREG_SETUP_12bit:
			eTempDeepColor = HDMI_DCF_12_BITS;
			break;
		}

		if (eTempDeepColor != eCurrDeepColor)
			SetupConfig_IF_HDMIDeepColor(eTempDeepColor);
	}

	MainApp_SetupAutoSettings(pstEventInfo);
}

static void APPNOINLINE _MainApp_DaemonEventSetLang(UINT32 dParam, MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	UINT8 bLangMatched=0;
	int i,j;
  	for(i=0;i<=LCD_2_NON;i++)
  	{
  		for(j=0;j<ISO639_2_CODE_VARIATION;j++)
  		{
	  		if((dParam&ISo639_2_CODE_MASK)==(lang_639_2_code[i][j]&ISo639_2_CODE_MASK))
	  		{
				bLangMatched=1;
	  			break;
	  		}
  		}
		if(bLangMatched)
			break;
  	}

	setupmenu_item_id_t eMenuLangItemID=-1;
	switch(i) //only system supported languages are listed in cases.
	{		//system supported languages >= Setup Menu customization supported languages
		case LCD_2_ENG:
			eMenuLangItemID=L3_ITEM_MenuLang_English;
			break;
		case LCD_2_CHI:
			eMenuLangItemID=L3_ITEM_MenuLang_SimpChinese;
			//eMenuLangItemID=L3_ITEM_MenuLang_TradChinese;
			break;
		case LCD_2_KOR:
			eMenuLangItemID=L3_ITEM_MenuLang_Korean;
			break;
		case LCD_2_JPN:
			eMenuLangItemID=L3_ITEM_MenuLang_Japanese;
			break;
		case LCD_2_FRE:
			eMenuLangItemID=L3_ITEM_MenuLang_French;
			break;
		case LCD_2_ESL:
			eMenuLangItemID=L3_ITEM_MenuLang_Spanish;
			break;
		case LCD_2_POR:
			eMenuLangItemID=L3_ITEM_MenuLang_Portuguese;
			break;
		case LCD_2_ITA:
			eMenuLangItemID=L3_ITEM_MenuLang_Italian;
			break;
		case LCD_2_DEU:
			eMenuLangItemID=L3_ITEM_MenuLang_German;
			break;
		case LCD_2_HUN:
			eMenuLangItemID=L3_ITEM_MenuLang_Hungarian;
			break;
		case LCD_2_POL:
			eMenuLangItemID=L3_ITEM_MenuLang_Polish;
			break;
		case LCD_2_RUS:
			eMenuLangItemID=L3_ITEM_MenuLang_Russian;
			break;
		case LCD_2_DUT:
			eMenuLangItemID=L3_ITEM_MenuLang_Dutch;
			break;
		case LCD_2_SVE:
			eMenuLangItemID=L3_ITEM_MenuLang_Swedish;
			break;
		case LCD_2_CES:
			eMenuLangItemID=L3_ITEM_MenuLang_Czech;
			break;
		case LCD_2_ELL:
			eMenuLangItemID=L3_ITEM_MenuLang_Greek;
			break;
		case LCD_2_FAS:
			eMenuLangItemID=L3_ITEM_MenuLang_Iranian;
			break;
		case LCD_2_VIE:
			eMenuLangItemID=L3_ITEM_MenuLang_Vietnamese;
			break;
		case LCD_2_TUR:
			eMenuLangItemID=L3_ITEM_MenuLang_Turkish;
			break;
		case LCD_2_IND:
			eMenuLangItemID=L3_ITEM_MenuLang_Indonesian;
			break;
		case LCD_2_THA:
			eMenuLangItemID=L3_ITEM_MenuLang_Thai;
			break;
		case LCD_2_ARA:
			eMenuLangItemID=L3_ITEM_MenuLang_Arabic;
			break;

		default:
			mainapp_printf("[%s]: System does not support this language = %d (iso639_2_lcd_t)\n",  __FUNCTION__,i);
			break;
	}

	if(SetupMenu_FindItem(eMenuLangItemID,1)) //only if Setup Menu has this item, set the language
	{
		_MainApp_MaintainGobjOnEject(pstEventInfo); //Same action as eject tray in.
		SetupMenu_SetFunc(eMenuLangItemID, 0, 0);
	}
}

void _MainApp_Setup_Initialize(void)
{
/*lingyong move setup init process to this function, attention: some function may not be included
in define EEPROM_FLASH_STORE_INFO and SUPPORT_NVM_DRV_VER1*/
	NLayer_CustomizationVerify(&stSetupMenu);
	#ifdef SUPPORT_PLAYER_STB_DVB_T
	NLayer_CustomizationVerify(&stDvbMainMenu);
	#endif
	SpkrMenu_CustomizationVerify();
	SetupMenu_HiddenItemSetting(0);

	int i;
	UINT8 bTempData = !Flash_1st_Write_Flag;
	UINT32 bCheckSum = 0;
	NVM_Read_Section(NVM_SECT_SETUP_ID, SETUP_VA_Flash_1st_Write, &bTempData, sizeof(bTempData));
	if (bTempData != Flash_1st_Write_Flag)	// 1st boot
	{
		bTempData = Flash_1st_Write_Flag;
		NVM_Write_Section(NVM_SECT_SETUP_ID, SETUP_VA_Flash_1st_Write, &bTempData, sizeof(bTempData));

		SetupRelatedMenu_InitSettings(1,0);
		SetupMenu_UpdateSetupMenuCheckSum();
#if defined(NET_WIFI_SUPPORT)
              NetMenu_InitSettings(1, 0, NET_ITEM_NULL);
#endif
	}
	else
	{
		bCheckSum = 0;
		for (i = SETUP_VA_TV_System; i < SETUP_VA_CheckSum; i++)
		{
			NVM_Read_Section(NVM_SECT_SETUP_ID, i, &bTempData, sizeof(bTempData));
			bCheckSum += bTempData;
		}

		UINT32 bOrigCheckSum;
		NVM_Read_Section(NVM_SECT_SETUP_ID, SETUP_VA_CheckSum, (BYTE *) & bOrigCheckSum, sizeof(bOrigCheckSum));
		if (bCheckSum != bOrigCheckSum)
		{
			mainapp_printf("[%s]: ERROR: Setup Menu checksum not the same\n", __FUNCTION__);
			SetupRelatedMenu_InitSettings(1,0);
			SetupMenu_UpdateSetupMenuCheckSum();
		}
		else
			SetupRelatedMenu_InitSettings(0,0);	//exe settings
#if defined(NET_WIFI_SUPPORT)
              NetMenu_InitSettings(0, 0, NET_ITEM_NULL);
#endif
	}
}

static void _MainApp_Gui_Initialize(void)
{
	// Set common font information
	GUI_FUNC_CALL(GEL_SetFontInfo(COM_UI_ui_font_info, COM_UI_FONT_BLOCK_NUM, COM_UI_FONT_STYLE_NUMBER, COM_UI_UI_LANG_NUM));
	GUI_FUNC_CALL(GEL_SetStrInfo(COM_UI_string_info, COM_UI_STR_ID_NUM));
	GUI_FUNC_CALL(GEL_SetSUBTFontbase(COM_UI_Size_34)); //set the font base (full font) used by subtitle, added by ted.chen
	GOBJ_MGR_CreateObject(pMainAppInstance->pGUIObjectTable[MAINAPP_GUIOBJ_SHARE], pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_SHARE,
						  (UINT32) & stUIEventHandlerInfo);
	GOBJ_MGR_CreateObject(pMainAppInstance->pGUIObjectTable[MAINAPP_GUIOBJ_VOLUME], pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_VOLUME,
						  (UINT32) & stUIEventHandlerInfo);
	#ifdef VFD_TEST
	GOBJ_MGR_CreateObject(pMainAppInstance->pGUIObjectTable[MAINAPP_GUIOBJ_VFD_TEST],pMainAppInstance->GUIObjData,MAINAPP_GUIOBJ_VFD_TEST,
		(UINT32) & stUIEventHandlerInfo);
	#endif
}
static void _MainApp_APInit(void)
{
	memset(&stUIEventHandlerInfo, 0, sizeof(MainAppUIEventHandlerInfo_t));
	_MainApp_Gui_Initialize();
	_MainApp_Setup_Initialize();

	BG_Pic_Show(Logo_Origin_Align); //this is for Setup Menu skin thumbnail on VPP

#ifdef PLFNV_SUPPORT_DIVX_DRM
	DivXDRM_Initialize();
#endif
#ifdef SUPPORT_RESUME
	if (Rsm_IF_Initialize(RsmCallBackFunc) != RSM_SUCCESS)
		mainapp_printf("ERROR: resume init not OK\n");
#endif

#ifdef NET_SUPPORT
	NetDaemonCfg_t stNetDaemon;
	stNetDaemon.pfEventCallBack = NetDaemon_callback;

	if(NET_IF_Initialize(&stNetDaemon) != NET_DAEMON_IF_SUCCESSFUL)
		mainapp_error("daemon init net daemon\n");
	if(APP_NET_Initialize() != APP_NET_IF_SUCCESSFUL)
		mainapp_error("app init net daemon\n");
	
	if(NetTest_Init(1)!=0)
		mainapp_printf("ERROR: Init Net Testing Thread\n");
	#ifdef NET_NETWORKTEST_SUPPORT
	if(NetworkTest_Init()!=0)
		mainapp_printf("ERROR: Init Network Testing Thread\n");
	#endif
#endif
	int dRet = 0;
	dRet = ISP_Init(
			SmartUpgradeRoutine,
			SmartUpgrade_CallbackFunc
			);
	if(dRet!=ISP_SUCCESS)
		mainapp_printf("ERROR: Init ISP fail !!!\n");

	App_FileLoader_Init();
	#if(SUPPORT_iPOD_DOCKING==1)
	GPIO_IF_RegisterService(DETECT_NUM_IPOD);
	#endif
	#ifdef NET_MUSIC_SUPPORT
	void _COMGUI_List_AnimInit( void );
	_COMGUI_List_AnimInit(  );
	#endif
}

#ifdef SAMBA_SUPPORT
static void APPNOINLINE _MainApp_DaemonEventNetLinkerDetached(MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	UINT32 dMainAppState = 0;
	AppPtn_Handle_t stSMBPtn = NULL;
	AppPtn_Handle_t stSysPtnHnd = pstEventInfo->stSysDriveCtrlInfo.stSysCurPtnHnd;
	MAINAPP_GetMainAppState(&dMainAppState);
	if (dMainAppState == MAIN_APP_NORMAL)
	{
		UINT32 dSysAppIdx = TOTAL_SYS_APP_SIZE;
		INT32 sdRet = MAIN_APP_SUCCESS;
		/*check sys app active state. */
		AppPtn_GetSMBPtnHandle(&stSMBPtn);
		if(stSysPtnHnd != stSMBPtn)
		{
			AppPtn_RemoveSMBPtn();
			sdRet = MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
			switch (sdRet)
			{
				case MAIN_APP_NO_ACTIVE_SYSTEM_APP:
					diag_printf("Linker detached on MAIN_APP_NO_ACTIVE_SYSTEM_APP\n");
					AppPtn_RemoveSMBPtn();
					_MainApp_AutoSWSysDrive(pstEventInfo);
					if (!GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU)
							&& !GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_SMART_UPGRADTE)
							&& !GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_SETUPMENU)
							&& !GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_SETUPMENUSPKR)
							&& !GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_SUPERSETUPMENU)
							&& !GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_REGIONMENU)
#if defined(NET_WIFI_SUPPORT)
							&& !GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_SETUPMENUNETWORK)
#endif
							&& !GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_CUSTOMERMENU))
								{
						GOBJ_MGR_CreateObject(pMainAppInstance->pGUIObjectTable[MAINAPP_GUIOBJ_MAINMENU],
									pMainAppInstance->GUIObjData,
									MAINAPP_GUIOBJ_MAINMENU,
									(UINT32) pstEventInfo);
						}
					break;

				case MAIN_APP_SUCCESS:
				default:
					break;
				}
		}
		else
		{
			sdRet = MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
			switch (sdRet)
			{
				case MAIN_APP_NO_ACTIVE_SYSTEM_APP:
					AppPtn_RemoveSMBPtn();
					_MainApp_AutoSWSysDrive(pstEventInfo);
					if (!GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_GUIOBJ_MAINMENU)
							&& !GOBJ_MGR_GUIObjectExist(pMainAppInstance->GUIObjData, MAINAPP_SMART_UPGRADTE))
								{
									GOBJ_MGR_CreateObject(
									pMainAppInstance->pGUIObjectTable[MAINAPP_GUIOBJ_MAINMENU],
									pMainAppInstance->GUIObjData,
									MAINAPP_GUIOBJ_MAINMENU,
									(UINT32) pstEventInfo);
						}
					break;

				case MAIN_APP_SUCCESS:
				{
					if (dSysAppIdx == SYS_APP_FILE_PLAYER)
					{
						sdRet = MAINAPP_TerminateSystemApp(SYS_APP_FILE_PLAYER);
						if (sdRet == SYSTEM_APP_SUCCESS)
							SysAppTerminateAckCallBack = _MainApp_DaemonEventNetLinkerDetached;
						else
							mainapp_error(" Terminate System App[%d] error \n", sdRet);
					}
					break;
				}

				default:
					break;
			}
		}
	}
	else
		mainapp_printf("%s on main app state[%d].\n", __FUNCTION__, dMainAppState);
}

static void APPNOINLINE _MainApp_SysAppActiveAckForSMB(MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	SysAppActiveAckCallBack = NULL;
	SysAppActiveFailCallBack = NULL;
	AppPtn_SetFilter(pstEventInfo->stSysDriveCtrlInfo.stSysCurPtnHnd,PTNMNG_FILTER_NATIVE32|PTNMNG_FILTER_NES,APPPTN_SET_FILTER_SET);
}

static void APPNOINLINE _MainApp_SysAppActiveFailForSMB(MainAppUIEventHandlerInfo_t * pstEventInfo)
{
	SysAppActiveAckCallBack = NULL;
	SysAppActiveFailCallBack = NULL;
	AppPtn_SetFilter(pstEventInfo->stSysDriveCtrlInfo.stSysCurPtnHnd,PTNMNG_FILTER_NATIVE32|PTNMNG_FILTER_NES,APPPTN_SET_FILTER_SET);
	ShareGobj_IF_ShowHintMsg(HINT_MSG_LINK_ERROR, HINT_MSG_TIMEOUT_5S, MSG_PARAM_RESET_BG_MSG);
}
#endif

void MAINAPP_DaemonEventHandler(UINT32 dMessage, UINT32 dParam)
{
	MainAppUIEventHandlerInfo_t *pstEventInfo = &stUIEventHandlerInfo;
	UINT32 dSysAppIdx = TOTAL_SYS_APP_SIZE;
	switch (dMessage)
	{
	case DMN_EVENT_BEGIN_IDENTIFY_DISC:
		SSMON_IF_Stop();
		mainapp_printf("DMN_EVENT_DISC_LOADING received.\n");
		pstEventInfo->dAppEvent = dMessage;
		pstEventInfo->eDiscState = DISC_LOADING;
		_MainApp_DaemonEventBegainIdentifyDisc(dParam, pstEventInfo);
		break;

	case DMN_EVENT_DATA_DISC_INSERED:
		SSMON_IF_Start();
		mainapp_printf("[MainApp]: DMN_EVENT_DATA_DISC_INSERED received \n");
	#ifdef SUPPORT_ISO_IMAGE   //lgz 2010-12-8 12:4 add
		// close file loader
		if (PSREG_VALUE(PSREG_FILE_LOADER) == PSREG_FILE_LOADER_IDENTIFY_DISC)
		{			
			fldr_printf("Fileloader insert UNSUPPORT_DISC\n");		
 			AppPtn_DiscEject(DI_Handle_FILE_LOADER);
			DI_TrayOut(gFileLoaderDIHandle);
			fs_fsetinfo(
				pstEventInfo->stSysAppCtrlInfo.stFileAppCtrlInfo.dFileLoaderId,
				FS_INFO_SET_FILE_LOADER_CLOSE_PATH,
				"close",
				5);
			PSREG_SET_VALUE(PSREG_FILE_LOADER, PSREG_FILE_LOADER_RESUME);
			SSMON_IF_Start();
			MSG_FILTER_DispatchMessageEX(UI_EVENT_RETURN, IMAGE_PARAMETER_UNSUPPORT);
			break;
		}
		else
		#endif // SUPPORT_ISO_IMAGE
                 {
		pstEventInfo->dAppEvent = dMessage;
		pstEventInfo->eDiscState = DISC_LOAD_DOWN;
		_MainApp_DaemonEventDataDiscInserted(dParam, pstEventInfo);
                }
		break;
	case DMN_EVENT_VCD_DISC_INSERTED:
		SSMON_IF_Start();
		mainapp_printf("VCD_DISC_INSERTED received.\n");
		pstEventInfo->dAppEvent = dMessage;
		pstEventInfo->eDiscState = DISC_LOAD_DOWN;
		_MainApp_DaemonEventVcdDiscInserted(dParam, pstEventInfo);
		break;
	case DMN_EVENT_MINUSVR_DISC_INSERTED:
		SSMON_IF_Start();
		mainapp_printf("MINUSVR_DISC_INSERTED received.\n");
		pstEventInfo->dAppEvent = dMessage;
		pstEventInfo->eDiscState = DISC_LOAD_DOWN;
		_MainApp_DaemonEventMinusvrDiscInserted(dParam, pstEventInfo);
		break;

	case DMN_EVENT_BD_VIDEO_DISC_INSERTED:
		SSMON_IF_Start();
		mainapp_printf("BD_VIDEO_DISC_INSERTED received.\n");
		pstEventInfo->dAppEvent = dMessage;
		pstEventInfo->eDiscState = DISC_LOAD_DOWN;
		_MainApp_DaemonEventBDDiscInserted(dParam, pstEventInfo);
		break;
	case DMN_EVENT_AVCHD_DISC_INSERTED:
		SSMON_IF_Start();
		mainapp_printf("AVCHD_DISC_INSERTED received.\n");
		pstEventInfo->dAppEvent = dMessage;
		pstEventInfo->eDiscState = DISC_LOAD_DOWN;
		_MainApp_DaemonEventAVChdDiscInserted(dParam, pstEventInfo);
		break;
#ifdef KOK_SUPPORT
	case DMN_EVENT_KOK_DISC_INSERED:
		mainapp_printf("KOK_DISC_INSERTED received.\n");
		pstEventInfo->dAppEvent = dMessage;
		pstEventInfo->eDiscState = DISC_LOAD_DOWN;
		_MainApp_DaemonEventKokDiscInserted(dParam, pstEventInfo);
		break;
#endif
	case DMN_EVENT_DVD_VIDEO_DISC_INSERTED:
		SSMON_IF_Start();
		mainapp_printf("DVD_VIDEO_DISC_INSERTED received.\n");
		pstEventInfo->dAppEvent = dMessage;
		pstEventInfo->eDiscState = DISC_LOAD_DOWN;
		_MainApp_DaemonEventDvdDiscInserted(dParam, pstEventInfo);
		break;
	case DMN_EVENT_SVCD_DISC_INSERTED:
		SSMON_IF_Start();
		mainapp_printf("SVCD_DISC_INSERTED received.\n");
		pstEventInfo->dAppEvent = dMessage;
		pstEventInfo->eDiscState = DISC_LOAD_DOWN;
		_MainApp_DaemonEventSvcdDiscInserted(dParam, pstEventInfo);
		break;
	case DMN_EVENT_PLUSVR_DISC_INSERTED:
		SSMON_IF_Start();
		mainapp_printf("PLUSVR_DISC_INSERTED received.\n");
		pstEventInfo->dAppEvent = dMessage;
		pstEventInfo->eDiscState = DISC_LOAD_DOWN;
		_MainApp_DaemonEventPlusvrDiscInserted(dParam, pstEventInfo);
		break;
	case DMN_EVENT_CDDA_DISC_INSERTED:
		SSMON_IF_Start();
		mainapp_printf("CDDA_DISC_INSERTED received.\n");
		pstEventInfo->dAppEvent = dMessage;
		pstEventInfo->eDiscState = DISC_LOAD_DOWN;
		_MainApp_DaemonEventCddaDiscInserted(dParam, pstEventInfo);
		break;
	case DMN_EVENT_UNKNOW_DISC_INSERTED:
		#ifdef SUPPORT_ISO_IMAGE
		// close file loader
		if (PSREG_VALUE(PSREG_FILE_LOADER) == PSREG_FILE_LOADER_IDENTIFY_DISC)
		{
			fldr_printf("Fileloader insert UNKNOW_DISC\n");
			fs_fsetinfo(
				pstEventInfo->stSysAppCtrlInfo.stFileAppCtrlInfo.dFileLoaderId,
				FS_INFO_SET_FILE_LOADER_CLOSE_PATH,
				"close",
				5);
			PSREG_SET_VALUE(PSREG_FILE_LOADER, PSREG_FILE_LOADER_RESUME);
			SSMON_IF_Start();
			MSG_FILTER_DispatchMessageEX(UI_EVENT_RETURN, IMAGE_PARAMETER_UNSUPPORT); //lgz 2010-12-8 12:4 add
			break;
		}
		#endif // SUPPORT_ISO_IMAGE

		SSMON_IF_Start();
		mainapp_printf("UNKNOW_DISC_INSERTED received\n");
		pstEventInfo->dAppEvent = dMessage;
		pstEventInfo->eDiscState = DISC_LOAD_DOWN;
		_MainApp_DaemonEventErrorDisc(dParam, pstEventInfo);
		break;
	case DMN_EVENT_NO_DISC_INSERTED:
		#ifdef SUPPORT_ISO_IMAGE
		if (PSREG_VALUE(PSREG_FILE_LOADER) == PSREG_FILE_LOADER_IDENTIFY_DISC)
		{
			fldr_printf("Fileloader insert UNKNOW_DISC\n");
			fs_fsetinfo(
				pstEventInfo->stSysAppCtrlInfo.stFileAppCtrlInfo.dFileLoaderId,
				FS_INFO_SET_FILE_LOADER_CLOSE_PATH,
				"close",
				5);
			PSREG_SET_VALUE(PSREG_FILE_LOADER, PSREG_FILE_LOADER_RESUME);
			SSMON_IF_Start();
			MSG_FILTER_DispatchMessageEX(UI_EVENT_RETURN, IMAGE_PARAMETER_UNSUPPORT); //lgz 2010-12-8 12:4 add
			break;
		}
		#endif // SUPPORT_ISO_IMAGE

		SSMON_IF_Start();
		mainapp_printf("NO_DISC_INSERTED received\n");
		pstEventInfo->dAppEvent = dMessage;
		pstEventInfo->eDiscState = DISC_LOAD_DOWN;
		_MainApp_DaemonEventUnknownDisc(dParam, pstEventInfo);
		break;

	case DMN_EVENT_DI_UNMOUNT_SUCCESS:
		mainapp_printf("DMN_EVENT_EJECT_SUCCESS received\n");
		AppPtn_Umount(gdPhisycalLoaderDiscDevNum);
		break;

	case DMN_EVENT_DI_AUTO_TRAYIN:
		mainapp_printf("DMN_EVENT_DI_AUTO_TRAYIN \n");
		mainapp_printf("Tray In\n");
		ShareGobj_IF_ShowHintMsg(HINT_MSG_CLOSE, HINT_MSG_TIMEOUT, MSG_PARAM_RESET_BG_MSG);
		PSREG_SET_VALUE(PSREG_STAT_MainAppStatus, PSREG_MAINAPPSTATUS_TRAYIN);
		pstEventInfo->eTrayState = PHYSICAL_TRAY_IN;
		break;
	case DMN_EVENT_DI_AUTO_TRAYOUT:
		mainapp_printf("DMN_EVENT_DI_AUTO_TRAYOUT \n");
		ShareGobj_IF_ShowHintMsg(HINT_MSG_OPEN, HINT_MSG_TIMEOUT, MSG_PARAM_RESET_BG_MSG);
		PSREG_SET_VALUE(PSREG_STAT_MainAppStatus, PSREG_MAINAPPSTATUS_TRAYOUT);
		pstEventInfo->eTrayState = PHYSICAL_TRAY_OUT;
		break;

	case DMN_EVENT_DI_DISCEJECT_SUCCESS:
		SSMON_IF_Start();
		mainapp_printf("DMN_EVENT_DI_DISCEJECT_SUCCESS \n");
		#if 0//do not show open icon here,or it will show it two times,swjiang,2010.12.21
		ShareGobj_IF_ShowHintMsg(HINT_MSG_DISC_LOADING, HINT_MSG_TIMEOUT_INFINITE, MSG_PARAM_CLEAR_HINT_MSG);
		#ifdef SUPPORT_ISO_IMAGE
		if ((PSREG_VALUE(PSREG_FILE_LOADER) != PSREG_FILE_LOADER_RESUME_CLOSE)
			&& (PSREG_VALUE(PSREG_FILE_LOADER) != PSREG_FILE_LOADER_RESUME))
		#endif	
		ShareGobj_IF_ShowHintMsg(HINT_MSG_OPEN, HINT_MSG_TIMEOUT, MSG_PARAM_RESET_BG_MSG);
		#endif
		PSREG_SET_VALUE(PSREG_STAT_MainAppStatus, PSREG_MAINAPPSTATUS_TRAYOUT);
		pstEventInfo->eTrayState = PHYSICAL_TRAY_OUT;

		if (ISPStartToUpgrade == TRUE)
		{
			ISPStartToUpgrade = FALSE;
			ISP_Start();
		}
		break;

	case DMN_EVENT_CARD_DEV_ATTACHED:
	case DMN_EVENT_USB_HDD_ATTACHED:
	case DMN_EVENT_IDE_HDD_ATTACHED:
		mainapp_printf("Storage attached received.\n");
		pstEventInfo->dAppEvent = dMessage;
		#ifdef SUPPORT_PLAYER_STB_DVB_T
		if ((MID_PartitionList_AddItem(dParam)) < 0)
		{
			break;
		}
                #endif
		_MainApp_DaemonEventStorageAttached(dParam, pstEventInfo);
		break;

	case DMN_EVENT_IDE_HDD_DETACHED:
	case DMN_EVENT_CARD_DEV_DETACHED:
	case DMN_EVENT_USB_HDD_DETACHED:
		SSMON_IF_Start();
		mainapp_printf("Storage detached received.\n");
		pstEventInfo->dAppEvent = dMessage;
		pstEventInfo->dDetachDevIdx = dParam;
		#ifdef SUPPORT_PLAYER_STB_DVB_T
		//Start:wurh add for mantis:0119127.Solve the DVB-T state show screen saver issue.
		MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
		if(dSysAppIdx == SYS_APP_DVB)
		{
			SSMON_IF_Stop();
		}
		//End:wurh add for mantis:0119127.Solve the DVB-T state show screen saver issue.
		if ((MID_PartitionList_RemoveItem(dParam)) < 0)
		{
			break;
		}
                #endif
		 _MainApp_DaemonEventStorageDetached(pstEventInfo);
	#ifdef SUPPORT_PLAYER_STB_DVB_T//lr.wu added 11-03-02 force send it to dvb 
		MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
		MAINAPP_SendEvent4DvbAP(dSysAppIdx, dMessage | PASS_TO_SYSAPP, dParam);
	#endif
		break;

	case DMN_EVENT_IDE_LOADER_ATTACHED:
		mainapp_printf("DMN_EVENT_IDE_LOADER_ATTACHED received.\n");
		gMessHandle = gMsgHandle;
		gdPhisycalLoaderDiscDevNum = dParam;
		mainapp_printf("dDiscDevNum:%u\n",dParam);
		AppPtn_DiscLoaderAttach(dParam, DI_Handle_PHYSICAL);
		DI_Open(gMsgHandle, dParam,&gPhisycalLoaderDIHandle);
		PtnMng_Mount(dParam, FILE_SYSTEM_UNKNOW, DEVICE_TYPE_DISC);//add loader info to partition manager table 2010.08.27 added by sheng
		pstEventInfo->DIHandle[DI_Handle_PHYSICAL] = gPhisycalLoaderDIHandle;
		break;

	case DMN_EVENT_IDE_LOADER_DETACHED:
		mainapp_printf("DMN_EVENT_IDE_LOADER_DETACHED received.\n");
		break;

	case DMN_EVENT_USB_LOADER_ATTACHED:
		MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
		_MAINAPP_SendEvent4NetworkAPs(dSysAppIdx, dMessage, dParam);
		break;
	case DMN_EVENT_USB_LOADER_DETACHED:
		MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
		_MAINAPP_SendEvent4NetworkAPs(dSysAppIdx, dMessage, dParam);
		break;
	case DMN_EVENT_SERVO_ATTACHED:
		mainapp_printf("DMN_EVENT_SERVO_ATTACHED received.\n");
		gMessHandle = gMsgHandle;
		gdPhisycalLoaderDiscDevNum = dParam;
		mainapp_printf("dDiscDevNum:%u\n",dParam);
		AppPtn_DiscLoaderAttach(dParam, DI_Handle_PHYSICAL);
		fs_autodetect_set_servo_devicemount_name(dParam);
		DI_Open(gMsgHandle, dParam,&gPhisycalLoaderDIHandle);
		pstEventInfo->DIHandle[DI_Handle_PHYSICAL] = gPhisycalLoaderDIHandle;
		//Service IOP Standby-2-On Commands
		DRV_IopStandbyOnRequest_t stPwrOnTable = {0};
		if(DRV_IOPStandby_POTGet(&stPwrOnTable) == DRV_SUCCESS)
			if(stPwrOnTable.eCmd == IOP_STBY_ON_CMD_Eject)
				DI_TrayOut(gPhisycalLoaderDIHandle);
		break;

	case DMN_EVENT_SERVO_DETACHED:
		mainapp_printf("DMN_EVENT_SERVO_DETACHED received.\n");
		break;
#ifdef SUPPORT_ISO_IMAGE
	case DMN_EVENT_FILE_LOADER_ATTACHED:
		fldr_printf("DMN_EVENT_FILE_LOADER_ATTACHED received. dDiscDevNum %d\n",dParam);
		gMessHandle = gMsgHandle;
		gdFileLoaderDiscDevNum = dParam;
		AppPtn_DiscLoaderAttach(dParam, DI_Handle_FILE_LOADER);
		DI_Open(gMsgHandle, dParam, &gFileLoaderDIHandle);
		PtnMng_Mount(dParam, FILE_SYSTEM_UNKNOW, DEVICE_TYPE_DISC);//add loader info to partition manager table 2010.08.27 added by sheng
		pstEventInfo->DIHandle[DI_Handle_FILE_LOADER] = gFileLoaderDIHandle;
		PSREG_SET_VALUE(PSREG_FILE_LOADER, PSREG_FILE_LOADER_IDENTIFY_DISC);
		break;

	case DMN_EVENT_FILE_LOADER_DETACHED:
		fldr_printf("DMN_EVENT_FILE_LOADER_DETACHED received.\n");
		if (DI_Close(gFileLoaderDIHandle) != DIMON_IF_SUCCESSFUL)
			fldr_printf("DI_Close fail\n");

		if (PSREG_VALUE(PSREG_FILE_LOADER) == PSREG_FILE_LOADER_RESUME)
			PSREG_SET_VALUE(PSREG_FILE_LOADER, PSREG_FILE_LOADER_RESUME_CLOSE);
		else
			PSREG_SET_VALUE(PSREG_FILE_LOADER, PSREG_FILE_LOADER_CLOSE);
		break;
#if defined(NET_WIFI_SUPPORT)
	case DMN_EVENT_WIFI_DONGLE_ATTACHED:
		pstEventInfo->stComGobj.stSetupGobj.bWifiDongleAtt=1;
		_MainApp_SetupRelatedMenu2MainMenu(pstEventInfo, MAINAPP_SETUPMENU); 
		_MainApp_SetupRelatedMenu2MainMenu(pstEventInfo, MAINAPP_SETUPMENUNETWORK); 
		break;
	case DMN_EVENT_WIFI_DONGLE_DETACHED:
		pstEventInfo->stComGobj.stSetupGobj.bWifiDongleAtt=0;
		_MainApp_SetupRelatedMenu2MainMenu(pstEventInfo, MAINAPP_SETUPMENU); 
		_MainApp_SetupRelatedMenu2MainMenu(pstEventInfo, MAINAPP_SETUPMENUNETWORK); 
		break;
#endif
#endif // SUPPORT_ISO_IMAGE
	case DMN_EVENT_NAND_ATTACHED:
		diag_printf("nand flash attached\n");
		if (mount("/dev/nda/", "/nanda0", "fatfs") != 0) {
			diag_printf("[%s] Line %d: start format nand to FAT\n", __FUNCTION__, __LINE__);
			if (fs_format("/dev/nda/0", "fatfs") != 0) {
				diag_printf("format fail\n");
				return;
			}
			diag_printf("[%s] Line %d: end format nand to FAT\n", __FUNCTION__, __LINE__);

			if (mount("/dev/nda/", "/nanda0", "fatfs") != 0) {
				diag_printf("format fail\n");
			} else {
				diag_printf("format success and nand device is mounted as FAT!\n");
			}
		} else {
			diag_printf("nand device is mounted as FAT!\n");
		}

		break;
	case DMN_EVENT_NAND_DETACHED:
		break;
#ifdef NET_SUPPORT
	case DMN_EVENT_NET_LINKER_ATTACHED:
	case DMN_EVENT_NET_INTI_DONE://DMN_EVENT_NET_LINKER_ATTACHED:
		mainapp_printf("DMN_EVENT_NET_LINKER_ATTACHED received.\n");
		pstEventInfo->stNetDriveCtrlInfo.eState = NET_DRIVE_STATE_BOOTUP;
	#ifdef SAMBA_SUPPORT
		AppPtn_AddSMBPtn();
	#endif
		MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
		_MAINAPP_SendEvent4NetworkAPs(dSysAppIdx, dMessage, dParam);
		break;
	case DMN_EVENT_NET_LINKER_DETACHED:
		mainapp_printf("DMN_EVENT_NET_LINKER_DETACHED received.\n");
		pstEventInfo->stNetDriveCtrlInfo.eState = NET_DRIVE_STATE_SHUT_DOWN;
//		pstEventInfo->stSysAppCtrlInfo.stSmartUpgradeInfo.stFirst  = ISP_IF_ACTIVE_SYS; //2010/8/16 zjh modify for isp
	#ifdef SAMBA_SUPPORT
		_MainApp_DaemonEventNetLinkerDetached(pstEventInfo);
	#endif
		MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
		_MAINAPP_SendEvent4NetworkAPs(dSysAppIdx, dMessage, dParam);
		break;
#endif
	case DMN_EVENT_SCREEN_SAVER_ON:
		mainapp_printf("DMN_EVENT_SCREEN_SAVER_ON received.\n");
		pstEventInfo->dAppEvent = dMessage;
		_MainApp_DaemonEventScrnSaverOn(dParam, pstEventInfo);
		break;
	case DMN_EVENT_SCREEN_SAVER_OFF:
		mainapp_printf("[MainApp] DMN_EVENT_SCREEN_SAVER_OFF received.\n");
		pstEventInfo->dAppEvent = dMessage;
		_MainApp_DaemonEventScrnSaverOff(dParam, pstEventInfo);
		break;

	case DMN_EVENT_APP_TIMER:
#ifdef SUPPORT_PLAYER_STB_DVB_T
		mainapp_ResendEventInTransition();
		Dvb_SysTime_Timeout();
#endif
		break;
#ifdef SUPPORT_PLAYER_STB_DVB_T
	case DMN_EVENT_WKTIMER_TRIGGERED:
		{
			UINT32 dWKTimerMsg, dWKTimerParam,dActiveSysApp;
			int sdEvent;
			UINT8 bRet = TRUE;
			_MainAPP_WKHMToDVBT();
			bRet = DVB_WkTimer_Handler(dParam, &dWKTimerMsg, &dWKTimerParam, &sdEvent);
			if (dWKTimerMsg == 0)
		break;
			if(bRet)
			{
				if(MAINAPP_ActivateSystemApp(SYS_APP_DVB, 0) == MAIN_APP_TERMINATE_CURRENT_APP_FIRST)
					MAINAPP_SwitchSystemApp(SYS_APP_DVB, 0);
			}
			switch (MAINAPP_GetActiveSystemAppIndex(&dActiveSysApp))
			{
				case MAIN_APP_SUCCESS:
					diag_printf("%s,%d\n",__FUNCTION__,__LINE__);
					SYSAPP_IF_SendGlobalEventWithIndex(dActiveSysApp, dWKTimerMsg | PASS_TO_SYSAPP, dWKTimerParam);
					diag_printf("1dMessage is %d\n",dWKTimerMsg);
					if (sdEvent >= 0)
						SYSAPP_IF_SendGlobalEventWithIndex(dActiveSysApp, sdEvent, dWKTimerParam);
		break;
				case MAIN_APP_IN_TRANSITION:
					mainapp_printf("[MainApp] In Transition State, wait and retry.\n");
					mainapp_SetEventInTransition(dWKTimerMsg, dWKTimerParam);
		break;
				default:
					break;
			}
		}
		break;
#endif
	case DMN_EVENT_AUTO_POWER_OFF:
		{
			#ifdef SUPPORT_PLAYER_STB_DVB_T
			MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
			if(dSysAppIdx == SYS_APP_DVB)
			{
				MSG_FILTER_DispatchMessageEX(DMN_EVENT_AUTOSTANDBY_UI, 0);
			}
			else
			#endif
			{
				MAINAPP_SendGlobalEvent(UI_EVENT_POWER, 0);
			}
		}
		break;
	case DMN_EVENT_HDMI_EDID_VALID:
		mainapp_printf("[MainApp] DMN_EVENT_HDMI_EDID_VALID received.\n");
		pstEventInfo->dAppEvent = dMessage;
		_MainApp_DaemonEventHdmiEdidValid(dParam, pstEventInfo);
		break;

	case DMN_EVENT_SET_LANGUAGE: //for CEC set menu language
		mainapp_printf("[MainApp] DMN_EVENT_SET_LANGUAGE received.\n");
		pstEventInfo->dAppEvent = dMessage;
		_MainApp_DaemonEventSetLang(dParam, pstEventInfo);
		break;

	case DMN_EVENT_HDMI_EDID_INVALID:
		mainapp_printf("[MainApp] DMN_EVENT_HDMI_EDID_INVALID received.\n");
		MainApp_SetupAutoSettings(pstEventInfo);
		break;

	case DMN_EVENT_CONTENT_FRAME_RATE_CHG:
	case DMN_EVENT_CONTENT_XVCOLOR_SUPPORT_CHG:
		mainapp_printf("[MainApp] DMN_EVENT_CONTENT_.._CHG received.\n");
		MainApp_SetupAutoSettings(pstEventInfo);
		break;

	case DMN_EVENT_MAINAPP_STARTS:
		mainapp_printf("[MainApp] DMN_EVENT_MAINAPP_STARTS received.\n");
#if (IC_VER == QAE377)
		HWInit8600();
#endif
		_MainApp_APInit();
		break;

	case DMN_EVENT_SCREEN_SAVER_UPDATE:
		break;
	#if (SUPPORT_iPOD_DOCKING==1)
	case DMN_EVENT_GPIO_IPOD_IN:
		mainapp_printf("[MainApp] DMN_EVENT_GPIO_IPOD_IN received.\n");
		 pstEventInfo->bIpodStatus=IPOD_IN;
		break;
	case DMN_EVENT_GPIO_IPOD_OUT:
		mainapp_printf("[MainApp] DMN_EVENT_GPIO_IPOD_OUT received.\n");	
		pstEventInfo->bIpodStatus=IPOD_OUT;
		MAINAPP_GetActiveSystemAppIndex(&dSysAppIdx);
		if(dSysAppIdx==SYS_APP_IPOD_PLAYER)
		{
			INT32 sdRet = MAIN_APP_SUCCESS;
			sdRet = MAINAPP_TerminateSystemApp(dSysAppIdx);
			if (sdRet == SYSTEM_APP_SUCCESS)
				SysAppTerminateAckCallBack = _MainApp_UIEventOnSetup;
			else
				mainapp_error(" Terminate System App[%d]\n", sdRet);	
		}	
		break;
	#endif
	default:
		mainapp_printf("Un-handled Daemon Eventid: 0x%x.\n", dMessage);
		break;
	}

	return;
}

void MAINAPP_AppEventHandler(UINT32 dMessage, UINT32 dParam)
{
	MainAppUIEventHandlerInfo_t *pstEventInfo = &stUIEventHandlerInfo;
	switch (dMessage)
	{
	case SYSTEM_APP_ACTIVATE_ACK:
		if (SysAppActiveAckCallBack)
		{
			SysAppActiveAckCallBack(pstEventInfo);
			SysAppActiveAckCallBack = NULL;
		}
		break;
	case SYSTEM_APP_TRANSITION_FAILED:
		if (SysAppActiveFailCallBack)
		{
			SysAppActiveFailCallBack(pstEventInfo);
			SysAppActiveFailCallBack = NULL;
		}
		break;
	case SYSTEM_APP_TERMINATE_ACK:
		app_set_DisplayMixerMode(MIXER_UI_720P);
		if (SysAppTerminateAckCallBack)
		{
			SysAppTerminateAckCallBack(pstEventInfo);
			SysAppTerminateAckCallBack = NULL;
		}
		//Check If it's Ready for FinalizePlatform
		StandByMode eMode = MAINAPP_AWAKE;
		MAINAPP_IsEnteringStandby(&eMode);
		if (eMode == MAINAPP_STANDBY)
			_MainApp_FinalizeDone();
		break;

	case SYSTEM_APP_TRANSITION_TIMEOUT:
		break;
	default:
		break;
	}
}

static void MAINAPP_SetFilterTable(void)
{
	int dRet = 0;
	//VIDEO
	dRet |= FMW_Set_FilterType(MW_FILTER_VIDEO,
							   FILTER_VIDEO_MPG | FILTER_VIDEO_AVI | FILTER_VIDEO_VOB |
								FILTER_VIDEO_DAT | FILTER_VIDEO_ASF | FILTER_VIDEO_MKV |
								FILTER_VIDEO_RM | FILTER_VIDEO_WMV | FILTER_VIDEO_TSF |
								FILTER_VIDEO_QT | FILTER_VIDEO_FLASH
								#ifdef SUPPORT_ISO_IMAGE
								| FILTER_VIDEO_ISO
								#endif
								);
	//AUDIO
	dRet |= FMW_Set_FilterType(MW_FILTER_AUDIO,
								FILTER_AUDIO_MP3 | FILTER_AUDIO_DTS | FILTER_AUDIO_WMA |
								FILTER_AUDIO_WAV | FILTER_AUDIO_RA | FILTER_AUDIO_M4A |
								FILTER_AUDIO_RA | FILTER_AUDIO_AC3 | FILTER_AUDIO_AAC | FILTER_AUDIO_FLAC| FILTER_AUDIO_VORBIS|FILTER_AUDIO_MKA|FILTER_AUDIO_TSF);
	//PHOTO
	dRet |= FMW_Set_FilterType(MW_FILTER_PHOTO,
								FILTER_PHOTO_JPG | FILTER_PHOTO_BMP | FILTER_PHOTO_TIF |
								FILTER_PHOTO_GIF | FILTER_PHOTO_PNG);
	//TEXT
	dRet |= FMW_Set_FilterType(MW_FILTER_TEXT, FILTER_EBOOK_TXT);
	//GAME NATIVE32
	#ifdef GAME_SUPPORT
	dRet |= FMW_Set_FilterType(MW_FILTER_NATIVE32, FILTER_NATIVE32);
	//GAME NES
	dRet |= FMW_Set_FilterType(MW_FILTER_NES, FILTER_NES);
	//GAME MD
	dRet |= FMW_Set_FilterType(MW_FILTER_MD, FILTER_MD_SMD | FILTER_MD_GEN  | FILTER_MD_BIN);
	#endif
//ROM BIN
	//dRet |= FMW_Set_FilterType(MW_FILTER_BIN, FILTER_BIN);
}

#if defined(SUPPORT_BDJ) || defined(SUPPORT_LINUX)
//sjhuang add here to boot linux
int MAINAPP_BootLinux(PtnMng_CallbackType_e ePtnMngCallbackEventType, UINT32 dparam)
{
	MwDriveName_t pstMntName;
	UINT32 dDevType=0, dDevfsType;
	#if 0 //modify by george.chang 2011.11.14 for load BDMV forder from USB
	UINT32 dDevType, dDevfsType;
	PtnMng_GetMountInfoByDevIndex(dparam, &pstMntName, &dDevType, &dDevfsType);

	if (PTNMNG_CALLBACK_MOUNT_DONE == ePtnMngCallbackEventType)
	{
		diag_printf("in  %s dDevType=%d \n",__FUNCTION__,dDevType );
		if (DEVICE_TYPE_USB == dDevType)
		{
			extern void linux_boot_main(void);

			linux_boot_main();
		}
	}
	#endif

	if (PTNMNG_CALLBACK_MOUNT_DONE == ePtnMngCallbackEventType)
	{
		if (DEVICE_TYPE_USB == dDevType)
		{
			PtnMng_GetMountInfoByDevIndex(dparam, &pstMntName, &dDevType, &dDevfsType);
			diag_printf("in  %s dDevType=%d \n",__FUNCTION__,dDevType );

			extern void linux_boot_main(void);

			linux_boot_main();


		}
	}
	#if (SUPPORT_BDMV_USB_LOAD==1)
	PtnCallbackFunc(ePtnMngCallbackEventType,dparam);
	#endif
	return 0;
}
#endif

int MAINAPP_InitSysApp(UINT32 * dTotalApp, int dHandle)
{
	SYSAPP_IF_Initialize(TOTAL_SYS_APP_SIZE, apstSysAppInst);
	*dTotalApp = TOTAL_SYS_APP_SIZE;
	#ifdef SUPPORT_PLAYER_STB_DVB_T
	Sys_Timer_Section_Init();
	DVB_SysTime_Init();
	DVB_WkTimer_Initialize();
	WKTIMER_IF_Initialize();
	WKTIMER_IF_Start();
        #endif
	//Initialize system app 1 by 1.
	UINT8 i;
	for (i = 0; i < TOTAL_SYS_APP_SIZE; i++)
		SYSAPP_IF_InitializeSysApp(i, MainAppQueueHandle);
	//Before Initial partition manager, it must init filter table for media type first
	//then set the filter table for media type
	if (PTNMNG_SUCCESSFUL != FMW_FilterTable_Init())
		mainapp_printf("FMW_FilterTable_Init error\n");
	//set the filter table
	MAINAPP_SetFilterTable();
	
	//initialize setup config.
	SetupConfig_IF_Initialize();

	//initialize screen saver daemon
#if (SCREEN_SAVER_ENABLE == 1)
	SSMON_IF_Initialize(dSSCounterLimit, dSSUpdatePeriod, pSSInvalieKey, (sizeof(pSSInvalieKey) / sizeof(UINT32)));
	SSMON_IF_Start();
#endif

#if defined(IR_BOT_RANDOM_K)||defined(IR_BOT_SEQ_K)
	//initialize Ir Robot
	AUTO_IF_Initialize(dHandle, AUTO_MDL_MODE);
	AUTO_IF_Start();
#endif
//hong.zhang 1119 added for animation
	ANIM_IF_Initialize();

	
	return MAIN_APP_SUCCESS;
}

int MAINAPP_FinalizePlatform(void)
{
	/* //Move to _MainApp_Finalizing
	//------------------------------------------------------------ (1)
	//Disable Output Signal first. (Video DAC, Audio DAC, HDMI)
	HDMI_IF_Set_TMDSOff();
	AUDIF_PowerDown_DAC();
	//Note: DAC cannot be controlled, if AVPD SEL_ON.
	SetAvpdEnable(SEL_OFF);
	SetDacDisable(DAC_DISABLE,DAC_DISABLE,DAC_DISABLE, DAC_DISABLE);
	//Un-do CEC, and send <inactive source> command.
	DRV_CEC_Finalize();
	//???
	DEVCBK_Finalize();
	*/
	//------------------------------------------------------------ (2)
	//Note: It is important to terminate system app first (After disable all output).
	//SYS_APP Finalized
	UINT8 i;
	for (i = 0; i < TOTAL_SYS_APP_SIZE; i++)
		MAINAPP_FinalizeSystemApp(i);
	//Note: It is used by SystemApp.
	SYSAPP_TIMER_ReleaseTimer();
	//Note: It is used by SystemApp.
	SetupConfig_IF_Finalize();
	//DI Daemon in standby mode. (Servo Related)
	MainAppUIEventHandlerInfo_t *pstEventInfo = &stUIEventHandlerInfo;
	for(i=0; i<DI_Handle_TYPE_MAX; i++)
		DI_TrayIn(pstEventInfo->DIHandle[i]);
	for(i=0; i<DI_Handle_TYPE_MAX; i++)
		DI_Close(pstEventInfo->DIHandle[i]);
	DI_Finalize();
	//Note: GUI Object should be removed from ANIM's list.
	GOBJ_MGR_DestroyAllObject(pMainAppInstance->GUIObjData);
	//ANIM Daemon Finalized
	ANIM_IF_Finalize();
	//Screen Saver Finalized
	SSMON_IF_Finalize();
#ifdef SUPPORT_RESUME
	//Note: RSM daemon is controlled by MAIN_APP and SYS_APP
	Rsm_IF_Finalize();
#endif
#ifdef PLFNV_SUPPORT_DIVX_DRM
	//???
	DivXDRM_Finalize();
#endif

	//GPIO DeTECT
	GPIO_IF_Finalize();   //yanyuan 2010-09-09 

#ifdef SUPPORT_VFD_DRIVER
	//VFD Finalized
	PSREG_SET_VALUE(PSREG_STAT_MainAppStatus, PSREG_MAINAPPSTATUS_VFD_END);
	VFD_Finalize();
#endif
#if (SUPPORT_iPOD_DOCKING==1)
   // ipod_detect_Finalize();
#endif
	//PTN Finalized
	if (APPPTN_SUCCESSFUL != AppPtn_Finalize())
		mainapp_printf("AppPtn_Finalize error\n");
	if (PTNMNG_SUCCESSFUL != FMW_FilterTable_Finalize())
		mainapp_printf("FMW_FilterTable_Finalize error\n");
#if defined(SUPPORT_BDJ) || defined(SUPPORT_LINUX)
	//finalize ptnmng
	if(PtnMng_Finalize()!=0)
		diag_printf("PtnMng_Initialize error\n");
#endif
#ifdef NET_SUPPORT
	if(APP_NET_Finalize() != APP_NET_IF_SUCCESSFUL)
		mainapp_error("fialize app net\n");
	//Network Daemon Finalized
	if(NET_IF_Finalize() != NET_DAEMON_IF_SUCCESSFUL)
		mainapp_error("fialize net daemon\n");
#endif

	// Vector font uninit
	int sdFontUnInit = VF_FontUninit();
	if( sdFontUnInit != FONT_SUCCESS )
		mainapp_printf("[VFONT] VF_FontInit() failed = 0x%x!\n", sdFontUnInit);
	
	//GUI engine uninit.
	int sdGengUnInit = GEL_Close();
	if(sdGengUnInit)
		mainapp_printf("[GUI ENG]GEL_Close() failed = 0x%x\n", sdGengUnInit);

	//Note: MSG_FILTER is controlled by MAIN_APP, SYS_APP, and Screen Saver
	MSG_FILTER_Finalize();
	//------------------------------------------------------------ (3)
	//Initialize Standby IR
	extern const UINT32 g_u32CustomCode;
	extern const DRV_IRStandbySupCmd_t g_u8IrStandbyTable[];
	extern UINT8 IrStandby_Get_CmdCount(void);
	ir_Standby_Initialize((UINT8 *)g_u8IrStandbyTable, IrStandby_Get_CmdCount(), g_u32CustomCode);
	//Init VFD/CEC/etc
	cec_Standby_Initialize();

	//set gpio before poweroff
	set_gpio_before_poweroff();

	//Un-Init Normal IOP
	DRV_IOPUninit();
	//Initialize Standby IOP
	DRV_IopStandbyCfg_t stIopStandbyCfg = {0};
	stIopStandbyCfg.pbIopCode = IOP_CODE_CHOOSED;
	stIopStandbyCfg.pfSetupIrIop	= ir_Standby_Get_SetupIopFunct();
	#ifdef SUPPORT_VFD_DRIVER
	stIopStandbyCfg.pfSetupVfdIop	= vfd_Standby_Get_SetupIopFunct();
	#else
	stIopStandbyCfg.pfSetupVfdIop	= NULL;
	#endif
	stIopStandbyCfg.pfSetupCecIop	= cec_Standby_Get_SetupIopFunct();
	DRV_IOPStandby_Initialize(&stIopStandbyCfg);
#ifdef SUPPORT_PLAYER_STB_DVB_T
	#ifdef SUPPORT_SPDC2XX_POWER_DOWN
		allcoate_serial_dvbt_Tuner_Power(FALSE);
	#endif
		WKTIMER_IF_Finalize();
		DVB_WkTimer_Finalize();
		DVB_SysTime_UInit();
		Sys_Timer_Section_UInit();
#endif

	return MAIN_APP_SUCCESS;
}
