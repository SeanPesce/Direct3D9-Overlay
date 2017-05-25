// Author: Sean Pesce
// Original d3d9.dll wrapper base by Michael Koch

#pragma once


#define WIN32_LEAN_AND_MEAN		

#define D3D_DEBUG_INFO 1

#include <Windows.h>
#include <algorithm>
#include "d3d9.h"
#include "SP_IO.hpp"
#include "spIDirect3D9.h"
#include "spIDirect3DDevice9.h"
#include "spIDirect3DSwapChain9.h"

// Standard lifetime (in milliseconds) of an overlay text feed message
#define _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_ 2000

#define _SP_D3D9_LOG_DEFAULT_FILE_ "_sp_d3d9_debug.log"
#define _SP_SP_D3D9_LOG_INIT_(file) {std::string str_tmp_="[";append_current_date_string(&str_tmp_,false,SP_DATE_MMDDYYYY);str_tmp_.append("  ");append_current_timestamp_string(&str_tmp_,false);file_write_text(file,str_tmp_.append("]  Attached to process\n").c_str());}
#define _SP_D3D9_LOG_EV_MSG_BUFF_SIZE_ 128
#define _SP_D3D9_LOG_EVENT_(msg, ...) _SP_D3D9_LOG_SPEC_EVENT_(_SP_D3D9_LOG_DEFAULT_FILE_, msg, ##__VA_ARGS__)
#define _SP_D3D9_LOG_SPEC_EVENT_(file, msg, ...) {std::string str_tmp_="";append_current_timestamp_string(&str_tmp_,true);char c_str_tmp_[_SP_D3D9_LOG_EV_MSG_BUFF_SIZE_];snprintf(c_str_tmp_,_SP_D3D9_LOG_EV_MSG_BUFF_SIZE_,msg, ##__VA_ARGS__);file_append_text(file,str_tmp_.append(" ").append(c_str_tmp_).c_str());}

#ifdef D3D_DEBUG_INFO
	#include <DxErr.h>
	#pragma comment(lib, "dxerr.lib")
	#define _SP_D3D9_CHECK_FAILED_(f) {HRESULT hres_tmp_;if(FAILED(hres_tmp_ = f)){_SP_D3D9_LOG_EVENT_("%s (%s) - Occurred in:  thread %d;  %s (line %d)",DXGetErrorString(hres_tmp_),DXGetErrorDescription(hres_tmp_),GetCurrentThreadId(),__FUNCTION__,__LINE__);}}
	#define _SP_D3D9_CHECK_AND_RETURN_FAILED_(f) {HRESULT hres_tmp_;if(FAILED(hres_tmp_ = f)){_SP_D3D9_LOG_EVENT_("%s (%s) - Occurred in:  thread %d;  %s (line %d)",DXGetErrorString(hres_tmp_),DXGetErrorDescription(hres_tmp_),GetCurrentThreadId(),__FUNCTION__,__LINE__);}return hres_tmp_;}
#else
	#define _SP_D3D9_CHECK_FAILED_(f) {HRESULT hres_tmp_;if(FAILED(hres_tmp_ = f)){_SP_D3D9_LOG_EVENT_("D3D9 ERROR - Occurred in:  thread %d;  %s (line %d)",GetCurrentThreadId(),__FUNCTION__,__LINE__);}}
	#define _SP_D3D9_CHECK_AND_RETURN_FAILED_(f) {HRESULT hres_tmp_;if(FAILED(hres_tmp_ = f)){_SP_D3D9_LOG_EVENT_("D3D9 ERROR - Occurred in:  thread %d;  %s (line %d)",GetCurrentThreadId(),__FUNCTION__,__LINE__);}return hres_tmp_;}
#endif // D3D_DEBUG_INFO