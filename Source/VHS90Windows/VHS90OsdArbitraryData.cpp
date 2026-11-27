#include "VHS90OsdArbitraryData.h"
#include "AE_EffectCBSuites.h"
#include "VHS90OsdTextData.h"
#include <algorithm>
#include <cstring>
#include <string_view>
#include <vector>

namespace {
constexpr A_short kOsdTextArbId = 9013;
constexpr A_HandleSize kEmptyBlobSize = 12;

class HandleSuiteScope {
public:
    explicit HandleSuiteScope(PF_InData* in) noexcept : basic_(in ? in->pica_basicP : nullptr) {
        if (basic_ && basic_->AcquireSuite(kPFHandleSuite, kPFHandleSuiteVersion1,
            reinterpret_cast<const void**>(&suite_)) == 0) acquired_ = true;
        else suite_ = nullptr;
    }
    ~HandleSuiteScope() { if (acquired_ && basic_) basic_->ReleaseSuite(kPFHandleSuite,kPFHandleSuiteVersion1); }
    const PF_HandleSuite1* get() const noexcept { return suite_; }
private:
    SPBasicSuite* basic_ = nullptr;
    const PF_HandleSuite1* suite_ = nullptr;
    bool acquired_ = false;
};

bool readBlob(const PF_HandleSuite1* hs, PF_ArbitraryH handle, std::vector<std::uint8_t>& out) {
    if (!hs || !handle) return false;
    const A_HandleSize n=hs->host_get_handle_size(handle);
    if (n<kEmptyBlobSize || n>static_cast<A_HandleSize>(vhs90::kOsdTextMaxUtf8Bytes+12)) return false;
    void* p=hs->host_lock_handle(handle);
    if (!p) return false;
    try { const auto* b=static_cast<const std::uint8_t*>(p); out.assign(b,b+static_cast<std::size_t>(n)); }
    catch (...) { hs->host_unlock_handle(handle); return false; }
    hs->host_unlock_handle(handle);
    vhs90::OsdTextData check;
    return vhs90::unflattenOsdText(out.data(),out.size(),check);
}
PF_ArbitraryH makeBlob(const PF_HandleSuite1* hs,const std::vector<std::uint8_t>& bytes) {
    if (!hs || bytes.empty()) return nullptr;
    PF_ArbitraryH h=hs->host_new_handle(static_cast<A_HandleSize>(bytes.size()));
    if (!h) return nullptr;
    void* p=hs->host_lock_handle(h);
    if (!p) { hs->host_dispose_handle(h); return nullptr; }
    std::memcpy(p,bytes.data(),bytes.size());
    hs->host_unlock_handle(h);
    return h;
}
bool makeEmptyBytes(std::vector<std::uint8_t>& bytes) {
    vhs90::OsdTextData empty;
    return vhs90::flattenOsdText(empty,bytes);
}
PF_Err callbackNew(const PF_HandleSuite1* hs,PF_ArbParamsExtra* e) {
    std::vector<std::uint8_t> bytes;
    if (!makeEmptyBytes(bytes)) return PF_Err_INTERNAL_STRUCT_DAMAGED;
    *e->u.new_func_params.arbPH=makeBlob(hs,bytes);
    return *e->u.new_func_params.arbPH ? PF_Err_NONE : PF_Err_OUT_OF_MEMORY;
}
PF_Err callbackCopy(const PF_HandleSuite1* hs,PF_ArbParamsExtra* e) {
    std::vector<std::uint8_t> bytes;
    if (!readBlob(hs,e->u.copy_func_params.src_arbH,bytes)) return PF_Err_INTERNAL_STRUCT_DAMAGED;
    *e->u.copy_func_params.dst_arbPH=makeBlob(hs,bytes);
    return *e->u.copy_func_params.dst_arbPH ? PF_Err_NONE : PF_Err_OUT_OF_MEMORY;
}
PF_Err callbackUnflatten(const PF_HandleSuite1* hs,PF_ArbParamsExtra* e) {
    vhs90::OsdTextData parsed;
    if (!vhs90::unflattenOsdText(e->u.unflatten_func_params.flat_dataPV,
            static_cast<std::size_t>(e->u.unflatten_func_params.buf_sizeLu),parsed))
        return PF_Err_INTERNAL_STRUCT_DAMAGED;
    std::vector<std::uint8_t> bytes;
    if (!vhs90::flattenOsdText(parsed,bytes)) return PF_Err_INTERNAL_STRUCT_DAMAGED;
    *e->u.unflatten_func_params.arbPH=makeBlob(hs,bytes);
    return *e->u.unflatten_func_params.arbPH ? PF_Err_NONE : PF_Err_OUT_OF_MEMORY;
}
}

PF_Err VHS90CreateOsdTextDefault(PF_InData* in_data,PF_ArbitraryH* out_handle) {
    if (!in_data || !out_handle) return PF_Err_BAD_CALLBACK_PARAM;
    *out_handle=nullptr;
    HandleSuiteScope scope(in_data);
    const auto* hs=scope.get();
    if (!hs) return PF_Err_INTERNAL_STRUCT_DAMAGED;
    std::vector<std::uint8_t> bytes;
    if (!makeEmptyBytes(bytes)) return PF_Err_INTERNAL_STRUCT_DAMAGED;
    *out_handle=makeBlob(hs,bytes);
    return *out_handle ? PF_Err_NONE : PF_Err_OUT_OF_MEMORY;
}

PF_Err VHS90ReadOsdText(PF_InData* in_data, PF_ArbitraryH handle, std::string& utf8_text) {
    if (!in_data || !handle) { utf8_text.clear(); return PF_Err_NONE; }
    HandleSuiteScope scope(in_data);
    const auto* hs = scope.get();
    if (!hs) return PF_Err_INTERNAL_STRUCT_DAMAGED;
    std::vector<std::uint8_t> bytes;
    if (!readBlob(hs, handle, bytes)) return PF_Err_INTERNAL_STRUCT_DAMAGED;
    vhs90::OsdTextData parsed;
    if (!vhs90::unflattenOsdText(bytes.data(), bytes.size(), parsed))
        return PF_Err_INTERNAL_STRUCT_DAMAGED;
    try { utf8_text = std::move(parsed.utf8); }
    catch (...) { return PF_Err_OUT_OF_MEMORY; }
    return PF_Err_NONE;
}

PF_Err VHS90CreateOsdText(PF_InData* in_data,const std::string& utf8_text,PF_ArbitraryH* out_handle) {
    if(!in_data || !out_handle) return PF_Err_BAD_CALLBACK_PARAM;
    *out_handle=nullptr;
    vhs90::OsdTextData parsed;
    if(!vhs90::setOsdText(parsed,utf8_text)) return PF_Err_BAD_CALLBACK_PARAM;
    std::vector<std::uint8_t> bytes;
    if(!vhs90::flattenOsdText(parsed,bytes)) return PF_Err_BAD_CALLBACK_PARAM;
    HandleSuiteScope scope(in_data);
    const auto* hs=scope.get();
    if(!hs) return PF_Err_INTERNAL_STRUCT_DAMAGED;
    *out_handle=makeBlob(hs,bytes);
    return *out_handle ? PF_Err_NONE : PF_Err_OUT_OF_MEMORY;
}

PF_Err VHS90OsdArbitraryCallback(PF_InData* in_data,PF_ArbParamsExtra* e) {
    if (!in_data || !e || e->id!=kOsdTextArbId) return PF_Err_BAD_CALLBACK_PARAM;
    HandleSuiteScope scope(in_data);
    const auto* hs=scope.get();
    if (!hs) return PF_Err_INTERNAL_STRUCT_DAMAGED;
    switch (e->which_function) {
        case PF_Arbitrary_NEW_FUNC: return callbackNew(hs,e);
        case PF_Arbitrary_DISPOSE_FUNC:
            if (e->u.dispose_func_params.arbH) hs->host_dispose_handle(e->u.dispose_func_params.arbH);
            return PF_Err_NONE;
        case PF_Arbitrary_COPY_FUNC: return callbackCopy(hs,e);
        case PF_Arbitrary_FLAT_SIZE_FUNC: {
            std::vector<std::uint8_t> b;
            if (!readBlob(hs,e->u.flat_size_func_params.arbH,b)) return PF_Err_INTERNAL_STRUCT_DAMAGED;
            *e->u.flat_size_func_params.flat_data_sizePLu=static_cast<A_u_long>(b.size());
            return PF_Err_NONE;
        }
        case PF_Arbitrary_FLATTEN_FUNC: {
            std::vector<std::uint8_t> b;
            if (!readBlob(hs,e->u.flatten_func_params.arbH,b) ||
                e->u.flatten_func_params.buf_sizeLu!=b.size() || !e->u.flatten_func_params.flat_dataPV)
                return PF_Err_INTERNAL_STRUCT_DAMAGED;
            std::memcpy(e->u.flatten_func_params.flat_dataPV,b.data(),b.size());
            return PF_Err_NONE;
        }
        case PF_Arbitrary_UNFLATTEN_FUNC: return callbackUnflatten(hs,e);
        case PF_Arbitrary_INTERP_FUNC: {
            std::vector<std::uint8_t> b;
            const PF_ArbitraryH source = (e->u.interp_func_params.tF >= 1.0)
                ? e->u.interp_func_params.right_arbH : e->u.interp_func_params.left_arbH;
            if (!readBlob(hs,source,b)) return PF_Err_INTERNAL_STRUCT_DAMAGED;
            *e->u.interp_func_params.interpPH=makeBlob(hs,b);
            return *e->u.interp_func_params.interpPH ? PF_Err_NONE : PF_Err_OUT_OF_MEMORY;
        }
        case PF_Arbitrary_COMPARE_FUNC: {
            std::vector<std::uint8_t> a,b;
            if (!readBlob(hs,e->u.compare_func_params.a_arbH,a) ||
                !readBlob(hs,e->u.compare_func_params.b_arbH,b)) return PF_Err_INTERNAL_STRUCT_DAMAGED;
            *e->u.compare_func_params.compareP=(a==b)?PF_ArbCompare_EQUAL:PF_ArbCompare_NOT_EQUAL;
            return PF_Err_NONE;
        }
        case PF_Arbitrary_PRINT_SIZE_FUNC: {
            std::vector<std::uint8_t> b;
            if (!readBlob(hs,e->u.print_size_func_params.arbH,b)) return PF_Err_INTERNAL_STRUCT_DAMAGED;
            vhs90::OsdTextData d;
            if (!vhs90::unflattenOsdText(b.data(),b.size(),d)) return PF_Err_INTERNAL_STRUCT_DAMAGED;
            *e->u.print_size_func_params.print_sizePLu=static_cast<A_u_long>(d.utf8.size()+1);
            return PF_Err_NONE;
        }
        case PF_Arbitrary_PRINT_FUNC: {
            std::vector<std::uint8_t> b;
            if (!readBlob(hs,e->u.print_func_params.arbH,b)) return PF_Err_INTERNAL_STRUCT_DAMAGED;
            vhs90::OsdTextData d;
            if (!vhs90::unflattenOsdText(b.data(),b.size(),d)) return PF_Err_INTERNAL_STRUCT_DAMAGED;
            const auto required=static_cast<A_u_long>(d.utf8.size()+1);
            if (!e->u.print_func_params.print_bufferPC || e->u.print_func_params.print_sizeLu<required)
                return PF_Err_INTERNAL_STRUCT_DAMAGED;
            std::memcpy(e->u.print_func_params.print_bufferPC,d.utf8.c_str(),required);
            return PF_Err_NONE;
        }
        case PF_Arbitrary_SCAN_FUNC: {
            const auto* p=e->u.scan_func_params.bufPC;
            const auto n=static_cast<std::size_t>(e->u.scan_func_params.bytes_to_scanLu);
            if (!p || n>vhs90::kOsdTextMaxUtf8Bytes) return PF_Err_INTERNAL_STRUCT_DAMAGED;
            vhs90::OsdTextData d;
            if (!vhs90::setOsdText(d,std::string_view(p,n))) return PF_Err_INTERNAL_STRUCT_DAMAGED;
            std::vector<std::uint8_t> b;
            if (!vhs90::flattenOsdText(d,b)) return PF_Err_INTERNAL_STRUCT_DAMAGED;
            *e->u.scan_func_params.arbPH=makeBlob(hs,b);
            return *e->u.scan_func_params.arbPH ? PF_Err_NONE : PF_Err_OUT_OF_MEMORY;
        }
        default: return PF_Err_INTERNAL_STRUCT_DAMAGED;
    }
}
