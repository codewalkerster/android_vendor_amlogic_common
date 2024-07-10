#ifndef AMLBT_FWLOG_H
#define AMLBT_FWLOG_H

#include <utils/Log.h>
#include "userial.h"
#include <string>
#include <vector>

#define fwlog_PATH_PROPERTY "persist.bluetooth.btfwlogpath"
#define DEFAULT_fwlog_PATH "/data/vendor/fw_log.txt"

void fwlog_init (void);
void fwlog_close (void);
std::string getTimestamp(void);
void writeHexData(int fd, const std::vector<uint8_t>& data, size_t startIdx );
void updateLogFile(void);
void writefwlogdata(const std::vector<uint8_t>& data);

#endif/*AMLBT_FWLOG_H*/

