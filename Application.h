#pragma once


#include <stdlib.h>

#include "stm32f4xx_hal.h"
#include "gfx.h"
#include "UC1698.h"
#include "math.h"
#include "string.h"

struct application {	
	void(*run)(void);		
};

extern const struct application Application;