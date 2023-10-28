/****************************************************************************
 * @file     turbowriter.h
 * @version  V3.00
 * @brief    NandLoader header file for TurboWriter.
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include <types_common.h>

#define NAND_PAGE_512B		512
#define NAND_PAGE_2KB		2048
#define NAND_PAGE_4KB		4096
#define NAND_PAGE_8KB		8192

/* F/W update information */
typedef struct fw_update_info_t {
	uint16_t imageNo;
	uint16_t imageFlag;
	uint16_t startBlock;
	uint16_t endBlock;
	uint32_t executeAddr;
	uint32_t fileLen;
	char imageName[16];
} FW_UPDATE_INFO_T;

typedef struct fw_nand_image_t {
	uint32_t actionFlag;
	uint32_t fileLength;
	uint32_t imageNo;
	char imageName[16];
	uint32_t imageType;
	uint32_t executeAddr;	// endblock
	uint32_t blockNo;	// startblock
	uint32_t dummy;
} FW_NAND_IMAGE_T;

typedef struct fw_nor_image_t {
	uint32_t actionFlag;
	uint32_t fileLength;
	uint32_t imageNo;
	char imageName[16];
	uint32_t imageType;
	uint32_t executeAddr;
	uint32_t flashOffset;
	uint32_t endAddr;
} FW_NOR_IMAGE_T;

typedef struct fw_normal_t {
	uint32_t actionFlag;
	uint32_t length;
	uint32_t address;
} FW_NORMAL_T;

typedef struct fmi_sm_info_t {
	uint32_t uSectorPerFlash;
	uint32_t uBlockPerFlash;
	uint32_t uPagePerBlock;
	uint32_t uSectorPerBlock;
	uint32_t uLibStartBlock;
	uint32_t nPageSize;
	uint32_t uBadBlockCount;
	BOOL bIsMulticycle;
	BOOL bIsMLCNand;
	BOOL bIsNandECC4;
	BOOL bIsNandECC8;
	BOOL bIsNandECC12;
	BOOL bIsNandECC15;
	BOOL bIsNandECC24;
	BOOL bIsCheckECC;
} FMI_SM_INFO_T;

typedef struct fmi_sd_info_t {
	uint32_t totalSectorN;
	uint32_t uCardType;	// sd2.0, sd1.1, or mmc
	uint32_t uRCA;		// relative card address
	BOOL bIsCardInsert;
} FMI_SD_INFO_T;

typedef struct nvt_pt_rec {	/* partition record */
	uint8_t ucState;	/* Current state of partition */
	uint8_t uStartHead;	/* Beginning of partition head */
	uint8_t ucStartSector;	/* Beginning of partition sector */
	uint8_t ucStartCylinder;	/* Beginning of partition cylinder */
	uint8_t ucPartitionType;	/* Partition type, refer to the subsequent definitions */
	uint8_t ucEndHead;	/* End of partition - head */
	uint8_t ucEndSector;	/* End of partition - sector */
	uint8_t ucEndCylinder;	/* End of partition - cylinder */
	uint32_t uFirstSec;	/* Number of Sectors Between the MBR and the First Sector in the Partition */
	uint32_t uNumOfSecs;	/* Number of Sectors in the Partition */
} NVT_PT_REC_T;

/*****************************/
extern FMI_SD_INFO_T *pSD;
extern FMI_SM_INFO_T *pSM0, *pSM1;

extern BOOL bIsMatch;
extern uint32_t *pUpdateImage;
extern uint8_t *pInfo;
extern uint32_t infoBuf;
extern int8_t nIsSysImage;
extern uint32_t volatile gRxLen;
extern uint8_t *gRxPtr;
extern int8_t gCurType;
extern int32_t gCurBlock, gCurPage;
extern uint32_t NandBlockSize;

/* extern flags */
extern uint8_t g_u8BulkState;
extern BOOL udcOnLine;
extern BOOL g_bIsFirstRead;
extern uint32_t usbDataReady;
extern uint8_t g_u8UsbState;
extern uint8_t g_u8Address;
extern uint8_t g_u8Config;
extern uint8_t g_u8Flag;
extern uint8_t g_au8SenseKey[];
extern uint32_t g_u32Address;
extern uint32_t g_u32Length;
extern uint32_t g_u32OutToggle;	// for Error Detection
extern uint8_t g_u8Size;
extern BOOL g_bCBWInvalid;
extern struct CBW g_sCBW;
extern struct CSW g_sCSW;
extern BOOL volatile bIsSdInit;

/*****************************/
int sicSMpread(int chipSel, unsigned PBA, int page, uint8_t * buff);
int sicSMpwrite(int PBA, int page, uint8_t * buff);
int sicSMblock_erase(int PBA);
int fmiMarkBadBlock(uint32_t block);
int ChangeNandImageType(uint32_t imageNo, uint32_t imageType);
int sicSMchip_erase(uint32_t startBlcok, uint32_t endBlock);
int DelNandImage(uint32_t imageNo);
void fmiInitDevice(void);
int sicSMInit(void);
int nvtSetNandImageInfo(FW_UPDATE_INFO_T * pFW);
int searchImageInfo(FW_UPDATE_INFO_T * pFW, uint32_t * ptr);
int nvtGetNandImageInfo(unsigned int *image);
int CheckBadBlockMark(uint32_t block);
int CheckBadBlockMark_512(uint32_t block);
int fmiInitSDDevice(int);
int SdChipErase(void);
int ChangeSdImageType(uint32_t imageNo, uint32_t imageType);
int DelSdImage(uint32_t imageNo);
int nvtGetSdImageInfo(unsigned int *image);
int fmiSD_Write(uint32_t uSector, uint32_t uBufcnt, uint32_t uSAddr);
int fmiSD_Read(uint32_t uSector, uint32_t uBufcnt, uint32_t uDAddr);
int nvtSetSdImageInfo(FW_UPDATE_INFO_T * pFW);
void nvtSdFormat(uint32_t firstSector, uint32_t totalSector);
/*****************************/
void apuDacOn(uint8_t level);
