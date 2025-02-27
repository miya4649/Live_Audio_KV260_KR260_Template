/*******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdpdma_video_example.c
*
*
* This file contains a design example using the DPDMA driver (XDpDma)
* This example demonstrates the use of DPDMA for displaying a Graphics Overlay
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.0	aad 10/19/17	Initial Release
* 1.1   aad 02/22/18    Fixed the header
*</pre>
*
******************************************************************************/

/* 02/01/25 Modified by miya */

/***************************** Include Files *********************************/

#include "xil_exception.h"
#include "xil_printf.h"
#include "xil_cache.h"
#include "xdpdma_video_example.h"

/************************** Constant Definitions *****************************/
#define SAMPLE_FREQ		48000

#ifndef SDT
#define DPPSU_DEVICE_ID		XPAR_PSU_DP_DEVICE_ID
#define AVBUF_DEVICE_ID		XPAR_PSU_DP_DEVICE_ID
#define DPDMA_DEVICE_ID		XPAR_XDPDMA_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_SCUGIC_0_DEVICE_ID
#define DPPSU_INTR_ID		151
#define DPDMA_INTR_ID		154

#define DPPSU_BASEADDR		XPAR_PSU_DP_BASEADDR
#define AVBUF_BASEADDR		XPAR_PSU_DP_BASEADDR
#define DPDMA_BASEADDR		XPAR_PSU_DPDMA_BASEADDR
#else
#define DPPSU_BASEADDR		XPAR_XDPPSU_0_BASEADDR
#define AVBUF_BASEADDR		XPAR_XDPPSU_0_BASEADDR
#define DPDMA_BASEADDR		XPAR_XDPDMA_0_BASEADDR
#define INTC_BASEADDR       XPAR_XSCUGIC_0_BASEADDR
#endif

#define BUFFERSIZE			1280 * 720 * 4		/* HTotal * VTotal * BPP */
#define LINESIZE			1280 * 4			/* HTotal * BPP */
#define STRIDE				LINESIZE			/* The stride value should
													be aligned to 256*/

/************************** Variable Declarations ***************************/
XDpPsu DpPsu;
XAVBuf AVBuf;
XScuGic Intr;
Run_Config RunCfg;


/**************************** Type Definitions *******************************/

/*****************************************************************************/
/**
*
* Main function to call the DPDMA Video example.
*
* @param	None
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
*
******************************************************************************/
int main()
{
	int Status;

	Xil_DCacheDisable();
	Xil_ICacheDisable();

	xil_printf("DPDMA Generic Video Example Test \r\n");
	Status = DpdmaVideoExample(&RunCfg);
	if (Status != XST_SUCCESS) {
			xil_printf("DPDMA Video Example Test Failed\r\n");
			return XST_FAILURE;
	}

	xil_printf("Successfully ran DPDMA Video Example Test\r\n");

    return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* The purpose of this function is to illustrate how to use the XDpDma device
* driver in Graphics overlay mode.
*
* @param	RunCfgPtr is a pointer to the application configuration structure.
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
*****************************************************************************/
int DpdmaVideoExample(Run_Config *RunCfgPtr)

{
	/* Initialize the application configuration */
	InitRunConfig(RunCfgPtr);
	sleep(1);
	DpPsu_Run(RunCfgPtr);
	SetupInterrupts(RunCfgPtr);
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* The purpose of this function is to initialize the application configuration.
*
* @param	RunCfgPtr is a pointer to the application configuration structure.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void InitRunConfig(Run_Config *RunCfgPtr)
{
	/* Initial configuration parameters. */
		RunCfgPtr->DpPsuPtr   = &DpPsu;
		RunCfgPtr->IntrPtr   = &Intr;
		RunCfgPtr->AVBufPtr  = &AVBuf;
		RunCfgPtr->VideoMode = XVIDC_VM_1280x720_60_P;
		RunCfgPtr->Bpc		 = XVIDC_BPC_8;
		RunCfgPtr->ColorEncode			= XDPPSU_CENC_RGB;
		RunCfgPtr->UseMaxCfgCaps		= 1;
		RunCfgPtr->LaneCount			= LANE_COUNT_2;
		RunCfgPtr->LinkRate				= LINK_RATE_270GBPS;
		RunCfgPtr->EnSynchClkMode		= 0;
		RunCfgPtr->UseMaxLaneCount		= 1;
		RunCfgPtr->UseMaxLinkRate		= 0;
}

/*****************************************************************************/
/**
*
* The purpose of this function is to initialize the DP Subsystem (XDpDma,
* XAVBuf, XDpPsu)
*
* @param	RunCfgPtr is a pointer to the application configuration structure.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
int InitDpDmaSubsystem(Run_Config *RunCfgPtr)
{
	u32 Status;
	XDpPsu		*DpPsuPtr = RunCfgPtr->DpPsuPtr;
	XDpPsu_Config	*DpPsuCfgPtr;
	XAVBuf		*AVBufPtr = RunCfgPtr->AVBufPtr;

	/* Initialize DisplayPort driver. */
#ifndef SDT
	DpPsuCfgPtr = XDpPsu_LookupConfig(DPPSU_DEVICE_ID);
#else
	DpPsuCfgPtr = XDpPsu_LookupConfig(DPPSU_BASEADDR);
#endif
	XDpPsu_CfgInitialize(DpPsuPtr, DpPsuCfgPtr, DpPsuCfgPtr->BaseAddr);
	/* Initialize Video Pipeline driver */
#ifndef SDT
	XAVBuf_CfgInitialize(AVBufPtr, DpPsuPtr->Config.BaseAddr, AVBUF_DEVICE_ID);
#else
	XAVBuf_CfgInitialize(AVBufPtr, DpPsuPtr->Config.BaseAddr);
#endif

	/* Initialize the DisplayPort TX core. */
	Status = XDpPsu_InitializeTx(DpPsuPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/* Set the format graphics frame for Video Pipeline*/
	Status = XAVBuf_SetInputLiveVideoFormat(AVBufPtr, RGB_12BPC);
	if (Status != XST_SUCCESS) {
			return XST_FAILURE;
	}
	/* Set the output Video Format */
	XAVBuf_SetOutputVideoFormat(AVBufPtr, RGB_8BPC);

	/* Select the Video/Audio input source */
	XAVBuf_InputVideoSelect(AVBufPtr, XAVBUF_VIDSTREAM1_LIVE, XAVBUF_VIDSTREAM2_NONE);
	XAVBuf_InputAudioSelect(AVBufPtr, XAVBUF_AUDSTREAM1_LIVE, XAVBUF_AUDSTREAM2_NO_AUDIO);

	/* Set the Pixel Clock */
	XDpPsu_MainStreamAttributes *MsaConfig = &DpPsuPtr->MsaConfig;
	RunCfgPtr->PixClkHz = MsaConfig->PixelClockHz;
	XAVBuf_SetPixelClock(RunCfgPtr->PixClkHz);

	/* Start setting the Audio Parameter */
	XDpPsu_WriteReg(DpPsuPtr->Config.BaseAddr, XDPPSU_TX_AUDIO_CONTROL, 0);
	usleep(10000);

	/* Disable Audio Buffers */
	XAVBuf_EnableAudio0Buffers(AVBufPtr, 0);
	XAVBuf_EnableAudio1Buffers(AVBufPtr, 0);

	/* Send the Audio Info Data */
	uint8_t db1, db2, db3, db4;
	int IF_type = 0x84;
	int IF_info_length = 27;
	int IF_version = 0x11;
	int IF_audio_channel_count = 1; // 0:refer to stream header 1:2ch 2:3ch 3:4ch 4:5ch 5:6ch 6:7ch 7:8ch
	int IF_audio_coding_type = 1; // 0:refer to stream header 1:L-PCM
	int IF_sampling_frequency = 3; // 0: refer to stream header 1:32kHz 2:44.1kHz 3:48kHz 4:88.2kHz 5:96kHz 6:176.4kHz 7:192kHz
    if (SAMPLE_FREQ == 44100)
    {
      IF_sampling_frequency = 2;
    }
	int IF_sample_size = 1; // 0:refer to stream header 1:16bit 2:20bit 3:24bit
	int IF_channel_allocation = 0; // 0:1ch-L 2ch-R
	db1 = 0x00;
	db2 = IF_type;
	db3 = IF_info_length & 0xFF;
	db4 = (IF_version << 2) | (IF_info_length >> 8);
	uint32_t IF00 = (db4 << 24) | (db3 << 16) | (db2 << 8) | db1;

	db1 = IF_audio_channel_count | (IF_audio_coding_type << 4);
	db2 = (IF_sampling_frequency << 2) | IF_sample_size;
	db3 = 0;
	db4 = IF_channel_allocation;
	uint32_t IF04 = (db4 << 24) | (db3 << 16) | (db2 << 8) | db1;

	XDpPsu_WriteReg(DpPsuPtr->Config.BaseAddr, XDPPSU_TX_AUDIO_INFO_DATA + 0, IF00);
	XDpPsu_WriteReg(DpPsuPtr->Config.BaseAddr, XDPPSU_TX_AUDIO_INFO_DATA + 4, IF04);

	for (int i = 8; i < 32; i += 4)
	{
		XDpPsu_WriteReg(DpPsuPtr->Config.BaseAddr, XDPPSU_TX_AUDIO_INFO_DATA + i, 0);
	}

	/* Set the Audio Channel (2ch - 1) */
	XDpPsu_WriteReg(DpPsuPtr->Config.BaseAddr, XDPPSU_TX_AUDIO_CHANNELS, 1);

	XDpPsu_WriteReg(DpPsuPtr->Config.BaseAddr, XDPPSU_TX_AUDIO_CONTROL, 1);
	usleep(10000);

	/* Set the Audio Volume */
    // XAVBuf_AudioMixerVolumeControl(AVBufPtr, 255, 255);
    // There is a bug in the Library XAVBuf_AudioMixerVolumeControl(). The register parameter specification is u16, but the function input parameter is u8.
	XAVBuf_WriteReg(AVBufPtr->Config.BaseAddr, XAVBUF_AUD_MIXER_VOLUME_CONTROL, 0x20002000);
	/* Configure Video pipeline for graphics channel */
	XAVBuf_ConfigureGraphicsPipeline(AVBufPtr);
	/* Configure the output video pipeline */
	XAVBuf_ConfigureOutputVideo(AVBufPtr);
	/* Disable the global alpha, since we are using the pixel based alpha */
	XAVBuf_SetBlenderAlpha(AVBufPtr, 0, 0);
	/* Set the clock mode */
	XDpPsu_CfgMsaEnSynchClkMode(DpPsuPtr, RunCfgPtr->EnSynchClkMode);
	/* Set the clock source depending on the use case.
	 * Here for simplicity we are using PS clock as the source*/
	XAVBuf_SetAudioVideoClkSrc(AVBufPtr, XAVBUF_PS_CLK, XAVBUF_PL_CLK);
	/* Issue a soft reset after selecting the input clock sources */
	XAVBuf_SoftReset(AVBufPtr);
	XAVBuf_AudioSoftReset(AVBufPtr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* The purpose of this function is to setup call back functions for the DP
* controller interrupts.
*
* @param	RunCfgPtr is a pointer to the application configuration structure.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void SetupInterrupts(Run_Config *RunCfgPtr)
{
	XDpPsu *DpPsuPtr = RunCfgPtr->DpPsuPtr;
	XScuGic		*IntrPtr = RunCfgPtr->IntrPtr;
	XScuGic_Config	*IntrCfgPtr;
	u32  IntrMask = XDPPSU_INTR_HPD_IRQ_MASK | XDPPSU_INTR_HPD_EVENT_MASK;

	XDpPsu_WriteReg(DpPsuPtr->Config.BaseAddr, XDPPSU_INTR_DIS, 0xFFFFFFFF);
	XDpPsu_WriteReg(DpPsuPtr->Config.BaseAddr, XDPPSU_INTR_MASK, 0xFFFFFFFF);

	XDpPsu_SetHpdEventHandler(DpPsuPtr, DpPsu_IsrHpdEvent, RunCfgPtr);
	XDpPsu_SetHpdPulseHandler(DpPsuPtr, DpPsu_IsrHpdPulse, RunCfgPtr);

#ifndef SDT
	/* Initialize interrupt controller driver. */
	IntrCfgPtr = XScuGic_LookupConfig(INTC_DEVICE_ID);
	XScuGic_CfgInitialize(IntrPtr, IntrCfgPtr, IntrCfgPtr->CpuBaseAddress);

	/* Register ISRs. */
	XScuGic_Connect(IntrPtr, DPPSU_INTR_ID,
			(Xil_InterruptHandler)XDpPsu_HpdInterruptHandler, RunCfgPtr->DpPsuPtr);

	/* Trigger DP interrupts on rising edge. */
	XScuGic_SetPriorityTriggerType(IntrPtr, DPPSU_INTR_ID, 0x0, 0x03);


	/* Initialize exceptions. */
	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
			(Xil_ExceptionHandler)XScuGic_DeviceInterruptHandler,
			INTC_DEVICE_ID);

	/* Enable exceptions for interrupts. */
	Xil_ExceptionEnableMask(XIL_EXCEPTION_IRQ);
	Xil_ExceptionEnable();

	/* Enable DP interrupts. */
	XScuGic_Enable(IntrPtr, DPPSU_INTR_ID);
	XDpPsu_WriteReg(DpPsuPtr->Config.BaseAddr, XDPPSU_INTR_EN, IntrMask);
#else
    XSetupInterruptSystem(RunCfgPtr->DpPsuPtr, &XDpPsu_HpdInterruptHandler, RunCfgPtr->DpPsuPtr->Config.IntrId,
            RunCfgPtr->DpPsuPtr->Config.IntrParent, XINTERRUPT_DEFAULT_PRIORITY);
	XDpPsu_WriteReg(DpPsuPtr->Config.BaseAddr, XDPPSU_INTR_EN, IntrMask);
#endif
}
