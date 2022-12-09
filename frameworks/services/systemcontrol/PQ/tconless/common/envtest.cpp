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
#include <unistd.h>
#include "UbootEnv.h"

enum ENV_OPT{
    ENV_OPT_READ,
    ENV_OPT_WRITE,
    ENV_OPT_END,
};

static const char *opt_str = "w:r:h";
static void help(char *appName)
{
    printf(
        "Usage:\n"
        "  %s [-r <name>] [-w <name> <value>] [-h]\n"
        "\n"
        "Parameters:\n"
        "  -h : show help\n"
        "  -r <name> : read uboot env\n"
        "  -w <name> <value> : write uboot env\n",
        appName
    );
}

int main(int argc, char **argv)
{
    int ch;
    char *env;
    char *wr_value;
    int opt = ENV_OPT_END;
    UbootEnv *ubootenv = new UbootEnv;
    char rd_val[1024];

    memset(rd_val, 0, sizeof(rd_val));
    while ((ch = getopt(argc, argv, opt_str)) != -1) {
        switch (ch) {
        case 'h': help(argv[0]); exit(0);
        case 'r': env = argv[optind-1]; opt = ENV_OPT_READ; break;
        case 'w':
            opt = ENV_OPT_WRITE;
            env = argv[optind-1];
            if (optind < argc) {
                wr_value = argv[optind];
            } else {
                help(argv[0]);
                exit(0);
            }
            break;
        default: break;
        }
    }

    if (!ubootenv) {
        printf("Alloc uboot env fail...\n");
        goto __main_exit;
    }

    switch (opt) {
    case ENV_OPT_READ:
        if (ubootenv->read(env, rd_val)) {
            printf("Read [%s]=%s\n", env, rd_val);
        } else {
            printf("Read [%s] fail...\n", env);
        }
        break;
    case ENV_OPT_WRITE:
        if (ubootenv->write(env, wr_value)) {
            printf("Write [%s]=%s\n", env, wr_value);
        } else {
            printf("Write [%s] fail...\n", env);
        }
    }

__main_exit:
    if (ubootenv)
        delete ubootenv;

    return 0;
}

