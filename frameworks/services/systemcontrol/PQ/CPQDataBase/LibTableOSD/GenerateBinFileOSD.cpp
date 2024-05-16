#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "PQTableLoaderOSD.h"

typedef struct _OUTPUT_BIN {
    const char *PQOsdFileName;
    const char *PQOsdBinName;
} OUTPUT_BIN;

OUTPUT_BIN BIN[] = {
    {"AML_PQ_OSD_Table_Public.cpp", "UI_PQSetting.bin"},
};

int main(int argc, char** argv)
{
    char sPanelFilePath[128], sTmp[128];
    *sPanelFilePath = '\0';
    *sTmp = '\0';

    FILE *pCurPanel = fopen("./curFile", "r");
    if (pCurPanel) {
        if (fscanf(pCurPanel, "%s", sTmp) < 0) {
            return 0;
        }
        fclose(pCurPanel);
    }

    char *pTok = NULL;
    char *file_name = NULL;
    char *temp = NULL;
    char chip[10] = {'\0'};
    pTok = strtok(sTmp, "=");
    if (pTok != NULL) {
        pTok = strtok(NULL, "=");
        if (pTok != NULL) {
            if (*pTok == '.')
                pTok ++;
            if (*pTok == '/')
                pTok ++;

            printf("pTok:%s\n", pTok);
            file_name = strstr(pTok, "AML_PQ_OSD");
            printf("file_name:%s\n", file_name);

            temp = strstr(pTok, "/");
            if (temp != NULL) {
                printf("temp:%s\n", temp);
                strncpy(chip, temp, file_name - temp);

                char cmd[50];
                sprintf(cmd, "mkdir OutPut/%s", chip);
                int res = system(cmd);
                if (res != 0) {
                    printf("mkdir failed\n");
                } else {
                    printf("mkdir success\n");
                }
            }
            printf("chip:%s\n", chip);

            unsigned int iCnt = 0;
            for (iCnt = 0; iCnt < sizeof(BIN) / sizeof(OUTPUT_BIN); iCnt ++) {
                if (!strcmp(BIN[iCnt].PQOsdFileName, file_name)) {
                    if (chip[0] == '\0') {
                        sprintf(sPanelFilePath, "OutPut/%s", BIN[iCnt].PQOsdBinName);
                    } else {
                        sprintf(sPanelFilePath, "OutPut%s%s", chip, BIN[iCnt].PQOsdBinName);
                    }
                    printf("sPanelFilePath:%s\n", sPanelFilePath);

                    PQTableGenerate_Osd(sPanelFilePath);
                    return 1;
                }
            }
        }
    }

    return 0;
}
