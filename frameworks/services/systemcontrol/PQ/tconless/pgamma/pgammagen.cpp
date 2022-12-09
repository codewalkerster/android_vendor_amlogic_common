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

#include "CTconPGamma.h"

static const char *opt_str = "hif:p:b:";

static void help(char *appName)
{
    printf(
        "Usage:\n"
        "  %s [-h] [-i] [-f <flash path>] [-p <pmu path>] [-b <bin path>]\n"
        "\n"
        "Parameters:\n"
        "  -h : show help\n"
        "  -i : show pgamma info only\n"
        "  -f <flash path> : flash node path, use default path if not set or exist\n"
        "  -p <pmu path> : set pmu bin path which might from customers\n"
        "  -b <bin path> : set pgamma bin path to save\n",
        appName
    );
}

int main(int argc, char **argv)
{
    int ch;
    char *save_path = NULL;
    char *flash_path = NULL;
    char *pmu_path = NULL;
    CTconPGamma *demuraDev = NULL;
    bool justDispInfo = false;

    while ((ch = getopt(argc, argv, opt_str)) != -1) {
        printf("argc=%d, opt=%c, optind=%d\n", argc, ch, optind);
        switch (ch) {
        case 'h': help(argv[0]); exit(0);
        case 'i': justDispInfo = true; break;
        case 'f': flash_path = argv[optind-1]; break;
        case 'p': pmu_path = argv[optind-1]; break;
        case 'b': save_path = argv[optind-1]; break;
        default: break;
        }
    }

    demuraDev = CTconPGamma::GetInstance();
    if (demuraDev && !demuraDev->Init(flash_path, pmu_path)) {
        demuraDev->PrintInfo(0);
        if (!justDispInfo && save_path) {  //if not just display info, generate bin
            if (demuraDev->GenerateBin(save_path) < 0) {
                printf("Gen %s failed, exit...\n", save_path);
            } else {
                printf("Gen %s Ok\n", save_path);
            }
        }
    } else {
        printf("PGamma device init fail...\n");
    }
    if (demuraDev)
        demuraDev->UnInit();
    return 0;
}


