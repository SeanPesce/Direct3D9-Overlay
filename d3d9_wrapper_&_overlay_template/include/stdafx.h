// Author: Sean Pesce
// Original d3d9.dll wrapper base by Michael Koch

#pragma once


#define WIN32_LEAN_AND_MEAN		

#include <Windows.h>
#include <algorithm>
#include "d3d9.h"
#include "SP_IO.hpp"
#include "myIDirect3D9.h"
#include "myIDirect3DDevice9.h"
#include "spIDirect3DSwapChain9.h"

// Standard lifetime (in milliseconds) of an overlay text feed message
#define _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_ 2000
