#define LOG_TAG "fwlog_bt"
#include <cutils/properties.h>
#include <cutils/android_filesystem_config.h>

#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <stdlib.h>
#include <sys/poll.h>
#include <asm/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <dirent.h>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <vector>
#include <chrono>
#include <stdint.h>
#include <time.h>


#include "amlbt_fwlog.h"

//c++
#include <iostream>
#include <string>

typedef long time64_t;

std::string getTimestamp(void) {
    time64_t rawTime;
    struct tm* timeInfo;
    char buffer[80];

    time(&rawTime);
    timeInfo = localtime(&rawTime);

    strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", timeInfo);
    return std::string(buffer);
}

std::string get_fw_log_path(void ) {
  char fw_path[PROPERTY_VALUE_MAX];
  property_get(fwlog_PATH_PROPERTY, fw_path, DEFAULT_fwlog_PATH);
  std::string result(fw_path);


  return result;
}



void writeHexData(int fd, const std::vector<uint8_t>& data, size_t startIdx = 0) {
/*
	auto startTime = std::chrono::high_resolution_clock::now();
*/
    //write time in logFile
    std::chrono::microseconds microseconds;
    std::ostringstream time;

    auto now = std::chrono::system_clock::now();
    auto timestamp = (time64_t)std::chrono::system_clock::to_time_t(now);
    std::tm* localTime = std::localtime(&timestamp);
    char buffer[128];

    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localTime);
    microseconds = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()) % 1000000;

    //format 2024-04-17 13:13:13.123456
    time << buffer << "." << std::setw(6) << std::setfill('0') << microseconds.count() << " ";

    std::string TimeString = time.str();
    write(fd, TimeString.c_str(), TimeString.size());

    //write fw log in logFile
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
	//ALOGE("writeHexData abner");

	for (size_t i = startIdx; i < data.size(); i++) {
        ss << std::setw(2) << static_cast<int>(data[i]) << " ";
    }

    std::string hexString = ss.str();


    write(fd, hexString.c_str(), hexString.size());
	/*
	auto endTime = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
	ALOGE("abner test,delater:%lld",duration.count());
	*/

}

void writefwlogdata(const std::vector<uint8_t>& data) {
	writeHexData(fwlogfile_fd, data,  2);

	dataCount++;

	if (dataCount == 500000) {
		updateLogFile();
		dataCount = 0;
	}
}

bool compareFilesByNumber(const std::string& file1, const std::string& file2) {

    //ALOGI("%s: abner file1= %s,file2= %s", __func__, file1.c_str(),file2.c_str());


    std::string number1 = file1.substr(14);
    std::string number2 = file2.substr(14);


    long long num1 = std::stoll(number1);
    long long num2 = std::stoll(number2);


    return num1 < num2;
}


void updateLogFile(void) {
    std::string timestamp = getTimestamp();

	std::string logPath = get_fw_log_path();
    std::string newFilename = logPath + "_" + timestamp;
    ALOGE("%s",__func__);

    if (fwlogfile_fd != INVALID_FD) {
        close(fwlogfile_fd);
        fwlogfile_fd = INVALID_FD;
    }

	std::string logFilePrefix = "fw_log.txt_";
    const int maxLogFileCount = 5;

    std::vector<std::string> logFiles;
    std::string logFileDir = logPath.substr(0, logPath.find_last_of('/'));

    DIR* dir = opendir(logFileDir.c_str());

    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string filename = entry->d_name;

            if (filename.compare(0, logFilePrefix.length(), logFilePrefix) == 0) {
				//ALOGE("%s:abner  find %s", __func__, filename.c_str());
                logFiles.push_back(filename);
            }
        }
        closedir(dir);
    }
	if (logFiles.size() >= maxLogFileCount) {

		std::sort(logFiles.begin(), logFiles.end(), compareFilesByNumber);


			std::string oldestFile = logFileDir + "/" + logFiles.front();
			//ALOGE("%s: Removing %s", __func__, oldestFile.c_str());
			remove(oldestFile.c_str());
			logFiles.erase(logFiles.begin());

	}


    if (std::rename(logPath.c_str(), newFilename.c_str()) != 0) {
        ALOGE("%s: Unable to rename fw_log.txt to %s, errno: %s", __func__, newFilename.c_str(), strerror(errno));
      //  return;
    }

    mode_t prevmask = umask(0);
    fwlogfile_fd = open(logPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    umask(prevmask);

    if (fwlogfile_fd != INVALID_FD) {
        ALOGE("%s: to open %s", __func__, logPath.c_str());
      //  return;
    }

}


void fwlog_init (void) {
	updateLogFile();
   if (fwlogfile_fd == INVALID_FD)
   ALOGE("%s: unable open fwlogfile_fd", __func__);
}

void fwlog_close (void) {
	if (fwlogfile_fd != INVALID_FD) close(fwlogfile_fd);
    fwlogfile_fd = INVALID_FD;
}
