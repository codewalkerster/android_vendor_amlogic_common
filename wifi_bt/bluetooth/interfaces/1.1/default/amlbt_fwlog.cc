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

#define fwlog_version "2025-05-26"

static const int INVALID_FD = -1;
static int fwlogfile_fd = INVALID_FD;

static int32_t dataCount = 0;
static int32_t prop_dataCount = 0;
static int32_t prop_fw_buff_level = 0;
static int32_t prop_maxLogFileCount = 0;

#define TOGGLE_BT_FW_MAX_FILE_NUM   "persist.log.tag.fw_max_file_num_bt"
#define TOGGLE_BT_FW_BUFFER_LEVEL   "persist.log.tag.fw_buffer_bt"

#define default_max_file_count  2      //0: one file
#define default_fw_buff_level  300 //300MB/411B = 765383

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
	int32_t tmp_prop_fw_buff_level = 0;
	int32_t one_package_size = 0;

	writeHexData(fwlogfile_fd, data,  2);
	dataCount++;
	tmp_prop_fw_buff_level =  get_property_as_int32_t(TOGGLE_BT_FW_BUFFER_LEVEL,default_fw_buff_level);
	if(tmp_prop_fw_buff_level != prop_fw_buff_level )
	{
	   ALOGE("prop_fw_buff_level %d M change to %d M ",prop_fw_buff_level,tmp_prop_fw_buff_level);
	   prop_fw_buff_level = tmp_prop_fw_buff_level;
	   one_package_size = (data.size()-2) * 3 + 27;
	   prop_dataCount = (prop_fw_buff_level * 1024 * 1024) / one_package_size;
	   ALOGE("one_package_size:%d  prop_dataCount:%d  dataCount:%d ",one_package_size,prop_dataCount,dataCount);
	}
	if (dataCount >= prop_dataCount)
	{
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

	prop_maxLogFileCount = get_property_as_int32_t(TOGGLE_BT_FW_MAX_FILE_NUM,default_max_file_count);
	ALOGE("%s prop_maxLogFileCount:%d ",__func__,prop_maxLogFileCount);

	if (logFiles.size() >= prop_maxLogFileCount) {
		int32_t deleteCount = logFiles.size() - prop_maxLogFileCount +1;
		std::sort(logFiles.begin(), logFiles.end(), compareFilesByNumber);

		for (size_t i = 0; i < deleteCount; ++i)
		{
	        std::string filename = logFiles[i];
	        std::string oldestFile = logFileDir + "/" + filename;
	        remove(oldestFile.c_str());
	        ALOGE("%s: Removing %s", __func__, oldestFile.c_str());
        }

		logFiles.erase(logFiles.begin(), logFiles.begin() + deleteCount);
		/*
		std::string oldestFile = logFileDir + "/" + logFiles.front();
		ALOGE("%s: Removing %s", __func__, oldestFile.c_str());
		remove(oldestFile.c_str());
		logFiles.erase(logFiles.begin());
		*/

	}

    if (std::rename(logPath.c_str(), newFilename.c_str()) != 0) {
        ALOGE("%s: Unable to rename fw_log.txt to %s, errno: %s", __func__, newFilename.c_str(), strerror(errno));
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
		    ret = -1;
		    break;
		}
    }
    fclose(source);
    gzclose(dest);
	ALOGE("%s: gzipCompress finish", __func__);

	return ret;
}


void file_gzip_compress_work_thread(void)
{
	std::string logFilePrefix = "fw_log.txt_";
	std::string logPath = get_fw_log_path();
	std::vector<std::string> logFiles;
	std::string logFileDir = logPath.substr(0, logPath.find_last_of('/'));

	ALOGE("%s: file_gzip_compress_work_thread run...", __func__);

	while(Flag_gzip_thread){
		if(Flag_gzip_start)
		{

			ALOGE("%s: gzip file work start...", __func__);

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

			Flag_gzip_start = 0;
			ALOGE("%s: gzip file work finish...", __func__);
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
	if (fwlogfile_fd == INVALID_FD)
	ALOGE("%s: unable open fwlogfile_fd", __func__);
}

void fwlog_close (void) {
	ALOGE("%s ", __func__);

	if (fwlogfile_fd != INVALID_FD) close(fwlogfile_fd);
    fwlogfile_fd = INVALID_FD;

	Flag_gzip_start = 0;
	Flag_gzip_thread = 0;

}
