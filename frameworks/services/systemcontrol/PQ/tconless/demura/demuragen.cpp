#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "CTconDemura.h"

static const char *opt_str = "hif:b:";

static void help(char *appName)
{
    printf(
        "Usage:\n"
        "  %s [-h] [-i] [-f <flash path>] [-b <bin path>]\n"
        "\n"
        "Parameters:\n"
        "  -h : show help\n"
        "  -f <flash path> : flash node path, use default path if not set or exist\n"
        "  -b <bin path> : set demura bin path to save\n",
        appName
    );
}

int main(int argc, char **argv)
{
    int ch;
    char *save_path = NULL;
    char *flash_path = NULL;
    CTconDemura *demuraDev = NULL;
    bool justDispInfo = false;

    while ((ch = getopt(argc, argv, opt_str)) != -1) {
        printf("argc=%d, opt=%c, optind=%d\n", argc, ch, optind);
        switch (ch) {
        case 'h': help(argv[0]); exit(0);
        case 'i': justDispInfo = true; break;
        case 'f': flash_path = argv[optind-1]; break;
        case 'b': save_path = argv[optind-1]; break;
        default: break;
        }
    }

    demuraDev = CTconDemura::GetInstance();
    if (demuraDev && !demuraDev->Init(flash_path)) {
        demuraDev->PrintInfo(0);
        if (!justDispInfo && save_path) {  //if not just display info, generate bin
            if (demuraDev->GenerateBin(save_path) < 0) {
                printf("Gen %s failed, exit...\n", save_path);
            } else {
                printf("Gen %s Ok\n", save_path);
            }
        }
    } else {
        printf("Demura device init fail...\n");
    }
    if (demuraDev)
        demuraDev->UnInit();
    return 0;
}

