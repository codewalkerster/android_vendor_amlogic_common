/******************************************************************************
*
*  Copyright (C) 2009-2012 Broadcom Corporation
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at:
*
*  http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*
******************************************************************************/

/******************************************************************************
*
*  Filename:      bt_vendor_aml.c
*
*  Description:   Amlogic vendor specific library implementation
*
******************************************************************************/

#define LOG_TAG "bt_vendor"

#include <unistd.h>
#include <utils/Log.h>
#include <cutils/properties.h>
#include <string.h>
#include "bt_vendor_aml.h"
#include "upio.h"
#include "userial_vendor.h"
#include <errno.h>

#ifndef BTVND_DBG
#define BTVND_DBG TRUE
#endif

#if (BTVND_DBG == TRUE)
#define BTVNDDBG(param, ...) { ALOGD(param, ## __VA_ARGS__); }
#else
#define BTVNDDBG(param, ...) {}
#endif

#ifndef PROPERTY_VALUE_MAX
#define PROPERTY_VALUE_MAX 92
#endif

/******************************************************************************
**  Externs
******************************************************************************/

void hw_config_start(void);
uint8_t hw_lpm_enable(uint8_t turn_on);
uint32_t hw_lpm_get_idle_timeout(void);
void hw_lpm_set_wake_state(uint8_t wake_assert);
#if (SCO_CFG_INCLUDED == TRUE)
void hw_sco_config(void);
#endif
void vnd_load_conf(const char *p_path);
#if (HW_END_WITH_HCI_RESET == TRUE)
void hw_epilog_process(void);
#endif
extern void ms_delay(uint32_t timeout);

/******************************************************************************
**  Variables
******************************************************************************/

bt_vendor_callbacks_t *bt_vendor_cbacks = NULL;
uint8_t vnd_local_bd_addr[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/******************************************************************************
**  Local type definitions
******************************************************************************/

/******************************************************************************
**  Static Variables
******************************************************************************/
static const char DRIVER_PROP_NAME[] = "vendor.sys.amlbtsdiodriver";

static const tUSERIAL_CFG userial_init_cfg =
{
	(USERIAL_DATABITS_8 | USERIAL_PARITY_NONE | USERIAL_STOPBITS_1),
	USERIAL_BAUD_115200
};



/******************************************************************************
**  Functions
******************************************************************************/

/*****************************************************************************
**
**   BLUETOOTH VENDOR INTERFACE LIBRARY FUNCTIONS
**
*****************************************************************************/
static int init(const bt_vendor_callbacks_t *p_cb, unsigned char *local_bdaddr)
{
	ALOGI("init");

	if (p_cb == NULL)
	{
		ALOGE("init failed with no user callbacks!");
		return -1;
	}

#if (VENDOR_LIB_RUNTIME_TUNING_ENABLED == TRUE)
	ALOGW("*****************************************************************");
	ALOGW("*****************************************************************");
	ALOGW("** Warning - BT Vendor Lib is loaded in debug tuning mode!");
	ALOGW("**");
	ALOGW("** If this is not intentional, rebuild libbt-vendor.so ");
	ALOGW("** with VENDOR_LIB_RUNTIME_TUNING_ENABLED=FALSE and ");
	ALOGW("** check if any run-time tuning parameters needed to be");
	ALOGW("** carried to the build-time configuration accordingly.");
	ALOGW("*****************************************************************");
	ALOGW("*****************************************************************");
#endif

	userial_vendor_init();
	upio_init();

	vnd_load_conf(VENDOR_LIB_CONF_FILE);

	/* store reference to user callbacks */
	bt_vendor_cbacks = (bt_vendor_callbacks_t *)p_cb;

	/* This is handed over from the stack */
	memcpy(vnd_local_bd_addr, local_bdaddr, 6);

	return 0;
}

int sdio_bt_completed()
{
	int retry_cnt = 1;
	int bt_fd = -1;
	int ret = -1;

	while (retry_cnt < 200)
	{
		usleep(20000);
		bt_fd = open("/dev/stpbt", O_RDWR | O_NOCTTY | O_NONBLOCK);
		if (bt_fd >= 0)
			break;
		else
			ALOGE("%s: Can't open stpbt: %s. retry_cnt=%d\n", __FUNCTION__, strerror(errno), retry_cnt);

		retry_cnt++;
	}

	if (bt_fd >= 0)
	{
		ALOGD("%s: open stpbt successfully.[%d]...\n", __FUNCTION__, bt_fd);
		close(bt_fd);
		return bt_fd;
	}
	else
	{
		ALOGE("%s: Can't open stpbt: %s.\n", __FUNCTION__, strerror(errno));
		return -1;
	}

	return ret;
}

int insmod_bt_sdio_driver()
{
	char driver_status[PROPERTY_VALUE_MAX];

	property_get(DRIVER_PROP_NAME, driver_status, "amldriverunkown");
	ALOGD("%s: driver_status = %s ", __FUNCTION__, driver_status);
	if (strcmp("true", driver_status) == 0)
	{
		ALOGW("%s: sdio_bt.ko is already insmod!", __FUNCTION__);
		return 0;
	}
	ALOGD("%s: set vendor.sys.amlbtsdiodriver true\n", __FUNCTION__);
	property_set(DRIVER_PROP_NAME, "true");
	//ms_delay(2000);
	if (sdio_bt_completed() >= 0)
		ALOGD("%s: insmod sdio_bt.ko successfully!", __FUNCTION__);
	else
		ALOGE("%s: insmod sdio_bt.ko failed!!!!!!!!!!!!!", __FUNCTION__);
	return 0;
}

int rmmod_bt_sdio_driver()
{
	ALOGD("%s: set vendor.sys.amlbtsdiodriver false\n", __FUNCTION__);
	property_set(DRIVER_PROP_NAME, "false");
	ms_delay(500);
	/*when disable bt, can't use open btsdio node to communicate with sdio_bt driver,
	 *  because the rmmod sdio_bt driver procedure will prevent libbt to open btsdio node.
	 */
	return 0;
}

/** Requested operations */
static int op(bt_vendor_opcode_t opcode, void *param)
{
	int retval = 0;

	BTVNDDBG("op for %d", opcode);

	switch (opcode)
	{
	case BT_VND_OP_POWER_CTRL:
	{
		int *state = (int *)param;
		if (*state == BT_VND_PWR_OFF)
		{
			ALOGD("=== power off BT ===");
			rmmod_bt_sdio_driver();
			upio_set_bluetooth_power(UPIO_BT_POWER_OFF);
		}
		else if (*state == BT_VND_PWR_ON)
		{
			ALOGD("=== power on BT ===");
			upio_set_bluetooth_power(UPIO_BT_POWER_ON);
			insmod_bt_sdio_driver();
		}
	}
	break;

	case BT_VND_OP_FW_CFG:
	{
		hw_config_start();
	}
	break;

	case BT_VND_OP_SCO_CFG:
	{
#if (SCO_CFG_INCLUDED == TRUE)
		hw_sco_config();
#else
		retval = -1;
#endif
	}
	break;

	case BT_VND_OP_USERIAL_OPEN:
	{
		int (*fd_array)[] = (int (*)[])param;
		int fd, idx;

		fd = userial_vendor_open((tUSERIAL_CFG *)&userial_init_cfg);
		if (fd != -1)
		{
			for (idx = 0; idx < CH_MAX; idx++)
				(*fd_array)[idx] = fd;

			retval = 1;
		}
		/* retval contains numbers of open fd of HCI channels */
	}
	break;

	case BT_VND_OP_USERIAL_CLOSE:
	{
		userial_vendor_close();
	}
	break;

	case BT_VND_OP_GET_LPM_IDLE_TIMEOUT:
	{
		uint32_t *timeout_ms = (uint32_t *)param;
		*timeout_ms = hw_lpm_get_idle_timeout();
	}
	break;

	case BT_VND_OP_LPM_SET_MODE:
	{
		usleep(100000);
		uint8_t *mode = (uint8_t *)param;
		retval = hw_lpm_enable(*mode);
	}
	break;

	case BT_VND_OP_LPM_WAKE_SET_STATE:
	{
		uint8_t *state = (uint8_t *)param;
		uint8_t wake_assert = (*state == BT_VND_LPM_WAKE_ASSERT) ? \
				      TRUE : FALSE;

		hw_lpm_set_wake_state(wake_assert);
	}
	break;

	case BT_VND_OP_SET_AUDIO_STATE:
	{
		retval = hw_set_audio_state((bt_vendor_op_audio_state_t *)param);
	}
	break;

	case BT_VND_OP_EPILOG:
	{
#if (HW_END_WITH_HCI_RESET == FALSE)
		if (bt_vendor_cbacks)
		{
			bt_vendor_cbacks->epilog_cb(BT_VND_OP_RESULT_SUCCESS);
		}
#else
		hw_epilog_process();
#endif
	}
	break;

	case BT_VND_OP_A2DP_OFFLOAD_START:
	case BT_VND_OP_A2DP_OFFLOAD_STOP:
	default:
		break;
	}

	return retval;
}

/** Closes the interface */
static void cleanup(void)
{
	BTVNDDBG("cleanup");

	upio_cleanup();

	bt_vendor_cbacks = NULL;
}

// Entry point of DLib
const bt_vendor_interface_t BLUETOOTH_VENDOR_LIB_INTERFACE = {
	sizeof(bt_vendor_interface_t),
	init,
	op,
	cleanup
};
