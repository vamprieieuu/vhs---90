#pragma once
#include "AE_Effect.h"
#include <string>
#include <string_view>

PF_Err VHS90CreateOsdTextDefault(PF_InData* in_data, PF_ArbitraryH* out_handle);
PF_Err VHS90OsdArbitraryCallback(PF_InData* in_data, PF_ArbParamsExtra* extra);
PF_Err VHS90ReadOsdText(PF_InData* in_data, PF_ArbitraryH handle, std::string& utf8_text);
PF_Err VHS90WriteOsdText(PF_InData* in_data, PF_ArbitraryH* handle, std::string_view utf8_text);
