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
#include <zlib.h>
#include <thread>

#define fwlog_version "2025-07-02"

static const int INVALID_FD = -1;
static int fwlogfile_fd = INVALID_FD;

static int32_t dataCount = 0;
static int32_t prop_dataCount = 0;
static int32_t prop_fw_buff_level = 0;
static int32_t prop_maxLogFileCount = 0;

#define TOGGLE_BT_FW_MAX_FILE_NUM   "persist.log.tag.fw_max_file_num_bt"
#define TOGGLE_BT_FW_BUFFER_LEVEL   "persist.log.tag.fw_buffer_bt"

#define default_max_file_count  3      //1: one file
#define default_fw_buff_level  300     //300MB/411B = 765383

static int32_t Flag_gzip_thread = 0;
static int32_t Flag_gzip_start = 0;



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

int32_t get_property_as_int32_t(const char *key, int32_t default_value) {
	char value[PROP_VALUE_MAX];
	property_get(key, value, "");

	if (value[0] == '\0') {
		return default_value;
	}

	char *endptr;
	long result = strtol(value, &endptr, 10);

	if (value == endptr || *endptr != '\0') {
		return default_value;
	}

	if (result < INT32_MIN || result > INT32_MAX) {
		return default_value;
	}

	return (int32_t)result;
}


void writeHexData(int fd, const std::vector<uint8_t>& data, size_t startIdx = 0) {
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

	for (size_t i = startIdx; i < data.size(); i++) {
		ss << std::setw(2) << static_cast<int>(data[i]) << " ";
	}

	std::string hexString = ss.str();

	write(fd, hexString.c_str(), hexString.size());
}

void writefwlogdata(const std::vector<uint8_t>& data) {
	int32_t tmp_prop_fw_buff_level = 0;
	int32_t one_package_size = 0;

	writeHexData(fwlogfile_fd, data,  2);
	dataCount++;
	tmp_prop_fw_buff_level =  get_property_as_int32_t(TOGGLE_BT_FW_BUFFER_LEVEL,default_fw_buff_level);
	if(tmp_prop_fw_buff_level != prop_fw_buff_level) {
		ALOGE("prop_fw_buff_level %d M change to %d M ",prop_fw_buff_level,tmp_prop_fw_buff_level);
		prop_fw_buff_level = tmp_prop_fw_buff_level;
		one_package_size = (data.size()-2) * 3 + 27;
		prop_dataCount = (prop_fw_buff_level * 1024 * 1024) / one_package_size;
		ALOGE("one_package_size:%d  prop_dataCount:%d  dataCount:%d ",one_package_size,prop_dataCount,dataCount);
	}
	if (dataCount >= prop_dataCount) {
		updateLogFile();
		dataCount = 0;
	}
}

bool compareFilesByNumber(const std::string& file1, const std::string& file2) {

	std::string number1 = file1.substr(14);
	std::string number2 = file2.substr(14);

	long long num1 = std::stoll(number1);
	long long num2 = std::stoll(number2);

	return num1 < num2;
}


void updateLogFile(void) {

	std::string logPath = get_fw_log_path();
	std::vector<std::string> logFiles;
	std::string logFileDir = logPath.substr(0, logPath.find_last_of('/'));
	std::string logFilePrefix = "fw_log.txt_";
	const std::string gzSuffix = ".gz";

	ALOGE("%s",__func__);
	if (fwlogfile_fd != INVALID_FD) {
		close(fwlogfile_fd);
		fwlogfile_fd = INVALID_FD;
	}

	std::vector<int> fileNumbers;
	DIR* dir = opendir(logFileDir.c_str());
	if (dir) {
		struct dirent* entry;
		while ((entry = readdir(dir)) != nullptr) {
			std::string filename = entry->d_name;
			// check fw_log.txt_num or fw_log.txt_num.gz
			if (filename.find(logFilePrefix) == 0) {
				std::string numPart;
				if (filename.size() > gzSuffix.size() &&filename.substr(filename.size() - gzSuffix.size()) == gzSuffix) {
					numPart = filename.substr(logFilePrefix.length(),filename.length() - logFilePrefix.length() - gzSuffix.length());
				} else {
					numPart = filename.substr(logFilePrefix.length());
				}
				char* endPtr = nullptr;
				long fileNum = std::strtol(numPart.c_str(), &endPtr, 10);
				if (endPtr != numPart.c_str() && *endPtr == '\0' && fileNum >= 0) {
					fileNumbers.push_back(static_cast<int>(fileNum));
				} else {
					ALOGE("%s: Invalid log file number in %s", __func__, filename.c_str());
				}
			}
		}
		closedir(dir);
	}

	int nextFileNum = 1;
	if (!fileNumbers.empty()) {
		// find max num +1
		auto maxIt = std::max_element(fileNumbers.begin(), fileNumbers.end());
		nextFileNum = *maxIt + 1 ;
	}

	std::string newFilename = logFileDir + "/" + logFilePrefix + std::to_string(nextFileNum);
	if (std::rename(logPath.c_str(), newFilename.c_str())) {
		ALOGE("%s: Rename failed! %s -> %s [%s]",__func__, logPath.c_str(), newFilename.c_str(), strerror(errno));
	} else {
		ALOGE("%s: Renamed current log to %s", __func__, newFilename.c_str());
	}
	mode_t prevmask = umask(0);
	fwlogfile_fd = open(logPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	umask(prevmask);
	if (fwlogfile_fd != INVALID_FD) {
		ALOGE("%s: to open %s", __func__, logPath.c_str());
	}

	Flag_gzip_start = 1;
}

int gzipCompress(const char* input_path, const char* output_path) {
	FILE* source = NULL;
	gzFile dest = NULL;
	unsigned char buffer[128]= {0};
	int num_read = 0,num_write = 0;
	mode_t prevmask = umask(2);
	int ret = 0;

	source = fopen(input_path, "rb");
	if(source == NULL)
	{
		ALOGE("%s:  can not fopen %s", __func__,input_path);
		return -1;
	}

	dest = gzopen(output_path, "wb");
	if(dest  == NULL){
		ALOGE("%s:  can not gzopen %s", __func__,output_path);
		fclose(source);
		return -1;
	}

	umask(prevmask);

	ALOGE("%s: start gzipCompress...", __func__);
	while ((num_read = fread(buffer, 1, sizeof(buffer), source)) > 0) {
		num_write = gzwrite(dest, buffer, num_read);
		if(num_write <= 0)
		{
			ALOGE("%s: gzwrite err %d , exit", __func__,num_write);
			ret = -1;
			break;
		}
		if(!Flag_gzip_thread)
		{
			ALOGE("%s: Flag_gzip_thread = 0 bt close,now,thread exit", __func__);
			ret = -2;
			break;
		}
	}
	fclose(source);
	gzclose(dest);
	ALOGE("%s: gzipCompress finish", __func__);

	return ret;
	}

void fwlog_gzipCompress(void) {
	std::string logFilePrefix = "fw_log.txt_";
	std::string logPath = get_fw_log_path();
	std::vector<std::string> logFiles;
	std::string logFileDir = logPath.substr(0, logPath.find_last_of('/'));

	DIR* dir = opendir(logFileDir.c_str());
	if (dir) {
		struct dirent* entry;
		while ((entry = readdir(dir)) != nullptr) {
			std::string filename = entry->d_name;
				if (filename.compare(0, logFilePrefix.length(), logFilePrefix) == 0 &&
				!(filename.size() >= 3 && filename.substr(filename.size() - 3) == ".gz")) {
				//ALOGE("%s: find %s", __func__, filename.c_str());
				logFiles.push_back(filename);
			}
		}
		closedir(dir);
	}
	//gzipCompress
	for (auto it = logFiles.begin(); it != logFiles.end(); ) {
		const std::string& filename = *it;
		std::string Filename = logFileDir + "/" + filename;
		std::string gFilename = Filename + ".gz";
		ALOGE("in:%s out:%s",Filename.c_str(),gFilename.c_str());
		// check gzip suscessed?
		if (!gzipCompress(Filename.c_str(), gFilename.c_str())) {
			// gzipCompress suscessed,remove file
			if (remove(Filename.c_str()) == 0) {
				it = logFiles.erase(it);
			} else {
				ALOGE("remove file failed:%s",Filename.c_str());
				++it;
			}
		} else {
			// failed..skip
			ALOGE("%s: gzipCompress %s failed", __func__, Filename.c_str());
			remove(gFilename.c_str()); //remove gz file
			++it;
		}
	}
}

void fwlog_sort_gzfile(void) {
	std::string logPath = get_fw_log_path();
	std::string logFileDir = logPath.substr(0, logPath.find_last_of('/'));
	std::string logFilePrefix = "fw_log.txt_";
	const std::string gzSuffix = ".gz";

	std::vector<int> fileNumbers;
	DIR* dir = opendir(logFileDir.c_str());
	if (dir) {
		struct dirent* entry;
		while ((entry = readdir(dir)) != nullptr) {
			std::string filename = entry->d_name;
			if (filename.find(logFilePrefix) == 0) {
				std::string numPart;
				if (filename.size() > gzSuffix.size() && filename.substr(filename.size() - gzSuffix.size()) == gzSuffix) {
					numPart = filename.substr(logFilePrefix.length(),filename.length() - logFilePrefix.length() - gzSuffix.length());
				}
				char* endPtr = nullptr;
				long fileNum = std::strtol(numPart.c_str(), &endPtr, 10);
				if (endPtr != numPart.c_str() && *endPtr == '\0' && fileNum >= 1) {
					fileNumbers.push_back(static_cast<int>(fileNum));
				} else {
					ALOGE("%s: Invalid log file number in %s", __func__, filename.c_str());
				}
			}
		}
		closedir(dir);
	}

	prop_maxLogFileCount = get_property_as_int32_t(TOGGLE_BT_FW_MAX_FILE_NUM, default_max_file_count);
	ALOGE("%s: Max log files: %d", __func__, prop_maxLogFileCount);

	if (fileNumbers.size() > prop_maxLogFileCount) {
		std::sort(fileNumbers.begin(), fileNumbers.end());
		int filesToDelete = fileNumbers.size() - prop_maxLogFileCount;
		ALOGE("%s: Need to delete %d old log files", __func__, filesToDelete);
		for (int i = 0; i < filesToDelete; i++) {
			int oldestNum = fileNumbers[i];
			std::string oldestBase = logFileDir + "/" + logFilePrefix + std::to_string(oldestNum);
			std::string oldestGz = oldestBase + gzSuffix;
			if (access(oldestGz.c_str(), F_OK) == 0) {
				if (remove(oldestGz.c_str())) {
					ALOGE("%s: Failed to remove %s [%s]", __func__, oldestGz.c_str(), strerror(errno));
				} else {
					ALOGE("%s: Removed compressed log: %s", __func__, oldestGz.c_str());
				}
			}
		}

		std::vector<int> remainingFiles;
		for (size_t i = filesToDelete; i < fileNumbers.size(); i++) {
			remainingFiles.push_back(fileNumbers[i]);
		}
		std::sort(remainingFiles.begin(), remainingFiles.end());
		for (size_t i = 0; i < remainingFiles.size(); i++) {
			int oldNum = remainingFiles[i];
			int newNum = i + 1;
			if (oldNum == newNum) {
				continue;
			}
			std::string oldBase = logFileDir + "/" + logFilePrefix + std::to_string(oldNum);
			std::string newBase = logFileDir + "/" + logFilePrefix + std::to_string(newNum);
			std::string oldGz = oldBase + gzSuffix;
			std::string newGz = newBase + gzSuffix;
			if (access(oldGz.c_str(), F_OK) == 0) {
				if (std::rename(oldGz.c_str(), newGz.c_str()) == 0) {
					ALOGE("%s: Renamed %s to %s", __func__, oldGz.c_str(), newGz.c_str());
				} else {
					ALOGE("%s: Failed to rename %s to %s [%s]",__func__, oldGz.c_str(), newGz.c_str(), strerror(errno));
				}
			}
		}
	}
}


void file_gzip_compress_work_thread(void) {
	ALOGE("%s: file_gzip_compress_work_thread run...", __func__);
	while(Flag_gzip_thread) {
		if(Flag_gzip_start)
		{
			ALOGE("%s: gzip work start...", __func__);
			fwlog_gzipCompress();
			fwlog_sort_gzfile();
			Flag_gzip_start = 0;
			ALOGE("%s: gzip work finish...", __func__);
		} else {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
	ALOGE("%s: file_gzip_compress_work_thread exit...", __func__);
}


void fwlog_init (void) {
	ALOGE("%s %s ",__func__,fwlog_version);
	ALOGE("prop_fw_buff_level:%d prop_dataCount:%d prop_maxLogFileCount:%d ",prop_fw_buff_level,prop_dataCount,prop_maxLogFileCount);

	std::thread t(file_gzip_compress_work_thread);
	t.detach();
	Flag_gzip_thread = 1;

	dataCount = 0;
	updateLogFile();
	if (fwlogfile_fd == INVALID_FD) {
		ALOGE("%s: unable open fwlogfile_fd", __func__);
	}
}

void fwlog_close (void) {
	ALOGE("%s ", __func__);

	if (fwlogfile_fd != INVALID_FD) {
		close(fwlogfile_fd);
	}
	fwlogfile_fd = INVALID_FD;

	Flag_gzip_start = 0;
	Flag_gzip_thread = 0;
}
