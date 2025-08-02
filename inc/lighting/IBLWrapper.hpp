#pragma once
#include "VkBase+.h"
#include "common.h"

class IBLWrapper {
public:
	void LoadHDR(std::string path);
	void IBLSetup();
};