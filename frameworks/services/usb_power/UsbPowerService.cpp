/*
 * Copyright (C) 2023 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *	@author   Hao Qi
 *	@version  1.0
 *	@date	  2024/05/06
 *	@par function description:
 *	1. Get the current current level, parse the nodes and props configured in the Json file,
 *	2. and write the corresponding nodes and props.
 */

#include <iostream>
#include <string>
#include <android-base/macros.h>
#include <android-base/file.h>
#include <android-base/properties.h>
#include <cutils/properties.h>

#define LOG_TAG "usb_power"

#include <fcntl.h>

#include <vector>

#include <thread>

#include <android-base/logging.h>
#include <android-base/stringprintf.h>
#include <android-base/strings.h>
#include <android-base/threads.h>
#include <utils/Log.h>

#include <cutils/android_filesystem_config.h>

#include <json/reader.h>
#include <json/value.h>
#include <fstream>

#include <filesystem>

//writeToFile
bool writeToFile(const char *path, const char *val) {
	//std::cout << "Writing to " << path << ": " << val << std::endl;
	ALOGD("writing to %s : %s\n", path, val);

	const int SIZE = 20;
	int ret, fd, len;
	char value[SIZE];

	if (!path)
		return false;

	fd = open(path, O_WRONLY, 0);
	if (fd < 0) {
		ALOGE("writeToFile: Could not open '%s' err: %d\n", path, errno);
		return false;
	}

	ret = write(fd, val, strlen(val)+1);
	ret = write(fd, "\n", 2);

	close(fd);
	return true;
}

void parseJson(std::istream& inputStream, const std::string& status) {
	Json::Value root;
	Json::Reader reader;

	bool parsingSuccessful = reader.parse(inputStream, root);
	if (!parsingSuccessful) {
		std::cerr << "Failed to parse power_config.json\n"	<< std::endl;
		return;
	}

	Json::Value levelObj = root["levels"][status];
	if (levelObj.isArray()) {
		for (const auto& obj : levelObj) {
			if (obj.isMember("sys_write")) {
				const auto& sysWriteObj = obj["sys_write"];
				writeToFile(sysWriteObj["path"].asString().c_str(), sysWriteObj["value"].asString().c_str());
			} else if (obj.isMember("prop_write")) {
				const auto& propWriteObj = obj["prop_write"];
				property_set(propWriteObj["path"].asString().c_str(), propWriteObj["value"].asString().c_str());
			}
		}
	} else {
		std::cout << "Level " << status << " not found or invalid" << std::endl;
	}
}

int main(int argc, char** argv) {
	//std::cout << "start current_service" << std::endl;
	char buf_cc[PROPERTY_VALUE_MAX];
	char buf_enable[PROPERTY_VALUE_MAX];

	int ret = property_get("ro.boot.cc_enable", buf_enable, "");

	//std::cout << "buf_enable:  " << buf_enable << std::endl;
	if (ret <= 0 || strcmp(buf_enable, "1") != 0) {
		ALOGE("failed to get cc_enable or disable cc\n");
		return 1;
	}
	ret = property_get("ro.boot.cc.status", buf_cc, "");
	if (ret <= 0) {
		ALOGE("failed to get ro.boot.cc.status\n");
		return 1;
	}

	//std::cout << "buf_cc:  " << buf_cc << std::endl;

	std::string config_path = "/vendor/etc/power_config.json";

	std::ifstream file(config_path);
	if (!file.is_open()) {
		ALOGE("hao_qi Failed to open JSON file\n");
		return 1;
	}
	std::string powerStatus;

	if (strcmp(buf_cc, "0.5a@5v") == 0) {
		//ALOGD("The current current is 0.5a\n");
		powerStatus = "low";
	} else if (strcmp(buf_cc, "1.5a@5v") == 0) {
		//ALOGD("The current current is 1.5a\n");
		powerStatus = "middle";
	} else if (strcmp(buf_cc, "3a@5v") == 0) {
		//ALOGD("The current current is 3a\n");
		powerStatus = "high";
	}
	parseJson(file, powerStatus);

	file.close();
	return 0;
}
