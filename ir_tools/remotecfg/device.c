/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "common.h"

int CheckVersion(int fd)
{
	char verBuf[64];

	if (ioctl(fd, REMOTE_IOC_GET_DATA_VERSION, verBuf) < 0) {
		fprintf(stderr, "get driver version error=%s\n",
			strerror(errno));
		return FAIL;
	}
	/*check share data version info*/
	if (0 != strcasecmp(verBuf, SHARE_DATA_VERSION)) {
		fprintf(stderr, "interface version not match:"\
				"user[%s] unequal to kernel[%s]\n",
			SHARE_DATA_VERSION, verBuf);
		return FAIL;
	}
	return SUCC;
}

static int WriteSysFile(const char *dir, const char *fileName, unsigned int val)
{
	int fd;
	int len;
	char fileBuf[MAX_LINE_LEN];
	char wBuf[32];

	snprintf(fileBuf, MAX_LINE_LEN, "%s%s", dir, fileName);
	if ((fd = open(fileBuf, O_RDWR)) < 0) {
		return FAIL;
	}

	len = snprintf(wBuf, 32, "%u", val);
	if (write(fd, (void *)wBuf, len+1) < 0) {
		fprintf(stderr, "write %s: %s\n", fileBuf, strerror(errno));
		close(fd);
		return FAIL;
	}

	close(fd);
	return SUCC;
}

int SetCfgPara(int devFd, const char *sysDir, S_CFG_FILE_T *cfgFile)
{
	if (WriteSysFile(sysDir, "protocol", cfgFile->workMode))
		return FAIL;
//	WriteSysFile(sysDir, "repeat_enable", cfgFile->repeatEnable);
	if (WriteSysFile(sysDir, "debug_enable", cfgFile->debugEnable))
		return FAIL;

	if (ioctl(devFd, REMOTE_IOC_SET_SW_DECODE_PARA,
		&cfgFile->sw_data) < 0) {
		fprintf(stderr, "failed to set software decode paras: %s\n",
			strerror(errno));
		return FAIL;
	}

	return SUCC;
}

int SetTabPara(int devFd, S_TAB_FILE_T *tabFile)
{
	if (ioctl(devFd, REMOTE_IOC_SET_KEY_NUMBER,
		&tabFile->tab.map_size) < 0 ) {
		fprintf(stderr, "failed to set key number: %s\n",
			strerror(errno));
		return FAIL;
	}

	if (ioctl(devFd, REMOTE_IOC_SET_KEY_MAPPING_TAB,
		&tabFile->tab) < 0 ) {
		fprintf(stderr, "failed to set map table: %s\n",
			strerror(errno));
		return FAIL;
	}

	if (tabFile->wakeup_size <= 0)
		return SUCC;

	if (ioctl(devFd, REMOTE_IOC_SET_WAKEUP_NUMBER,
		  &tabFile->wakeup_size) < 0 ) {
		fprintf(stderr, "failed to set wakeup table: %s\n",
			strerror(errno));
		return FAIL;
	}

	if (ioctl(devFd, REMOTE_IOC_SET_WAKEUP_TAB,
		  &tabFile->wakeupTab) < 0 ) {
		fprintf(stderr, "failed to set wakeup table: %s\n",
			strerror(errno));
		return FAIL;
	}

	return SUCC;
}

int GetTabNum(int devFd, unsigned int *tabList)
{
	tabList[0] = 0;

	if (ioctl(devFd, REMOTE_IOC_GET_KEY_MAPPING_TAB, tabList) < 0) {
		fprintf(stderr, "failed to get tab num: %s\n",
			strerror(errno));
		return FAIL;
	}

	return SUCC;
}

int GetTabPara(int devFd, S_TAB_FILE_T *tabFile, unsigned int code)
{
	unsigned int *tmp = (unsigned int *)&tabFile->tab;

	tmp[0] = code;

	if (ioctl(devFd, REMOTE_IOC_GET_KEY_MAPPING_TAB, &tabFile->tab) < 0) {
		fprintf(stderr, "failed to get tab num: %s\n",
			strerror(errno));
		return FAIL;
	}

	return SUCC;
}

int GetWakeupPara(int devFd, struct ir_wakeup_tab *wakeupTab)
{
	if (ioctl(devFd, REMOTE_IOC_GET_WAKEUP_TAB, wakeupTab) < 0) {
		fprintf(stderr, "failed to get wakeup tab: %s\n",
			strerror(errno));
		return FAIL;
	}

	return SUCC;
}

int GetWakeupKey(int devFd, unsigned int *wakeupKey)
{
	if (ioctl(devFd, REMOTE_IOC_GET_WAKEUP_KEY, wakeupKey) < 0) {
		fprintf(stderr, "failed to get wakeup key: %s\n",
			strerror(errno));
		return FAIL;
	}

	return SUCC;
}

int OpenDevice(char *filename)
{
	return open(filename, O_RDWR);
}

int CloseDevice(int fd)
{
	if (fd >= 0)
		close(fd);
	return 0;
}
