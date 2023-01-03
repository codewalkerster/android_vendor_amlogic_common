/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: c++ file
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>

#include "lcdtype.h"
#include "TconRegLoader.h"
#include "TconRegHandler.h"

#define TOSTR(val) #val
#define dbgprint(debug, fmt, args...) \
    do { \
        if (debug) \
            printf("[DEBUG] " fmt, ##args); \
    } while (0)

#ifndef BIT
#define BIT(n) (1 << (n))
#define BIT_SET(val, bit) ((val) |= (bit))
#define BIT_CLR(val, bit) ((val) &= ~(bit))
#define BIT_CHECK(val, bit) ((val) & (bit))
#endif

// tcon sysfs dir
#define LCD_TCON_DIR_0  "/sys/class/lcd"
#define LCD_TCON_DIR_1  "/sys/class/aml_lcd/lcd0"

#define LANE_NUM_MAX 12
//////////////////////////lcd phy parameter////////////////////////////
// store phy value from argv
struct phyoptcfg_s {
    unsigned int mode;
    unsigned int vcm;
    unsigned int odt;
    unsigned int ref_bias;
    unsigned int vswing;
    unsigned int preem;
    unsigned int amp;
    unsigned int laneN;
    bool debug;
    unsigned int cfg;  // below OPT_BIT_XXX means bit function
#define PHYOPT_SET_BIT      BIT(0)
#define PHYOPT_MODE_BIT     BIT(1)
#define PHYOPT_VCM_BIT      BIT(2)
#define PHYOPT_ODT_BIT      BIT(3)
#define PHYOPT_BIAS_BIT     BIT(4)
#define PHYOPT_VSWING_BIT   BIT(5)
#define PHYOPT_PREEM_BIT    BIT(6)
#define PHYOPT_AMP_BIT      BIT(7)
#define PHYOPT_LANE_BIT     BIT(8)
#define PHYOPT_LANE_ALL_BIT BIT(9)
#define PHYOPT_SET(phyopt, bit) \
    do { \
        BIT_SET(phyopt.cfg, bit); \
    } while (0)
#define PHYOPT_CLR(phyopt, bit) \
    do { \
       BIT_CLR(phyopt.cfg, bit); \
    } while (0)
#define PHYOPT_CHECK(phyopt, bit) \
    BIT_CHECK(phyopt.cfg, bit)
};
///////////////////////////////////////////////////////////////////////

/////////////////////////////////lcd ss////////////////////////////////
static const char *lcd_ss_level_table[] = {
    "disable",
    "2000ppm",
    "4000ppm",
    "6000ppm",
    "8000ppm",
    "10000ppm",
    "12000ppm",
    "14000ppm",
    "16000ppm",
    "18000ppm",
    "20000ppm",
    "22000ppm",
    "24000ppm",
    "25000ppm",
    "28000ppm",
    "30000ppm",
    "32000ppm",
    "33000ppm",
    "36000ppm",
    "38500ppm",
    "40000ppm",
    "42000ppm",
    "44000ppm",
    "45000ppm",
    "48000ppm",
    "50000ppm",
    "50000ppm",
    "54000ppm",
    "55000ppm",
    "55000ppm",
    "60000ppm",
};

static const char *lcd_ss_freq_table[] = {
    "29.5KHz",
    "31.5KHz",
    "50KHz",
    "75KHz",
    "100KHz",
    "150KHz",
    "200KHz",
};

static const char *lcd_ss_mode_table[] = {
    "center ss",
    "up ss",
    "down ss",
};

struct tbl_s {
    const char **name_tbl;
    unsigned int size;
};

struct ss_tbl_s {
    struct tbl_s level_tbl;
    struct tbl_s freq_tbl;
    struct tbl_s mode_tbl;
};

static struct ss_tbl_s ss_tbl = {
    .level_tbl = {
        .name_tbl = lcd_ss_level_table,
        .size = sizeof(lcd_ss_level_table)/sizeof(lcd_ss_level_table[0]),
    },
    .freq_tbl = {
        .name_tbl = lcd_ss_freq_table,
        .size = sizeof(lcd_ss_freq_table)/sizeof(lcd_ss_freq_table[0]),
    },
    .mode_tbl = {
        .name_tbl = lcd_ss_mode_table,
        .size = sizeof(lcd_ss_mode_table)/sizeof(lcd_ss_mode_table[0]),
    },
};

// store ss value from argv
struct ssoptcfg_s {
    unsigned int mode;
    unsigned int level;
    unsigned int freq;
    bool debug;
    unsigned int cfg;  // below OPT_BIT_XXX means bit function
#define SSOPT_SET_BIT   BIT(0)  //0:get; 1:set
#define SSOPT_MODE_BIT  BIT(1)
#define SSOPT_LEVEL_BIT BIT(2)
#define SSOPT_FREQ_BIT  BIT(3)

#define SSOPT_SET(ssopt, bit) \
    do { \
        BIT_SET(ssopt.cfg, bit); \
    } while (0)
#define SSOPT_CLR(ssopt, bit) \
    do { \
        BIT_CLR(ssopt.cfg, bit); \
    } while (0)
#define SSOPT_CHECK(ssopt, bit) \
    BIT_CHECK(ssopt.cfg, bit)
};

///////////////////////////////////////////////////////////////////////

struct lcdopt_s {
    const char *name;

    /**
     * return:
     *   0 : success
     *   others : error
     */
    int (*opt_handler)(int argc, char *argv[]);
};

// argv not NULL, use custom path
// argv is  NULL, use default path
static int parse_fd(char *argv)
{
    int fd = -1;
    char lcd_path_buf[256] = { 0 };
    char *lcd_path_prefix = (char *)"/dev/lcd";

    memset(lcd_path_buf, 0, sizeof(lcd_path_buf));
    if (argv) {
        if (argv[0] == '/')
            strncpy(lcd_path_buf, argv, strlen(argv));
        else {
            snprintf(lcd_path_buf, sizeof(lcd_path_buf),
              "%s%d", lcd_path_prefix, atoi(argv));
        }
        if (access(lcd_path_buf, F_OK)) {
            printf("No path: %s\n", lcd_path_buf);
            if (atoi(argv) == 0) {  // index 0 try /dev/lcd and /dev/lcd0
                printf("Try path: %s\n", lcd_path_prefix);
                memset(lcd_path_buf, 0, sizeof(lcd_path_buf));
                strncpy(lcd_path_buf, lcd_path_prefix, strlen(lcd_path_prefix));
                if (access(lcd_path_buf, F_OK)) {
                    printf("No path %s, pls check...\n", lcd_path_buf);
                    goto __parse_path_exit;
                }
            } else {
                goto __parse_path_exit;
            }
        }
    } else {
        strncpy(lcd_path_buf, LCD_DEF_NODE1, strlen(LCD_DEF_NODE1));
        if (access(lcd_path_buf, F_OK)) {
            printf("No path: %s, Try path: %s...\n", lcd_path_buf, LCD_DEF_NODE2);
            memset(lcd_path_buf, 0, sizeof(lcd_path_buf));
            strncpy(lcd_path_buf, LCD_DEF_NODE2, strlen(LCD_DEF_NODE2));
            if (access(lcd_path_buf, F_OK)) {
                printf("No path %s, pls check...\n", lcd_path_buf);
                goto __parse_path_exit;
            }
        }
    }

    fd = open(lcd_path_buf, O_RDWR);
    if (fd < 0) {
        printf("Open %s error: %s, pls check...\n", lcd_path_buf, strerror(errno));
        goto __parse_path_exit;
    }
    printf("Device path: %s\n", lcd_path_buf);

__parse_path_exit:
    return fd;
}

static int power_handler(int argc, char *argv[])
{
    int ret = -1;
    unsigned int is_on = 0;
    char *ptr = NULL;
    char *dev = NULL;
    int ch = 0;
    int fd = -1;
    const char *opt_str = "d:";
    struct option opt_l_str[] = {
       {"dev",  required_argument, NULL, 'd'},
    };

    if (argc <= 1 || !argv) {
        printf("Invalid args, exit...\n");
        goto __power_handler_exit;
    }

    while ((ch = getopt_long(argc, argv, opt_str, opt_l_str, NULL)) != -1) {
        printf("argc=%d, ch=%c, optind=%d, optarg=%s\n", argc, ch, optind, optarg);
        switch (ch) {
        case 'd': dev = optarg; break;
        default: break;
        }
    }
    fd = parse_fd(dev);
    if (fd < 0)
        goto __power_handler_exit;

    is_on = !!strtoul(argv[optind], &ptr, 0);
    ret = ioctl(fd, LCD_IOC_CMD_POWER_CTRL, &is_on);
    if (ret < 0)
        printf("ioctl:%s(%u) error: %s\n", TOSTR(LCD_IOC_CMD_POWER_CTRL),
            LCD_IOC_CMD_POWER_CTRL, strerror(errno));
    else
        printf("power %s(%d) success.\n", is_on?"on":"off", is_on);

__power_handler_exit:
    if (fd >= 0)
        close(fd);
    return ret;
}

static int mute_handler(int argc, char *argv[])
{
    int ret = -1;
    unsigned int is_on = 0;
    char *ptr = NULL;
    char *dev = NULL;
    int ch = 0;
    int fd = -1;
    const char *opt_str = "d:";
    struct option opt_l_str[] = {
       {"dev",  required_argument, NULL, 'd'},
    };

    if (argc <= 1 || !argv) {
        printf("Invalid args, exit...\n");
        goto __mute_handler_exit;
    }
    while ((ch = getopt_long(argc, argv, opt_str, opt_l_str, NULL)) != -1) {
        printf("argc=%d, ch=%c, optind=%d, optarg=%s\n", argc, ch, optind, optarg);
        switch (ch) {
        case 'd': dev = optarg; break;
        default: break;
        }
    }
    fd = parse_fd(dev);
    if (fd < 0)
        goto __mute_handler_exit;

    is_on = !!strtoul(argv[optind], &ptr, 0);
    ret = ioctl(fd, LCD_IOC_CMD_MUTE_CTRL, &is_on);
    if (ret < 0)
        printf("ioctl:%s(%u) error: %s\n", TOSTR(LCD_IOC_CMD_MUTE_CTRL),
            LCD_IOC_CMD_MUTE_CTRL, strerror(errno));
    else
        printf("mute %s(%d) success.\n", is_on?"on":"off", is_on);

__mute_handler_exit:
    if (fd >= 0)
        close(fd);
    return ret;
}

static void print_phy_cfg(struct phy_config_s *phy)
{
    unsigned int i = 0;

    if (!phy)
        return;

    printf("ioctl_mode  = %d\n"
           "lane_num    = %d\n"
           "vswing      = %#x (%d)\n"
           "vcm         = %#x (%d)\n"
           "odt         = %#x (%d)\n"
           "ref_bias    = %#x (%d)\n"
           "vswing_level= %#x (%d)\n"
           "preem_level = %#x (%d)\n",
           phy->ioctl_mode,
           phy->lane_num,
           phy->vswing, phy->vswing,
           phy->vcm, phy->vcm,
           phy->odt, phy->odt,
           phy->ref_bias, phy->ref_bias,
           phy->vswing_level, phy->vswing_level,
           phy->preem_level, phy->preem_level
    );
    for (i = 0; i < LANE_NUM_MAX; i++)
        printf("lane[%02d] amp=%#x (%d), preem=%#x (%d)\n",
            i, phy->lane[i].amp, phy->lane[i].amp,
            phy->lane[i].preem, phy->lane[i].preem);
}

static void cfg_to_phy(struct phyoptcfg_s *phyopt, struct phy_config_s *phy)
{
#define _PHY_SET(cfg, bitn, physub, cfgsub, name) \
    do { \
        if (PHYOPT_CHECK((*cfg), bitn)) { \
            physub = cfgsub; \
            dbgprint(cfg->debug, name "=%#x (%d)\n", physub, physub); \
        } \
    } while (0)

    if (!phyopt || !phy)
        return;

    _PHY_SET(phyopt, PHYOPT_MODE_BIT, phy->ioctl_mode, phyopt->mode, "ioctl_mode");
    _PHY_SET(phyopt, PHYOPT_VCM_BIT,  phy->vcm, phyopt->vcm, "vcm");
    _PHY_SET(phyopt, PHYOPT_ODT_BIT,  phy->odt, phyopt->odt, "odt");
    _PHY_SET(phyopt, PHYOPT_BIAS_BIT, phy->ref_bias, phyopt->ref_bias, "ref_bias");
    if (phy->ioctl_mode == 0) {
        _PHY_SET(phyopt, PHYOPT_VSWING_BIT, phy->vswing_level, phyopt->vswing, "vswing_level");
        _PHY_SET(phyopt, PHYOPT_PREEM_BIT,  phy->preem_level, phyopt->preem, "preem_level");
    } else {
        _PHY_SET(phyopt, PHYOPT_VSWING_BIT, phy->vswing, phyopt->vswing, "vswing");
        if (PHYOPT_CHECK((*phyopt), PHYOPT_LANE_BIT)) {
            int i = 0;
            int lane_loop = 1;
            int lane_idx = phyopt->laneN;
            if (PHYOPT_CHECK((*phyopt), PHYOPT_LANE_ALL_BIT))
                lane_loop = LANE_NUM_MAX;
            for (i = 0; i < lane_loop; i++) {
                if (PHYOPT_CHECK((*phyopt), PHYOPT_LANE_ALL_BIT))
                    lane_idx = i;
                if (PHYOPT_CHECK((*phyopt), PHYOPT_PREEM_BIT)) {
                    phy->lane[lane_idx].preem = phyopt->preem;
                    dbgprint(phyopt->debug, "lane[%d] preem=%#x (%d)\n",
                        lane_idx, phyopt->preem, phyopt->preem);
                }
                if (PHYOPT_CHECK((*phyopt), PHYOPT_AMP_BIT)) {
                    phy->lane[lane_idx].amp = phyopt->amp;
                    dbgprint(phyopt->debug, "lane[%d] amp=%#x (%d)\n",
                        lane_idx, phyopt->amp, phyopt->amp);
                }
            }
        }
    }
#undef _PHY_SET
}

static int phy_handler(int argc, char *argv[])
{
    int ret = -1;
    int ch = 0;
    char *ptr = NULL;
    char *dev = NULL;
    const char *opt_str = "m:v:o:r:s:l:a:p:d:i";
    struct option opt_l_str[] = {
       {"mode",     required_argument, NULL, 'm'},
       {"vcm",      required_argument, NULL, 'v'},
       {"odt",      required_argument, NULL, 'o'},
       {"ref_bias", required_argument, NULL, 'r'},
       {"vswing",   required_argument, NULL, 's'},
       {"lane",     required_argument, NULL, 'l'},
       {"amp",      required_argument, NULL, 'a'},
       {"preem",    required_argument, NULL, 'p'},
       {"dev",      required_argument, NULL, 'd'},
       {"info",     no_argument,       NULL, 'i'},
    };
    struct phy_config_s phy;
    struct phyoptcfg_s phyopt;
    bool debug = false;
    int fd = -1;

    memset(&phy, 0, sizeof(phy));
    memset(&phyopt,  0, sizeof (phyopt));
    while ((ch = getopt_long(argc, argv, opt_str, opt_l_str, NULL)) != -1) {
        printf("argc=%d, ch=%c, optind=%d, optarg=%s\n", argc, ch, optind, optarg);
        switch (ch) {
        case 'm': phyopt.mode = strtoul(optarg, &ptr, 0); PHYOPT_SET(phyopt, PHYOPT_SET_BIT|PHYOPT_MODE_BIT); break;
        case 'v': phyopt.vcm  = strtoul(optarg, &ptr, 0); PHYOPT_SET(phyopt, PHYOPT_SET_BIT|PHYOPT_VCM_BIT); break;
        case 'o': phyopt.odt  = strtoul(optarg, &ptr, 0); PHYOPT_SET(phyopt, PHYOPT_SET_BIT|PHYOPT_ODT_BIT); break;
        case 'r': phyopt.ref_bias = strtoul(optarg, &ptr, 0); PHYOPT_SET(phyopt, PHYOPT_SET_BIT|PHYOPT_BIAS_BIT); break;
        case 's': phyopt.vswing   = strtoul(optarg, &ptr, 0); PHYOPT_SET(phyopt, PHYOPT_SET_BIT|PHYOPT_VSWING_BIT); break;
        case 'l':
            if (!strcasecmp(optarg, "all"))
                PHYOPT_SET(phyopt, PHYOPT_LANE_ALL_BIT);
            else
                phyopt.laneN = strtoul(optarg, &ptr, 0);
            PHYOPT_SET(phyopt, PHYOPT_SET_BIT|PHYOPT_LANE_BIT);
            break;
        case 'a': phyopt.amp = strtoul(optarg, &ptr, 0);   PHYOPT_SET(phyopt, PHYOPT_SET_BIT|PHYOPT_AMP_BIT); break;
        case 'p': phyopt.preem = strtoul(optarg, &ptr, 0); PHYOPT_SET(phyopt, PHYOPT_SET_BIT|PHYOPT_PREEM_BIT); break;
        case 'd': dev = optarg; break;
        case 'i': debug = phyopt.debug = true; break;
        default: break;
        }
    }

    fd = parse_fd(dev);
    if (fd < 0)
        goto __phy_handler_exit;

    ret = ioctl(fd, LCD_IOC_CMD_GET_PHY_PARAM, &phy);
    if (ret < 0) {
        printf("ioctl:%s(%u) error: %s\n", TOSTR(LCD_IOC_CMD_GET_PHY_PARAM),
            LCD_IOC_CMD_GET_PHY_PARAM, strerror(errno));
        goto __phy_handler_exit;
    }

    dbgprint(debug, "--------------------raw config--------------------\n");
    if (debug)
        print_phy_cfg(&phy);

    // save phyopt to phy
    cfg_to_phy(&phyopt, &phy);

    if (PHYOPT_CHECK(phyopt, PHYOPT_SET_BIT)) {
        dbgprint(debug, "--------------------modify config--------------------\n");
        if (debug)
            print_phy_cfg(&phy);

        ret = ioctl(fd, LCD_IOC_CMD_SET_PHY_PARAM, &phy);
        if (ret < 0) {
            printf("ioctl:%s(%u) error: %s\n", TOSTR(LCD_IOC_CMD_SET_PHY_PARAM),
                LCD_IOC_CMD_SET_PHY_PARAM, strerror(errno));
            goto __phy_handler_exit;
        }

        // get result
        ret = ioctl(fd, LCD_IOC_CMD_GET_PHY_PARAM, &phy);
        if (ret < 0) {
            printf("ioctl:%s(%u) error: %s\n", TOSTR(LCD_IOC_CMD_GET_PHY_PARAM),
                LCD_IOC_CMD_GET_PHY_PARAM, strerror(errno));
            goto __phy_handler_exit;
        }
    }

    dbgprint(debug, "--------------------result config--------------------\n");
    print_phy_cfg(&phy);

__phy_handler_exit:
    if (fd >= 0)
        close(fd);
    return ret;
}

// return true: valid param
static bool check_ss_param(struct aml_lcd_ss_ctl_s *ss)
{
    if (!ss)
        return false;

    if (ss->mode >= ss_tbl.mode_tbl.size ||
        ss->freq >= ss_tbl.freq_tbl.size ||
        ss->level >= ss_tbl.level_tbl.size)
        return false;

    return true;
}

static void cfg_to_ss(struct ssoptcfg_s *ssopt, struct aml_lcd_ss_ctl_s *ss)
{
#define _SS_SET(cfg, bitn, sssub, cfgsub, name) \
    do { \
        if (SSOPT_CHECK((*cfg), bitn)) { \
            sssub = cfgsub; \
            dbgprint(cfg->debug, name "=%#x (%d)\n", sssub, sssub); \
        } \
    } while (0)

    if (!ssopt || !ss)
        return;

    _SS_SET(ssopt, SSOPT_MODE_BIT, ss->mode, ssopt->mode, "mode");
    _SS_SET(ssopt, SSOPT_LEVEL_BIT, ss->level, ssopt->level, "level");
    _SS_SET(ssopt, SSOPT_FREQ_BIT, ss->freq, ssopt->freq, "freq");

#undef _SS_SET
}

static void ss_help(char *appname)
{
    unsigned int i = 0;
    printf("Usage:\n");
    printf("  %s [-h] [-d <dev>] [-m <mode>] [-l <level>] [-f <freq>]\n\n", appname);
    printf("Parameters:\n");
    printf("  -h : show this help\n");
    printf("  -d <dev>  : set dev path or index(/dev/lcdx) when using multiple lcd devices\n");
    printf("  -m <mode> : set ss mode, from (0~%d)\n", ss_tbl.mode_tbl.size-1);
    for (i = 0; i < ss_tbl.mode_tbl.size; i++)
        printf("    %d : %s\n", i, ss_tbl.mode_tbl.name_tbl[i]);
    printf("  -l <level>: set ss level, from (0~%d)\n", ss_tbl.level_tbl.size-1);
    for (i = 0; i < ss_tbl.level_tbl.size; i++)
        printf("    %d : %s\n", i, ss_tbl.level_tbl.name_tbl[i]);
    printf("  -f <freq> : set ss freq, from (0~%d)\n", ss_tbl.freq_tbl.size-1);
    for (i = 0; i < ss_tbl.freq_tbl.size; i++)
        printf("    %d : %s\n", i, ss_tbl.freq_tbl.name_tbl[i]);
}

static int ss_handler(int argc, char *argv[])
{
    int ret = -1;
    int ch = 0;
    char *ptr = NULL;
    char *dev = NULL;
    const char *opt_str = "l:f:m:dhi";
    struct option opt_l_str[] = {
       {"level", required_argument, NULL, 'l'},
       {"freq",  required_argument, NULL, 'f'},
       {"mode",  required_argument, NULL, 'm'},
       {"dev",   required_argument, NULL, 'd'},
       {"help",  required_argument, NULL, 'h'},
       {"info",  no_argument,       NULL, 'i'},
    };
    struct aml_lcd_ss_ctl_s ss;
    struct ssoptcfg_s ssopt;
    bool debug = false;
    bool needhelp = false;
    int fd = -1;

    memset(&ssopt, 0, sizeof(ssopt));
    while ((ch = getopt_long(argc, argv, opt_str, opt_l_str, NULL)) != -1) {
        printf("argc=%d, ch=%c, optind=%d, optarg=%s\n", argc, ch, optind, optarg);
        switch (ch) {
        case 'l': ssopt.level = strtoul(optarg, &ptr, 0); SSOPT_SET(ssopt, SSOPT_SET_BIT|SSOPT_LEVEL_BIT); break;
        case 'f': ssopt.freq  = strtoul(optarg, &ptr, 0); SSOPT_SET(ssopt, SSOPT_SET_BIT|SSOPT_FREQ_BIT); break;
        case 'm': ssopt.mode  = strtoul(optarg, &ptr, 0); SSOPT_SET(ssopt, SSOPT_SET_BIT|SSOPT_MODE_BIT); break;
        case 'd': dev = optarg; break;
        case 'i': debug = ssopt.debug = true; break;
        case 'h': needhelp = true; break;
        default: break;
        }
    }

    fd = parse_fd(dev);
    if (fd < 0)
        goto __ss_handler_exit;

    ret = ioctl(fd, LCD_IOC_CMD_GET_SS, &ss);
    if (ret < 0) {
        printf("ioctl:%s(%u) error: %s\n", TOSTR(LCD_IOC_CMD_GET_SS),
            LCD_IOC_CMD_GET_SS, strerror(errno));
        goto __ss_handler_exit;
    }

    dbgprint(debug,
        "--------------------raw config--------------------\n"
        "ss mode =%#x (%d)\n"
        "ss level=%#x (%d)\n"
        "ss freq =%#x (%d)\n",
        ss.mode, ss.mode,
        ss.level, ss.level,
        ss.freq, ss.freq);

    // save ssopt to ss
    cfg_to_ss(&ssopt, &ss);

    if (SSOPT_CHECK(ssopt, SSOPT_SET_BIT)) {
        if (!check_ss_param(&ss)) {
            printf("Invalid value, pls check your parameters...\n\n");
            ss_help(argv[0]);
            goto __ss_handler_exit;
        }
        dbgprint(debug,
            "--------------------modify config--------------------\n"
            "ss mode =%#x (%d)\n"
            "ss level=%#x (%d)\n"
            "ss freq =%#x (%d)\n",
            ss.mode, ss.mode,
            ss.level, ss.level,
            ss.freq, ss.freq);

        ret = ioctl(fd, LCD_IOC_CMD_SET_SS, &ss);
        if (ret < 0) {
            printf("ioctl:%s(%u) error: %s\n", TOSTR(LCD_IOC_CMD_SET_SS),
                LCD_IOC_CMD_SET_SS, strerror(errno));
            goto __ss_handler_exit;
        }

        // get result
        ret = ioctl(fd, LCD_IOC_CMD_GET_SS, &ss);
        if (ret < 0) {
            printf("ioctl:%s(%u) error: %s\n", TOSTR(LCD_IOC_CMD_GET_SS),
                LCD_IOC_CMD_GET_SS, strerror(errno));
            goto __ss_handler_exit;
        }
    }

    if (needhelp) {
        ss_help(argv[0]);
    } else {
        const char *mode_name =
            (ss.mode<ss_tbl.mode_tbl.size)?ss_tbl.mode_tbl.name_tbl[ss.mode]:"unknown";
        const char *level_name =
            (ss.level<ss_tbl.level_tbl.size)?ss_tbl.level_tbl.name_tbl[ss.level]:"unknown";
        const char * freq_name =
            (ss.freq<ss_tbl.freq_tbl.size)?ss_tbl.freq_tbl.name_tbl[ss.freq]:"unknown";
        dbgprint(debug, "--------------------result config--------------------\n");
        printf(
            "ss mode =%#x (%d): %s\n"
            "ss level=%#x (%d): %s\n"
            "ss freq =%#x (%d): %s\n",
            ss.mode, ss.mode, mode_name,
            ss.level, ss.level, level_name,
            ss.freq, ss.freq, freq_name
        );
    }

__ss_handler_exit:
    if (fd >= 0)
        close(fd);
    return ret;
}

static bool do_wreg_ioctl(char *dev, std::vector<TconRegPair> &vec, bool debug)
{
    //To be fixed
    return false;
}

static bool do_wreg_sysfs(std::vector<TconRegPair> &vec, bool debug)
{
    bool success = false;
    TconRegHandler *regHandler = new TconRegHandler;
    regHandler->setDumpTo(0, debug);
    if (!regHandler->init(LCD_TCON_DIR_0)
         && !regHandler->init(LCD_TCON_DIR_1)) {
        printf("Init fail, exit...\n");
        goto __do_wreg_sysfs_exit;
    }

    for (int i = 0; i < vec.size(); i++) {
        TconRegPair pair = vec[i];
        if (!regHandler->setReg(pair.reg, pair.value)) {
            printf("Set fail: Reg(%#x)=0x%08x (%u)\n",
                pair.reg, pair.value, pair.value);
        }
    }

    success = true;

__do_wreg_sysfs_exit:
    if (regHandler) {
        regHandler->uninit();
        delete regHandler;
    }
    return success;
}

static int tcon_handler(int argc, char *argv[])
{
    int ch = 0;
    int ret = -1;
    bool success = false;
    char *dev = NULL;
    char *loadpath = NULL;
    bool debug = false;
    std::vector<TconRegPair> vec;
    TconRegLoader *loader = NULL;
    const char *opt_str = "d:l:i";
    struct option opt_l_str[] = {
        {"dev",  required_argument, NULL, 'd'},
        {"load", required_argument, NULL, 'l'},
        {"info", no_argument,       NULL, 'i'}
    };

    if (argc <= 1 || !argv) {
        printf("Invalid args, exit...\n");
        goto __tcon_handler_exit;
    }

    while ((ch = getopt_long(argc, argv, opt_str, opt_l_str, NULL)) != -1) {
        printf("argc=%d, ch=%c, optind=%d, optarg=%s\n", argc, ch, optind, optarg);
        switch (ch) {
        case 'd': dev = optarg; break;
        case 'l': loadpath = optarg; break;
        case 'i': debug = true; break;
        default: break;
        }
    }

    // load tcon register file
    loader = new TconRegLoader;
    loader->setDumpTo(0, debug);
    if (!loader->load(vec, loadpath))
        goto __tcon_handler_exit;

    dbgprint(debug, "--------------------Print Loaded Reg Pair--------------------\n");
    if (debug) {
        for (int i = 0; i < vec.size(); i++) {
            TconRegPair pair = vec[i];
            dbgprint(debug, "Reg(%#x) = 0x%08x (%u)\n",
                pair.reg, pair.value, pair.value);
        }
    }

    // ioctl/sysfs set reg
    success = do_wreg_ioctl(dev, vec, debug);
    if (!success)
        success = do_wreg_sysfs(vec, debug);

    printf("Load and set [%s]: %s\n", loadpath, success?"success":"fail");

    ret = 0;

__tcon_handler_exit:
    if (loader)
        delete loader;
    return ret;
}

static struct lcdopt_s lcd_def_opts[] = {
    {"power", power_handler},
    {"mute",  mute_handler},
    {"phy",   phy_handler},
    {"ss",    ss_handler},
    {"tcon",  tcon_handler},
};

static void help(char *appName)
{
    printf(
        "Usage:\n"
        "  %s power|mute|phy|ss|tcon [-d <dev>] [OPTION]\n"
        "     -d <dev>: set dev path or index(/dev/lcdx) when using multiple lcd devices\n"
        "\n"
        "SUBCOMMAND usage:\n"
        "  %s power [-d <dev>] <0|1>\n"
        "     0 : power off\n"
        "     1 : power on\n"
        "  %s mute [-d <dev>] <0|1>\n"
        "     0 : mute off\n"
        "     1 : mute on\n"
        "  %s phy [-d <dev>] [-m <mode>] [-v <vcm>] [-o <odt>] [-r <ref_bias>] [-s <vswing>] [-l <lane N> [-a <amp>] [-p <preem>]]\n"
        "     No parameter means get phy config.\n"
        "     -m <mode>     : set phy config mode\n"
        "        0 : all lane use same vswing/preem. Ignore '-l'\n"
        "        1 : each lane use single line parameter\n"
        "     -v <vcm>      : set vcm value\n"
        "     -o <odt>      : set odt value\n"
        "     -r <ref_bias> : set ref_bias value\n"
        "     -s <vswing>   : set vswing value\n"
        "     -l <lane N>   : set amp & preem for lane N(0~11)\n"
        "                     set 'all' if need to set all lane\n"
        "     -a <amp>      : set amp for lane N\n"
        "     -p <preem>    : set preem for lane N\n"
        "\n"
        "  %s ss [-d <dev>] [-h] [-m <mode>] [-l <level>] [-f <freq>]\n"
        "     No parameter means get ss config.\n"
        "     -h : show more ss help\n"
        "     -m <mode>  : set ss mode\n"
        "     -l <level> : set ss level\n"
        "     -f <freq>  : set ss freq\n"
        "\n"
        "  %s tcon [-d <dev>] [-l <path>]\n"
        "     [-l <path>] : load tcon register file(.txt)\n",
        appName, appName, appName, appName, appName, appName
    );
}

int main(int argc, char *argv[])
{
    int ret = -1;
    int i = 0;
    int def_opt_num = sizeof(lcd_def_opts) / sizeof(struct lcdopt_s);

    if (argc > 1) {
        for (i = 0; i < def_opt_num; i++) {
            if (!strcasecmp(lcd_def_opts[i].name, argv[1])) {
                ret = lcd_def_opts[i].opt_handler(argc-1, &argv[1]);
            }
        }
    }

    if (ret)
        help(argv[0]);

    return ret;
}

