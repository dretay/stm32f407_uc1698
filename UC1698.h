#pragma once


#include <stdlib.h>
#include "types.h"
#include "main.h"
#include "stm32f4xx_hal.h"

struct uc1698 {	
	void(*init)(void);		
};

extern const struct uc1698 UC1698;