typedef unsigned char   undefined;

typedef unsigned char    bool;
typedef unsigned char    byte;
typedef unsigned int    dword;
typedef unsigned long long    GUID;
typedef pointer32 ImageBaseOffset32;

typedef long long    int64_t;
typedef long long    longlong;
typedef unsigned long long    qword;
typedef unsigned char    uchar;
typedef unsigned int    uint;
typedef unsigned long long    uint64_t;
typedef unsigned long    ulong;
typedef unsigned long long    ulonglong;
typedef unsigned char    undefined1;
typedef unsigned short    undefined2;
typedef unsigned int    undefined4;
typedef unsigned long long    undefined8;
typedef unsigned short    ushort;
typedef unsigned short    wchar16;
typedef unsigned int    wchar32;
typedef short    wchar_t;
typedef unsigned short    word;
#define unkbyte9   unsigned long long
#define unkbyte10   unsigned long long
#define unkbyte11   unsigned long long
#define unkbyte12   unsigned long long
#define unkbyte13   unsigned long long
#define unkbyte14   unsigned long long
#define unkbyte15   unsigned long long
#define unkbyte16   unsigned long long

#define unkuint9   unsigned long long
#define unkuint10   unsigned long long
#define unkuint11   unsigned long long
#define unkuint12   unsigned long long
#define unkuint13   unsigned long long
#define unkuint14   unsigned long long
#define unkuint15   unsigned long long
#define unkuint16   unsigned long long

#define unkint9   long long
#define unkint10   long long
#define unkint11   long long
#define unkint12   long long
#define unkint13   long long
#define unkint14   long long
#define unkint15   long long
#define unkint16   long long

#define unkfloat1   float
#define unkfloat2   float
#define unkfloat3   float
#define unkfloat5   double
#define unkfloat6   double
#define unkfloat7   double
#define unkfloat9   long double
#define unkfloat11   long double
#define unkfloat12   long double
#define unkfloat13   long double
#define unkfloat14   long double
#define unkfloat15   long double
#define unkfloat16   long double

#define BADSPACEBASE   void
#define code   void

typedef struct _s__RTTIBaseClassDescriptor _s__RTTIBaseClassDescriptor, *P_s__RTTIBaseClassDescriptor;

typedef struct _s__RTTIBaseClassDescriptor RTTIBaseClassDescriptor;

typedef RTTIBaseClassDescriptor *RTTIBaseClassDescriptor *32 __((image-base-relative));

typedef RTTIBaseClassDescriptor *32 __((image-base-relative)) *RTTIBaseClassDescriptor *32 __((image-base-relative)) *32 __((image-base-relative));

typedef struct PMD PMD, *PPMD;

struct PMD {
    int mdisp;
    int pdisp;
    int vdisp;
};

struct _s__RTTIBaseClassDescriptor {
    ImageBaseOffset32 pTypeDescriptor; // ref to TypeDescriptor (RTTI 0) for class
    dword numContainedBases; // count of extended classes in BaseClassArray (RTTI 2)
    struct PMD where; // member displacement structure
    dword attributes; // bit flags
    ImageBaseOffset32 pClassHierarchyDescriptor; // ref to ClassHierarchyDescriptor (RTTI 3) for class
};

typedef struct _s_IPToStateMapEntry _s_IPToStateMapEntry, *P_s_IPToStateMapEntry;

typedef struct _s_IPToStateMapEntry IPToStateMapEntry;

typedef int __ehstate_t;

struct _s_IPToStateMapEntry {
    ImageBaseOffset32 Ip;
    __ehstate_t state;
};

typedef union IMAGE_RESOURCE_DIRECTORY_ENTRY_DirectoryUnion IMAGE_RESOURCE_DIRECTORY_ENTRY_DirectoryUnion, *PIMAGE_RESOURCE_DIRECTORY_ENTRY_DirectoryUnion;

typedef struct IMAGE_RESOURCE_DIRECTORY_ENTRY_DirectoryStruct IMAGE_RESOURCE_DIRECTORY_ENTRY_DirectoryStruct, *PIMAGE_RESOURCE_DIRECTORY_ENTRY_DirectoryStruct;

struct IMAGE_RESOURCE_DIRECTORY_ENTRY_DirectoryStruct {
    dword OffsetToDirectory:31;
    dword DataIsDirectory:1;
};

union IMAGE_RESOURCE_DIRECTORY_ENTRY_DirectoryUnion {
    dword OffsetToData;
    struct IMAGE_RESOURCE_DIRECTORY_ENTRY_DirectoryStruct IMAGE_RESOURCE_DIRECTORY_ENTRY_DirectoryStruct;
};

typedef struct _s__RTTIClassHierarchyDescriptor _s__RTTIClassHierarchyDescriptor, *P_s__RTTIClassHierarchyDescriptor;

struct _s__RTTIClassHierarchyDescriptor {
    dword signature;
    dword attributes; // bit flags
    dword numBaseClasses; // number of base classes (i.e. rtti1Count)
    RTTIBaseClassDescriptor *32 __((image-base-relative)) *32 __((image-base-relative)) pBaseClassArray; // ref to BaseClassArray (RTTI 2)
};

typedef struct _s_FuncInfo _s_FuncInfo, *P_s_FuncInfo;

struct _s_FuncInfo {
    uint magicNumber_and_bbtFlags;
    __ehstate_t maxState;
    ImageBaseOffset32 dispUnwindMap;
    uint nTryBlocks;
    ImageBaseOffset32 dispTryBlockMap;
    uint nIPMapEntries;
    ImageBaseOffset32 dispIPToStateMap;
    int dispUnwindHelp;
    ImageBaseOffset32 dispESTypeList;
    int EHFlags;
};

typedef struct _s_UnwindMapEntry _s_UnwindMapEntry, *P_s_UnwindMapEntry;

struct _s_UnwindMapEntry {
    __ehstate_t toState;
    ImageBaseOffset32 action;
};

typedef struct _s__RTTICompleteObjectLocator _s__RTTICompleteObjectLocator, *P_s__RTTICompleteObjectLocator;

struct _s__RTTICompleteObjectLocator {
    dword signature;
    dword offset; // offset of vbtable within class
    dword cdOffset; // constructor displacement offset
    ImageBaseOffset32 pTypeDescriptor; // ref to TypeDescriptor (RTTI 0) for class
    ImageBaseOffset32 pClassDescriptor; // ref to ClassHierarchyDescriptor (RTTI 3)
};

typedef struct _s_UnwindMapEntry UnwindMapEntry;

typedef struct CLIENT_ID CLIENT_ID, *PCLIENT_ID;

struct CLIENT_ID {
    void *UniqueProcess;
    void *UniqueThread;
};

typedef struct _s__RTTIClassHierarchyDescriptor RTTIClassHierarchyDescriptor;

typedef struct _s_FuncInfo FuncInfo;

typedef uint64_t __uint64;

typedef struct _s__RTTICompleteObjectLocator RTTICompleteObjectLocator;

typedef struct tagMSG tagMSG, *PtagMSG;

typedef struct tagMSG MSG;

typedef struct HWND__ HWND__, *PHWND__;

typedef struct HWND__ *HWND;

typedef uint UINT;

typedef ulonglong UINT_PTR;

typedef UINT_PTR WPARAM;

typedef longlong LONG_PTR;

typedef LONG_PTR LPARAM;

typedef ulong DWORD;

typedef struct tagPOINT tagPOINT, *PtagPOINT;

typedef struct tagPOINT POINT;

typedef long LONG;

struct tagMSG {
    HWND hwnd;
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
    DWORD time;
    POINT pt;
};

struct HWND__ {
    int unused;
};

struct tagPOINT {
    LONG x;
    LONG y;
};

typedef struct tagWNDCLASSEXA tagWNDCLASSEXA, *PtagWNDCLASSEXA;

typedef struct tagWNDCLASSEXA WNDCLASSEXA;

typedef LONG_PTR LRESULT;

typedef LRESULT (*WNDPROC)(HWND, UINT, WPARAM, LPARAM);

typedef struct HINSTANCE__ HINSTANCE__, *PHINSTANCE__;

typedef struct HINSTANCE__ *HINSTANCE;

typedef struct HICON__ HICON__, *PHICON__;

typedef struct HICON__ *HICON;

typedef HICON HCURSOR;

typedef struct HBRUSH__ HBRUSH__, *PHBRUSH__;

typedef struct HBRUSH__ *HBRUSH;

typedef char CHAR;

typedef CHAR *LPCSTR;

struct tagWNDCLASSEXA {
    UINT cbSize;
    UINT style;
    WNDPROC lpfnWndProc;
    int cbClsExtra;
    int cbWndExtra;
    HINSTANCE hInstance;
    HICON hIcon;
    HCURSOR hCursor;
    HBRUSH hbrBackground;
    LPCSTR lpszMenuName;
    LPCSTR lpszClassName;
    HICON hIconSm;
};

struct HINSTANCE__ {
    int unused;
};

struct HICON__ {
    int unused;
};

struct HBRUSH__ {
    int unused;
};

typedef struct tagMSG *LPMSG;

typedef struct exception exception, *Pexception;

struct exception { // PlaceHolder Class Structure
};

typedef struct basic_filebuf<char,std::char_traits<char>_> basic_filebuf<char,std::char_traits<char>_>, *Pbasic_filebuf<char,std::char_traits<char>_>;

struct basic_filebuf<char,std::char_traits<char>_> { // PlaceHolder Class Structure
};

typedef struct ios_base ios_base, *Pios_base;

struct ios_base { // PlaceHolder Class Structure
};

typedef struct ctype<char> ctype<char>, *Pctype<char>;

struct ctype<char> { // PlaceHolder Class Structure
};

typedef struct _Locimp _Locimp, *P_Locimp;

struct _Locimp { // PlaceHolder Class Structure
};

typedef struct _cpinfo _cpinfo, *P_cpinfo;

typedef uchar BYTE;

struct _cpinfo {
    UINT MaxCharSize;
    BYTE DefaultChar[2];
    BYTE LeadByte[12];
};

typedef int BOOL;

typedef wchar_t WCHAR;

typedef WCHAR *LPWSTR;

typedef BOOL (*LOCALE_ENUMPROCW)(LPWSTR);

typedef struct _nlsversioninfo _nlsversioninfo, *P_nlsversioninfo;

typedef struct _nlsversioninfo *LPNLSVERSIONINFO;

struct _nlsversioninfo {
    DWORD dwNLSVersionInfoSize;
    DWORD dwNLSVersion;
    DWORD dwDefinedVersion;
};

typedef struct _cpinfo *LPCPINFO;

typedef DWORD LCTYPE;

typedef struct _OVERLAPPED _OVERLAPPED, *P_OVERLAPPED;

typedef ulonglong ULONG_PTR;

typedef union _union_540 _union_540, *P_union_540;

typedef void *HANDLE;

typedef struct _struct_541 _struct_541, *P_struct_541;

typedef void *PVOID;

struct _struct_541 {
    DWORD Offset;
    DWORD OffsetHigh;
};

union _union_540 {
    struct _struct_541 s;
    PVOID Pointer;
};

struct _OVERLAPPED {
    ULONG_PTR Internal;
    ULONG_PTR InternalHigh;
    union _union_540 u;
    HANDLE hEvent;
};

typedef struct _SECURITY_ATTRIBUTES _SECURITY_ATTRIBUTES, *P_SECURITY_ATTRIBUTES;

typedef void *LPVOID;

struct _SECURITY_ATTRIBUTES {
    DWORD nLength;
    LPVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
};

typedef struct _SYSTEMTIME _SYSTEMTIME, *P_SYSTEMTIME;

typedef struct _SYSTEMTIME SYSTEMTIME;

typedef ushort WORD;

struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
};

typedef enum _FINDEX_INFO_LEVELS {
    FindExInfoStandard=0,
    FindExInfoBasic=1,
    FindExInfoMaxInfoLevel=2
} _FINDEX_INFO_LEVELS;

typedef struct _TIME_ZONE_INFORMATION _TIME_ZONE_INFORMATION, *P_TIME_ZONE_INFORMATION;

typedef struct _TIME_ZONE_INFORMATION *LPTIME_ZONE_INFORMATION;

struct _TIME_ZONE_INFORMATION {
    LONG Bias;
    WCHAR StandardName[32];
    SYSTEMTIME StandardDate;
    LONG StandardBias;
    WCHAR DaylightName[32];
    SYSTEMTIME DaylightDate;
    LONG DaylightBias;
};

typedef struct _STARTUPINFOW _STARTUPINFOW, *P_STARTUPINFOW;

typedef BYTE *LPBYTE;

struct _STARTUPINFOW {
    DWORD cb;
    LPWSTR lpReserved;
    LPWSTR lpDesktop;
    LPWSTR lpTitle;
    DWORD dwX;
    DWORD dwY;
    DWORD dwXSize;
    DWORD dwYSize;
    DWORD dwXCountChars;
    DWORD dwYCountChars;
    DWORD dwFillAttribute;
    DWORD dwFlags;
    WORD wShowWindow;
    WORD cbReserved2;
    LPBYTE lpReserved2;
    HANDLE hStdInput;
    HANDLE hStdOutput;
    HANDLE hStdError;
};

typedef struct _STARTUPINFOW *LPSTARTUPINFOW;

typedef struct _WIN32_FIND_DATAW _WIN32_FIND_DATAW, *P_WIN32_FIND_DATAW;

typedef struct _WIN32_FIND_DATAW *LPWIN32_FIND_DATAW;

typedef struct _FILETIME _FILETIME, *P_FILETIME;

typedef struct _FILETIME FILETIME;

struct _WIN32_FIND_DATAW {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD dwReserved0;
    DWORD dwReserved1;
    WCHAR cFileName[260];
    WCHAR cAlternateFileName[14];
};

struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
};

typedef DWORD (*PTHREAD_START_ROUTINE)(LPVOID);

typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;

typedef struct _OVERLAPPED *LPOVERLAPPED;

typedef enum _FINDEX_SEARCH_OPS {
    FindExSearchNameMatch=0,
    FindExSearchLimitToDirectories=1,
    FindExSearchLimitToDevices=2,
    FindExSearchMaxSearchOp=3
} _FINDEX_SEARCH_OPS;

typedef enum _FINDEX_SEARCH_OPS FINDEX_SEARCH_OPS;

typedef struct _SECURITY_ATTRIBUTES *LPSECURITY_ATTRIBUTES;

typedef enum _FINDEX_INFO_LEVELS FINDEX_INFO_LEVELS;

typedef struct _RTL_CRITICAL_SECTION _RTL_CRITICAL_SECTION, *P_RTL_CRITICAL_SECTION;

typedef struct _RTL_CRITICAL_SECTION *PRTL_CRITICAL_SECTION;

typedef PRTL_CRITICAL_SECTION LPCRITICAL_SECTION;

typedef struct _RTL_CRITICAL_SECTION_DEBUG _RTL_CRITICAL_SECTION_DEBUG, *P_RTL_CRITICAL_SECTION_DEBUG;

typedef struct _RTL_CRITICAL_SECTION_DEBUG *PRTL_CRITICAL_SECTION_DEBUG;

typedef struct _LIST_ENTRY _LIST_ENTRY, *P_LIST_ENTRY;

typedef struct _LIST_ENTRY LIST_ENTRY;

struct _RTL_CRITICAL_SECTION {
    PRTL_CRITICAL_SECTION_DEBUG DebugInfo;
    LONG LockCount;
    LONG RecursionCount;
    HANDLE OwningThread;
    HANDLE LockSemaphore;
    ULONG_PTR SpinCount;
};

struct _RTL_CRITICAL_SECTION_DEBUG {
    WORD Type;
    WORD CreatorBackTraceIndex;
    struct _RTL_CRITICAL_SECTION *CriticalSection;
    LIST_ENTRY ProcessLocksList;
    DWORD EntryCount;
    DWORD ContentionCount;
    DWORD Flags;
    WORD CreatorBackTraceIndexHigh;
    WORD SpareWORD;
};

struct _LIST_ENTRY {
    struct _LIST_ENTRY *Flink;
    struct _LIST_ENTRY *Blink;
};

typedef struct _EXCEPTION_POINTERS _EXCEPTION_POINTERS, *P_EXCEPTION_POINTERS;

typedef LONG (*PTOP_LEVEL_EXCEPTION_FILTER)(struct _EXCEPTION_POINTERS *);

typedef struct _EXCEPTION_RECORD _EXCEPTION_RECORD, *P_EXCEPTION_RECORD;

typedef struct _EXCEPTION_RECORD EXCEPTION_RECORD;

typedef EXCEPTION_RECORD *PEXCEPTION_RECORD;

typedef struct _CONTEXT _CONTEXT, *P_CONTEXT;

typedef struct _CONTEXT *PCONTEXT;

typedef ulonglong DWORD64;

typedef union _union_54 _union_54, *P_union_54;

typedef struct _M128A _M128A, *P_M128A;

typedef struct _M128A M128A;

typedef struct _XSAVE_FORMAT _XSAVE_FORMAT, *P_XSAVE_FORMAT;

typedef struct _XSAVE_FORMAT XSAVE_FORMAT;

typedef XSAVE_FORMAT XMM_SAVE_AREA32;

typedef struct _struct_55 _struct_55, *P_struct_55;

typedef ulonglong ULONGLONG;

typedef longlong LONGLONG;

struct _EXCEPTION_POINTERS {
    PEXCEPTION_RECORD ExceptionRecord;
    PCONTEXT ContextRecord;
};

struct _EXCEPTION_RECORD {
    DWORD ExceptionCode;
    DWORD ExceptionFlags;
    struct _EXCEPTION_RECORD *ExceptionRecord;
    PVOID ExceptionAddress;
    DWORD NumberParameters;
    ULONG_PTR ExceptionInformation[15];
};

struct _struct_55 {
    M128A Header[2];
    M128A Legacy[8];
    M128A Xmm0;
    M128A Xmm1;
    M128A Xmm2;
    M128A Xmm3;
    M128A Xmm4;
    M128A Xmm5;
    M128A Xmm6;
    M128A Xmm7;
    M128A Xmm8;
    M128A Xmm9;
    M128A Xmm10;
    M128A Xmm11;
    M128A Xmm12;
    M128A Xmm13;
    M128A Xmm14;
    M128A Xmm15;
};

struct _M128A {
    ULONGLONG Low;
    LONGLONG High;
};

struct _XSAVE_FORMAT {
    WORD ControlWord;
    WORD StatusWord;
    BYTE TagWord;
    BYTE Reserved1;
    WORD ErrorOpcode;
    DWORD ErrorOffset;
    WORD ErrorSelector;
    WORD Reserved2;
    DWORD DataOffset;
    WORD DataSelector;
    WORD Reserved3;
    DWORD MxCsr;
    DWORD MxCsr_Mask;
    M128A FloatRegisters[8];
    M128A XmmRegisters[16];
    BYTE Reserved4[96];
};

union _union_54 {
    XMM_SAVE_AREA32 FltSave;
    struct _struct_55 s;
};

struct _CONTEXT {
    DWORD64 P1Home;
    DWORD64 P2Home;
    DWORD64 P3Home;
    DWORD64 P4Home;
    DWORD64 P5Home;
    DWORD64 P6Home;
    DWORD ContextFlags;
    DWORD MxCsr;
    WORD SegCs;
    WORD SegDs;
    WORD SegEs;
    WORD SegFs;
    WORD SegGs;
    WORD SegSs;
    DWORD EFlags;
    DWORD64 Dr0;
    DWORD64 Dr1;
    DWORD64 Dr2;
    DWORD64 Dr3;
    DWORD64 Dr6;
    DWORD64 Dr7;
    DWORD64 Rax;
    DWORD64 Rcx;
    DWORD64 Rdx;
    DWORD64 Rbx;
    DWORD64 Rsp;
    DWORD64 Rbp;
    DWORD64 Rsi;
    DWORD64 Rdi;
    DWORD64 R8;
    DWORD64 R9;
    DWORD64 R10;
    DWORD64 R11;
    DWORD64 R12;
    DWORD64 R13;
    DWORD64 R14;
    DWORD64 R15;
    DWORD64 Rip;
    union _union_54 u;
    M128A VectorRegister[26];
    DWORD64 VectorControl;
    DWORD64 DebugControl;
    DWORD64 LastBranchToRip;
    DWORD64 LastBranchFromRip;
    DWORD64 LastExceptionToRip;
    DWORD64 LastExceptionFromRip;
};

typedef struct _SYSTEMTIME *LPSYSTEMTIME;

typedef PTOP_LEVEL_EXCEPTION_FILTER LPTOP_LEVEL_EXCEPTION_FILTER;

typedef union _LARGE_INTEGER _LARGE_INTEGER, *P_LARGE_INTEGER;

typedef struct _struct_19 _struct_19, *P_struct_19;

typedef struct _struct_20 _struct_20, *P_struct_20;

struct _struct_19 {
    DWORD LowPart;
    LONG HighPart;
};

struct _struct_20 {
    DWORD LowPart;
    LONG HighPart;
};

union _LARGE_INTEGER {
    struct _struct_19 s;
    struct _struct_20 u;
    LONGLONG QuadPart;
};

typedef union _LARGE_INTEGER LARGE_INTEGER;

typedef struct _struct_314 _struct_314, *P_struct_314;

struct _struct_314 {
    ULONGLONG Alignment;
    ULONGLONG Region;
};

typedef struct _struct_317 _struct_317, *P_struct_317;

struct _struct_317 {
    ULONGLONG Depth:16;
    ULONGLONG Sequence:48;
    ULONGLONG HeaderType:1;
    ULONGLONG Reserved:3;
    ULONGLONG NextEntry:60;
};

typedef struct _struct_316 _struct_316, *P_struct_316;

struct _struct_316 {
    ULONGLONG Depth:16;
    ULONGLONG Sequence:48;
    ULONGLONG HeaderType:1;
    ULONGLONG Init:1;
    ULONGLONG Reserved:2;
    ULONGLONG NextEntry:60;
};

typedef struct _struct_315 _struct_315, *P_struct_315;

struct _struct_315 {
    ULONGLONG Depth:16;
    ULONGLONG Sequence:9;
    ULONGLONG NextEntry:39;
    ULONGLONG HeaderType:1;
    ULONGLONG Init:1;
    ULONGLONG Reserved:59;
    ULONGLONG Region:3;
};

typedef struct _RUNTIME_FUNCTION _RUNTIME_FUNCTION, *P_RUNTIME_FUNCTION;

struct _RUNTIME_FUNCTION {
    DWORD BeginAddress;
    DWORD EndAddress;
    DWORD UnwindData;
};

typedef struct _IMAGE_SECTION_HEADER _IMAGE_SECTION_HEADER, *P_IMAGE_SECTION_HEADER;

typedef union _union_238 _union_238, *P_union_238;

union _union_238 {
    DWORD PhysicalAddress;
    DWORD VirtualSize;
};

struct _IMAGE_SECTION_HEADER {
    BYTE Name[8];
    union _union_238 Misc;
    DWORD VirtualAddress;
    DWORD SizeOfRawData;
    DWORD PointerToRawData;
    DWORD PointerToRelocations;
    DWORD PointerToLinenumbers;
    WORD NumberOfRelocations;
    WORD NumberOfLinenumbers;
    DWORD Characteristics;
};

typedef struct _SLIST_ENTRY _SLIST_ENTRY, *P_SLIST_ENTRY;

typedef struct _SLIST_ENTRY *PSLIST_ENTRY.conflict;

typedef struct _SLIST_ENTRY *PSLIST_ENTRY;

struct _SLIST_ENTRY {
    PSLIST_ENTRY Next;
};

typedef struct _RUNTIME_FUNCTION *PRUNTIME_FUNCTION;

typedef enum _EXCEPTION_DISPOSITION {
    ExceptionContinueExecution=0,
    ExceptionContinueSearch=1,
    ExceptionNestedException=2,
    ExceptionCollidedUnwind=3
} _EXCEPTION_DISPOSITION;

typedef enum _EXCEPTION_DISPOSITION EXCEPTION_DISPOSITION;

typedef EXCEPTION_DISPOSITION (EXCEPTION_ROUTINE)(struct _EXCEPTION_RECORD *, PVOID, struct _CONTEXT *, PVOID);

typedef struct _IMAGE_SECTION_HEADER *PIMAGE_SECTION_HEADER;

typedef WCHAR *PCNZWCH;

typedef union _SLIST_HEADER _SLIST_HEADER, *P_SLIST_HEADER;

struct _struct_314 {
    ULONGLONG Alignment;
    ULONGLONG Region;
};

struct _struct_315 {
    ULONGLONG Depth:16;
    ULONGLONG Sequence:9;
    ULONGLONG NextEntry:39;
    ULONGLONG HeaderType:1;
    ULONGLONG Init:1;
    ULONGLONG Reserved:59;
    ULONGLONG Region:3;
};

struct _struct_316 {
    ULONGLONG Depth:16;
    ULONGLONG Sequence:48;
    ULONGLONG HeaderType:1;
    ULONGLONG Init:1;
    ULONGLONG Reserved:2;
    ULONGLONG NextEntry:60;
};

struct _struct_317 {
    ULONGLONG Depth:16;
    ULONGLONG Sequence:48;
    ULONGLONG HeaderType:1;
    ULONGLONG Reserved:3;
    ULONGLONG NextEntry:60;
};

union _SLIST_HEADER {
    struct _struct_314 s;
    struct _struct_315 Header8;
    struct _struct_316 Header16;
    struct _struct_317 HeaderX64;
};

typedef WCHAR *LPWCH;

typedef WCHAR *LPCWSTR;

typedef struct _M128A *PM128A;

typedef struct _UNWIND_HISTORY_TABLE_ENTRY _UNWIND_HISTORY_TABLE_ENTRY, *P_UNWIND_HISTORY_TABLE_ENTRY;

typedef struct _UNWIND_HISTORY_TABLE_ENTRY UNWIND_HISTORY_TABLE_ENTRY;

struct _UNWIND_HISTORY_TABLE_ENTRY {
    DWORD64 ImageBase;
    PRUNTIME_FUNCTION FunctionEntry;
};

typedef union _union_61 _union_61, *P_union_61;

typedef struct _struct_62 _struct_62, *P_struct_62;

struct _struct_62 {
    PM128A Xmm0;
    PM128A Xmm1;
    PM128A Xmm2;
    PM128A Xmm3;
    PM128A Xmm4;
    PM128A Xmm5;
    PM128A Xmm6;
    PM128A Xmm7;
    PM128A Xmm8;
    PM128A Xmm9;
    PM128A Xmm10;
    PM128A Xmm11;
    PM128A Xmm12;
    PM128A Xmm13;
    PM128A Xmm14;
    PM128A Xmm15;
};

union _union_61 {
    PM128A FloatingContext[16];
    struct _struct_62 s;
};

typedef union _union_63 _union_63, *P_union_63;

typedef ulonglong *PDWORD64;

typedef struct _struct_64 _struct_64, *P_struct_64;

struct _struct_64 {
    PDWORD64 Rax;
    PDWORD64 Rcx;
    PDWORD64 Rdx;
    PDWORD64 Rbx;
    PDWORD64 Rsp;
    PDWORD64 Rbp;
    PDWORD64 Rsi;
    PDWORD64 Rdi;
    PDWORD64 R8;
    PDWORD64 R9;
    PDWORD64 R10;
    PDWORD64 R11;
    PDWORD64 R12;
    PDWORD64 R13;
    PDWORD64 R14;
    PDWORD64 R15;
};

union _union_63 {
    PDWORD64 IntegerContext[16];
    struct _struct_64 s;
};

typedef struct _UNWIND_HISTORY_TABLE _UNWIND_HISTORY_TABLE, *P_UNWIND_HISTORY_TABLE;

typedef struct _UNWIND_HISTORY_TABLE *PUNWIND_HISTORY_TABLE;

struct _UNWIND_HISTORY_TABLE {
    DWORD Count;
    BYTE LocalHint;
    BYTE GlobalHint;
    BYTE Search;
    BYTE Once;
    DWORD64 LowAddress;
    DWORD64 HighAddress;
    UNWIND_HISTORY_TABLE_ENTRY Entry[12];
};

typedef union _SLIST_HEADER *PSLIST_HEADER;

typedef LARGE_INTEGER *PLARGE_INTEGER;

typedef CHAR *LPSTR;

typedef struct _KNONVOLATILE_CONTEXT_POINTERS _KNONVOLATILE_CONTEXT_POINTERS, *P_KNONVOLATILE_CONTEXT_POINTERS;

typedef struct _KNONVOLATILE_CONTEXT_POINTERS *PKNONVOLATILE_CONTEXT_POINTERS;

union _union_61 {
    PM128A FloatingContext[16];
    struct _struct_62 s;
};

union _union_63 {
    PDWORD64 IntegerContext[16];
    struct _struct_64 s;
};

struct _KNONVOLATILE_CONTEXT_POINTERS {
    union _union_61 u;
    union _union_63 u2;
};

typedef EXCEPTION_ROUTINE *PEXCEPTION_ROUTINE;

typedef DWORD LCID;

typedef struct IMAGE_DOS_HEADER IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;

struct IMAGE_DOS_HEADER {
    char e_magic[2]; // Magic number
    word e_cblp; // Bytes of last page
    word e_cp; // Pages in file
    word e_crlc; // Relocations
    word e_cparhdr; // Size of header in paragraphs
    word e_minalloc; // Minimum extra paragraphs needed
    word e_maxalloc; // Maximum extra paragraphs needed
    word e_ss; // Initial (relative) SS value
    word e_sp; // Initial SP value
    word e_csum; // Checksum
    word e_ip; // Initial IP value
    word e_cs; // Initial (relative) CS value
    word e_lfarlc; // File address of relocation table
    word e_ovno; // Overlay number
    word e_res[4][4]; // Reserved words
    word e_oemid; // OEM identifier (for e_oeminfo)
    word e_oeminfo; // OEM information; e_oemid specific
    word e_res2[10][10]; // Reserved words
    dword e_lfanew; // File address of new exe header
    byte e_program[64]; // Actual DOS program
};

typedef struct tm tm, *Ptm;

struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
};

typedef longlong INT_PTR;

typedef ULONG_PTR DWORD_PTR;

typedef ULONG_PTR SIZE_T;

typedef struct _Cvtvec _Cvtvec, *P_Cvtvec;

struct _Cvtvec {
    uint _Page;
    uint _Mbcurmax;
    int _Isclocale;
    uchar _Isleadbyte[32];
};

typedef longlong fpos_t;

typedef ulong ULONG;

typedef DWORD *LPDWORD;

typedef HINSTANCE HMODULE;

typedef struct HMENU__ HMENU__, *PHMENU__;

typedef struct HMENU__ *HMENU;

struct HMENU__ {
    int unused;
};

typedef struct _FILETIME *LPFILETIME;

typedef INT_PTR (*FARPROC)(void);

typedef struct tagRECT tagRECT, *PtagRECT;

struct tagRECT {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
};

typedef WORD *LPWORD;

typedef int INT;

typedef WORD ATOM;

typedef struct tagRECT *LPRECT;

typedef BOOL *LPBOOL;

typedef void *HGDIOBJ;

typedef BYTE *PBYTE;

typedef void *LPCVOID;

typedef struct _IMAGE_RUNTIME_FUNCTION_ENTRY _IMAGE_RUNTIME_FUNCTION_ENTRY, *P_IMAGE_RUNTIME_FUNCTION_ENTRY;

struct _IMAGE_RUNTIME_FUNCTION_ENTRY {
    ImageBaseOffset32 BeginAddress;
    dword EndAddress; // Apply ImageBaseOffset32 to see reference
    ImageBaseOffset32 UnwindInfoAddressOrData;
};

typedef struct IMAGE_RESOURCE_DIRECTORY_ENTRY_NameStruct IMAGE_RESOURCE_DIRECTORY_ENTRY_NameStruct, *PIMAGE_RESOURCE_DIRECTORY_ENTRY_NameStruct;

struct IMAGE_RESOURCE_DIRECTORY_ENTRY_NameStruct {
    dword NameOffset:31;
    dword NameIsString:1;
};

typedef struct IMAGE_LOAD_CONFIG_CODE_INTEGRITY IMAGE_LOAD_CONFIG_CODE_INTEGRITY, *PIMAGE_LOAD_CONFIG_CODE_INTEGRITY;

struct IMAGE_LOAD_CONFIG_CODE_INTEGRITY {
    word Flags;
    word Catalog;
    dword CatalogOffset;
    dword Reserved;
};

typedef struct IMAGE_DEBUG_DIRECTORY IMAGE_DEBUG_DIRECTORY, *PIMAGE_DEBUG_DIRECTORY;

struct IMAGE_DEBUG_DIRECTORY {
    dword Characteristics;
    dword TimeDateStamp;
    word MajorVersion;
    word MinorVersion;
    dword Type;
    dword SizeOfData;
    dword AddressOfRawData;
    dword PointerToRawData;
};

typedef struct IMAGE_FILE_HEADER IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

struct IMAGE_FILE_HEADER {
    word Machine; // 34404
    word NumberOfSections;
    dword TimeDateStamp;
    dword PointerToSymbolTable;
    dword NumberOfSymbols;
    word SizeOfOptionalHeader;
    word Characteristics;
};

typedef struct IMAGE_LOAD_CONFIG_DIRECTORY64 IMAGE_LOAD_CONFIG_DIRECTORY64, *PIMAGE_LOAD_CONFIG_DIRECTORY64;

typedef enum IMAGE_GUARD_FLAGS {
    IMAGE_GUARD_CF_INSTRUMENTED=256,
    IMAGE_GUARD_CFW_INSTRUMENTED=512,
    IMAGE_GUARD_CF_FUNCTION_TABLE_PRESENT=1024,
    IMAGE_GUARD_SECURITY_COOKIE_UNUSED=2048,
    IMAGE_GUARD_PROTECT_DELAYLOAD_IAT=4096,
    IMAGE_GUARD_DELAYLOAD_IAT_IN_ITS_OWN_SECTION=8192,
    IMAGE_GUARD_CF_EXPORT_SUPPRESSION_INFO_PRESENT=16384,
    IMAGE_GUARD_CF_ENABLE_EXPORT_SUPPRESSION=32768,
    IMAGE_GUARD_CF_LONGJUMP_TABLE_PRESENT=65536,
    IMAGE_GUARD_RF_INSTRUMENTED=131072,
    IMAGE_GUARD_RF_ENABLE=262144,
    IMAGE_GUARD_RF_STRICT=524288,
    IMAGE_GUARD_CF_FUNCTION_TABLE_SIZE_MASK_1=268435456,
    IMAGE_GUARD_CF_FUNCTION_TABLE_SIZE_MASK_2=536870912,
    IMAGE_GUARD_CF_FUNCTION_TABLE_SIZE_MASK_4=1073741824,
    IMAGE_GUARD_CF_FUNCTION_TABLE_SIZE_MASK_8=2147483648
} IMAGE_GUARD_FLAGS;

struct IMAGE_LOAD_CONFIG_CODE_INTEGRITY {
    word Flags;
    word Catalog;
    dword CatalogOffset;
    dword Reserved;
};

struct IMAGE_LOAD_CONFIG_DIRECTORY64 {
    dword Size;
    dword TimeDateStamp;
    word MajorVersion;
    word MinorVersion;
    dword GlobalFlagsClear;
    dword GlobalFlagsSet;
    dword CriticalSectionDefaultTimeout;
    qword DeCommitFreeBlockThreshold;
    qword DeCommitTotalFreeThreshold;
    pointer64 LockPrefixTable;
    qword MaximumAllocationSize;
    qword VirtualMemoryThreshold;
    qword ProcessAffinityMask;
    dword ProcessHeapFlags;
    word CsdVersion;
    word DependentLoadFlags;
    pointer64 EditList;
    pointer64 SecurityCookie;
    pointer64 SEHandlerTable;
    qword SEHandlerCount;
    pointer64 GuardCFCCheckFunctionPointer;
    pointer64 GuardCFDispatchFunctionPointer;
    pointer64 GuardCFFunctionTable;
    qword GuardCFFunctionCount;
    enum IMAGE_GUARD_FLAGS GuardFlags;
    struct IMAGE_LOAD_CONFIG_CODE_INTEGRITY CodeIntegrity;
    pointer64 GuardAddressTakenIatEntryTable;
    qword GuardAddressTakenIatEntryCount;
    pointer64 GuardLongJumpTargetTable;
    qword GuardLongJumpTargetCount;
    pointer64 DynamicValueRelocTable;
    pointer64 CHPEMetadataPointer;
    pointer64 GuardRFFailureRoutine;
    pointer64 GuardRFFailureRoutineFunctionPointer;
    dword DynamicValueRelocTableOffset;
    word DynamicValueRelocTableSection;
    word Reserved2;
    pointer64 GuardRFVerifyStackPointerFunctionPointer;
    dword HotPatchTableOffset;
    dword Reserved3;
    pointer64 EnclaveConfigurationPointer;
    pointer64 VolatileMetadataPointer;
    pointer64 GuardEHContinuationTable;
    qword GuardEHContinuationCount;
    pointer64 GuardXFGCheckFunctionPointer;
    pointer64 GuardXFGDispatchFunctionPointer;
    pointer64 GuardXFGTableDispatchFunctionPointer;
    pointer64 CastGuardOsDeterminedFailureMode;
    pointer64 GuardMemcpyFunctionPointer;
};

typedef struct IMAGE_RESOURCE_DIRECTORY_ENTRY IMAGE_RESOURCE_DIRECTORY_ENTRY, *PIMAGE_RESOURCE_DIRECTORY_ENTRY;

typedef union IMAGE_RESOURCE_DIRECTORY_ENTRY_NameUnion IMAGE_RESOURCE_DIRECTORY_ENTRY_NameUnion, *PIMAGE_RESOURCE_DIRECTORY_ENTRY_NameUnion;

union IMAGE_RESOURCE_DIRECTORY_ENTRY_DirectoryUnion {
    dword OffsetToData;
    struct IMAGE_RESOURCE_DIRECTORY_ENTRY_DirectoryStruct IMAGE_RESOURCE_DIRECTORY_ENTRY_DirectoryStruct;
};

struct IMAGE_RESOURCE_DIRECTORY_ENTRY_NameStruct {
    dword NameOffset:31;
    dword NameIsString:1;
};

union IMAGE_RESOURCE_DIRECTORY_ENTRY_NameUnion {
    struct IMAGE_RESOURCE_DIRECTORY_ENTRY_NameStruct IMAGE_RESOURCE_DIRECTORY_ENTRY_NameStruct;
    dword Name;
    word Id;
};

struct IMAGE_RESOURCE_DIRECTORY_ENTRY {
    union IMAGE_RESOURCE_DIRECTORY_ENTRY_NameUnion NameUnion;
    union IMAGE_RESOURCE_DIRECTORY_ENTRY_DirectoryUnion DirectoryUnion;
};

typedef struct IMAGE_OPTIONAL_HEADER64 IMAGE_OPTIONAL_HEADER64, *PIMAGE_OPTIONAL_HEADER64;

typedef struct IMAGE_DATA_DIRECTORY IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;

struct IMAGE_OPTIONAL_HEADER64 {
    word Magic;
    byte MajorLinkerVersion;
    byte MinorLinkerVersion;
    dword SizeOfCode;
    dword SizeOfInitializedData;
    dword SizeOfUninitializedData;
    ImageBaseOffset32 AddressOfEntryPoint;
    ImageBaseOffset32 BaseOfCode;
    pointer64 ImageBase;
    dword SectionAlignment;
    dword FileAlignment;
    word MajorOperatingSystemVersion;
    word MinorOperatingSystemVersion;
    word MajorImageVersion;
    word MinorImageVersion;
    word MajorSubsystemVersion;
    word MinorSubsystemVersion;
    dword Win32VersionValue;
    dword SizeOfImage;
    dword SizeOfHeaders;
    dword CheckSum;
    word Subsystem;
    word DllCharacteristics;
    qword SizeOfStackReserve;
    qword SizeOfStackCommit;
    qword SizeOfHeapReserve;
    qword SizeOfHeapCommit;
    dword LoaderFlags;
    dword NumberOfRvaAndSizes;
    struct IMAGE_DATA_DIRECTORY DataDirectory[16];
};

struct IMAGE_DATA_DIRECTORY {
    ImageBaseOffset32 VirtualAddress;
    dword Size;
};

typedef struct IMAGE_SECTION_HEADER IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;

typedef union Misc Misc, *PMisc;

typedef enum SectionFlags {
    IMAGE_SCN_TYPE_NO_PAD=8,
    IMAGE_SCN_RESERVED_0001=16,
    IMAGE_SCN_CNT_CODE=32,
    IMAGE_SCN_CNT_INITIALIZED_DATA=64,
    IMAGE_SCN_CNT_UNINITIALIZED_DATA=128,
    IMAGE_SCN_LNK_OTHER=256,
    IMAGE_SCN_LNK_INFO=512,
    IMAGE_SCN_RESERVED_0040=1024,
    IMAGE_SCN_LNK_REMOVE=2048,
    IMAGE_SCN_LNK_COMDAT=4096,
    IMAGE_SCN_GPREL=32768,
    IMAGE_SCN_MEM_16BIT=131072,
    IMAGE_SCN_MEM_PURGEABLE=131072,
    IMAGE_SCN_MEM_LOCKED=262144,
    IMAGE_SCN_MEM_PRELOAD=524288,
    IMAGE_SCN_ALIGN_1BYTES=1048576,
    IMAGE_SCN_ALIGN_2BYTES=2097152,
    IMAGE_SCN_ALIGN_4BYTES=3145728,
    IMAGE_SCN_ALIGN_8BYTES=4194304,
    IMAGE_SCN_ALIGN_16BYTES=5242880,
    IMAGE_SCN_ALIGN_32BYTES=6291456,
    IMAGE_SCN_ALIGN_64BYTES=7340032,
    IMAGE_SCN_ALIGN_128BYTES=8388608,
    IMAGE_SCN_ALIGN_256BYTES=9437184,
    IMAGE_SCN_ALIGN_512BYTES=10485760,
    IMAGE_SCN_ALIGN_1024BYTES=11534336,
    IMAGE_SCN_ALIGN_2048BYTES=12582912,
    IMAGE_SCN_ALIGN_4096BYTES=13631488,
    IMAGE_SCN_ALIGN_8192BYTES=14680064,
    IMAGE_SCN_LNK_NRELOC_OVFL=16777216,
    IMAGE_SCN_MEM_DISCARDABLE=33554432,
    IMAGE_SCN_MEM_NOT_CACHED=67108864,
    IMAGE_SCN_MEM_NOT_PAGED=134217728,
    IMAGE_SCN_MEM_SHARED=268435456,
    IMAGE_SCN_MEM_EXECUTE=536870912,
    IMAGE_SCN_MEM_READ=1073741824,
    IMAGE_SCN_MEM_WRITE=2147483648
} SectionFlags;

union Misc {
    dword PhysicalAddress;
    dword VirtualSize;
};

struct IMAGE_SECTION_HEADER {
    char Name[8];
    union Misc Misc;
    ImageBaseOffset32 VirtualAddress;
    dword SizeOfRawData;
    dword PointerToRawData;
    dword PointerToRelocations;
    dword PointerToLinenumbers;
    word NumberOfRelocations;
    word NumberOfLinenumbers;
    enum SectionFlags Characteristics;
};

typedef struct IMAGE_NT_HEADERS64 IMAGE_NT_HEADERS64, *PIMAGE_NT_HEADERS64;

struct IMAGE_FILE_HEADER {
    word Machine; // 34404
    word NumberOfSections;
    dword TimeDateStamp;
    dword PointerToSymbolTable;
    dword NumberOfSymbols;
    word SizeOfOptionalHeader;
    word Characteristics;
};

struct IMAGE_OPTIONAL_HEADER64 {
    word Magic;
    byte MajorLinkerVersion;
    byte MinorLinkerVersion;
    dword SizeOfCode;
    dword SizeOfInitializedData;
    dword SizeOfUninitializedData;
    ImageBaseOffset32 AddressOfEntryPoint;
    ImageBaseOffset32 BaseOfCode;
    pointer64 ImageBase;
    dword SectionAlignment;
    dword FileAlignment;
    word MajorOperatingSystemVersion;
    word MinorOperatingSystemVersion;
    word MajorImageVersion;
    word MinorImageVersion;
    word MajorSubsystemVersion;
    word MinorSubsystemVersion;
    dword Win32VersionValue;
    dword SizeOfImage;
    dword SizeOfHeaders;
    dword CheckSum;
    word Subsystem;
    word DllCharacteristics;
    qword SizeOfStackReserve;
    qword SizeOfStackCommit;
    qword SizeOfHeapReserve;
    qword SizeOfHeapCommit;
    dword LoaderFlags;
    dword NumberOfRvaAndSizes;
    struct IMAGE_DATA_DIRECTORY DataDirectory[16];
};

struct IMAGE_NT_HEADERS64 {
    char Signature[4];
    struct IMAGE_FILE_HEADER FileHeader;
    struct IMAGE_OPTIONAL_HEADER64 OptionalHeader;
};

typedef struct IMAGE_RESOURCE_DIR_STRING_U_8 IMAGE_RESOURCE_DIR_STRING_U_8, *PIMAGE_RESOURCE_DIR_STRING_U_8;

struct IMAGE_RESOURCE_DIR_STRING_U_8 {
    word Length;
    wchar16 NameString[4];
};

typedef struct IMAGE_BASE_RELOCATION IMAGE_BASE_RELOCATION, *PIMAGE_BASE_RELOCATION;

struct IMAGE_BASE_RELOCATION {
    dword VirtualAddress;
    dword SizeOfBlock;
};

typedef struct IMAGE_RESOURCE_DATA_ENTRY IMAGE_RESOURCE_DATA_ENTRY, *PIMAGE_RESOURCE_DATA_ENTRY;

struct IMAGE_RESOURCE_DATA_ENTRY {
    dword OffsetToData;
    dword Size;
    dword CodePage;
    dword Reserved;
};

typedef struct IMAGE_RESOURCE_DIRECTORY IMAGE_RESOURCE_DIRECTORY, *PIMAGE_RESOURCE_DIRECTORY;

struct IMAGE_RESOURCE_DIRECTORY {
    dword Characteristics;
    dword TimeDateStamp;
    word MajorVersion;
    word MinorVersion;
    word NumberOfNamedEntries;
    word NumberOfIdEntries;
};

typedef struct IMAGE_DIRECTORY_ENTRY_EXPORT IMAGE_DIRECTORY_ENTRY_EXPORT, *PIMAGE_DIRECTORY_ENTRY_EXPORT;

struct IMAGE_DIRECTORY_ENTRY_EXPORT {
    dword Characteristics;
    dword TimeDateStamp;
    word MajorVersion;
    word MinorVersion;
    ImageBaseOffset32 Name;
    dword Base;
    dword NumberOfFunctions;
    dword NumberOfNames;
    ImageBaseOffset32 AddressOfFunctions;
    ImageBaseOffset32 AddressOfNames;
    ImageBaseOffset32 AddressOfNameOrdinals;
};

typedef struct _iobuf _iobuf, *P_iobuf;

struct _iobuf {
    char *_ptr;
    int _cnt;
    char *_base;
    int _flag;
    int _file;
    int _charbuf;
    int _bufsiz;
    char *_tmpfname;
};

typedef struct _iobuf FILE;

typedef struct _CONSOLE_READCONSOLE_CONTROL _CONSOLE_READCONSOLE_CONTROL, *P_CONSOLE_READCONSOLE_CONTROL;

struct _CONSOLE_READCONSOLE_CONTROL {
    ULONG nLength;
    ULONG nInitialChars;
    ULONG dwCtrlWakeupMask;
    ULONG dwControlKeyState;
};

typedef struct _CONSOLE_READCONSOLE_CONTROL *PCONSOLE_READCONSOLE_CONTROL;

typedef int PMFN;

typedef struct _s_ThrowInfo _s_ThrowInfo, *P_s_ThrowInfo;

struct _s_ThrowInfo {
    uint attributes;
    PMFN pmfnUnwind;
    int pForwardCompat;
    int pCatchableTypeArray;
};

typedef struct TypeDescriptor TypeDescriptor, *PTypeDescriptor;

struct TypeDescriptor {
    void *pVFTable;
    void *spare;
    char name[0];
};

typedef struct _s_ThrowInfo ThrowInfo;

typedef ulonglong uintptr_t;

typedef ulong u_long;

typedef struct WSAData WSAData, *PWSAData;

typedef struct WSAData WSADATA;

struct WSAData {
    WORD wVersion;
    WORD wHighVersion;
    ushort iMaxSockets;
    ushort iMaxUdpDg;
    char *lpVendorInfo;
    char szDescription[257];
    char szSystemStatus[129];
};

typedef UINT_PTR SOCKET;

typedef ushort u_short;

typedef WSADATA *LPWSADATA;

typedef struct sockaddr sockaddr, *Psockaddr;

struct sockaddr {
    u_short sa_family;
    char sa_data[14];
};

typedef struct fd_set fd_set, *Pfd_set;

typedef uint u_int;

struct fd_set {
    u_int fd_count;
    SOCKET fd_array[64];
};

typedef struct timeval timeval, *Ptimeval;

struct timeval {
    long tv_sec;
    long tv_usec;
};

typedef struct <lambda_5856287d7ecd2be6c9197bb4007c3f6e> <lambda_5856287d7ecd2be6c9197bb4007c3f6e>, *P<lambda_5856287d7ecd2be6c9197bb4007c3f6e>;

struct <lambda_5856287d7ecd2be6c9197bb4007c3f6e> { // PlaceHolder Structure
};

typedef struct <lambda_8dff2cf36a5417162780cd64fa2883ef> <lambda_8dff2cf36a5417162780cd64fa2883ef>, *P<lambda_8dff2cf36a5417162780cd64fa2883ef>;

struct <lambda_8dff2cf36a5417162780cd64fa2883ef> { // PlaceHolder Structure
};

typedef struct <lambda_b4f66040be5f354a654fb75160c27bfc> <lambda_b4f66040be5f354a654fb75160c27bfc>, *P<lambda_b4f66040be5f354a654fb75160c27bfc>;

struct <lambda_b4f66040be5f354a654fb75160c27bfc> { // PlaceHolder Structure
};

typedef struct <lambda_f7f22ab5edc0698d5f6905b0d3f44752> <lambda_f7f22ab5edc0698d5f6905b0d3f44752>, *P<lambda_f7f22ab5edc0698d5f6905b0d3f44752>;

struct <lambda_f7f22ab5edc0698d5f6905b0d3f44752> { // PlaceHolder Structure
};

typedef struct <lambda_3e16ef9562a7dcce91392c22ab16ea36> <lambda_3e16ef9562a7dcce91392c22ab16ea36>, *P<lambda_3e16ef9562a7dcce91392c22ab16ea36>;

struct <lambda_3e16ef9562a7dcce91392c22ab16ea36> { // PlaceHolder Structure
};

typedef struct <lambda_628dfdc04ba53c8bfc02c9951375f3f5> <lambda_628dfdc04ba53c8bfc02c9951375f3f5>, *P<lambda_628dfdc04ba53c8bfc02c9951375f3f5>;

struct <lambda_628dfdc04ba53c8bfc02c9951375f3f5> { // PlaceHolder Structure
};

typedef struct _s_HandlerType _s_HandlerType, *P_s_HandlerType;

struct _s_HandlerType { // PlaceHolder Structure
};

typedef struct __acrt_thread_parameter __acrt_thread_parameter, *P__acrt_thread_parameter;

struct __acrt_thread_parameter { // PlaceHolder Structure
};

typedef struct EHExceptionRecord EHExceptionRecord, *PEHExceptionRecord;

struct EHExceptionRecord { // PlaceHolder Structure
};

typedef struct <lambda_1d892b297fdedeee07ed8124eeb2f2fa> <lambda_1d892b297fdedeee07ed8124eeb2f2fa>, *P<lambda_1d892b297fdedeee07ed8124eeb2f2fa>;

struct <lambda_1d892b297fdedeee07ed8124eeb2f2fa> { // PlaceHolder Structure
};

typedef struct <lambda_7777bce6b2f8c936911f934f8298dc43> <lambda_7777bce6b2f8c936911f934f8298dc43>, *P<lambda_7777bce6b2f8c936911f934f8298dc43>;

struct <lambda_7777bce6b2f8c936911f934f8298dc43> { // PlaceHolder Structure
};

typedef struct <lambda_a37b2b86f63e897a80ea819b0eb08c01> <lambda_a37b2b86f63e897a80ea819b0eb08c01>, *P<lambda_a37b2b86f63e897a80ea819b0eb08c01>;

struct <lambda_a37b2b86f63e897a80ea819b0eb08c01> { // PlaceHolder Structure
};

typedef struct <lambda_7f2adfce497ff2baa965cd4f576ecfd1> <lambda_7f2adfce497ff2baa965cd4f576ecfd1>, *P<lambda_7f2adfce497ff2baa965cd4f576ecfd1>;

struct <lambda_7f2adfce497ff2baa965cd4f576ecfd1> { // PlaceHolder Structure
};

typedef struct <lambda_2358e3775559c9db80273638284d5e45> <lambda_2358e3775559c9db80273638284d5e45>, *P<lambda_2358e3775559c9db80273638284d5e45>;

struct <lambda_2358e3775559c9db80273638284d5e45> { // PlaceHolder Structure
};

typedef struct _Mbstatet _Mbstatet, *P_Mbstatet;

struct _Mbstatet { // PlaceHolder Structure
};

typedef struct <lambda_410d79af7f07d98d83a3f525b3859a53> <lambda_410d79af7f07d98d83a3f525b3859a53>, *P<lambda_410d79af7f07d98d83a3f525b3859a53>;

struct <lambda_410d79af7f07d98d83a3f525b3859a53> { // PlaceHolder Structure
};

typedef struct _s_TryBlockMapEntry _s_TryBlockMapEntry, *P_s_TryBlockMapEntry;

struct _s_TryBlockMapEntry { // PlaceHolder Structure
};

typedef struct __crt_locale_data __crt_locale_data, *P__crt_locale_data;

struct __crt_locale_data { // PlaceHolder Structure
};

typedef struct <lambda_886d6c58226a84441f68b9f2b8217b83> <lambda_886d6c58226a84441f68b9f2b8217b83>, *P<lambda_886d6c58226a84441f68b9f2b8217b83>;

struct <lambda_886d6c58226a84441f68b9f2b8217b83> { // PlaceHolder Structure
};

typedef struct <lambda_c8e61daa512c69656564d9dbd35486e8> <lambda_c8e61daa512c69656564d9dbd35486e8>, *P<lambda_c8e61daa512c69656564d9dbd35486e8>;

struct <lambda_c8e61daa512c69656564d9dbd35486e8> { // PlaceHolder Structure
};

typedef struct <lambda_c376a267cfb53b6959b3b692ec76b120> <lambda_c376a267cfb53b6959b3b692ec76b120>, *P<lambda_c376a267cfb53b6959b3b692ec76b120>;

struct <lambda_c376a267cfb53b6959b3b692ec76b120> { // PlaceHolder Structure
};

typedef struct __crt_win32_buffer<char,__crt_win32_buffer_no_resizing> __crt_win32_buffer<char,__crt_win32_buffer_no_resizing>, *P__crt_win32_buffer<char,__crt_win32_buffer_no_resizing>;

struct __crt_win32_buffer<char,__crt_win32_buffer_no_resizing> { // PlaceHolder Structure
};

typedef struct <lambda_38119f0e861e05405d8a144b9b982f0a> <lambda_38119f0e861e05405d8a144b9b982f0a>, *P<lambda_38119f0e861e05405d8a144b9b982f0a>;

struct <lambda_38119f0e861e05405d8a144b9b982f0a> { // PlaceHolder Structure
};

typedef struct <lambda_22ebabd17bc4fa466a2aca6d8deb888d> <lambda_22ebabd17bc4fa466a2aca6d8deb888d>, *P<lambda_22ebabd17bc4fa466a2aca6d8deb888d>;

struct <lambda_22ebabd17bc4fa466a2aca6d8deb888d> { // PlaceHolder Structure
};

typedef struct <lambda_572fbb9fa0ab338edf41edfd4b5fcc8d> <lambda_572fbb9fa0ab338edf41edfd4b5fcc8d>, *P<lambda_572fbb9fa0ab338edf41edfd4b5fcc8d>;

struct <lambda_572fbb9fa0ab338edf41edfd4b5fcc8d> { // PlaceHolder Structure
};

typedef struct <lambda_a6f7d7db0129f75315ebf26d50c089f1> <lambda_a6f7d7db0129f75315ebf26d50c089f1>, *P<lambda_a6f7d7db0129f75315ebf26d50c089f1>;

struct <lambda_a6f7d7db0129f75315ebf26d50c089f1> { // PlaceHolder Structure
};

typedef struct <lambda_ab61a845afdef5b7c387490eaf3616ee> <lambda_ab61a845afdef5b7c387490eaf3616ee>, *P<lambda_ab61a845afdef5b7c387490eaf3616ee>;

struct <lambda_ab61a845afdef5b7c387490eaf3616ee> { // PlaceHolder Structure
};

typedef struct <lambda_d80eeec6fff315bfe5c115232f3240e3> <lambda_d80eeec6fff315bfe5c115232f3240e3>, *P<lambda_d80eeec6fff315bfe5c115232f3240e3>;

struct <lambda_d80eeec6fff315bfe5c115232f3240e3> { // PlaceHolder Structure
};

typedef struct __crt_lc_time_data __crt_lc_time_data, *P__crt_lc_time_data;

struct __crt_lc_time_data { // PlaceHolder Structure
};

typedef struct _s_ESTypeList _s_ESTypeList, *P_s_ESTypeList;

struct _s_ESTypeList { // PlaceHolder Structure
};

typedef struct <lambda_f03950bc5685219e0bcd2087efbe011e> <lambda_f03950bc5685219e0bcd2087efbe011e>, *P<lambda_f03950bc5685219e0bcd2087efbe011e>;

struct <lambda_f03950bc5685219e0bcd2087efbe011e> { // PlaceHolder Structure
};

typedef struct <lambda_c5860995281e5c4ce005b3de8f5874ee> <lambda_c5860995281e5c4ce005b3de8f5874ee>, *P<lambda_c5860995281e5c4ce005b3de8f5874ee>;

struct <lambda_c5860995281e5c4ce005b3de8f5874ee> { // PlaceHolder Structure
};

typedef struct <lambda_842d9ff0dc9ef11c61343bbaebe7f885> <lambda_842d9ff0dc9ef11c61343bbaebe7f885>, *P<lambda_842d9ff0dc9ef11c61343bbaebe7f885>;

struct <lambda_842d9ff0dc9ef11c61343bbaebe7f885> { // PlaceHolder Structure
};

typedef enum _crt_exit_cleanup_mode {
} _crt_exit_cleanup_mode;

typedef struct <lambda_2a444430fde8c29194d880d93eed5e8f> <lambda_2a444430fde8c29194d880d93eed5e8f>, *P<lambda_2a444430fde8c29194d880d93eed5e8f>;

struct <lambda_2a444430fde8c29194d880d93eed5e8f> { // PlaceHolder Structure
};

typedef struct __crt_seh_guarded_call<void> __crt_seh_guarded_call<void>, *P__crt_seh_guarded_call<void>;

struct __crt_seh_guarded_call<void> { // PlaceHolder Structure
};

typedef struct _xDISPATCHER_CONTEXT _xDISPATCHER_CONTEXT, *P_xDISPATCHER_CONTEXT;

struct _xDISPATCHER_CONTEXT { // PlaceHolder Structure
};

typedef struct <lambda_2ca749746f96040ea1ff4296b14379df> <lambda_2ca749746f96040ea1ff4296b14379df>, *P<lambda_2ca749746f96040ea1ff4296b14379df>;

struct <lambda_2ca749746f96040ea1ff4296b14379df> { // PlaceHolder Structure
};

typedef struct _Init_atexit _Init_atexit, *P_Init_atexit;

struct _Init_atexit { // PlaceHolder Structure
};

typedef struct <lambda_38ce7e780aa69e748d6df282ebc68efe> <lambda_38ce7e780aa69e748d6df282ebc68efe>, *P<lambda_38ce7e780aa69e748d6df282ebc68efe>;

struct <lambda_38ce7e780aa69e748d6df282ebc68efe> { // PlaceHolder Structure
};

typedef struct <lambda_3883c3dff614d5e0c5f61bb1ac94921c> <lambda_3883c3dff614d5e0c5f61bb1ac94921c>, *P<lambda_3883c3dff614d5e0c5f61bb1ac94921c>;

struct <lambda_3883c3dff614d5e0c5f61bb1ac94921c> { // PlaceHolder Structure
};

typedef struct _s_CatchableType _s_CatchableType, *P_s_CatchableType;

struct _s_CatchableType { // PlaceHolder Structure
};

typedef enum __crt_lowio_text_mode {
} __crt_lowio_text_mode;

typedef struct <lambda_01a7098693036236037e7cdb9bca3d73> <lambda_01a7098693036236037e7cdb9bca3d73>, *P<lambda_01a7098693036236037e7cdb9bca3d73>;

struct <lambda_01a7098693036236037e7cdb9bca3d73> { // PlaceHolder Structure
};

typedef struct <lambda_638799b9deba96c50f710eeac98168cd> <lambda_638799b9deba96c50f710eeac98168cd>, *P<lambda_638799b9deba96c50f710eeac98168cd>;

struct <lambda_638799b9deba96c50f710eeac98168cd> { // PlaceHolder Structure
};

typedef enum SLD_STATUS {
} SLD_STATUS;

typedef struct __crt_stdio_stream __crt_stdio_stream, *P__crt_stdio_stream;

struct __crt_stdio_stream { // PlaceHolder Structure
};

typedef struct __crt_multibyte_data __crt_multibyte_data, *P__crt_multibyte_data;

struct __crt_multibyte_data { // PlaceHolder Structure
};

typedef struct __crt_seh_guarded_call<int> __crt_seh_guarded_call<int>, *P__crt_seh_guarded_call<int>;

struct __crt_seh_guarded_call<int> { // PlaceHolder Structure
};

typedef struct __crt_win32_buffer<char,__crt_win32_buffer_internal_dynamic_resizing> __crt_win32_buffer<char,__crt_win32_buffer_internal_dynamic_resizing>, *P__crt_win32_buffer<char,__crt_win32_buffer_internal_dynamic_resizing>;

struct __crt_win32_buffer<char,__crt_win32_buffer_internal_dynamic_resizing> { // PlaceHolder Structure
};

typedef struct __crt_win32_buffer<wchar_t,__crt_win32_buffer_internal_dynamic_resizing> __crt_win32_buffer<wchar_t,__crt_win32_buffer_internal_dynamic_resizing>, *P__crt_win32_buffer<wchar_t,__crt_win32_buffer_internal_dynamic_resizing>;

struct __crt_win32_buffer<wchar_t,__crt_win32_buffer_internal_dynamic_resizing> { // PlaceHolder Structure
};

typedef struct <lambda_f6c7be5f7998530c34de24c7437d6b54> <lambda_f6c7be5f7998530c34de24c7437d6b54>, *P<lambda_f6c7be5f7998530c34de24c7437d6b54>;

struct <lambda_f6c7be5f7998530c34de24c7437d6b54> { // PlaceHolder Structure
};

typedef struct __acrt_ptd __acrt_ptd, *P__acrt_ptd;

struct __acrt_ptd { // PlaceHolder Structure
};

typedef struct <lambda_751a882b2c74d4b022dec766aa51a29a> <lambda_751a882b2c74d4b022dec766aa51a29a>, *P<lambda_751a882b2c74d4b022dec766aa51a29a>;

struct <lambda_751a882b2c74d4b022dec766aa51a29a> { // PlaceHolder Structure
};

typedef struct <lambda_9a32fed5bf61b6b509b2d3f6003082a1> <lambda_9a32fed5bf61b6b509b2d3f6003082a1>, *P<lambda_9a32fed5bf61b6b509b2d3f6003082a1>;

struct <lambda_9a32fed5bf61b6b509b2d3f6003082a1> { // PlaceHolder Structure
};

typedef struct <lambda_6e4b09c48022b2350581041d5f6b0c4c> <lambda_6e4b09c48022b2350581041d5f6b0c4c>, *P<lambda_6e4b09c48022b2350581041d5f6b0c4c>;

struct <lambda_6e4b09c48022b2350581041d5f6b0c4c> { // PlaceHolder Structure
};

typedef struct __acrt_stdio_stream_mode __acrt_stdio_stream_mode, *P__acrt_stdio_stream_mode;

struct __acrt_stdio_stream_mode { // PlaceHolder Structure
};

typedef struct <lambda_ffc046909424fd52df0ac47e64813305> <lambda_ffc046909424fd52df0ac47e64813305>, *P<lambda_ffc046909424fd52df0ac47e64813305>;

struct <lambda_ffc046909424fd52df0ac47e64813305> { // PlaceHolder Structure
};

typedef struct <lambda_99fb1378e971ab6e7edea83e3a7a83a2> <lambda_99fb1378e971ab6e7edea83e3a7a83a2>, *P<lambda_99fb1378e971ab6e7edea83e3a7a83a2>;

struct <lambda_99fb1378e971ab6e7edea83e3a7a83a2> { // PlaceHolder Structure
};

typedef struct __crt_deferred_errno_cache __crt_deferred_errno_cache, *P__crt_deferred_errno_cache;

struct __crt_deferred_errno_cache { // PlaceHolder Structure
};

typedef enum __acrt_rounding_mode {
} __acrt_rounding_mode;

typedef enum _crt_exit_return_mode {
} _crt_exit_return_mode;

typedef struct __crt_locale_pointers __crt_locale_pointers, *P__crt_locale_pointers;

struct __crt_locale_pointers { // PlaceHolder Structure
};

typedef struct _LocaleUpdate _LocaleUpdate, *P_LocaleUpdate;

struct _LocaleUpdate { // PlaceHolder Structure
};

typedef struct <lambda_d90129c13df834fdcbf8d2b88dafcf2d> <lambda_d90129c13df834fdcbf8d2b88dafcf2d>, *P<lambda_d90129c13df834fdcbf8d2b88dafcf2d>;

struct <lambda_d90129c13df834fdcbf8d2b88dafcf2d> { // PlaceHolder Structure
};

typedef struct _strflt _strflt, *P_strflt;

struct _strflt { // PlaceHolder Structure
};

typedef struct _Yarn<char> _Yarn<char>, *P_Yarn<char>;

struct _Yarn<char> { // PlaceHolder Structure
};

typedef struct _Facet_base _Facet_base, *P_Facet_base;

struct _Facet_base { // PlaceHolder Structure
};

typedef struct basic_ostream<wchar_t,std::char_traits<wchar_t>_> basic_ostream<wchar_t,std::char_traits<wchar_t>_>, *Pbasic_ostream<wchar_t,std::char_traits<wchar_t>_>;

struct basic_ostream<wchar_t,std::char_traits<wchar_t>_> { // PlaceHolder Structure
};

typedef struct basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_>, *Pbasic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_>;

struct basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> { // PlaceHolder Structure
};

typedef struct basic_string<char,std::char_traits<char>,std::allocator<char>_> basic_string<char,std::char_traits<char>,std::allocator<char>_>, *Pbasic_string<char,std::char_traits<char>,std::allocator<char>_>;

struct basic_string<char,std::char_traits<char>,std::allocator<char>_> { // PlaceHolder Structure
};

typedef struct basic_istream<char,std::char_traits<char>_> basic_istream<char,std::char_traits<char>_>, *Pbasic_istream<char,std::char_traits<char>_>;

struct basic_istream<char,std::char_traits<char>_> { // PlaceHolder Structure
};

typedef struct allocator<char> allocator<char>, *Pallocator<char>;

struct allocator<char> { // PlaceHolder Structure
};

typedef struct _Fac_tidy_reg_t _Fac_tidy_reg_t, *P_Fac_tidy_reg_t;

struct _Fac_tidy_reg_t { // PlaceHolder Structure
};

typedef struct _Locinfo _Locinfo, *P_Locinfo;

struct _Locinfo { // PlaceHolder Structure
};

typedef struct _Lockit _Lockit, *P_Lockit;

struct _Lockit { // PlaceHolder Structure
};

typedef struct _Init_locks _Init_locks, *P_Init_locks;

struct _Init_locks { // PlaceHolder Structure
};

typedef struct TryBlockMapEntry4 TryBlockMapEntry4, *PTryBlockMapEntry4;

struct TryBlockMapEntry4 { // PlaceHolder Structure
};

typedef struct FuncInfo4 FuncInfo4, *PFuncInfo4;

struct FuncInfo4 { // PlaceHolder Structure
};

typedef struct HandlerType4 HandlerType4, *PHandlerType4;

struct HandlerType4 { // PlaceHolder Structure
};

typedef struct TryBlockMap4 TryBlockMap4, *PTryBlockMap4;

struct TryBlockMap4 { // PlaceHolder Structure
};

typedef struct write_result write_result, *Pwrite_result;

struct write_result { // PlaceHolder Structure
};

typedef struct argument_list<char> argument_list<char>, *Pargument_list<char>;

struct argument_list<char> { // PlaceHolder Structure
};

typedef struct formatting_buffer formatting_buffer, *Pformatting_buffer;

struct formatting_buffer { // PlaceHolder Structure
};

typedef struct output_processor<char,__crt_stdio_output::string_output_adapter<char>,__crt_stdio_output::standard_base<char,__crt_stdio_output::string_output_adapter<char>_>_> output_processor<char,__crt_stdio_output::string_output_adapter<char>,__crt_stdio_output::standard_base<char,__crt_stdio_output::string_output_adapter<char>_>_>, *Poutput_processor<char,__crt_stdio_output::string_output_adapter<char>,__crt_stdio_output::standard_base<char,__crt_stdio_output::string_output_adapter<char>_>_>;

struct output_processor<char,__crt_stdio_output::string_output_adapter<char>,__crt_stdio_output::standard_base<char,__crt_stdio_output::string_output_adapter<char>_>_> { // PlaceHolder Structure
};

typedef struct string_output_adapter<char> string_output_adapter<char>, *Pstring_output_adapter<char>;

struct string_output_adapter<char> { // PlaceHolder Structure
};

typedef struct output_processor<char,__crt_stdio_output::string_output_adapter<char>,__crt_stdio_output::format_validation_base<char,__crt_stdio_output::string_output_adapter<char>_>_> output_processor<char,__crt_stdio_output::string_output_adapter<char>,__crt_stdio_output::format_validation_base<char,__crt_stdio_output::string_output_adapter<char>_>_>, *Poutput_processor<char,__crt_stdio_output::string_output_adapter<char>,__crt_stdio_output::format_validation_base<char,__crt_stdio_output::string_output_adapter<char>_>_>;

struct output_processor<char,__crt_stdio_output::string_output_adapter<char>,__crt_stdio_output::format_validation_base<char,__crt_stdio_output::string_output_adapter<char>_>_> { // PlaceHolder Structure
};

typedef enum floating_point_parse_result {
} floating_point_parse_result;

typedef struct c_string_character_source<char> c_string_character_source<char>, *Pc_string_character_source<char>;

struct c_string_character_source<char> { // PlaceHolder Structure
};

typedef struct big_integer big_integer, *Pbig_integer;

struct big_integer { // PlaceHolder Structure
};

typedef struct floating_point_string floating_point_string, *Pfloating_point_string;

struct floating_point_string { // PlaceHolder Structure
};

typedef struct c_string_character_source<wchar_t> c_string_character_source<wchar_t>, *Pc_string_character_source<wchar_t>;

struct c_string_character_source<wchar_t> { // PlaceHolder Structure
};

typedef struct floating_point_value floating_point_value, *Pfloating_point_value;

struct floating_point_value { // PlaceHolder Structure
};

typedef enum function_id {
} function_id;

typedef enum module_id {
} module_id;

typedef int (*_onexit_t)(void);

typedef struct lconv lconv, *Plconv;

struct lconv {
    char *decimal_point;
    char *thousands_sep;
    char *grouping;
    char *int_curr_symbol;
    char *currency_symbol;
    char *mon_decimal_point;
    char *mon_thousands_sep;
    char *mon_grouping;
    char *positive_sign;
    char *negative_sign;
    char int_frac_digits;
    char frac_digits;
    char p_cs_precedes;
    char p_sep_by_space;
    char n_cs_precedes;
    char n_sep_by_space;
    char p_sign_posn;
    char n_sign_posn;
    wchar_t *_W_decimal_point;
    wchar_t *_W_thousands_sep;
    wchar_t *_W_int_curr_symbol;
    wchar_t *_W_currency_symbol;
    wchar_t *_W_mon_decimal_point;
    wchar_t *_W_mon_thousands_sep;
    wchar_t *_W_positive_sign;
    wchar_t *_W_negative_sign;
};

typedef ulonglong size_t;

typedef ushort wint_t;

typedef struct threadlocaleinfostruct threadlocaleinfostruct, *Pthreadlocaleinfostruct;

typedef struct threadlocaleinfostruct *pthreadlocinfo;

typedef struct localerefcount localerefcount, *Plocalerefcount;

typedef struct localerefcount locrefcount;

typedef struct __lc_time_data __lc_time_data, *P__lc_time_data;

struct threadlocaleinfostruct {
    int refcount;
    uint lc_codepage;
    uint lc_collate_cp;
    uint lc_time_cp;
    locrefcount lc_category[6];
    int lc_clike;
    int mb_cur_max;
    int *lconv_intl_refcount;
    int *lconv_num_refcount;
    int *lconv_mon_refcount;
    struct lconv *lconv;
    int *ctype1_refcount;
    ushort *ctype1;
    ushort *pctype;
    uchar *pclmap;
    uchar *pcumap;
    struct __lc_time_data *lc_time_curr;
    wchar_t *locale_name[6];
};

struct localerefcount {
    char *locale;
    wchar_t *wlocale;
    int *refcount;
    int *wrefcount;
};

struct __lc_time_data {
    char *wday_abbr[7];
    char *wday[7];
    char *month_abbr[12];
    char *month[12];
    char *ampm[2];
    char *ww_sdatefmt;
    char *ww_ldatefmt;
    char *ww_timefmt;
    int ww_caltype;
    int refcount;
    wchar_t *_W_wday_abbr[7];
    wchar_t *_W_wday[7];
    wchar_t *_W_month_abbr[12];
    wchar_t *_W_month[12];
    wchar_t *_W_ampm[2];
    wchar_t *_W_ww_sdatefmt;
    wchar_t *_W_ww_ldatefmt;
    wchar_t *_W_ww_timefmt;
    wchar_t *_W_ww_locale_name;
};

typedef long __time32_t;

typedef struct localeinfo_struct localeinfo_struct, *Plocaleinfo_struct;

typedef struct threadmbcinfostruct threadmbcinfostruct, *Pthreadmbcinfostruct;

typedef struct threadmbcinfostruct *pthreadmbcinfo;

struct localeinfo_struct {
    pthreadlocinfo locinfo;
    pthreadmbcinfo mbcinfo;
};

struct threadmbcinfostruct {
    int refcount;
    int mbcodepage;
    int ismbcodepage;
    ushort mbulinfo[6];
    uchar mbctype[257];
    uchar mbcasemap[256];
    wchar_t *mblocalename;
};

typedef int errno_t;

typedef longlong intptr_t;

typedef struct localeinfo_struct *_locale_t;

typedef longlong __time64_t;

typedef size_t rsize_t;

typedef ushort wctype_t;




void FUN_180001040(void);
void FUN_1800012e0(void);
void FUN_180001350(void);
void FUN_180001430(void);
void FUN_1800014d8(void);
void FUN_180001510(void);
longlong * FUN_180001540(longlong *param_1,longlong param_2);
void FUN_1800015c0(longlong *param_1);
void FUN_1800015d0(longlong *param_1);
void FUN_1800024c0(void);
ulonglong FUN_1800024e0(undefined8 *param_1,void *param_2);
size_t FUN_180002530(undefined8 *param_1,void *param_2);
undefined8 * FUN_180002580(undefined8 *param_1,undefined8 *param_2,undefined8 *param_3);
longlong * FUN_1800025b0(longlong *param_1,longlong *param_2,undefined8 *param_3);
longlong * FUN_180002620(longlong *param_1,longlong *param_2,undefined8 *param_3);
undefined8 * FUN_180002690(undefined8 *param_1,undefined8 *param_2,undefined8 *param_3);
ulonglong * FUN_1800027c0(ulonglong *param_1,undefined8 *param_2,undefined8 *param_3);
void FUN_180002830(ulonglong param_1);
void FUN_180002870(longlong *param_1,longlong *param_2);
undefined8 * FUN_180002900(longlong *param_1,longlong *param_2,undefined8 *param_3);
undefined8 *FUN_180002ae0(undefined8 *param_1,ulonglong param_2,undefined8 param_3,undefined1 param_4);
undefined8 *FUN_180002c50(undefined8 *param_1,ulonglong param_2,undefined8 param_3,ulonglong param_4,longlong param_5,undefined8 *param_6,ulonglong param_7);
undefined8 *FUN_180002e20(undefined8 *param_1,ulonglong param_2,undefined8 param_3,undefined8 *param_4,ulonglong param_5);
undefined8 *FUN_180002fb0(undefined8 *param_1,ulonglong param_2,undefined8 param_3,ulonglong param_4,ulonglong param_5,byte param_6);
undefined8 *FUN_180003170(undefined8 *param_1,ulonglong param_2,undefined8 param_3,ulonglong param_4,byte param_5);
undefined8 *FUN_180003300(undefined8 *param_1,ulonglong param_2,undefined8 param_3,ulonglong param_4,undefined8 *param_5,ulonglong param_6);
longlong * FUN_1800034b0(longlong *param_1,longlong *param_2,longlong *param_3);
double FUN_180003530(uint *param_1);
_Facet_base * FUN_180003720(longlong param_1);
_Facet_base * FUN_180003830(longlong param_1);
_Facet_base * FUN_180003940(longlong param_1);
undefined8 * FUN_180003a50(undefined8 *param_1,undefined8 *param_2);
longlong * FUN_180003b30(longlong *param_1,undefined8 *param_2);
undefined8 *FUN_180003b70(undefined8 *param_1,undefined8 param_2,undefined8 *param_3,undefined8 *param_4);
ulonglong *FUN_180003d20(ulonglong *param_1,undefined8 param_2,undefined8 param_3,undefined8 *param_4,ulonglong param_5,undefined8 *param_6,ulonglong param_7);
longlong * FUN_180003e50(longlong *param_1,int param_2);
longlong *FUN_180004000(longlong *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6,undefined4 param_7,HINSTANCE param_8);
undefined8 * FUN_180004860(undefined8 *param_1,undefined8 *param_2,undefined8 *param_3);
undefined8 * FUN_180004a50(undefined8 *param_1,longlong param_2);
undefined8 * FUN_180004aa0(undefined8 *param_1,longlong param_2);
undefined8 * FUN_180004ae0(undefined8 *param_1,longlong param_2);
undefined8 * FUN_180004b20(undefined8 *param_1);
undefined8 * FUN_180004b50(undefined8 *param_1,longlong param_2);
undefined8 * FUN_180004b90(undefined8 *param_1);
exception * __thiscall std::exception::exception(exception *this,exception *param_1);
undefined8 * FUN_180004c00(undefined8 *param_1,longlong param_2);
undefined8 * FUN_180004c60(undefined8 *param_1,undefined8 *param_2,undefined8 *param_3);
undefined8 * FUN_180004d40(undefined8 *param_1,longlong param_2);
undefined8 * FUN_180004d80(undefined8 *param_1,longlong *param_2);
undefined8 * FUN_180004e00(undefined8 *param_1,longlong param_2);
void FUN_180004e60(longlong *param_1);
void FUN_180004e80(longlong *param_1);
void FUN_180004ec0(undefined8 *param_1);
void FUN_180004ef0(ios_base *param_1);
void FUN_180004ff0(longlong *param_1);
void FUN_180005050(undefined8 *param_1);
void FUN_180005170(longlong *param_1);
void FUN_1800051e0(undefined8 *param_1);
void FUN_1800054a0(_Locinfo *param_1);
void FUN_180005540(longlong *param_1);
void FUN_1800055a0(longlong param_1);
void ~sentry(longlong *param_1);
longlong * FUN_180005610(longlong *param_1,longlong *param_2);
longlong * FUN_1800056a0(longlong *param_1,longlong *param_2);
void FUN_1800056d0(longlong *param_1,undefined8 *param_2);
longlong * FUN_1800056f0(longlong *param_1,undefined4 param_2);
longlong FUN_1800059e0(longlong *param_1,longlong param_2);
ulonglong FUN_180005a00(undefined8 *param_1,uint param_2);
void FUN_180005c40(longlong *param_1,undefined8 *param_2);
void FUN_180005c60(longlong *param_1,undefined8 *param_2);
void FUN_180005c80(longlong *param_1);
void FUN_180005d70(longlong param_1,uint param_2);
void FUN_180005d7c(longlong param_1,uint param_2);
void FUN_180005d88(longlong param_1,uint param_2);
void FUN_180005d94(longlong param_1,uint param_2);
ios_base * FUN_180005da0(ios_base *param_1,uint param_2);
ios_base * FUN_180005de0(ios_base *param_1,uint param_2);
void * __thiscall FID_conflict:`scalar_deleting_destructor'(void *this,uint param_1);
void * __thiscall FID_conflict:`scalar_deleting_destructor'(void *this,uint param_1);
undefined8 * FUN_180005f70(undefined8 *param_1,ulonglong param_2);
undefined8 * FUN_180005ff0(undefined8 *param_1,uint param_2);
longlong * FUN_180006030(longlong param_1,uint param_2);
void * __thiscall std::ctype<char>::`scalar_deleting_destructor'(ctype<char> *this,uint param_1);
undefined8 * FUN_1800060e0(undefined8 *param_1,ulonglong param_2);
undefined8 * FUN_180006110(undefined8 *param_1,uint param_2);
undefined8 * FUN_180006170(undefined8 *param_1,ulonglong param_2);
LPVOID FUN_1800061a0(LPVOID param_1,ulonglong param_2);
undefined8 * FUN_1800061d0(undefined8 *param_1,uint param_2);
undefined8 * FUN_180006220(undefined8 *param_1,uint param_2);
void * __thiscall std::ios_base::`scalar_deleting_destructor'(ios_base *this,uint param_1);
undefined8 FUN_1800062b0(longlong param_1,longlong param_2);
ulonglong FUN_180006b90(longlong param_1,longlong param_2);
bool FUN_18000d240(undefined8 *param_1,longlong *param_2,longlong *param_3,longlong *param_4,longlong *param_5,longlong *param_6,undefined1 param_7,longlong *param_8,undefined8 param_9);
void FUN_18000d850(HWND param_1,int param_2,undefined8 param_3,longlong *param_4);
void __cdecl std::_Adjust_manually_vector_aligned(void **param_1,__uint64 *param_2);
longlong FUN_18000d950(longlong param_1);
undefined4 *FUN_18000d960(undefined8 param_1,undefined4 *param_2,undefined8 *param_3,longlong param_4,byte param_5,byte *param_6,ulonglong param_7);
undefined8 FUN_18000e060(longlong *param_1,longlong param_2);
undefined8 FUN_18000e220(longlong *param_1,longlong param_2);
undefined8 FUN_18000e3c0(longlong *param_1,longlong param_2);
longlong FUN_18000e620(longlong param_1);
void FUN_18000e640(longlong param_1);
undefined4 *FUN_18000e650(undefined8 param_1,undefined4 *param_2,undefined4 *param_3,longlong param_4,byte param_5,char *param_6,ulonglong param_7);
void _guard_check_icall(void);
void FUN_18000ece0(longlong *param_1);
longlong FUN_18000ed90(longlong param_1);
undefined8 *FUN_18000edb0(undefined8 param_1,undefined8 *param_2,undefined8 *param_3,byte param_4,longlong param_5);
void FUN_18000ee60(void);
void FUN_18000ee80(void);
void FUN_18000eea0(void);
void FUN_18000eec0(void);
void FUN_18000eee0(void);
ulonglong FUN_18000ef00(undefined8 param_1,ulonglong param_2);
longlong * FUN_18000ef60(longlong *param_1,undefined8 *param_2,ulonglong param_3);
longlong * FUN_18000eff0(longlong *param_1,undefined8 *param_2,ulonglong param_3);
undefined1 (*) [32] FUN_18000f150(undefined1 (*param_1) [32],ulonglong param_2,byte param_3);
undefined8 * FUN_18000f2c0(undefined8 *param_1);
int FUN_18000f2d0(undefined8 *param_1,longlong param_2,longlong param_3);
void FUN_18000f930(int *param_1,longlong param_2,longlong param_3,int param_4,int param_5,int param_6,uint param_7,int param_8);
void FUN_18000fdb0(longlong param_1,int *param_2,longlong param_3,int param_4,int param_5,int param_6,uint param_7,int param_8);
void FUN_18000ff00(longlong param_1,uint param_2);
void FUN_18000ff80(int *param_1,int *param_2,int param_3,uint param_4);
void FUN_180010160(longlong param_1,undefined8 param_2);
void __thiscall std::allocator<char>::deallocate(allocator<char> *this,char *param_1,__uint64 param_2);
void FUN_1800104e0(undefined8 param_1,LPVOID param_2,longlong param_3);
undefined4 * FUN_180010530(undefined8 param_1,undefined4 *param_2,undefined4 param_3);
undefined1 FUN_180010540(longlong param_1);
longlong * FUN_180010550(longlong param_1,longlong *param_2);
longlong * FUN_1800105a0(longlong param_1,longlong *param_2);
undefined1 FUN_1800105f0(undefined8 param_1,undefined1 param_2);
longlong FUN_180010600(undefined8 param_1,undefined8 *param_2,longlong param_3,undefined8 param_4,undefined8 *param_5);
undefined4 *FUN_180010620(undefined8 param_1,undefined4 *param_2,undefined8 *param_3,longlong param_4,byte param_5,uint param_6);
undefined4 *FUN_180010730(undefined8 param_1,undefined4 *param_2,undefined8 *param_3,longlong param_4,byte param_5,uint param_6);
undefined4 *FUN_180010840(undefined8 param_1,undefined4 *param_2,undefined8 *param_3,longlong param_4,byte param_5,double param_6);
undefined4 *FUN_180010b10(undefined8 param_1,undefined4 *param_2,undefined8 *param_3,longlong param_4,byte param_5,double param_6);
undefined4 *FUN_180010de0(undefined8 param_1,undefined4 *param_2,undefined8 *param_3,longlong param_4,byte param_5,undefined8 param_6);
undefined4 *FUN_180010e80(undefined8 param_1,undefined4 *param_2,undefined8 *param_3,longlong param_4,byte param_5,undefined8 param_6);
undefined4 *FUN_180010fa0(undefined8 param_1,undefined4 *param_2,undefined8 *param_3,longlong param_4,byte param_5,undefined8 param_6);
undefined8 *FUN_1800110c0(longlong *param_1,undefined8 *param_2,undefined4 *param_3,longlong param_4,byte param_5,char param_6);
undefined1 FUN_180011360(longlong param_1);
void FUN_180011370(longlong param_1,byte param_2);
byte * FUN_180011380(longlong param_1,byte *param_2,byte *param_3);
void FUN_1800113d0(longlong param_1,byte param_2);
byte * FUN_1800113e0(longlong param_1,byte *param_2,byte *param_3);
longlong * FUN_180011430(longlong param_1,longlong *param_2);
longlong FUN_180011480(undefined8 param_1,undefined8 *param_2,longlong param_3,undefined8 *param_4);
longlong FUN_1800114a0(longlong param_1,int *param_2,int param_3);
undefined8 FUN_1800114c0(longlong *param_1,undefined4 param_2,int *param_3);
longlong * FUN_180011500(longlong *param_1);
char * FUN_180011630(undefined4 param_1);
undefined8 FUN_180011720(void);
longlong * FUN_180011980(longlong *param_1,undefined8 *param_2);
undefined4 aescriptsGetLicenseServerConfig(char *param_1,char *param_2,char *param_3,char *param_4);
void FUN_180012600(longlong param_1,undefined8 param_2,char param_3);
LPVOID * FUN_180012770(LPVOID *param_1,undefined4 param_2);
undefined4 * FUN_180012940(undefined4 *param_1,undefined4 param_2);
longlong * FUN_180012960(undefined8 param_1,longlong *param_2,int param_3);
char * FUN_1800129e0(void);
void FUN_1800129f0(void);
void FUN_180012d40(LPCSTR param_1);
undefined8 FUN_180012d70(void);
int FUN_180012d80(longlong param_1,int param_2);
int FUN_180012f40(longlong param_1,int param_2);
longlong *FUN_180012fa0(longlong *param_1,ulonglong param_2,ulonglong param_3,undefined8 *param_4,ulonglong param_5);
char * FUN_180013150(undefined4 param_1);
int aescriptsSaveLicenseToFile(byte *param_1,undefined8 *param_2,char *param_3,undefined8 *param_4);
undefined8 * FUN_180013340(undefined8 param_1,undefined8 *param_2);
ulonglong *FUN_180013360(longlong param_1,ulonglong *param_2,longlong param_3,int param_4,byte param_5);
ulonglong * FUN_1800134d0(longlong param_1,ulonglong *param_2,longlong *param_3,byte param_4);
undefined8 FUN_1800135d0(undefined8 param_1);
void FUN_1800135e0(longlong param_1);
undefined8 FUN_1800137b0(undefined8 *param_1,undefined1 (*param_2) [32],char param_3,char *param_4);
undefined8 FUN_180016dc0(void);
longlong * FUN_180016dd0(longlong *param_1,longlong *param_2,longlong *param_3,longlong *param_4);
longlong * FUN_180016ff0(longlong *param_1,longlong *param_2,byte *param_3);
void FUN_1800174a0(undefined8 *param_1,char param_2,longlong *param_3);
void FUN_1800176e0(longlong param_1);
longlong * FUN_180017820(undefined8 *param_1,longlong *param_2,ulonglong param_3,ulonglong param_4);
ulonglong FUN_180017880(longlong *param_1);
ulonglong FUN_1800178c0(longlong param_1);
void FUN_180017930(undefined8 *param_1,undefined8 *param_2,longlong param_3,undefined1 (*param_4) [32],char param_5);
char * FUN_180017940(longlong param_1);
longlong FUN_180017960(longlong *param_1,undefined8 *param_2,ulonglong param_3);
longlong FUN_180017a10(longlong *param_1,undefined8 *param_2,ulonglong param_3);
int EffectMain(int param_1,undefined8 *param_2,undefined4 *param_3,longlong *param_4,longlong param_5);
void PluginDataEntryFunction(undefined8 param_1,undefined *param_2);
undefined * FUN_180017df0(void);
int FUN_180017e00(char *param_1,__uint64 param_2,char *param_3,undefined8 param_4);
undefined8 * FUN_180017e60(undefined8 *param_1);
double FUN_180017e80(longlong param_1,double param_2);
void FUN_180017eb0(longlong param_1,double param_2);
void FUN_180017ee0(longlong param_1,double param_2);
void FUN_180017f00(longlong param_1,double param_2,double param_3);
longlong * FUN_180017f40(longlong *param_1,longlong *param_2);
longlong * FUN_1800181c0(longlong *param_1,byte param_2,undefined8 param_3,undefined8 param_4);
ulonglong FUN_1800184a0(undefined8 *param_1,undefined8 *param_2);
longlong FUN_1800184f0(longlong *param_1,longlong *param_2);
size_t FUN_180018550(void *param_1,undefined8 *param_2);
ulonglong * FUN_1800185a0(ulonglong *param_1,undefined8 *param_2,undefined8 *param_3);
ulonglong * FUN_180018610(ulonglong *param_1,undefined8 *param_2,undefined8 *param_3);
longlong * FUN_180018680(longlong *param_1,undefined8 param_2);
void FUN_180018960(longlong *param_1,longlong *param_2);
void FUN_1800189a0(longlong *param_1,longlong *param_2);
void FUN_1800189e0(longlong *param_1,longlong *param_2);
undefined8 * FUN_180018a20(longlong *param_1,undefined8 *param_2,undefined8 *param_3);
undefined8 * FUN_180018c10(longlong *param_1,undefined8 *param_2,undefined8 *param_3);
undefined8 * FUN_180018e10(longlong *param_1,undefined8 *param_2,undefined8 *param_3);
undefined8 * FUN_180019020(longlong *param_1,undefined8 *param_2,undefined8 *param_3);
undefined8 * FUN_180019290(longlong *param_1,longlong *param_2,undefined8 *param_3);
undefined8 * FUN_1800193b0(longlong *param_1,longlong *param_2,undefined8 *param_3);
void FUN_1800194f0(undefined8 param_1,undefined8 param_2,longlong *param_3);
undefined4 FUN_180019570(longlong *param_1,longlong *param_2,ulonglong param_3,char *param_4,char param_5);
longlong * FUN_180019a70(longlong *param_1,undefined8 param_2,ulonglong param_3);
undefined8 ******* FUN_180019d20(undefined8 *******param_1,undefined8 *******param_2);
undefined8 FUN_18001a0a0(undefined8 *param_1);
void FUN_18001a0d0(longlong param_1,longlong param_2);
void FUN_18001a480(undefined8 *param_1,undefined8 *param_2,undefined8 *param_3);
undefined8 * FUN_18001a530(undefined8 *param_1,undefined8 *param_2,undefined8 *param_3);
void FUN_18001a830(longlong param_1,longlong param_2,ulonglong param_3,longlong *param_4);
void FUN_18001aa30(undefined8 *******param_1,undefined8 *******param_2,longlong param_3,undefined1 param_4);
ulonglong FUN_18001ac10(longlong param_1,ulonglong param_2,ulonglong param_3,char *param_4,ulonglong param_5);
undefined8 * FUN_18001ace0(undefined8 *param_1,undefined8 *param_2,undefined8 *param_3);
undefined8 * FUN_18001ad90(undefined8 *param_1,undefined8 *param_2,undefined8 *param_3);
void FUN_18001ae30(undefined8 *param_1,undefined8 *param_2,undefined8 *param_3);
void FUN_18001b040(undefined8 param_1,undefined8 *param_2,undefined8 *param_3);
void FUN_18001b140(undefined8 param_1,undefined8 *param_2,undefined8 *param_3);
void FUN_18001b1f0(undefined8 param_1,longlong *param_2);
_Facet_base * FUN_18001b250(longlong param_1);
undefined8 * FUN_18001b360(undefined8 *param_1);
longlong * FUN_18001b3f0(longlong *param_1,char *param_2,uint param_3,int param_4,int param_5);
longlong * FUN_18001b580(longlong *param_1,int param_2);
longlong * FUN_18001b660(longlong *param_1,char *param_2,uint param_3,int param_4,int param_5);
undefined8 * FUN_18001b7f0(undefined8 *param_1);
undefined8 * FUN_18001b890(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_18001b8e0(undefined8 *param_1,undefined8 *param_2);
void FUN_18001ba10(undefined8 *param_1);
void FUN_18001ba60(longlong *param_1);
void FUN_18001bb10(longlong *param_1);
void FUN_18001bb40(longlong param_1);
void FUN_18001bbc0(longlong *param_1);
void FUN_18001bc50(longlong *param_1);
void FUN_18001bd10(longlong *param_1);
void FUN_18001bf40(undefined1 *param_1);
void FUN_18001c140(longlong *param_1);
void FUN_18001c1b0(longlong *param_1);
void FUN_18001c220(longlong *param_1);
void FUN_18001c350(longlong *param_1);
void FUN_18001c550(longlong *param_1);
longlong * FUN_18001c580(longlong *param_1,undefined4 param_2);
longlong * FUN_18001c790(longlong *param_1,undefined8 param_2);
longlong * FUN_18001c9a0(longlong *param_1,undefined8 param_2);
longlong * FUN_18001cbb0(longlong *param_1,byte *param_2);
longlong * FUN_18001ccc0(longlong *param_1);
longlong * FUN_18001cd30(longlong *param_1,longlong *param_2,int *param_3);
longlong * FUN_18001cda0(longlong *param_1,undefined1 param_2);
void FUN_18001ce00(longlong *param_1);
void FUN_18001cea0(longlong *param_1);
void FUN_18001cf34(longlong param_1,uint param_2);
void FUN_18001cf40(longlong param_1,uint param_2);
longlong * FUN_18001cf50(longlong *param_1,uint param_2);
longlong * FUN_18001cfe0(longlong *param_1,uint param_2);
longlong * FUN_18001d020(longlong param_1,uint param_2);
longlong * FUN_18001d060(longlong param_1,uint param_2);
undefined8 FUN_18001d0a0(void);
undefined8 *FUN_18001d0f0(undefined8 *param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
ulonglong FUN_18001d4a0(longlong *param_1);
int FUN_18001d5f0(longlong *param_1,uint param_2,int param_3,longlong *param_4,longlong *param_5);
void FUN_18001d7e0(longlong *param_1,longlong param_2,longlong param_3,longlong param_4);
bool FUN_18001d880(longlong *param_1);
undefined8 FUN_18001d970(longlong *param_1,longlong param_2);
undefined8 FUN_18001db10(longlong *param_1,longlong param_2);
ulonglong FUN_18001dcb0(undefined8 param_1,char *param_2,longlong *param_3,longlong *param_4,longlong param_5,int *param_6);
undefined8 FUN_18001efb0(undefined8 param_1,char *param_2,longlong *param_3,longlong *param_4,longlong param_5,int *param_6);
uint FUN_180020300(undefined8 param_1,char *param_2,longlong *param_3,longlong *param_4,uint param_5,longlong param_6);
void FUN_1800209b0(longlong param_1,longlong param_2,int param_3);
void FUN_180020a80(longlong param_1,longlong *param_2);
longlong * FUN_180020b00(longlong *param_1,longlong *param_2,longlong *param_3);
undefined8 FUN_180020d60(longlong *param_1,char param_2);
void FUN_180020fa0(longlong param_1);
ulonglong FUN_180020fc0(longlong *param_1);
void __thiscall std::basic_filebuf<>::_Reset_back(basic_filebuf<> *this);
void FUN_180021050(void);
void FUN_180021070(longlong *param_1);
void FUN_180021130(longlong param_1);
void FUN_180021150(longlong *param_1,int *param_2,ulonglong param_3);
int __cdecl aescriptsGetLicLibVersion(char *param_1);
ulonglong FUN_1800216f0(undefined8 param_1,ulonglong param_2);
ulonglong FUN_180021770(undefined8 param_1,ulonglong param_2);
void FUN_1800217e0(longlong *param_1,undefined8 *param_2,ulonglong param_3,ulonglong param_4);
longlong * FUN_180021820(longlong *param_1,longlong param_2,uint param_3);
ulonglong FUN_180021900(int param_1,int param_2,int param_3,int param_4,int param_5);
undefined4 FUN_1800219b0(longlong param_1);
ulonglong FUN_180021c70(longlong *param_1,int *param_2,int *param_3);
longlong * FUN_1800228d0(longlong *param_1,int param_2);
longlong * FUN_1800232b0(longlong *param_1);
void FUN_1800233c0(longlong *param_1);
void FUN_180023460(longlong *param_1);
int __cdecl memcmp(void *_Buf1,void *_Buf2,size_t _Size);
uint FUN_180023510(undefined8 *param_1,undefined8 *param_2);
undefined8 FUN_180023580(int param_1,sockaddr *param_2,int param_3,int param_4);
void FUN_1800236a0(longlong *param_1,char *****param_2,undefined8 param_3,undefined8 param_4);
int FUN_180023db0(int param_1,uint param_2,int param_3);
void FUN_180023e60(undefined8 param_1,LPVOID param_2,longlong param_3);
void FUN_180023eb0(undefined8 param_1,LPVOID param_2,longlong param_3);
void FUN_180023f00(undefined8 param_1,LPVOID param_2,longlong param_3);
void FUN_180023f50(undefined8 param_1,LPVOID param_2,longlong param_3);
undefined8 * FUN_180023fa0(undefined8 *param_1,longlong *param_2);
longlong * FUN_180024ab0(longlong *param_1,longlong *param_2,byte *param_3);
void FUN_180024c10(undefined1 *param_1);
undefined8 FUN_180025300(longlong param_1);
undefined4 FUN_1800255d0(int param_1,undefined8 *param_2,undefined8 *param_3,longlong *param_4,longlong *param_5);
int FUN_180027f50(longlong *param_1,longlong *param_2);
undefined1 FUN_1800296b0(void);
longlong *FUN_1800296c0(undefined8 param_1,longlong *param_2,longlong *param_3,longlong *param_4,longlong param_5,uint *param_6,short *param_7);
undefined8 *FUN_180029840(undefined8 param_1,undefined8 *param_2,undefined8 *param_3,undefined4 *param_4,longlong param_5,uint *param_6,uint *param_7);
longlong *FUN_1800299e0(undefined8 param_1,longlong *param_2,longlong *param_3,longlong *param_4,longlong param_5,uint *param_6,uint *param_7);
longlong *FUN_180029b30(undefined8 param_1,longlong *param_2,longlong *param_3,longlong *param_4,longlong param_5,uint *param_6,uint *param_7);
longlong *FUN_180029c80(undefined8 param_1,longlong *param_2,longlong *param_3,longlong *param_4,longlong param_5,uint *param_6,float *param_7);
longlong *FUN_180029e40(undefined8 param_1,longlong *param_2,longlong *param_3,longlong *param_4,longlong param_5,uint *param_6,double *param_7);
undefined8 *FUN_18002a000(undefined8 param_1,undefined8 *param_2,undefined8 *param_3,undefined8 *param_4,longlong param_5,uint *param_6,double *param_7);
longlong *FUN_18002a230(undefined8 param_1,longlong *param_2,longlong *param_3,longlong *param_4,longlong param_5,uint *param_6,ulonglong *param_7);
longlong *FUN_18002a370(undefined8 param_1,longlong *param_2,longlong *param_3,longlong *param_4,longlong param_5,uint *param_6,ulonglong *param_7);
longlong *FUN_18002a4c0(undefined8 param_1,longlong *param_2,longlong *param_3,longlong *param_4,longlong param_5,uint *param_6,ulonglong *param_7);
byte * FUN_18002a610(undefined8 param_1,byte *param_2,longlong *param_3,longlong *param_4,longlong param_5,uint *param_6,undefined1 *param_7);
undefined8 FUN_18002aa20(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4,undefined8 *param_5,undefined8 param_6,undefined8 param_7,undefined8 *param_8);
ulonglong FUN_18002aa40(undefined8 param_1,undefined8 param_2,longlong param_3,longlong param_4,ulonglong param_5);
undefined8 FUN_18002aa60(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4,undefined8 *param_5);
longlong * FUN_18002aa70(longlong *param_1,longlong *param_2,byte *param_3);
longlong * FUN_18002ab80(longlong param_1,longlong *param_2,longlong param_3);
longlong * FUN_18002acb0(longlong param_1,longlong *param_2,longlong param_3);
uint FUN_18002ad80(FILE *param_1);
void FUN_18002ae00(undefined8 *param_1,char *param_2,ulonglong param_3);
int FUN_18002ae40(longlong *param_1,longlong *param_2);
longlong * FUN_18002afd0(longlong *param_1,longlong *param_2);
undefined8 FUN_18002b260(longlong *param_1,longlong *param_2,longlong *param_3,longlong *param_4,char *param_5);
undefined8 FUN_18002b710(char *param_1,longlong *param_2);
longlong * FUN_18002be70(longlong *param_1,longlong *param_2);
void FUN_18002bfb0(longlong *param_1,undefined1 *param_2);
longlong *FUN_18002c1a0(longlong *param_1,longlong *param_2,byte *param_3,longlong param_4,int param_5,longlong *param_6);
ulonglong FUN_18002d6a0(undefined8 *param_1,undefined8 *param_2,longlong param_3,char param_4,char *param_5);
ulonglong FUN_18002d730(undefined8 *param_1,undefined8 *param_2,undefined8 param_3,char param_4,char *param_5);
undefined4 FUN_18002e120(char *param_1,char *param_2,char *param_3,char *param_4,char **********param_5);
undefined8 FUN_18002f930(undefined8 *param_1);
undefined8 FUN_18002fd90(longlong *param_1);
undefined8 FUN_180030750(longlong *param_1,longlong *param_2,uint param_3);
longlong FUN_180030b10(longlong param_1);
undefined8 FUN_180030d80(longlong *param_1,char param_2,char *****param_3,undefined8 param_4,char param_5);
longlong * FUN_1800315d0(longlong *param_1,longlong *param_2,int param_3,int param_4);
undefined8 **** FUN_1800316c0(undefined8 ****param_1,longlong *param_2);
int FUN_180031b60(longlong *param_1);
void FUN_180031d50(byte *param_1,uint param_2,longlong param_3,undefined8 param_4);
longlong * FUN_180031fd0(longlong *param_1,longlong *param_2);
void FUN_180032460(longlong param_1,longlong param_2);
void FUN_180032490(longlong ****param_1,longlong ****param_2);
undefined8 *FUN_180033910(undefined8 *param_1,ulonglong param_2,undefined8 *param_3,ulonglong param_4);
longlong * FUN_180033a20(longlong *param_1,undefined4 param_2);
longlong * FUN_180033a80(longlong *param_1,uint param_2,undefined8 param_3,undefined8 param_4);
longlong * FUN_180033b10(longlong *param_1,uint param_2);
longlong * FUN_180033cb0(longlong *param_1,uint param_2,undefined8 *param_3,undefined8 param_4);
longlong * FUN_180033e90(longlong *param_1,uint param_2,undefined8 *param_3,undefined8 param_4);
longlong *FUN_180034080(longlong *param_1,uint param_2,undefined4 param_3,undefined8 param_4,longlong param_5,longlong param_6,char param_7,uint param_8);
longlong *FUN_180034500(longlong *param_1,uint param_2,uint param_3,undefined8 param_4,char param_5);
ulonglong FUN_1800349c0(byte *param_1,undefined8 *param_2,ulonglong param_3,undefined1 (*param_4) [32],char param_5,longlong *param_6,undefined8 *******param_7,longlong *param_8,longlong *param_9,longlong *param_10,char param_11);
ulonglong FUN_180037ef0(byte *param_1,undefined8 *param_2,longlong param_3,undefined1 (*param_4) [32],char param_5,char param_6);
longlong * FUN_180038c10(longlong *param_1,longlong *param_2,char param_3);
longlong * FUN_180038fa0(longlong *param_1,undefined8 param_2);
longlong * FUN_180039000(longlong *param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
longlong * FUN_180039090(longlong *param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
undefined8 FUN_180039120(char *param_1);
ulonglong * FUN_18003aa00(longlong *param_1,uint param_2);
int FUN_18003ad70(longlong *param_1,longlong param_2,int param_3);
longlong * FUN_18003aec0(longlong *param_1,uint param_2,undefined8 param_3,undefined8 param_4);
longlong * FUN_18003af50(longlong *param_1,longlong *param_2,byte param_3);
longlong FUN_18003bae0(longlong param_1,char *param_2,uint param_3,int param_4);
void FUN_18003bb90(longlong *param_1,char *param_2,uint param_3,int param_4);
int FUN_18003bc90(longlong param_1,int param_2);
longlong * FUN_18003be30(longlong *param_1,longlong *param_2,uint param_3);
longlong * FUN_18003c030(longlong *param_1,longlong *param_2,int param_3);
undefined8 FUN_18003c230(longlong *param_1,longlong param_2);
uint FUN_18003d010(longlong param_1,uint param_2);
void FUN_18003d100(longlong param_1,int *param_2);
undefined8 FUN_18003d8f0(longlong param_1,longlong *param_2);
longlong * FUN_18003f7c0(longlong *param_1,undefined8 param_2,longlong param_3);
int FUN_18003f910(int param_1,longlong *param_2);
undefined8 * FUN_18003fa00(undefined8 *param_1,longlong *param_2);
int FUN_18003fae0(undefined1 (*param_1) [32],longlong *param_2,undefined8 *param_3,char *param_4,undefined8 *param_5,char param_6,longlong *param_7,undefined8 param_8,char param_9);
longlong * FUN_180041de0(longlong *param_1,undefined8 param_2,undefined4 param_3);
fpos_t * FUN_180041fe0(basic_filebuf<> *param_1,fpos_t *param_2,longlong param_3,int param_4);
longlong * FUN_1800420b0(longlong *param_1,longlong *param_2,longlong *param_3);
undefined8 FUN_180042170(longlong param_1,longlong *param_2);
int FUN_1800424a0(longlong *param_1,longlong *param_2,longlong *param_3,longlong *param_4,longlong *param_5,longlong *param_6,undefined8 param_7,byte *param_8,longlong *param_9);
int FUN_1800440a0(int param_1,longlong *param_2);
void FUN_180044360(longlong *param_1,DWORD param_2,char param_3,byte param_4);
longlong FUN_180045590(longlong param_1,char *param_2,size_t param_3);
int FUN_1800455f0(int param_1,int param_2);
ulonglong FUN_180045620(longlong *param_1);
void FUN_1800456c0(longlong *param_1,longlong *param_2,longlong *param_3,longlong *param_4,longlong *param_5);
undefined8 FUN_1800460d0(char *param_1);
longlong * FUN_1800460f0(longlong param_1,longlong *param_2);
longlong * FUN_1800461b0(longlong *param_1,undefined8 ****param_2,longlong *param_3,char param_4);
longlong * FUN_1800466a0(longlong *param_1,undefined8 *param_2,longlong *param_3,char param_4);
longlong *FUN_180046ab0(longlong *param_1,longlong *param_2,longlong *param_3,longlong *param_4,char param_5,char param_6);
longlong * FUN_180046f30(longlong *param_1,longlong *param_2,longlong *param_3);
longlong * FUN_1800470f0(longlong *param_1,longlong *param_2,longlong *param_3);
void FUN_180047320(undefined8 *param_1,char param_2,longlong *param_3,char param_4);
longlong * FUN_180047820(longlong *param_1,byte *param_2,byte *param_3);
char * FUN_1800479f0(char *param_1,undefined8 *param_2);
longlong * FUN_180047a70(longlong *param_1,longlong *param_2);
int FUN_180047b20(longlong *param_1);
undefined8 * FUN_180047b70(longlong *param_1,undefined8 *param_2);
undefined8 FUN_180047c50(longlong *param_1,longlong *param_2,int *param_3);
undefined8 FUN_180047fb0(undefined8 *param_1,undefined8 *param_2,undefined8 param_3,longlong param_4,char param_5);
uint FUN_1800488e0(longlong param_1);
ulonglong FUN_180048b90(longlong *param_1);
undefined8 FUN_18004e700(longlong *******param_1,longlong *param_2,byte *param_3,undefined8 param_4,byte param_5,char *param_6);
longlong * FUN_1800513d0(longlong *param_1,undefined8 param_2,longlong param_3);
longlong FUN_180051570(longlong *param_1,undefined8 *param_2,ulonglong param_3);
longlong FUN_180051720(longlong *param_1,undefined8 *param_2,ulonglong param_3);
undefined8 aescriptsAddBlockedSerial(longlong param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5);
undefined4 aescriptsCheckOveruse(longlong param_1);
undefined8 aescriptsConfigure(int param_1,int param_2);
void aescriptsDropLicenseFromLicenseServer(undefined8 *param_1,longlong param_2);
undefined8 aescriptsGetLicLibVersion(void);
ulonglong aescriptsGetLicenseDataFromFile(byte *param_1,longlong param_2,undefined1 (*param_3) [32]);
undefined4 aescriptsGetLicenseServerConfig(char *param_1,char *param_2,char *param_3,char *param_4);
bool aescriptsGetMachineId(undefined8 *param_1);
undefined8 aescriptsInitLicenseData(undefined1 (*param_1) [32]);
void aescriptsLoadLicenseFromFile(byte *param_1,undefined8 *param_2,longlong param_3,undefined1 (*param_4) [32],char param_5,char param_6);
void aescriptsRequestLicenseFromLicenseServer(undefined8 *param_1,undefined8 *param_2,longlong param_3,char *param_4);
int aescriptsSaveLicenseToFile(byte *param_1,undefined8 *param_2,char *param_3,undefined8 *param_4);
ulonglong aescriptsValidateLicense(undefined8 *param_1,undefined8 *param_2,longlong param_3,undefined1 (*param_4) [32],char param_5);
int FUN_180052f40(char *param_1,char *param_2,undefined8 param_3,undefined8 param_4);
_Init_locks * __thiscall std::_Init_locks::_Init_locks(_Init_locks *this);
_Lockit * __thiscall std::_Lockit::_Lockit(_Lockit *this,int param_1);
void FUN_180053020(void);
void __thiscall std::_Lockit::~_Lockit(_Lockit *this);
void FUN_180053090(void);
void FUN_18005309c(void);
bool __uncaught_exception(void);
undefined8 * FUN_1800530b0(undefined8 *param_1);
undefined8 * FUN_1800530d0(undefined8 *param_1,longlong param_2);
undefined8 * FUN_18005310c(undefined8 *param_1,longlong param_2);
undefined8 * FUN_180053154(undefined8 *param_1,longlong param_2);
undefined8 * FUN_180053190(undefined8 *param_1,longlong param_2);
undefined8 * FUN_1800531cc(undefined8 *param_1,longlong param_2);
undefined8 * FUN_180053214(undefined8 *param_1,longlong param_2);
void FUN_18005325c(void);
void FUN_18005327c(longlong param_1);
void FUN_1800532a0(longlong param_1);
void FUN_1800532c4(longlong param_1);
char * FUN_1800532e8(int param_1);
_Locimp * __thiscall std::locale::_Locimp::_Locimp(_Locimp *this,bool param_1);
_Yarn<char> * __thiscall std::_Yarn<char>::operator=(_Yarn<char> *this,char *param_1);
void * __thiscall std::locale::_Locimp::`scalar_deleting_destructor'(_Locimp *this,uint param_1);
void __cdecl std::_Facet_Register(_Facet_base *param_1);
undefined8 FUN_1800534a4(void);
_Locimp * __cdecl std::locale::_Init(bool param_1);
void __cdecl std::locale::_Locimp::_Locimp_dtor(_Locimp *param_1);
void __cdecl std::_Locinfo::_Locinfo_ctor(_Locinfo *param_1,char *param_2);
void __cdecl std::_Locinfo::_Locinfo_dtor(_Locinfo *param_1);
_Locimp * __cdecl std::locale::_Locimp::_New_Locimp(bool param_1);
void __cdecl std::locale::_Setgloballocale(void *param_1);
void _Deletegloballocale(longlong *param_1);
void FUN_180053748(void);
UINT * FUN_18005377c(UINT *param_1);
ulonglong FUN_18005383c(uint param_1,UINT *param_2);
_Cvtvec * __cdecl _Getcvt(_Cvtvec *__return_storage_ptr__);
ulonglong FUN_180053a08(uint param_1,UINT *param_2);
void FUN_180053b4c(longlong param_1);
void __cdecl std::ios_base::_Ios_base_dtor(ios_base *param_1);
void __thiscall std::ios_base::_Tidy(ios_base *this);
byte FUN_180053cbc(undefined8 *param_1);
DWORD __stdcall GetCurrentThreadId(void);
void FUN_180053ce0(undefined8 *param_1);
void FUN_180053d44(longlong param_1);
void FUN_180053d50(longlong param_1);
void FUN_180053d5c(longlong param_1);
int FUN_180053d68(uint *param_1,ulonglong *param_2);
bool FUN_180053f1c(longlong param_1);
bool FUN_180053f34(void);
void FUN_180053f4c(longlong param_1);
void FUN_180053f58(longlong param_1);
void FUN_180053f64(longlong param_1);
void _Mtx_init_in_situ(undefined4 *param_1,undefined4 param_2);
void FUN_180053fa8(uint *param_1);
undefined8 _Mtx_unlock(longlong param_1);
void FUN_180053fd8(void);
longlong * FUN_180054084(longlong *param_1,ulonglong param_2,undefined8 param_3,undefined8 *param_4);
undefined8 * FUN_180054190(undefined8 *param_1,undefined4 param_2,undefined8 param_3,char *param_4);
void FUN_180054274(int param_1);
void FUN_1800542b0(int param_1);
basic_string<> * __thiscall std::basic_string<>::assign(basic_string<> *this,char *param_1);
undefined ** FUN_180054368(void);
basic_string<> * FUN_180054370(undefined8 param_1,basic_string<> *param_2,int param_3);
char * FUN_180054400(void);
uint _Stolx(byte *param_1,longlong *param_2,uint param_3,undefined4 *param_4);
uint _Stoulx(byte *param_1,undefined8 *param_2,uint param_3,undefined4 *param_4);
ulonglong _Stollx(byte *param_1,longlong *param_2,uint param_3,undefined4 *param_4);
ulonglong _Stoullx(byte *param_1,undefined8 *param_2,uint param_3,undefined4 *param_4);
undefined8 * FUN_1800549c0(undefined8 *param_1,undefined8 param_2);
_iobuf * FUN_1800549dc(char *param_1,uint param_2,int param_3);
_iobuf * thunk_FUN_1800549dc(char *param_1,uint param_2,int param_3);
void __stdcall DeleteCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
void FUN_180054ae0(undefined8 param_1);
void __stdcall EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
void __stdcall LeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
void FUN_180054b00(PVOID param_1);
ulonglong FUN_180054b3c(LPCWSTR param_1,uint param_2,char *param_3,int param_4,undefined8 param_5,int param_6,UINT param_7,int param_8);
int _Xtime_diff_to_millis2(longlong *param_1,longlong *param_2);
longlong _Xtime_get_ticks(void);
undefined4 xtime_get(ulonglong *param_1,int param_2);
undefined8 FUN_180054f90(longlong *param_1);
undefined8 FUN_180054fa8(void);
void __crtGetSystemTimePreciseAsFileTime(LPFILETIME param_1);
void FUN_18005537c(void);
undefined8 * FUN_18005538c(undefined8 *param_1,ulonglong param_2);
void FUN_1800553b8(size_t param_1);
void thunk_FUN_18005be54(LPVOID param_1);
void thunk_FUN_1800553b8(size_t param_1);
void __raise_securityfailure(_EXCEPTION_POINTERS *param_1);
void FUN_180055438(void);
void FUN_18005550c(void);
void FUN_180055520(undefined4 param_1);
void capture_current_context(PCONTEXT param_1);
void capture_previous_context(PCONTEXT param_1);
undefined8 __GSHandlerCheck(undefined8 param_1,undefined8 param_2,undefined8 param_3,longlong param_4);
ulonglong __GSHandlerCheckCommon(undefined8 param_1,longlong param_2);
void __GSHandlerCheck_EH4(EHExceptionRecord *param_1,__uint64 param_2,_CONTEXT *param_3,_xDISPATCHER_CONTEXT *param_4);
void __chkstk(void);
void __cdecl __security_check_cookie(uintptr_t _StackCookie);
ulonglong FUN_180055830(undefined8 param_1,int param_2,longlong param_3);
undefined8 FUN_180055880(undefined8 param_1,undefined8 param_2);
byte FUN_180055998(bool param_1);
int __cdecl dllmain_dispatch(HINSTANCE__ *param_1,ulong param_2,void *param_3);
void entry(HINSTANCE__ *param_1,ulong param_2,void *param_3);
void __cdecl`eh_vector_constructor_iterator'(void *param_1,__uint64 param_2,__uint64 param_3,_func_void_void_ptr *param_4,_func_void_void_ptr *param_5);
void __cdecl`eh_vector_destructor_iterator'(void *param_1,__uint64 param_2,__uint64 param_3,_func_void_void_ptr *param_4);
void __cdecl __ArrayUnwind(void *param_1,__uint64 param_2,__uint64 param_3,_func_void_void_ptr *param_4);
void thunk_FUN_18005be54(LPVOID param_1);
ulonglong __scrt_acquire_startup_lock(void);
undefined8 __scrt_dllmain_after_initialize_c(void);
bool FUN_180055d48(void);
undefined1 __scrt_dllmain_crt_thread_attach(void);
undefined1 __scrt_dllmain_crt_thread_detach(void);
void __scrt_dllmain_exception_filter(undefined8 param_1,int param_2,undefined8 param_3,undefined *param_4,int param_5,undefined8 param_6);
void __scrt_dllmain_uninitialize_c(void);
void FUN_180055e30(void);
ulonglong __scrt_initialize_crt(int param_1);
undefined8 FUN_180055e90(uint param_1);
ulonglong __scrt_is_nonwritable_in_current_image(longlong param_1);
void __scrt_release_startup_lock(char param_1);
undefined8 __scrt_uninitialize_crt(bool param_1,char param_2);
_onexit_t __cdecl _onexit(_onexit_t _Func);
int __cdecl atexit(_func_5014 *param_1);
undefined8 FUN_180056058(void);
void thunk_FUN_18005be54(LPVOID param_1);
void __cdecl __security_init_cookie(void);
void FUN_1800562b8(void);
void FUN_1800562c8(void);
undefined * FUN_1800562d4(void);
void FUN_1800562dc(void);
undefined * FUN_1800562f8(void);
void FUN_180056300(void);
void FUN_180056308(undefined4 param_1);
void FUN_180056454(void);
void FUN_180056490(void);
bool __scrt_is_ucrt_dll_in_use(void);
undefined8 _CallSETranslator<>(undefined4 *param_1,undefined8 param_2,undefined8 param_3);
int __cdecl _CallSETranslator<>(EHExceptionRecord *param_1,__uint64 *param_2,_CONTEXT *param_3,_xDISPATCHER_CONTEXT *param_4,FuncInfo4 *param_5,ulong param_6,__uint64 *param_7,int param_8);
longlong FUN_180056588(longlong param_1,int param_2);
__int64 __cdecl FH4::DecompFuncInfo(uchar *param_1,FuncInfo4 *param_2,__uint64 param_3,int param_4,bool param_5);
bool FUN_180056710(ulonglong *param_1,longlong param_2);
byte FUN_18005673c(undefined8 param_1,byte *param_2);
void __cdecl __FrameHandler3::FrameUnwindToEmptyState(__uint64 *param_1,_xDISPATCHER_CONTEXT *param_2,_s_FuncInfo *param_3);
void __cdecl __FrameHandler4::FrameUnwindToEmptyState(__uint64 *param_1,_xDISPATCHER_CONTEXT *param_2,FuncInfo4 *param_3);
longlong * FUN_1800567dc(longlong *param_1,ulonglong *param_2,longlong param_3,longlong *param_4);
__uint64 * __cdecl __FrameHandler4::GetEstablisherFrame(__uint64 *param_1,_xDISPATCHER_CONTEXT *param_2,FuncInfo4 *param_3,__uint64 *param_4);
undefined8 *FUN_1800568cc(undefined8 *param_1,undefined8 param_2,int param_3,ulonglong *param_4,longlong param_5);
undefined4 * FUN_180056a0c(undefined4 *param_1,int *param_2,int param_3);
void FUN_180056b78(undefined8 *param_1,ULONG_PTR param_2,ULONG_PTR param_3,ULONG_PTR param_4,ULONG_PTR param_5,ULONG_PTR param_6,int param_7,undefined8 param_8,undefined8 param_9,undefined8 *param_10,byte param_11);
void FUN_180056c7c(undefined8 *param_1,ULONG_PTR param_2,ULONG_PTR param_3,ULONG_PTR param_4,ULONG_PTR param_5,undefined8 param_6,int param_7,int param_8,longlong param_9,undefined8 *param_10,byte param_11);
void FUN_180056d9c(longlong param_1,longlong param_2);
undefined8 * _CreateFrameInfo(undefined8 *param_1,undefined8 param_2);
void FUN_180056f9c(longlong param_1);
undefined8 _GetImageBase(void);
undefined8 _GetThrowImageBase(void);
void _SetImageBase(undefined8 param_1);
void FUN_180057030(undefined8 param_1);
void FUN_180057048(EHExceptionRecord *param_1,__uint64 param_2,_CONTEXT *param_3,_xDISPATCHER_CONTEXT *param_4);
void __CxxFrameHandler4(EHExceptionRecord *param_1,__uint64 param_2,_CONTEXT *param_3,_xDISPATCHER_CONTEXT *param_4);
void __stdcall _CxxThrowException(void *pExceptionObject,ThrowInfo *pThrowInfo);
undefined8 FUN_180057230(void);
void FUN_18005723c(void);
void __DestructExceptionObject(int *param_1);
void FUN_1800572c8(undefined8 param_1,undefined *UNRECOVERED_JUMPTABLE);
undefined4 _IsExceptionObjectToBeDestroyed(longlong param_1);
longlong __AdjustPointer(longlong param_1,int *param_2);
undefined8 FUN_180057320(undefined8 *param_1);
longlong FUN_180057388(void);
longlong FUN_18005739c(void);
void FUN_1800573b0(void);
void __std_exception_copy(longlong *param_1,longlong *param_2);
void __std_exception_destroy(undefined8 *param_1);
char * FUN_180057490(char *param_1,uint param_2,ulonglong param_3);
int __cdecl memcmp(void *_Buf1,void *_Buf2,size_t _Size);
void FUN_1800575f0(undefined1 *param_1,undefined1 *param_2,longlong param_3);
undefined8 * FUN_180057610(undefined8 *param_1,undefined8 *param_2,ulonglong param_3);
undefined8 FUN_180057ca0(undefined1 *param_1,undefined1 param_2,longlong param_3,undefined8 param_4);
undefined1 (*) [32] FUN_180057cc0(undefined1 (*param_1) [32],byte param_2,ulonglong param_3);
bool __uncaught_exception(void);
undefined8 FUN_180058068(PEXCEPTION_RECORD param_1,PVOID param_2,longlong param_3,longlong *param_4);
ulonglong __vcrt_initialize(void);
bool __vcrt_thread_attach(void);
undefined1 __vcrt_thread_detach(void);
undefined8 __vcrt_uninitialize(char param_1);
undefined1 FUN_1800582cc(void);
uint __std_type_info_compare(longlong param_1,longlong param_2);
void FUN_180058304(PSLIST_HEADER param_1);
void FUN_180058330(undefined *param_1);
void __vcrt_freeptd(undefined *param_1);
void FUN_1800583a0(void);
LPVOID __vcrt_getptd_noexit(void);
longlong __vcrt_getptd_noinit(void);
uint __vcrt_initialize_ptd(void);
undefined1 __vcrt_uninitialize_ptd(void);
int __cdecl __FrameHandler3::GetCurrentState(__uint64 *param_1,_xDISPATCHER_CONTEXT *param_2,_s_FuncInfo *param_3);
int __cdecl __FrameHandler3::GetUnwindTryBlock(__uint64 *param_1,_xDISPATCHER_CONTEXT *param_2,_s_FuncInfo *param_3);
void __cdecl __FrameHandler3::SetState(__uint64 *param_1,_s_FuncInfo *param_2,int param_3);
void __cdecl __FrameHandler3::SetUnwindTryBlock(__uint64 *param_1,_xDISPATCHER_CONTEXT *param_2,_s_FuncInfo *param_3,int param_4);
void FUN_1800585d4(longlong param_1,ulonglong *param_2);
void FUN_1800585dc(longlong param_1,ulonglong *param_2);
undefined4 FUN_1800585e4(longlong param_1,longlong param_2,ulonglong param_3);
int FUN_18005864c(longlong param_1,longlong param_2,ulonglong param_3);
int __cdecl BuildCatchObjectHelperInternal<>(EHExceptionRecord *param_1,void *param_2,_s_HandlerType *param_3,_s_CatchableType *param_4);
int __cdecl BuildCatchObjectHelperInternal<>(EHExceptionRecord *param_1,void *param_2,HandlerType4 *param_3,_s_CatchableType *param_4);
void __cdecl BuildCatchObjectInternal<>(EHExceptionRecord *param_1,void *param_2,_s_HandlerType *param_3,_s_CatchableType *param_4);
void __cdecl BuildCatchObjectInternal<>(EHExceptionRecord *param_1,void *param_2,HandlerType4 *param_3,_s_CatchableType *param_4);
void __cdecl CatchIt<>(EHExceptionRecord *param_1,__uint64 *param_2,_CONTEXT *param_3,_xDISPATCHER_CONTEXT *param_4,_s_FuncInfo *param_5,_s_HandlerType *param_6,_s_CatchableType *param_7,_s_TryBlockMapEntry *param_8,int param_9,__uint64 *param_10,uchar param_11,uchar param_12);
void __cdecl CatchIt<>(EHExceptionRecord *param_1,__uint64 *param_2,_CONTEXT *param_3,_xDISPATCHER_CONTEXT *param_4,FuncInfo4 *param_5,HandlerType4 *param_6,_s_CatchableType *param_7,TryBlockMapEntry4 *param_8,int param_9,__uint64 *param_10,uchar param_11,uchar param_12);
void FUN_180058e6c(EHExceptionRecord *param_1,__uint64 *param_2,_CONTEXT *param_3,_xDISPATCHER_CONTEXT *param_4,_s_FuncInfo *param_5,byte param_6,int param_7,__uint64 *param_8);
void FUN_180059344(EHExceptionRecord *param_1,__uint64 *param_2,_CONTEXT *param_3,_xDISPATCHER_CONTEXT *param_4,FuncInfo4 *param_5,uchar param_6,int param_7,__uint64 *param_8);
void FUN_180059840(EHExceptionRecord *param_1,__uint64 *param_2,_CONTEXT *param_3,_xDISPATCHER_CONTEXT *param_4,_s_FuncInfo *param_5,int param_6,int param_7,__uint64 *param_8);
void __cdecl FindHandlerForForeignException<>(EHExceptionRecord *param_1,__uint64 *param_2,_CONTEXT *param_3,_xDISPATCHER_CONTEXT *param_4,FuncInfo4 *param_5,int param_6,int param_7,__uint64 *param_8);
int __cdecl TypeMatchHelper<>(_s_HandlerType *param_1,_s_CatchableType *param_2,_s_ThrowInfo *param_3);
int __cdecl TypeMatchHelper<>(HandlerType4 *param_1,_s_CatchableType *param_2,_s_ThrowInfo *param_3);
undefined8 FUN_180059fd0(EHExceptionRecord *param_1,__uint64 *param_2,_CONTEXT *param_3,_xDISPATCHER_CONTEXT *param_4,_s_FuncInfo *param_5,int param_6,__uint64 *param_7,byte param_8);
undefined8 FUN_18005a208(EHExceptionRecord *param_1,__uint64 *param_2,_CONTEXT *param_3,_xDISPATCHER_CONTEXT *param_4,FuncInfo4 *param_5,int param_6,__uint64 *param_7,byte param_8);
uint * FUN_18005a49c(uint *param_1,longlong param_2,longlong param_3,uint param_4);
TryBlockMap4 * __thiscall FH4::TryBlockMap4::TryBlockMap4(TryBlockMap4 *this,FuncInfo4 *param_1,__uint64 param_2);
undefined8 * FUN_18005a5f0(undefined8 *param_1,longlong param_2);
undefined8 * FUN_18005a62c(undefined8 *param_1);
void * __cdecl __FrameHandler3::CxxCallCatchBlock(_EXCEPTION_RECORD *param_1);
void * __cdecl __FrameHandler4::CxxCallCatchBlock(_EXCEPTION_RECORD *param_1);
void FUN_18005aa88(longlong param_1);
int __cdecl ExFilterRethrow(_EXCEPTION_POINTERS *param_1,EHExceptionRecord *param_2,int *param_3);
int __cdecl ExFilterRethrowFH4(_EXCEPTION_POINTERS *param_1,EHExceptionRecord *param_2,int param_3,int *param_4);
void __cdecl __FrameHandler3::FrameUnwindToState(__uint64 *param_1,_xDISPATCHER_CONTEXT *param_2,_s_FuncInfo *param_3,int param_4);
void FUN_18005ae60(longlong *param_1,ulonglong *param_2,longlong param_3,int param_4);
int __cdecl __FrameHandler3::GetHandlerSearchState(__uint64 *param_1,_xDISPATCHER_CONTEXT *param_2,_s_FuncInfo *param_3);
undefined1 FUN_18005b1f4(longlong param_1,int *param_2);
uchar __cdecl Is_bad_exception_allowed(_s_ESTypeList *param_1);
void FUN_18005b36c(longlong param_1,longlong *param_2);
void FUN_18005b408(undefined8 param_1,undefined *UNRECOVERED_JUMPTABLE,undefined8 param_3);
void __cdecl _CallMemberFunction2(void *param_1,void *param_2,void *param_3,int param_4);
void FUN_18005b424(int *param_1,int param_2,int param_3,longlong *param_4,longlong *param_5);
int FUN_18005b4f4(longlong *param_1,int param_2,longlong param_3,int param_4,longlong *param_5);
void _NLG_Notify(void);
void FUN_18005b5f0(void);
void FUN_18005b600(void);
void __except_validate_context_record(longlong param_1);
undefined8 __vcrt_initialize_locks(void);
undefined8 __vcrt_uninitialize_locks(void);
FARPROC FUN_18005b6bc(uint param_1,LPCSTR param_2,uint *param_3,uint *param_4);
void __vcrt_FlsAlloc(undefined8 param_1);
void __vcrt_FlsFree(undefined4 param_1);
void __vcrt_FlsGetValue(undefined4 param_1);
void __vcrt_FlsSetValue(DWORD param_1,LPVOID param_2);
void __vcrt_InitializeCriticalSectionEx(LPCRITICAL_SECTION param_1,DWORD param_2,undefined4 param_3);
void _CallSettingFrame(void);
void _CallSettingFrame_LookupContinuationIndex(void);
void _CallSettingFrame_NotifyContinuationAddr(void);
void _CallSettingFrameEncoded(undefined8 param_1,undefined8 param_2,undefined8 param_3);
void __acrt_call_reportfault(int param_1,DWORD param_2,DWORD param_3);
void FUN_18005bc04(undefined8 param_1);
void FUN_18005bc0c(wchar_t *param_1,wchar_t *param_2,wchar_t *param_3,uint param_4,uintptr_t param_5);
void FUN_18005bcbc(void);
void FUN_18005bcdc(void);
void __cdecl _invoke_watson(wchar_t *param_1,wchar_t *param_2,wchar_t *param_3,uint param_4,uintptr_t param_5);
double FUN_18005bd54(double param_1,int *param_2);
LPVOID _calloc_base(ulonglong param_1,ulonglong param_2);
void FUN_18005be54(LPVOID param_1);
uint FUN_18005be68(void);
void FUN_18005be94(undefined4 param_1);
bool __cdecl __crt_strtox::is_overflow_condition<>(uint param_1,__uint64 param_2);
ulong __cdecl __crt_strtox::parse_integer<>(__crt_locale_pointers *param_1,longlong *param_2,uint param_3,byte param_4);
ulong __cdecl __crt_strtox::parse_integer<>(__crt_locale_pointers *param_1,longlong *param_2,uint param_3,uint param_4);
__uint64 __cdecl __crt_strtox::parse_integer<>(__crt_locale_pointers *param_1,longlong *param_2,uint param_3,byte param_4);
_LocaleUpdate * __thiscall _LocaleUpdate::_LocaleUpdate(_LocaleUpdate *this,__crt_locale_pointers *param_1);
void FUN_18005cc14(longlong *param_1);
uint _ischartype_l(int param_1,uint param_2,_locale_t param_3);
long __cdecl _wtol(wchar_t *_Str);
int __cdecl FID_conflict:atoi(char *_Str);
int __cdecl common_vsprintf<>(__uint64 param_1,char *param_2,__uint64 param_3,char *param_4,__crt_locale_pointers *param_5,char *param_6);
int __cdecl common_vsprintf<>(__uint64 param_1,char *param_2,__uint64 param_3,char *param_4,__crt_locale_pointers *param_5,char *param_6);
bool __thiscall __crt_stdio_output::formatting_buffer::ensure_buffer_is_big_enough<char>(formatting_buffer *this,__uint64 param_1);
void type_case_integer_parse_into_buffer<>(longlong param_1,ulonglong param_2,uint param_3,byte param_4);
void type_case_integer_parse_into_buffer<>(longlong param_1,ulonglong param_2,ulonglong param_3,byte param_4);
void __cdecl __crt_stdio_output::write_multiple_characters<>(string_output_adapter<char> *param_1,char param_2,int param_3,int *param_4);
void __cdecl __crt_stdio_output::crop_zeroes(char *param_1,__crt_locale_pointers *param_2);
void __cdecl __crt_stdio_output::force_decimal_point(char *param_1,__crt_locale_pointers *param_2);
bool __thiscall __crt_stdio_output::output_processor<>::parse_int_from_format_string(output_processor<> *this,int *param_1);
int __thiscall __crt_stdio_output::output_processor<>::process(output_processor<> *this);
int __thiscall __crt_stdio_output::output_processor<>::process(output_processor<> *this);
bool __thiscall __crt_stdio_output::output_processor<>::state_case_normal(output_processor<> *this);
undefined8 state_case_normal_tchar(longlong param_1);
bool __thiscall __crt_stdio_output::output_processor<>::state_case_size(output_processor<> *this);
bool __thiscall __crt_stdio_output::output_processor<>::state_case_type(output_processor<> *this);
undefined8 FUN_18005dda0(undefined4 param_1);
undefined8 type_case_Z(longlong param_1);
undefined8 type_case_a(ulonglong *param_1);
undefined8 type_case_c_tchar(longlong param_1);
ulonglong type_case_integer(longlong param_1,uint param_2,byte param_3);
ulonglong type_case_n(longlong param_1);
undefined8 type_case_s(longlong param_1);
undefined8 write_stored_string_tchar(longlong param_1);
void __thiscall __crt_stdio_output::string_output_adapter<char>::write_string(string_output_adapter<char> *this,char *param_1,int param_2,int *param_3,__crt_deferred_errno_cache *param_4);
int __cdecl common_vsprintf<>(__uint64 param_1,char *param_2,__uint64 param_3,char *param_4,__crt_locale_pointers *param_5,char *param_6);
int __stdio_common_vsprintf_s(__uint64 param_1,char *param_2,__uint64 param_3,char *param_4,__crt_locale_pointers *param_5,char *param_6);
longlong FUN_18005e58c(byte *param_1,byte *param_2);
longlong FUN_18005e630(byte *param_1,byte *param_2);
char * __cdecl strncpy(char *_Dest,char *_Source,size_t _Count);
undefined8 FUN_18005eb74(void);
undefined8 FUN_18005ebb0(void);
undefined8 FUN_18005f0b0(void);
double FUN_18005f7b0(double param_1,double param_2);
double FUN_18005fd70(double param_1,double param_2,int param_3);
undefined8 __acrt_initialize_stdio(void);
undefined * __acrt_iob_func(ulonglong param_1);
void __acrt_uninitialize_stdio(bool param_1);
undefined8 _get_stream_buffer_pointers(longlong param_1,longlong *param_2,longlong *param_3,longlong *param_4);
void FUN_180060120(longlong param_1);
void FUN_18006012c(longlong param_1);
_iobuf * __cdecl common_fsopen<char>(char *param_1,char *param_2,int param_3);
_iobuf * __cdecl common_fsopen<char>(char *param_1,char *param_2,int param_3);
errno_t __cdecl fopen_s(FILE **_File,char *_Filename,char *_Mode);
int __cdecl _fclose_nolock(FILE *_File);
int __cdecl fclose(FILE *_File);
void __thiscall __crt_seh_guarded_call<void>::operator()<>(__crt_seh_guarded_call<void> *this,<> *param_1,<> *param_2,<> *param_3);
void __thiscall __crt_seh_guarded_call<void>::operator()<>(__crt_seh_guarded_call<void> *this,<> *param_1,<> *param_2,<> *param_3);
int __thiscall __crt_seh_guarded_call<int>::operator()<>(__crt_seh_guarded_call<int> *this,<> *param_1,<> *param_2,<> *param_3);
int __cdecl common_flush_all(bool param_1);
undefined8 __acrt_stdio_flush_nolock(FILE *param_1);
int __cdecl _fflush_nolock(FILE *_File);
int __cdecl common_flush_all(bool param_1);
int __cdecl fflush(FILE *_File);
ulonglong _fgetc_nolock(FILE *param_1);
int __cdecl fgetc(FILE *_File);
int __cdecl fgetpos(FILE *_File,fpos_t *_Pos);
int __cdecl fputc(int _Ch,FILE *_File);
size_t __cdecl _fread_nolock_s(void *_DstBuf,size_t _DstSize,size_t _ElementSize,size_t _Count,FILE *_File);
size_t __cdecl fread(void *_DstBuf,size_t _ElementSize,size_t _Count,FILE *_File);
size_t __cdecl fread_s(void *_DstBuf,size_t _DstSize,size_t _ElementSize,size_t _Count,FILE *_File);
int __cdecl fsetpos(FILE *_File,fpos_t *_Pos);
int __cdecl common_fseek(longlong param_1,undefined8 param_2,uint param_3);
bool __cdecl common_fseek_binary_mode_read_only_fast_track_nolock(longlong *param_1,longlong param_2,int param_3);
int __cdecl common_fseek_nolock(FILE *param_1,longlong param_2,int param_3);
int __cdecl common_fseek(longlong param_1,undefined8 param_2,uint param_3);
void FUN_180060ebc(undefined8 param_1,int param_2);
__uint64 operator()<>(undefined8 param_1,longlong *param_2,<> *param_3,longlong *param_4);
__uint64 __thiscall <>::operator()(<> *this);
size_t __cdecl _fwrite_nolock(void *_DstBuf,size_t _Size,size_t _Count,FILE *_File);
size_t __cdecl fwrite(void *_Str,size_t _Size,size_t _Count,FILE *_File);
int __cdecl __acrt_mbs_to_wcs_cp<>(char *param_1,__crt_win32_buffer<> *param_2,uint param_3);
int __thiscall __crt_seh_guarded_call<int>::operator()<>(__crt_seh_guarded_call<int> *this,<> *param_1,<> *param_2,<> *param_3);
int __thiscall <>::operator()(<> *this);
int __cdecl setvbuf(FILE *_File,char *_Buf,int _Mode,size_t _Size);
int __cdecl _ungetc_nolock(int _Ch,FILE *_File);
int __cdecl ungetc(int _Ch,FILE *_File);
undefined4 __acrt_errno_from_os_error(int param_1);
void __acrt_errno_map_os_error(int param_1);
ulong * __cdecl __doserrno(void);
ulong * __cdecl __doserrno(void);
double FUN_180061728(double param_1,int param_2);
LPVOID _malloc_base(ulonglong param_1);
undefined4 common_strtod_l<>(longlong param_1,longlong *param_2,__crt_locale_pointers *param_3);
undefined8 common_strtod_l<>(longlong param_1,longlong *param_2,__crt_locale_pointers *param_3);
SLD_STATUS __cdecl __crt_strtox::parse_floating_point<>(__crt_locale_pointers *param_1,c_string_character_source<char> *param_2,float *param_3);
SLD_STATUS __cdecl __crt_strtox::parse_floating_point<>(__crt_locale_pointers *param_1,c_string_character_source<char> *param_2,double *param_3);
floating_point_parse_result __cdecl __crt_strtox::parse_floating_point_from_source<>(__crt_locale_pointers *param_1,c_string_character_source<char> *param_2,floating_point_string *param_3);
floating_point_parse_result __cdecl __crt_strtox::parse_floating_point_possible_infinity<>(char *param_1,c_string_character_source<char> *param_2,char *param_3);
floating_point_parse_result __cdecl __crt_strtox::parse_floating_point_possible_nan<>(char *param_1,c_string_character_source<char> *param_2,char *param_3);
bool __cdecl FID_conflict:parse_floating_point_possible_nan_is_snan<>(char *param_1,c_string_character_source<char> *param_2);
bool __cdecl FID_conflict:parse_floating_point_possible_nan_is_snan<>(char *param_1,c_string_character_source<char> *param_2);
SLD_STATUS __cdecl __crt_strtox::parse_floating_point_write_result<float>(floating_point_parse_result param_1,floating_point_string *param_2,float *param_3);
SLD_STATUS __cdecl __crt_strtox::parse_floating_point_write_result<double>(floating_point_parse_result param_1,floating_point_string *param_2,double *param_3);
undefined8 FUN_180062700(undefined8 *param_1);
undefined8 FUN_18006273c(undefined8 *param_1);
SLD_STATUS __cdecl __crt_strtox::assemble_floating_point_value(__uint64 param_1,int param_2,bool param_3,bool param_4,floating_point_value *param_5);
SLD_STATUS __cdecl __crt_strtox::assemble_floating_point_value_from_big_integer(big_integer *param_1,uint param_2,bool param_3,bool param_4,floating_point_value *param_5);
SLD_STATUS __cdecl __crt_strtox::convert_decimal_string_to_floating_type_common(floating_point_string *param_1,floating_point_value *param_2);
SLD_STATUS __cdecl __crt_strtox::convert_hexadecimal_string_to_floating_type_common(floating_point_string *param_1,floating_point_value *param_2);
__uint64 __cdecl __crt_strtox::divide(big_integer *param_1,big_integer *param_2);
errno_t __cdecl memcpy_s(void *_Dst,rsize_t _DstSize,void *_Src,rsize_t _MaxCount);
void FUN_180065034(longlong param_1,longlong *param_2);
void FUN_18006503c(longlong param_1,longlong *param_2);
longlong __cdecl FID_conflict:_strtoi64(char *_String,char **_EndPtr,int _Radix);
long __cdecl strtol(char *_Str,char **_EndPtr,int _Radix);
long __cdecl wcstol(wchar_t *_Str,wchar_t **_EndPtr,int _Radix);
char * __cdecl common_getenv<char>(char *param_1);
char * __cdecl common_getenv_nolock<char>(char *param_1);
wchar_t * __cdecl common_getenv_nolock<wchar_t>(wchar_t *param_1);
undefined8 FUN_18006524c(ulonglong *param_1,wchar_t *param_2,ulonglong param_3,wchar_t *param_4);
undefined8 thunk_FUN_18006524c(ulonglong *param_1,wchar_t *param_2,ulonglong param_3,wchar_t *param_4);
char * __cdecl common_getenv<char>(char *param_1);
void terminate(void);
int __cdecl _tolower_l(int _C,_locale_t _Locale);
int __cdecl _toupper_l(int _C,_locale_t _Locale);
int __cdecl tolower(int _C);
int __cdecl toupper(int _C);
bool __cdecl __crt_time_is_leap_year<int>(int param_1);
int FUN_1800656a4(tm *param_1,__int64 *param_2);
tm * __cdecl FID_conflict:_localtime64(__time32_t *_Time);
__time64_t __cdecl _time64(__time64_t *_Time);
int __cdecl _mkdir(char *_Path);
void FUN_180065b38(undefined8 *param_1);
void __cdecl common_end_thread(uint param_1);
__acrt_thread_parameter * __cdecl create_thread_parameter(void *param_1,void *param_2);
uintptr_t __cdecl _beginthreadex(void *_Security,uint _StackSize,_StartAddress *_StartAddress,void *_ArgList,uint _InitFlag,uint *_ThrdAddr);
void FUN_180065d38(uint param_1);
void FUN_180065d50(double param_1);
ulonglong FUN_1800673f0(void);
undefined8 __acrt_initialize_locks(void);
void __acrt_lock(int param_1);
undefined8 __acrt_uninitialize_locks(void);
void __acrt_unlock(int param_1);
void _lock_locales(void);
void FUN_18006838c(void);
wchar_t * operator()<>(undefined8 param_1,int *param_2,undefined8 *param_3,int *param_4);
wchar_t * FUN_1800683dc(undefined8 *param_1);
char * __cdecl setlocale(int _Category,char *_Locale);
ushort * __cdecl __pctype_func(void);
int __cdecl islower(int _C);
int __cdecl isspace(int _C);
int __cdecl isupper(int _C);
UINT __cdecl ___lc_codepage_func(void);
wchar_t ** __cdecl ___lc_locale_name_func(void);
int __cdecl ___mb_cur_max_func(void);
undefined4 FUN_18006899c(undefined8 *param_1);
wchar_t * FUN_180069000(wchar_t *param_1);
void __cdecl abort(void);
size_t __cdecl __strncnt(char *_String,size_t _Cnt);
int __cdecl __acrt_wcs_to_mbs_cp<>(wchar_t *param_1,__crt_win32_buffer<> *param_2,uint param_3);
ulonglong FUN_180069208(undefined1 (*param_1) [32],ulonglong param_2);
ulonglong FUN_180069358(undefined1 (*param_1) [32],ulonglong param_2);
void FUN_180069530(undefined8 param_1);
int __cdecl _callnewh(size_t _Size);
ulonglong _query_new_handler(void);
void _initterm(undefined8 *param_1,undefined8 *param_2);
undefined8 _initterm_e(undefined8 *param_1,undefined8 *param_2);
undefined4 _seh_filter_dll(int param_1,undefined8 param_2);
undefined4 _seh_filter_exe(int param_1,undefined8 param_2);
void __thiscall __crt_seh_guarded_call<void>::operator()<>(__crt_seh_guarded_call<void> *this,<> *param_1,<> *param_2,<> *param_3);
void __thiscall <>::operator()(<> *this);
bool FUN_1800698d4(int param_1);
void __cdecl common_exit(int param_1,_crt_exit_cleanup_mode param_2,_crt_exit_return_mode param_3);
void __cdecl exit_or_terminate_process(uint param_1);
void __cdecl try_cor_exit_process(uint param_1);
void FUN_180069a44(undefined8 param_1);
void __cdecl _cexit(void);
void FUN_180069a5c(int param_1);
undefined4 FUN_180069a68(void);
void __cdecl parse_command_line<char>(char *param_1,char **param_2,char *param_3,__uint64 *param_4,__uint64 *param_5);
LPVOID __acrt_allocate_buffer_for_argv(ulonglong param_1,ulonglong param_2,ulonglong param_3);
int _configure_narrow_argv(int param_1);
int __cdecl common_initialize_environment_nolock<char>(void);
int __cdecl common_initialize_environment_nolock<wchar_t>(void);
undefined8 * FUN_180069eec(char *param_1);
undefined8 * FUN_180069ff4(wchar_t *param_1);
void free_environment<>(undefined8 *param_1);
int __cdecl initialize_environment_by_cloning_nolock<char>(void);
int __cdecl initialize_environment_by_cloning_nolock<wchar_t>(void);
void uninitialize_environment_internal<>(undefined8 *param_1);
void uninitialize_environment_internal<>(undefined8 *param_1);
longlong __dcrt_get_or_create_narrow_environment_nolock(void);
longlong __dcrt_get_or_create_wide_environment_nolock(void);
void __dcrt_uninitialize_environments_nolock(void);
int __cdecl common_initialize_environment_nolock<char>(void);
int __thiscall __crt_seh_guarded_call<int>::operator()<>(__crt_seh_guarded_call<int> *this,<> *param_1,<> *param_2,<> *param_3);
int __thiscall __crt_seh_guarded_call<int>::operator()<>(__crt_seh_guarded_call<int> *this,<> *param_1,<> *param_2,<> *param_3);
int __thiscall <>::operator()(<> *this);
int __thiscall <>::operator()(<> *this);
void FUN_18006a704(undefined8 param_1);
void _execute_onexit_table(undefined8 param_1);
undefined8 _initialize_onexit_table(longlong *param_1);
void _register_onexit_function(undefined8 param_1,undefined8 param_2);
undefined8 FUN_18006a7d4(void);
undefined1 FUN_18006a7f8(void);
undefined1 FUN_18006a808(void);
undefined8 FUN_18006a850(void);
undefined1 FUN_18006a890(void);
void FUN_18006a8ec(void);
bool __acrt_thread_attach(void);
undefined1 __acrt_thread_detach(void);
undefined8 __acrt_uninitialize(bool param_1);
undefined1 FUN_18006a95c(void);
DWORD __stdcall GetCurrentProcessId(void);
errno_t __cdecl strcpy_s(char *_Dst,rsize_t _SizeInBytes,char *_Src);
int __cdecl strncmp(char *_Str1,char *_Str2,size_t _MaxCount);
int __cdecl wcsncmp(wchar_t *_Str1,wchar_t *_Str2,size_t _MaxCount);
void operator()<>(undefined8 param_1,int *param_2,undefined8 *param_3,int *param_4);
void operator()<>(undefined8 param_1,int *param_2,undefined8 *param_3,int *param_4);
void operator()<>(undefined8 param_1,int *param_2,undefined8 *param_3,int *param_4);
void operator()<>(undefined8 param_1,int *param_2,undefined8 *param_3,int *param_4);
void __cdecl construct_ptd_array(__acrt_ptd *param_1);
void FUN_18006ac94(__acrt_ptd *param_1);
void __cdecl destroy_ptd_array(__acrt_ptd *param_1);
void __cdecl replace_current_thread_locale_nolock(__acrt_ptd *param_1,__crt_locale_data *param_2);
void __acrt_freeptd(void);
void FUN_18006ae58(void);
__acrt_ptd * FUN_18006af2c(void);
ulonglong __acrt_getptd_noexit(void);
ulonglong __acrt_initialize_ptd(void);
undefined1 __acrt_uninitialize_ptd(void);
ushort FUN_18006b100(undefined8 param_1);
int _errcode(uint param_1);
double FUN_18006b15c(uint param_1,uint param_2,undefined8 param_3,double param_4,ulonglong param_5);
double FUN_18006b250(uint param_1,uint param_2,undefined8 param_3,undefined8 param_4,double param_5,ulonglong param_6);
bool FUN_18006b358(uint param_1,double *param_2,ulonglong param_3);
double FUN_18006b5c0(double param_1,double param_2);
void _raise_exc(uint *param_1,ulonglong *param_2,ulonglong param_3,uint param_4,uint *param_5,uint *param_6);
void _raise_exc_ex(uint *param_1,ulonglong *param_2,ulonglong param_3,uint param_4,uint *param_5,uint *param_6,int param_7);
void _raise_excf(uint *param_1,ulonglong *param_2,ulonglong param_3,uint param_4,uint *param_5,uint *param_6);
void _set_errno_from_matherr(int param_1);
undefined8 FUN_18006b95c(int param_1,int param_2,undefined8 param_3,undefined8 param_4,undefined8 param_5,uint param_6);
double FUN_18006ba30(double param_1,int *param_2);
undefined8 _set_exp(undefined8 param_1,short param_2);
undefined8 _sptype(ulonglong param_1);
uint _clrfp(void);
uint _ctrlfp(uint param_1,uint param_2);
void _set_statfp(uint param_1);
uint _statfp(void);
LPVOID _calloc_base(ulonglong param_1,ulonglong param_2);
void _free_base(LPVOID param_1);
int __cdecl iswctype(wint_t _C,wctype_t _Type);
int __cdecl _isctype_l(int _C,int _Type,_locale_t _Locale);
void __acrt_update_locale_info(longlong param_1,longlong *param_2);
void __acrt_update_multibyte_info(longlong param_1,longlong *param_2);
void __thiscall __crt_seh_guarded_call<void>::operator()<>(__crt_seh_guarded_call<void> *this,<> *param_1,<> *param_2,<> *param_3);
void __thiscall __crt_seh_guarded_call<void>::operator()<>(__crt_seh_guarded_call<void> *this,<> *param_1,<> *param_2,<> *param_3);
void __thiscall __crt_seh_guarded_call<void>::operator()<>(__crt_seh_guarded_call<void> *this,<> *param_1,<> *param_2,<> *param_3);
void __thiscall <>::operator()(<> *this);
void FUN_18006c1d8(undefined8 *param_1,wchar_t *param_2,rsize_t param_3);
wchar_t * FUN_18006c250(undefined1 (*param_1) [32]);
undefined4 FUN_18006c2d4(void);
void __acrt_uninitialize_locale(void);
void FUN_18006c310(wchar_t *param_1,rsize_t param_2,wchar_t *param_3);
undefined8 FUN_18006c3b4(undefined1 (*param_1) [32],wchar_t *param_2);
void FUN_18006c51c(undefined8 *param_1,undefined8 *param_2);
wchar_t * FUN_18006c5dc(wchar_t *param_1,wchar_t *param_2,rsize_t param_3,wchar_t *param_4,rsize_t param_5,uint *param_6);
void FUN_18006ca5c(wchar_t *param_1,rsize_t param_2,int param_3,undefined8 param_4);
wchar_t * __cdecl _wsetlocale(int _Category,wchar_t *_Locale);
wchar_t * FUN_18006cb68(longlong param_1);
wchar_t * FUN_18006cd54(longlong param_1,int param_2,wchar_t *param_3);
longlong FUN_18006cfe4(longlong param_1,int param_2,wchar_t *param_3);
ulonglong parse_bcp47(undefined1 (*param_1) [32],wchar_t *param_2);
uint FUN_18006d538(longlong param_1,undefined8 *param_2);
ulonglong FUN_18006d580(wchar_t *param_1,longlong *param_2);
ulonglong FUN_18006d60c(longlong param_1,longlong *param_2);
ulonglong FUN_18006d714(longlong param_1,longlong *param_2);
undefined8 string_is_alpha(longlong param_1,ulonglong param_2);
LPVOID _malloc_base(ulonglong param_1);
int __cdecl _mbtowc_l(wchar_t *_DstCh,char *_SrcCh,size_t _SrcSizeInBytes,_locale_t _Locale);
void FUN_18006d9f4(wchar_t *param_1,char *param_2,size_t param_3);
errno_t __cdecl _wctomb_s_l(int *_SizeConverted,char *_MbCh,size_t _SizeInBytes,wchar_t _WCh,_locale_t _Locale);
int __cdecl fp_format_a(double *param_1,char *param_2,__uint64 param_3,char *param_4,__uint64 param_5,int param_6,bool param_7,uint param_8,__crt_locale_pointers *param_9,__acrt_rounding_mode param_10);
int __cdecl fp_format_e(double *param_1,char *param_2,__uint64 param_3,char *param_4,__uint64 param_5,int param_6,bool param_7,uint param_8,__crt_locale_pointers *param_9,__acrt_rounding_mode param_10);
undefined8 FUN_18006e008(undefined1 *param_1,ulonglong param_2,int param_3,char param_4,int param_5,int *param_6,byte param_7,__crt_locale_pointers *param_8);
int __cdecl fp_format_f_internal(char *param_1,__uint64 param_2,int param_3,_strflt *param_4,bool param_5,__crt_locale_pointers *param_6);
void FUN_18006e344(ulonglong *param_1,char *param_2,__uint64 param_3,char *param_4,rsize_t param_5,int param_6,char param_7,int param_8,int param_9,__crt_locale_pointers *param_10);
bool __cdecl should_round_up(double *param_1,__uint64 param_2,short param_3,__acrt_rounding_mode param_4);
ulong FUN_18006e5a4(double *param_1,char *param_2,ulonglong param_3,char *param_4,rsize_t param_5,int param_6,int param_7,ulonglong param_8,__crt_locale_pointers *param_9,__acrt_rounding_mode param_10);
int __cdecl _fileno(FILE *_File);
int __cdecl _get_printf_count_output(void);
ulonglong __acrt_should_use_temporary_buffer(FILE *param_1);
ulonglong __acrt_stdio_begin_temporary_buffering_nolock(FILE *param_1);
void __acrt_stdio_end_temporary_buffering_nolock(char param_1,FILE *param_2);
void __cdecl initialize_inherited_file_handles_nolock(void);
void __cdecl initialize_stdio_handles_nolock(void);
bool __acrt_initialize_lowio(void);
undefined1 __acrt_uninitialize_lowio(void);
ulonglong FUN_18006ece0(ulonglong param_1);
ulonglong FUN_18006ee30(ulonglong param_1);
uint FUN_18006f020(double param_1);
void FUN_18006f098(float param_1);
ulonglong FUN_18006f0ac(ulonglong param_1,undefined8 param_2,uint param_3);
float FUN_18006f150(float param_1,undefined8 param_2,uint param_3);
void FUN_18006f1dc(float param_1);
void FUN_18006f1f0(ulonglong param_1);
undefined8 __acrt_initialize_fma3(void);
void __remainder_piby2d2f_forC(ulonglong param_1,double *param_2,uint *param_3);
ulonglong FUN_18006f410(ulonglong param_1);
uint FUN_18006f620(double param_1,undefined8 param_2,undefined8 param_3,ulonglong param_4);
BOOL operator()<>(undefined8 param_1,int *param_2,undefined8 *param_3,int *param_4);
void * __cdecl try_get_function(function_id param_1,char *param_2,module_id *param_3,module_id *param_4);
undefined8 __acrt_AppPolicyGetProcessTerminationMethodInternal(undefined8 param_1);
undefined8 __acrt_AppPolicyGetThreadInitializationTypeInternal(undefined8 param_1);
undefined8 __acrt_AreFileApisANSI(void);
void __acrt_CompareStringEx(ushort *param_1,DWORD param_2,PCNZWCH param_3,int param_4,PCNZWCH param_5,int param_6,undefined8 param_7,undefined8 param_8,undefined8 param_9);
void __acrt_EnumSystemLocalesEx(undefined8 param_1,undefined4 param_2,undefined8 param_3,undefined8 param_4);
void __acrt_FlsAlloc(undefined8 param_1);
void __acrt_FlsFree(undefined4 param_1);
void __acrt_FlsGetValue(undefined4 param_1);
void __acrt_FlsSetValue(DWORD param_1,LPVOID param_2);
void __acrt_GetLocaleInfoEx(ushort *param_1,LCTYPE param_2,LPWSTR param_3,int param_4);
void __acrt_GetUserDefaultLocaleName(wchar_t *param_1,int param_2);
void __acrt_InitializeCriticalSectionEx(LPCRITICAL_SECTION param_1,DWORD param_2,undefined4 param_3);
void __acrt_IsValidLocaleName(ushort *param_1);
void __acrt_LCIDToLocaleName(uint param_1,wchar_t *param_2,int param_3);
void __acrt_LCMapStringEx(ushort *param_1,DWORD param_2,LPCWSTR param_3,int param_4,LPWSTR param_5,int param_6,undefined8 param_7,undefined8 param_8,undefined8 param_9);
void __acrt_LocaleNameToLCID(ushort *param_1,undefined4 param_2);
void __acrt_RoInitialize(undefined4 param_1);
void __acrt_RoUninitialize(void);
bool __acrt_can_use_vista_locale_apis(void);
void __acrt_eagerly_load_locale_apis(void);
undefined8 __acrt_initialize_winapi_thunks(void);
undefined8 __acrt_uninitialize_winapi_thunks(char param_1);
int __cdecl _fcloseall(void);
void __acrt_stdio_free_buffer_nolock(undefined8 *param_1);
void __cdecl __acrt_stdio_allocate_stream(void);
void __cdecl __acrt_stdio_free_stream(undefined8 *param_1);
void __cdecl find_or_allocate_unused_stream_nolock(void);
char * __cdecl __acrt_stdio_parse_mode<char>(char *param_1);
FILE * __cdecl _openfile(char *_Filename,char *_Mode,int _ShFlag,FILE *_File);
int __thiscall __crt_seh_guarded_call<int>::operator()<>(__crt_seh_guarded_call<int> *this,<> *param_1,<> *param_2,<> *param_3);
int __cdecl _close(int _FileHandle);
int __cdecl _close_nolock(int _FileHandle);
int __thiscall __crt_seh_guarded_call<int>::operator()<>(__crt_seh_guarded_call<int> *this,<> *param_1,<> *param_2,<> *param_3);
int __cdecl _commit(int _FileHandle);
void __cdecl write_double_translated_ansi_nolock(int param_1,char *param_2,uint param_3);
DWORD * __cdecl write_text_ansi_nolock(int param_1,char *param_2,uint param_3);
DWORD * __cdecl write_text_utf16le_nolock(int param_1,char *param_2,uint param_3);
DWORD * __cdecl write_text_utf8_nolock(int param_1,char *param_2,uint param_3);
int __cdecl _write(int _FileHandle,void *_Buf,uint _MaxCharCount);
int __cdecl _write_nolock(int _FileHandle,void *_Buf,uint _MaxCharCount);
ulonglong __acrt_stdio_refill_and_read_narrow_nolock(FILE *param_1);
__int64 __cdecl common_ftell<__int64>(longlong param_1);
__int64 __cdecl common_ftell_nolock<__int64>(FILE *param_1);
__int64 __cdecl common_ftell_read_mode_nolock(FILE *param_1,__int64 param_2,longlong param_3);
__int64 __cdecl common_ftell_translated_utf8_nolock(FILE *param_1,__int64 param_2);
__int64 __cdecl count_newline_bytes(char *param_1,char *param_2,__crt_lowio_text_mode param_3);
__int64 __cdecl common_ftell<__int64>(longlong param_1);
__int64 __cdecl common_ftell_nolock<__int64>(FILE *param_1);
bool __cdecl write_buffer_nolock<char>(undefined1 param_1,FILE *param_2);
bool __cdecl stream_is_at_end_of_file_nolock(longlong *param_1);
ulonglong __acrt_stdio_flush_and_write_narrow_nolock(byte param_1,FILE *param_2);
int __cdecl translate_text_mode_nolock<wchar_t>(int param_1,wchar_t *param_2,__uint64 param_3);
int __cdecl translate_ansi_or_utf8_nolock(int param_1,char *param_2,__uint64 param_3,wchar_t *param_4,__uint64 param_5);
int __cdecl _read(int _FileHandle,void *_DstBuf,uint _MaxCharCount);
int __cdecl _read_nolock(int _FileHandle,void *_DstBuf,uint _MaxCharCount);
__int64 __cdecl common_lseek<__int64>(int param_1,__int64 param_2,int param_3);
__int64 __cdecl common_lseek_nolock<__int64>(int param_1,__int64 param_2,int param_3);
__int64 __cdecl common_lseek<__int64>(int param_1,__int64 param_2,int param_3);
__int64 __cdecl common_lseek_nolock<__int64>(int param_1,__int64 param_2,int param_3);
void __acrt_MultiByteToWideChar(UINT param_1,DWORD param_2,LPCSTR param_3,int param_4,LPWSTR param_5,int param_6);
void __acrt_stdio_allocate_buffer_nolock(undefined8 *param_1);
ulonglong FUN_180072da0(ulonglong param_1,ulonglong param_2);
void fegetround(void);
errno_t __cdecl wcscpy_s(wchar_t *_Dst,rsize_t _SizeInWords,wchar_t *_Src);
int __cdecl _wcsnicoll(wchar_t *_Str1,wchar_t *_Str2,size_t _MaxCount);
int __cdecl _wcsnicoll_l(wchar_t *_Str1,wchar_t *_Str2,size_t _MaxCount,_locale_t _Locale);
int __cdecl _strnicoll(char *_Str1,char *_Str2,size_t _MaxCount);
int __cdecl _strnicoll_l(char *_Str1,char *_Str2,size_t _MaxCount,_locale_t _Locale);
int __cdecl _isleadbyte_l(int _C,_locale_t _Locale);
int __cdecl __acrt_LCMapStringA_stat(__crt_locale_pointers *param_1,wchar_t *param_2,ulong param_3,char *param_4,int param_5,char *param_6,int param_7,int param_8,int param_9);
void __acrt_LCMapStringA(__crt_locale_pointers *param_1,wchar_t *param_2,ulong param_3,char *param_4,int param_5,char *param_6,int param_7,int param_8,int param_9);
undefined4 * FUN_1800734f4(void);
undefined4 * FUN_1800734fc(void);
undefined4 * FUN_180073504(void);
undefined8 FUN_18007350c(void);
undefined8 FUN_180073514(void);
undefined8 FUN_18007351c(undefined4 *param_1);
undefined8 FUN_18007354c(undefined4 *param_1);
undefined8 FUN_18007357c(undefined4 *param_1);
int __cdecl common_gmtime_s<__int64>(tm *param_1,__int64 *param_2);
int __cdecl compute_year(__int64 *param_1,bool *param_2);
tm * __cdecl __getgmtimebuf(void);
int __cdecl common_gmtime_s<__int64>(tm *param_1,__int64 *param_2);
bool FUN_180073888(int *param_1);
void FUN_180073b7c(int param_1,int param_2,int param_3,int param_4,int param_5,int param_6,int param_7,int param_8,int param_9,int param_10,int param_11);
void FUN_180073d74(wchar_t *param_1,wchar_t *param_2,LPSTR param_3,rsize_t param_4);
void FUN_180073e58(wchar_t *param_1);
void FUN_1800740ec(void);
void __cdecl tzset_nolock(void);
void FUN_18007437c(wchar_t *param_1,wchar_t *param_2,LPSTR param_3,uint param_4);
void __cdecl __tzset(void);
int __cdecl _isindst(tm *_Time);
int __cdecl _wmkdir(wchar_t *_Path);
int __acrt_get_begin_thread_init_policy(void);
undefined8 __acrt_get_process_end_policy(void);
undefined8 FUN_180074528(int param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4,undefined8 param_5,undefined8 param_6,uint param_7);
bool _exception_enabled(uint param_1,ulonglong param_2);
undefined8 FUN_18007464c(undefined8 param_1,uint param_2,undefined8 param_3,int param_4,uint param_5,uint param_6,undefined8 param_7,undefined8 param_8,int param_9);
float FUN_180074774(undefined8 param_1,uint param_2,float param_3,int param_4,uint param_5,uint param_6,float param_7,float param_8,int param_9);
ulonglong _handle_nan(ulonglong param_1);
uint FUN_1800748c4(uint param_1);
ulonglong FUN_1800748e0(undefined8 param_1,undefined8 param_2,ulonglong param_3,uint param_4);
float FUN_180074a04(float param_1,float param_2,float param_3,uint param_4);
__uint64 __cdecl _mbstowcs_l_helper(wchar_t *param_1,char *param_2,__uint64 param_3,__crt_locale_pointers *param_4);
errno_t __cdecl _mbstowcs_s_l(size_t *_PtNumOfCharConverted,wchar_t *_DstBuf,size_t _SizeInWords,char *_SrcBuf,size_t _MaxCount,_locale_t _Locale);
errno_t __cdecl mbstowcs_s(size_t *_PtNumOfCharConverted,wchar_t *_DstBuf,size_t _SizeInWords,char *_SrcBuf,size_t _MaxCount);
__uint64 __cdecl _wcstombs_l_helper(char *param_1,wchar_t *param_2,__uint64 param_3,__crt_locale_pointers *param_4);
errno_t __cdecl _wcstombs_s_l(size_t *_PtNumOfCharConverted,char *_Dst,size_t _DstSizeInBytes,wchar_t *_Src,size_t _MaxCountInBytes,_locale_t _Locale);
int __cdecl InternalGetLocaleInfoA(__crt_locale_pointers *param_1,wchar_t *param_2,ulong param_3,char *param_4,int param_5);
undefined8 FUN_1800754a4(__crt_locale_pointers *param_1,int param_2,wchar_t *param_3,ulong param_4,longlong *param_5);
BOOL __acrt_GetStringTypeA(__crt_locale_pointers *param_1,DWORD param_2,LPCSTR param_3,int param_4,LPWORD param_5,UINT param_6,int param_7);
ulonglong operator()<>(undefined8 param_1,int *param_2,undefined8 param_3,int *param_4);
void __acrt_get_sigabrt_handler(void);
void FUN_180075874(undefined8 param_1);
int __cdecl raise(int _SigNum);
LPVOID _realloc_base(LPVOID param_1,ulonglong param_2);
void __acrt_WideCharToMultiByte(uint param_1,uint param_2,LPCWSTR param_3,int param_4,LPSTR param_5,int param_6,LPCSTR param_7,LPBOOL param_8);
void __ascii_wcsicmp(ushort *param_1,ushort *param_2);
int __cdecl _wcsicmp(wchar_t *_Str1,wchar_t *_Str2);
int __cdecl _wcsicmp_l(wchar_t *_Str1,wchar_t *_Str2,_locale_t _Locale);
int __cdecl __acrt_wcs_to_mbs_cp<>(wchar_t *param_1,__crt_win32_buffer<> *param_2,uint param_3);
int FUN_180075f7c(undefined8 *param_1,undefined8 *param_2);
undefined8 FUN_180076188(char *param_1,char *param_2,ulonglong param_3,longlong *param_4);
int __cdecl expand_argument_wildcards<char>(char *param_1,char *param_2,argument_list<char> *param_3);
int thunk_FUN_180075f7c(undefined8 *param_1,undefined8 *param_2);
ulonglong __acrt_GetModuleFileNameA(HMODULE param_1,undefined8 param_2,ulonglong param_3);
void FUN_180076748(undefined8 param_1,int *param_2,undefined8 *param_3,int *param_4);
int __cdecl getSystemCP(int param_1);
void __cdecl setSBCS(__crt_multibyte_data *param_1);
void __cdecl setSBUpLow(__crt_multibyte_data *param_1);
int FUN_180076bf8(int param_1,char param_2,__acrt_ptd *param_3,__crt_multibyte_data **param_4);
__crt_multibyte_data * __cdecl update_thread_multibyte_data_internal(__acrt_ptd *param_1,__crt_multibyte_data **param_2);
undefined8 __acrt_initialize_multibyte(void);
void __acrt_update_thread_multibyte_data(void);
undefined8 _setmbcp_nolock(int param_1,__crt_multibyte_data *param_2);
int __cdecl x_ismbbtype_l(__crt_locale_pointers *param_1,uint param_2,int param_3,int param_4);
int __cdecl _ismbblead(uint _C);
undefined1 FUN_180077234(void);
LPSTR __dcrt_get_narrow_environment_from_os(void);
undefined8 * __dcrt_get_wide_environment_from_os(void);
ulonglong FUN_180077400(char *param_1,int param_2);
ulonglong FUN_180077720(wchar_t *param_1,int param_2);
LPVOID FUN_180077a44(longlong *param_1);
LPVOID FUN_180077b2c(longlong *param_1);
ulonglong thunk_FUN_180077400(char *param_1,int param_2);
ulonglong thunk_FUN_180077720(wchar_t *param_1,int param_2);
LPVOID _recalloc_base(LPVOID param_1,ulonglong param_2,ulonglong param_3);
bool FUN_180077cc4(void);
undefined8 __acrt_execute_initializers(undefined8 *param_1,undefined8 *param_2);
undefined8 __acrt_execute_uninitializers(longlong param_1,longlong param_2);
bool FUN_180077da8(void);
void FUN_180077dc8(undefined8 param_1);
undefined8 FUN_180077dd0(undefined8 param_1);
void __acrt_add_locale_ref(longlong param_1);
void __acrt_free_locale(LPVOID param_1);
int __acrt_locale_add_lc_time_reference(undefined **param_1);
void __acrt_locale_free_lc_time_if_unreferenced(undefined **param_1);
int __acrt_locale_release_lc_time_reference(undefined **param_1);
void __acrt_release_locale_ref(longlong param_1);
undefined ** __acrt_update_thread_locale_data(void);
undefined ** _updatetlocinfoEx_nolock(undefined8 *param_1,undefined **param_2);
undefined4 _get_fpsr(void);
void FUN_180078230(undefined4 param_1);
void _fclrf(void);
undefined4 FUN_180078270(void);
BOOL __stdcall GetStringTypeW(DWORD dwInfoType,LPCWSTR lpSrcStr,int cchSrc,LPWORD lpCharType);
void __acrt_locale_free_monetary(longlong param_1);
undefined8 FUN_18007838c(longlong param_1);
void __acrt_locale_free_numeric(undefined8 *param_1);
undefined8 FUN_18007892c(longlong param_1);
void __cdecl free_crt_array_internal(void **param_1,__uint64 param_2);
bool __cdecl initialize_lc_time(__crt_lc_time_data *param_1,__crt_locale_data *param_2);
void __acrt_locale_free_time(void **param_1);
undefined8 __acrt_locale_initialize_time(__crt_locale_data *param_1);
errno_t __cdecl wcscat_s(wchar_t *_Dst,rsize_t _SizeInWords,wchar_t *_Src);
errno_t __cdecl wcsncat_s(wchar_t *_Dst,rsize_t _SizeInWords,wchar_t *_Src,rsize_t _MaxCount);
errno_t __cdecl wcsncpy_s(wchar_t *_Dst,rsize_t _SizeInWords,wchar_t *_Src,rsize_t _MaxCount);
size_t __cdecl wcscspn(wchar_t *_Str,wchar_t *_Control);
wchar_t * __cdecl wcspbrk(wchar_t *_Str,wchar_t *_Control);
void FUN_180079428(longlong param_1);
void GetLocaleNameFromLangCountry(undefined8 *param_1);
void GetLocaleNameFromLanguage(undefined8 *param_1);
uint FUN_180079638(wchar_t *param_1);
uint FUN_180079924(wchar_t *param_1);
long ProcessCodePage(wchar_t *param_1,longlong param_2);
undefined8 TestDefaultCountry(wchar_t *param_1);
bool TranslateName(longlong param_1,int param_2,longlong *param_3);
undefined8 FUN_180079bdc(short *param_1,uint *param_2,LPWSTR param_3);
uint FUN_180079e40(ushort *param_1);
void GetLcidFromLangCountry(uint *param_1);
void GetLcidFromLanguage(byte *param_1);
uint FUN_18007a090(ushort *param_1);
uint FUN_18007a2dc(ushort *param_1);
int LcidFromHexString(ushort *param_1);
UINT ProcessCodePage(wchar_t *param_1,longlong param_2);
undefined8 TestDefaultLanguage(uint param_1,int param_2);
ulonglong TranslateName(longlong param_1,int param_2,longlong *param_3);
undefined8 __acrt_get_qualified_locale_downlevel(longlong param_1,UINT *param_2,LPWSTR param_3);
__uint64 __cdecl __crt_mbstring::__mbrtowc_utf8(wchar_t *param_1,char *param_2,__uint64 param_3,_Mbstatet *param_4);
__uint64 __cdecl __crt_mbstring::__mbsrtowcs_utf8(wchar_t *param_1,char **param_2,__uint64 param_3,_Mbstatet *param_4);
__uint64 __cdecl __crt_mbstring::__c32rtomb_utf8(char *param_1,wchar32 param_2,_Mbstatet *param_3);
__uint64 __cdecl __crt_mbstring::__c32rtomb_utf8(char *param_1,wchar32 param_2,_Mbstatet *param_3);
ulong FUN_18007ab00(char *param_1,ulonglong param_2,int param_3,int *param_4,int param_5,int param_6);
bool FUN_18007ac5c(ulonglong param_1,int param_2,undefined4 *param_3,char *param_4,rsize_t param_5);
int __cdecl _isatty(int _FileHandle);
undefined8 * __acrt_lowio_create_handle_array(void);
void __acrt_lowio_destroy_handle_array(LPCRITICAL_SECTION param_1);
undefined4 __acrt_lowio_ensure_fh_exists(uint param_1);
void __acrt_lowio_lock_fh(uint param_1);
undefined8 __acrt_lowio_set_os_handle(uint param_1,HANDLE param_2);
void __acrt_lowio_unlock_fh(uint param_1);
int __cdecl _alloc_osfhnd(void);
int __cdecl _free_osfhnd(int param_1);
intptr_t __cdecl _get_osfhandle(int _FileHandle);
int FUN_18007c424(uint param_1,wchar_t *param_2,int param_3);
undefined4 __acrt_DownlevelLocaleNameToLCID(ushort *param_1);
int __ascii_wcsnicmp(ushort *param_1,ushort *param_2,longlong param_3);
int __cdecl _wcsnicmp(wchar_t *_Str1,wchar_t *_Str2,size_t _MaxCount);
int __cdecl _wcsnicmp_l(wchar_t *_Str1,wchar_t *_Str2,size_t _MaxCount,_locale_t _Locale);
int __cdecl __ascii_strnicmp(char *_Str1,char *_Str2,size_t _MaxCount);
int __cdecl _strnicmp(char *_Str1,char *_Str2,size_t _MaxCount);
int __cdecl _strnicmp_l(char *_Str1,char *_Str2,size_t _MaxCount,_locale_t _Locale);
int __cdecl common_sopen_dispatch<char>(char *param_1,int param_2,int param_3,int param_4,int *param_5,int param_6);
ulong FUN_18007c9bc(uint param_1,byte *param_2,uint param_3,char *param_4);
byte * FUN_18007cc50(byte *param_1,uint param_2,int param_3,byte param_4);
int __cdecl truncate_ctrl_z_if_present(int param_1);
errno_t __cdecl _sopen_nolock(int *UnlockFlag,int *_FileHandle,char *_Filename,int _OpenFlag,int _ShareFlag,int _PermissionFlag,int _SecureFlag);
errno_t __cdecl FID_conflict:_sopen_s(int *_FileHandle,char *_Filename,int _OpenFlag,int _ShareFlag,int _PermissionMode);
ulong FUN_18007d06c(undefined4 *param_1,uint *param_2,LPCWSTR param_3,uint param_4,int param_5,byte param_6);
wint_t __cdecl _putwch_nolock(wchar_t _WCh);
uint FUN_18007d490(uint param_1);
uint __acrt_fenv_get_control(void);
uint __acrt_fenv_get_status(void);
void __acrt_fenv_set_control(uint param_1);
void __acrt_fenv_set_status(uint param_1);
ulonglong __acrt_CompareStringW(ushort *param_1,DWORD param_2,undefined1 (*param_3) [32],int param_4,undefined1 (*param_5) [32],int param_6);
int __cdecl InternalCompareStringA(__crt_locale_pointers *param_1,wchar_t *param_2,ulong param_3,char *param_4,int param_5,char *param_6,int param_7,int param_8);
void __acrt_CompareStringA(__crt_locale_pointers *param_1,wchar_t *param_2,ulong param_3,char *param_4,int param_5,char *param_6,int param_7,int param_8);
__uint64 __cdecl __crt_mbstring::__wcsrtombs_utf8(char *param_1,wchar_t **param_2,__uint64 param_3,_Mbstatet *param_4);
errno_t __cdecl strncpy_s(char *_Dst,rsize_t _SizeInBytes,char *_Src,rsize_t _MaxCount);
wint_t __cdecl _towlower_l(wint_t _C,_locale_t _Locale);
void __cdecl qsort(void *_Base,size_t _NumOfElements,size_t _SizeOfElements,_PtFuncCompare *_PtFuncCompare);
byte * FUN_18007e2b0(byte *param_1,byte *param_2);
byte * FUN_18007e350(byte *param_1,byte *param_2);
void FUN_18007e718(uchar *param_1,uchar *param_2);
uchar * __cdecl _mbsdec_l(uchar *_Start,uchar *_Pos,_locale_t _Locale);
bool FUN_18007e7b4(void);
BOOL __acrt_SetEnvironmentVariableA(char *param_1,char *param_2);
undefined8 _msize_base(longlong param_1);
int __cdecl common_xtox<>(ulong param_1,wchar_t *param_2,__uint64 param_3,uint param_4,bool param_5);
int __cdecl common_xtox_s<>(ulong param_1,wchar_t *param_2,__uint64 param_3,uint param_4,bool param_5);
errno_t __cdecl _itow_s(int _Val,wchar_t *_DstBuf,size_t _SizeInWords,int _Radix);
int __cdecl wcscmp(wchar_t *_Str1,wchar_t *_Str2);
__uint64 __cdecl __crt_mbstring::__mbrtoc32_utf8(wchar32 *param_1,char *param_2,__uint64 param_3,_Mbstatet *param_4);
errno_t __cdecl _controlfp_s(uint *_CurrentState,uint _NewValue,uint _Mask);
undefined8 fegetenv(uint *param_1);
undefined8 fesetenv(uint *param_1);
undefined8 feholdexcept(undefined8 *param_1);
double __cdecl ceil(double _X);
double FUN_18007ef00(void);
undefined4 FUN_18007f4ac(void);
undefined8 FUN_18007f4b4(undefined4 *param_1);
int __cdecl _setmode_nolock(int _FileHandle,int _Mode);
int __cdecl _chsize_nolock(int _FileHandle,longlong _Size);
bool __dcrt_lowio_ensure_console_output_initialized(void);
void FUN_18007f784(void);
BOOL __dcrt_write_console(void *param_1,DWORD param_2,LPDWORD param_3);
__uint64 __cdecl __crt_mbstring::__c16rtomb_utf8(char *param_1,wchar16 param_2,_Mbstatet *param_3);
void __acrt_LCMapStringW(ushort *param_1,DWORD param_2,undefined1 (*param_3) [32],int param_4,LPWSTR param_5,int param_6);
uint __cdecl _clearfp(void);
uint __cdecl _control87(uint _NewValue,uint _Mask);
uint __cdecl _control87(uint _NewValue,uint _Mask);
void FUN_18007fcd0(undefined8 param_1,undefined8 param_2,int param_3);
undefined8 FUN_18007fcf0(undefined8 param_1,undefined8 param_2,int param_3,uint param_4,undefined8 param_5);
int __stdcall __WSAFDIsSet(SOCKET param_1,fd_set *param_2);
void inet_ntop(void);
PIMAGE_SECTION_HEADER __cdecl _FindPESection(PBYTE pImageBase,DWORD_PTR rva);
BOOL __cdecl _IsNonwritableInCurrentImage(PBYTE pTarget);
bool FUN_18007fe50(short *param_1);
void __GSHandlerCheck_SEH(PEXCEPTION_RECORD param_1,PVOID param_2,longlong param_3,longlong *param_4);
void __GSHandlerCheck_EH(EHExceptionRecord *param_1,__uint64 param_2,_CONTEXT *param_3,_xDISPATCHER_CONTEXT *param_4);
void _local_unwind(PVOID param_1,PVOID param_2);
undefined1 (*) [16] FUN_18007ffb0(undefined1 (*param_1) [16],uint param_2);
char * FUN_1800800ec(ulonglong param_1,char param_2);
ushort * FUN_18008016c(ushort *param_1,ushort param_2);
void _guard_dispatch_icall(void);
void _guard_dispatch_icall(void);
void FUN_180080230(undefined8 param_1,longlong param_2);
void FUN_1800802a0(undefined8 param_1,longlong param_2);
undefined8 FUN_180080450(undefined8 param_1,longlong param_2);
void FUN_180080540(undefined8 param_1,longlong param_2);
undefined8 FUN_1800805f0(void);
void FUN_180080630(undefined8 param_1,longlong param_2);
void FUN_1800806c0(undefined8 param_1,longlong param_2);
void FUN_1800806e0(undefined8 param_1,longlong param_2);
void `protected:_void___cdecl_CWinApp::UpdatePrinterSelection(int)___ptr64'::__l1::dtor$1(undefined8 param_1,longlong param_2);
undefined8 FUN_1800807a0(void);
undefined8 FUN_1800807e0(undefined8 param_1,longlong param_2);
void FUN_180080880(undefined8 param_1,longlong param_2);
void FUN_1800808d0(undefined8 param_1,longlong param_2);
void FUN_180080900(undefined8 param_1,longlong param_2);
undefined8 FUN_180080a90(void);
void FUN_180080ae0(undefined8 param_1,longlong param_2);
void FUN_180080b10(undefined8 param_1,longlong param_2);
undefined8 FUN_180080b60(undefined8 param_1,longlong param_2);
undefined8 FUN_180080bb0(undefined8 param_1,longlong param_2);
undefined8 FUN_180080c20(undefined8 param_1,longlong param_2);
undefined8 FUN_180080cd0(undefined8 param_1,longlong param_2);
void FUN_180080d30(undefined8 param_1,longlong param_2);
void FUN_180080d70(undefined8 param_1,longlong param_2);
void FUN_180080dc0(undefined8 param_1,longlong param_2);
void FUN_180080df0(undefined8 param_1,longlong param_2);
void FUN_180080e40(undefined8 param_1,longlong param_2);
void FUN_180080f30(undefined8 param_1,longlong param_2);
void FUN_180080f90(undefined8 param_1,longlong param_2);
void FUN_180080fd0(undefined8 param_1,longlong param_2);
undefined8 FUN_1800810e0(void);
undefined8 FUN_180081120(undefined8 param_1,longlong param_2);
void FUN_1800813d0(undefined8 param_1,longlong param_2);
void FUN_180081400(undefined8 param_1,longlong param_2);
void FUN_180081430(undefined8 param_1,longlong param_2);
void FUN_180081530(undefined8 param_1,longlong param_2);
void FUN_180081570(undefined8 param_1,longlong param_2);
void FUN_1800815a0(undefined8 param_1,longlong param_2);
void FUN_1800815d0(undefined8 param_1,longlong param_2);
void FUN_180081600(undefined8 param_1,longlong param_2);
void FUN_180081630(undefined8 param_1,longlong param_2);
void FUN_1800816e0(undefined8 param_1,longlong param_2);
void FUN_180081720(undefined8 param_1,longlong param_2);
void FUN_180081750(undefined8 param_1,longlong param_2);
void FUN_180081780(undefined8 param_1,longlong param_2);
undefined8 FUN_1800818a0(void);
void FUN_1800818e0(undefined8 param_1,longlong param_2);
void FUN_180081910(undefined8 param_1,longlong param_2);
void FUN_1800819f0(undefined8 param_1,longlong param_2);
void FUN_180081a30(undefined8 param_1,longlong param_2);
undefined8 FUN_180081a60(undefined8 param_1,longlong param_2);
void FUN_180081b20(undefined8 param_1,longlong param_2);
void FUN_180081ba0(undefined8 param_1,longlong param_2);
void FUN_180081bf0(undefined8 param_1,longlong param_2);
void FUN_180081c90(undefined8 param_1,longlong param_2);
void FUN_180081d20(undefined8 param_1,longlong param_2);
void FUN_180081d60(undefined8 param_1,longlong param_2);
void FUN_180081db0(undefined8 param_1,longlong param_2);
void FUN_180081de0(undefined8 param_1,longlong param_2);
void FUN_180081e10(undefined8 param_1,longlong param_2);
void FUN_180081e40(undefined8 param_1,longlong param_2);
void FUN_180081e70(undefined8 param_1,longlong param_2);
void FUN_180081ea0(undefined8 param_1,longlong param_2);
void FUN_180081ed0(undefined8 param_1,longlong param_2);
undefined8 FUN_180081f90(undefined8 param_1,longlong param_2);
undefined8 FUN_180081fc0(undefined8 param_1,longlong param_2);
undefined8 FUN_180081ff0(void);
void FUN_180082030(undefined8 param_1,longlong param_2);
void FUN_1800820c0(undefined8 param_1,longlong param_2);
void FUN_180082160(undefined8 param_1,longlong param_2);
void FUN_1800823c0(undefined8 param_1,longlong param_2);
void FUN_180082400(undefined8 param_1,longlong param_2);
void FUN_180082440(undefined8 param_1,longlong param_2);
void FUN_180082470(undefined8 param_1,longlong param_2);
void FUN_1800824a0(undefined8 param_1,longlong param_2);
void FUN_1800824d0(undefined8 param_1,longlong param_2);
void FUN_180082500(undefined8 param_1,longlong param_2);
undefined8 FUN_180082540(undefined8 param_1,longlong param_2);
void FUN_180082620(undefined8 param_1,longlong param_2);
void FUN_180082880(undefined8 param_1,longlong param_2);
void FUN_180082af0(undefined8 param_1,longlong param_2);
void FUN_180082d70(undefined8 param_1,longlong param_2);
void FUN_180082ff0(undefined8 param_1,longlong param_2);
undefined8 FUN_180083040(undefined8 param_1,longlong param_2);
void FUN_1800830b0(undefined8 param_1,longlong param_2);
void FUN_1800830e0(undefined8 param_1,longlong param_2);
void FUN_180083190(undefined8 param_1,longlong param_2);
undefined8 FUN_1800831c0(undefined8 param_1,longlong param_2);
undefined8 FUN_1800831f0(undefined8 param_1,longlong param_2);
undefined8 FUN_180083220(undefined8 param_1,longlong param_2);
undefined8 FUN_180083250(undefined8 param_1,longlong param_2);
undefined8 FUN_180083280(undefined8 param_1,longlong param_2);
undefined8 FUN_1800832b0(undefined8 param_1,longlong param_2);
void FUN_1800832e0(undefined8 param_1,longlong param_2);
undefined8 FUN_180083320(undefined8 param_1,longlong param_2);
undefined8 FUN_180083350(undefined8 param_1,longlong param_2);
void FUN_180083375(undefined8 param_1,longlong param_2);
void FUN_18008338c(undefined8 param_1,longlong param_2);
void FUN_1800833a5(void);
void FUN_1800833b9(undefined8 *param_1,longlong param_2);
void FUN_1800833ef(undefined8 param_1,longlong param_2);
void FUN_18008341b(undefined8 param_1,longlong param_2);
undefined4 FUN_180083447(undefined8 param_1,longlong param_2);
bool FUN_1800834a6(undefined8 *param_1);
undefined8 FUN_1800834be(undefined8 param_1,longlong param_2);
undefined8 FUN_18008354f(undefined8 param_1,longlong param_2);
undefined4 FUN_1800835ee(undefined8 param_1,longlong param_2);
void FUN_180083684(_EXCEPTION_POINTERS *param_1,longlong param_2);
void FUN_1800836a9(undefined8 param_1,longlong param_2);
void FUN_18008372f(_EXCEPTION_POINTERS *param_1,longlong param_2);
void FUN_18008375e(undefined8 param_1,longlong param_2);
void FUN_1800837ef(undefined8 *param_1);
void FUN_180083805(void);
void FUN_180083828(undefined8 *param_1);
void FUN_18008383e(void);
void FUN_180083861(undefined8 param_1,longlong param_2);
void FUN_18008388c(undefined8 param_1,longlong param_2);
void FUN_1800838a4(undefined8 param_1,longlong param_2);
void FUN_1800838bf(undefined8 param_1,longlong param_2);
void FUN_1800838dc(undefined8 param_1,longlong param_2);
void FUN_1800838f6(undefined8 param_1,longlong param_2);
void FUN_180083910(undefined8 param_1,longlong param_2);
void FUN_180083928(undefined8 param_1,longlong param_2);
void FUN_180083940(void);
void FUN_180083959(void);
void FUN_180083972(undefined8 *param_1);
void FUN_180083990(undefined8 param_1,longlong param_2);
void FUN_1800839aa(void);
void FUN_1800839c0(undefined8 *param_1);
undefined8 FUN_1800839db(undefined8 *param_1);
void FUN_180083a07(undefined8 param_1,longlong param_2);
void FUN_180083a21(undefined8 param_1,longlong param_2);
void FUN_180083a42(void);
void FUN_180083a5b(void);
void FUN_180083a74(undefined8 param_1,longlong param_2);
void FUN_180083a8e(undefined8 param_1,longlong param_2);
void FUN_180083aa5(undefined8 param_1,longlong param_2);
void FUN_180083abc(void);
void Unwind@180083ad5(undefined8 param_1,longlong param_2);
void FUN_180083ae1(undefined8 param_1,longlong param_2);
void FUN_180083b02(void);
void FUN_180083b1b(void);
void FUN_180083b34(undefined8 param_1,longlong param_2);
bool FUN_180083b90(undefined8 *param_1);
void FUN_180083bd0(void);
void __thiscall std::_Fac_tidy_reg_t::~_Fac_tidy_reg_t(_Fac_tidy_reg_t *this);
void FUN_180083cf4(void);
void __thiscall _Init_atexit::~_Init_atexit(_Init_atexit *this);

