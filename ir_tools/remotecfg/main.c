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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <libgen.h>

#include "common.h"

#define DEVICE_FILE "/dev/amremote"
#define SYSDIR  "/sys/class/remote/amremote/"

/* support multi input devices */
#define DEVICE_FILE_0 "/dev/amremote0"
#define SYSDIR_0  "/sys/class/remote0/amremote0/"

unsigned char dbg_flag = 0;

static void usage(char *name)
{
	fprintf(stderr,"Usage: %s [-c cfg_file] [-t tab_file] [-d] [-g]\n", name);
	exit(EXIT_FAILURE);
}

static void printTabFile(S_TAB_FILE_T *tabFile)
{
	int i;

	prDbg("custom_name = %s\n", tabFile->tab.custom_name);
	prDbg("fn_key_scancode = 0x%x\n", tabFile->tab.cursor_code.fn_key_scancode);
	prDbg("cursor_left_scancode = 0x%x\n", tabFile->tab.cursor_code.cursor_left_scancode);
	prDbg("cursor_right_scancode = 0x%x\n", tabFile->tab.cursor_code.cursor_right_scancode);
	prDbg("cursor_up_scancode = 0x%x\n", tabFile->tab.cursor_code.cursor_up_scancode);
	prDbg("cursor_down_scancode = 0x%x\n", tabFile->tab.cursor_code.cursor_down_scancode);
	prDbg("cursor_ok_scancode = 0x%x\n", tabFile->tab.cursor_code.cursor_ok_scancode);
	prDbg("custom_code = 0x%x\n", tabFile->tab.custom_code);
	prDbg("release_delay = %d\n", tabFile->tab.release_delay);
	prDbg("map_size = %d\n", tabFile->tab.map_size);
	for (i = 0; i < tabFile->tab.map_size; i++)
		prDbg("key[%d]:  \t%d\t%d\n", i,
			tabFile->tab.codemap[i].code & 0xFFFF,
			tabFile->tab.codemap[i].code >> 16);
	for (i = 0; i < tabFile->wakeup_size; i++)
		prDbg("wakeup[%d]:  \t0x%08x\t%d\t%d\n", i,
			tabFile->wakeupTab[i].frame_code,
			tabFile->wakeupTab[i].ir_reason,
			tabFile->wakeupTab[i].report_val);


}

static void printCfgFile(S_CFG_FILE_T *cfgFile)
{
	prDbg("work_mode = %d\n", cfgFile->workMode);
//	prDbg("repeat_enable = %d\n", cfgFile->repeatEnable);
	prDbg("debug_enable = %d\n", cfgFile->debugEnable);
	prDbg("max_frame_time = %d\n", cfgFile->sw_data.max_frame_time);
}

static int handleCfgFile(char *name, char *val, void *data)
{
	S_CFG_FILE_T *cfgFile = (S_CFG_FILE_T *)data;

	if (MATCH("work_mode", name))
		cfgFile->workMode = strtoul(val, NULL, 0);
//	else if (MATCH("repeat_enable", name))
//		cfgFile->repeatEnable = strtoul(val, NULL, 0);
	else if (MATCH("debug_enable", name))
		cfgFile->debugEnable = strtoul(val, NULL, 0);
	else if (MATCH("max_frame_time", name))
		cfgFile->sw_data.max_frame_time = strtoul(val, NULL, 0);
	else
		return FAIL;

	return SUCC;

}

static int handleTabFile(char *name, char *val, void *data)
{
	S_TAB_FILE_T *tabFile = (S_TAB_FILE_T *)data;

	if (MATCH("custom_name", name))
	{
		 strncpy(tabFile->tab.custom_name, val, CUSTOM_NAME_LEN-1);
		 tabFile->tab.custom_name[CUSTOM_NAME_LEN-1] = '\0';
	}
	else if (MATCH("fn_key_scancode", name))
		tabFile->tab.cursor_code.fn_key_scancode = strtoul(val, NULL, 0);
	else if (MATCH("cursor_left_scancode", name))
		tabFile->tab.cursor_code.cursor_left_scancode = strtoul(val, NULL, 0);
	else if (MATCH("cursor_right_scancode", name))
		tabFile->tab.cursor_code.cursor_right_scancode = strtoul(val, NULL, 0);
	else if (MATCH("cursor_up_scancode", name))
		tabFile->tab.cursor_code.cursor_up_scancode = strtoul(val, NULL, 0);
	else if (MATCH("cursor_down_scancode", name))
		tabFile->tab.cursor_code.cursor_down_scancode = strtoul(val, NULL, 0);
	else if (MATCH("cursor_ok_scancode", name))
		tabFile->tab.cursor_code.cursor_ok_scancode = strtoul(val, NULL, 0);
	else if (MATCH("custom_code", name))
		tabFile->tab.custom_code = strtoul(val, NULL, 0);
	else if (MATCH("release_delay", name))
		tabFile->tab.release_delay = strtoul(val, NULL, 0);
	else if (MATCH("vendor", name))
		tabFile->tab.id.vendor = strtoul(val, NULL, 0);
	else if (MATCH("product", name))
		tabFile->tab.id.product = strtoul(val, NULL, 0);
	else if (MATCH("version", name))
		tabFile->tab.id.version = strtoul(val, NULL, 0);
	else if (MATCH("mapcode", name)) {
		tabFile->tab.codemap[tabFile->tab.map_size].code = atoi(val);
		tabFile->tab.map_size ++;
	}
	else
		return FAIL;

	return SUCC;
}

int SetConfigFile(int devfd, char *cfgdir)
{
	int ret = 0;
	S_CFG_FILE_T *cfgFile = NULL;

	prDbg("cfgdir = %s\n", cfgdir);
	cfgFile = malloc(sizeof(S_CFG_FILE_T));
	if (!cfgFile) {
		fprintf(stderr, "failed to allocate memory: %s\n",
			strerror(errno));
		ret = -1;
		goto err;
	}
	memset(cfgFile, 0, sizeof(S_CFG_FILE_T));
	if (ParseFile((const char *)cfgdir,
		handleCfgFile, (void *)cfgFile) < 0) {
		fprintf(stderr, "failed to parse file: %s\n", cfgdir);
		ret = -1;
		goto err;
	}
	printCfgFile(cfgFile);

	if (SetCfgPara(devfd, SYSDIR, cfgFile))
		if (SetCfgPara(devfd, SYSDIR_0, cfgFile))
			fprintf(stderr, "open %s: %s\n", SYSDIR_0, strerror(errno));
err:
	if (cfgFile)
		free(cfgFile);
	return ret;
}

int SetTabFile(int devfd, char *tabdir)
{
	int ret = 0;
	S_TAB_FILE_T *tabFile = NULL;

	prDbg("tabdir = %s\n", tabdir);
	tabFile = (S_TAB_FILE_T *)malloc(sizeof(S_TAB_FILE_T) + (MAX_KEYMAP_SIZE << 2));
	if (!tabFile)
	{
		fprintf(stderr, "failed to allocate memory: %s\n", strerror(errno));
		ret = -1;
		goto err;
	}
	memset(tabFile, 0, sizeof(S_TAB_FILE_T) + (MAX_KEYMAP_SIZE << 2));
	memset(&tabFile->tab.cursor_code, 0xff, sizeof(struct cursor_codemap));
	if (ParseFile((const char *)tabdir, handleTabFile, (void *)tabFile) < 0)
	{
		fprintf(stderr, "failed to parse file: %s\n", tabdir);
		ret = -1;
		goto err;
	}
	printTabFile(tabFile);
	SetTabPara(devfd, tabFile);
err:
	if (tabFile)
		free(tabFile);
	return ret;
}

int GetTabFile(int devfd)
{
	int ret = 0, i;
	unsigned int wakeupKey, tabList[16] = {0};
	S_TAB_FILE_T *tabFile = NULL;
	struct ir_wakeup_tab *wakeupTab = NULL;

	tabFile = (S_TAB_FILE_T *)malloc(sizeof(S_TAB_FILE_T) + (MAX_KEYMAP_SIZE << 2));
	wakeupTab = (struct ir_wakeup_tab *)malloc(sizeof(struct ir_wakeup_tab) * (MAX_WAKEUP_SIZE + 1));
	if (!tabFile || !wakeupTab)
	{
		fprintf(stderr, "failed to allocate memory: %s\n", strerror(errno));
		ret = -1;
		goto err;
	}

	memset(wakeupTab, 0, sizeof(struct ir_wakeup_tab) * (MAX_WAKEUP_SIZE + 1));
	memset(tabFile, 0, sizeof(S_TAB_FILE_T) + (MAX_KEYMAP_SIZE << 2));

	if (GetTabNum(devfd, tabList) < 0)
	{
		fprintf(stderr, "failed to get keymap num\n");
		ret = -1;
		goto err;
	}

	i = 0;
	while (tabList[i])
	{
		if (GetTabPara(devfd, tabFile, tabList[i]) < 0)
		{
			fprintf(stderr, "failed to get Tab 0x%X\n",
				tabList[i]);
			ret = -1;
			goto err;
		}

		printTabFile(tabFile);
		fprintf(stdout, "\n");
		i++;
	}

	if (GetWakeupPara(devfd, wakeupTab) < 0)
	{
		fprintf(stderr, "failed to get wakeup Tab\n");
		ret = -1;
		goto err;
	}

	if (GetWakeupKey(devfd, &wakeupKey) < 0)
	{
		fprintf(stderr, "failed to get wakeup Tab\n");
		ret = -1;
		goto err;
	}

	i = 0;
	fprintf(stdout, "Wakeup Key: 0x%x\n", wakeupKey);
	fprintf(stdout, "Wakeup Tab:\n");
	while (wakeupTab[i].frame_code)
	{
		fprintf(stdout, "0x%08X\t%d\t%d\n", wakeupTab[i].frame_code,
			wakeupTab[i].ir_reason, wakeupTab[i].report_val);
		i++;
	}
err:
	if (tabFile)
		free(tabFile);
	if (wakeupTab)
		free(wakeupTab);

	return ret;
}

int main(int argc, char *argv[])
{
	int ch;
	int devfd = -1;
	int ret;
	char *cfgfile = NULL;
	char *tabfile = NULL;
	unsigned char get_tab_flag = 0;

	opterr = 0; /*disable the 'getopt' debug info*/
	while ((ch = getopt(argc, argv, "dgc:t:")) != -1) {
		switch (ch) {
		case 'd':
			dbg_flag = 1;
			break;
		case 'c':
			cfgfile = optarg;
			break;
		case 't':
			tabfile = optarg;
			break;
		case 'g':
			get_tab_flag = 1;
			break;
		case '?':
			usage(basename(argv[0]));
			break;
		}

	}

	devfd = OpenDevice(DEVICE_FILE);
	if (devfd< 0) {
		devfd = OpenDevice(DEVICE_FILE_0);
		if (devfd< 0) {
			fprintf(stderr, "open failed:%s\n", strerror(errno));
			return FAIL;
		}
	}

	if (CheckVersion(devfd) < 0)
		goto err;

	if (get_tab_flag)
		GetTabFile(devfd);

	if (cfgfile)
		if (SetConfigFile(devfd, cfgfile))
			goto err;

	if (tabfile)
		if (SetTabFile(devfd, tabfile))
			goto err;
err:
	CloseDevice(devfd);
	return 0;
}
