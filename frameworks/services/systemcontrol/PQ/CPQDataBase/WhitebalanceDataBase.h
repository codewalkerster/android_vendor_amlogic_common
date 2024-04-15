/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: header file
 */

#ifndef WHITEBALANCEDATABASE_H_
#define WHITEBALANCEDATABASE_H_

#include <cutils/properties.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "PQTableTypeOSD.h"
#include "CConfigFile.h"
#include "PQType.h"

#define CRI_DATA_RGBGO_START                        (0)
#define CRI_DATE_RGBOGO_DATA_LEN                    (sizeof(RGB_GAIN_OFFSET) + 1)
#define CRI_DATE_RGBOGO_INDEX_MAX                   (8)

#define CRI_DATA_WB_GAMMA_OFFSET                    (CRI_DATA_RGBGO_START + (CRI_DATE_RGBOGO_DATA_LEN * PQ_SRC_MAX * PQ_SIGFMT_MAX * CRI_DATE_RGBOGO_INDEX_MAX))
#define CRI_DATE_WB_GAMMA_DATA_LEN                  (sizeof(Multipoint_GAMMA_DATA) + 1)
#define CRI_DATE_WB_GAMMA_INDEX_MAX                 (8)

#define CRI_DATA_GAMMA_OFFSET                       (CRI_DATA_WB_GAMMA_OFFSET + (CRI_DATE_WB_GAMMA_DATA_LEN * PQ_SRC_MAX * PQ_SIGFMT_MAX * CRI_DATE_WB_GAMMA_INDEX_MAX ))
#define CRI_DATE_GAMMA_DATA_LEN                     (sizeof(tcon_gamma_table_t) + 1)
#define CRI_DATE_GAMMA_INDEX_MAX                    (11)

class WhitebalanceDataBase {
public:
    WhitebalanceDataBase();
    ~WhitebalanceDataBase();
    int Init();

    bool SetRGBGainOffsetData(RGB_GAIN_OFFSET *pData, int level, pq_source_input_t src = PQ_SRC_DEFAULT, pq_sig_fmt_t timing = PQ_SIGFMT_MAX);
    bool GetRGBGainOffsetData(RGB_GAIN_OFFSET *pData, int level, pq_source_input_t src = PQ_SRC_DEFAULT, pq_sig_fmt_t timing = PQ_SIGFMT_MAX);

    bool SetWhitebalanceGammaData(Multipoint_GAMMA_DATA *pData, int level, pq_source_input_t src = PQ_SRC_DEFAULT, pq_sig_fmt_t timing = PQ_SIGFMT_MAX);
    bool GetWhitebalanceGammaData(Multipoint_GAMMA_DATA *pData, int level, pq_source_input_t src = PQ_SRC_DEFAULT, pq_sig_fmt_t timing = PQ_SIGFMT_MAX);

    bool SetGammaTableData(unsigned short *pData, int level, int type);
    bool GetGammaTableData(unsigned short *pData, int level, int type);

private:
    bool ReadDataFromFile(const char *file_name, int offset, int nsize, void *data_buf);
    bool SaveDataToFile(const char *file_name, int offset, int nsize, void *data_buf);

};
#endif
