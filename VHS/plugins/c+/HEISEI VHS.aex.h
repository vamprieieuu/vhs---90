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

typedef struct _s__RTTICompleteObjectLocator _s__RTTICompleteObjectLocator, *P_s__RTTICompleteObjectLocator;

struct _s__RTTICompleteObjectLocator {
    dword signature;
    dword offset; // offset of vbtable within class
    dword cdOffset; // constructor displacement offset
    ImageBaseOffset32 pTypeDescriptor; // ref to TypeDescriptor (RTTI 0) for class
    ImageBaseOffset32 pClassDescriptor; // ref to ClassHierarchyDescriptor (RTTI 3)
};

typedef struct CLIENT_ID CLIENT_ID, *PCLIENT_ID;

struct CLIENT_ID {
    void *UniqueProcess;
    void *UniqueThread;
};

typedef struct _s__RTTIClassHierarchyDescriptor RTTIClassHierarchyDescriptor;

typedef uint64_t __uint64;

typedef struct _s__RTTICompleteObjectLocator RTTICompleteObjectLocator;

typedef struct exception exception, *Pexception;

struct exception { // PlaceHolder Class Structure
};

typedef struct future_error future_error, *Pfuture_error;

struct future_error { // PlaceHolder Class Structure
};

typedef struct _cpinfo _cpinfo, *P_cpinfo;

typedef uint UINT;

typedef uchar BYTE;

struct _cpinfo {
    UINT MaxCharSize;
    BYTE DefaultChar[2];
    BYTE LeadByte[12];
};

typedef struct _cpinfo *LPCPINFO;

typedef union _Fval _Fval, *P_Fval;

union _Fval {
    ushort _Sh[8];
    float _Val;
};

typedef struct _SYSTEM_INFO _SYSTEM_INFO, *P_SYSTEM_INFO;

typedef struct _SYSTEM_INFO *LPSYSTEM_INFO;

typedef union _union_552 _union_552, *P_union_552;

typedef ulong DWORD;

typedef void *LPVOID;

typedef ulonglong ULONG_PTR;

typedef ULONG_PTR DWORD_PTR;

typedef ushort WORD;

typedef struct _struct_553 _struct_553, *P_struct_553;

struct _struct_553 {
    WORD wProcessorArchitecture;
    WORD wReserved;
};

union _union_552 {
    DWORD dwOemId;
    struct _struct_553 s;
};

struct _SYSTEM_INFO {
    union _union_552 u;
    DWORD dwPageSize;
    LPVOID lpMinimumApplicationAddress;
    LPVOID lpMaximumApplicationAddress;
    DWORD_PTR dwActiveProcessorMask;
    DWORD dwNumberOfProcessors;
    DWORD dwProcessorType;
    DWORD dwAllocationGranularity;
    WORD wProcessorLevel;
    WORD wProcessorRevision;
};

typedef struct _OVERLAPPED _OVERLAPPED, *P_OVERLAPPED;

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

typedef int BOOL;

struct _SECURITY_ATTRIBUTES {
    DWORD nLength;
    LPVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
};

typedef enum _FINDEX_INFO_LEVELS {
    FindExInfoStandard=0,
    FindExInfoBasic=1,
    FindExInfoMaxInfoLevel=2
} _FINDEX_INFO_LEVELS;

typedef struct _STARTUPINFOW _STARTUPINFOW, *P_STARTUPINFOW;

typedef wchar_t WCHAR;

typedef WCHAR *LPWSTR;

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

typedef union _RTL_RUN_ONCE _RTL_RUN_ONCE, *P_RTL_RUN_ONCE;

typedef union _RTL_RUN_ONCE *PRTL_RUN_ONCE;

typedef PRTL_RUN_ONCE LPINIT_ONCE;

union _RTL_RUN_ONCE {
    PVOID Ptr;
};

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

typedef struct _RTL_CONDITION_VARIABLE _RTL_CONDITION_VARIABLE, *P_RTL_CONDITION_VARIABLE;

typedef struct _RTL_CONDITION_VARIABLE RTL_CONDITION_VARIABLE;

typedef RTL_CONDITION_VARIABLE *PCONDITION_VARIABLE;

struct _RTL_CONDITION_VARIABLE {
    PVOID Ptr;
};

typedef struct _RTL_SRWLOCK _RTL_SRWLOCK, *P_RTL_SRWLOCK;

typedef struct _RTL_SRWLOCK RTL_SRWLOCK;

typedef RTL_SRWLOCK *PSRWLOCK;

struct _RTL_SRWLOCK {
    PVOID Ptr;
};

typedef struct _RTL_CRITICAL_SECTION _RTL_CRITICAL_SECTION, *P_RTL_CRITICAL_SECTION;

typedef struct _RTL_CRITICAL_SECTION *PRTL_CRITICAL_SECTION;

typedef PRTL_CRITICAL_SECTION LPCRITICAL_SECTION;

typedef struct _RTL_CRITICAL_SECTION_DEBUG _RTL_CRITICAL_SECTION_DEBUG, *P_RTL_CRITICAL_SECTION_DEBUG;

typedef struct _RTL_CRITICAL_SECTION_DEBUG *PRTL_CRITICAL_SECTION_DEBUG;

typedef long LONG;

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

typedef PRTL_CRITICAL_SECTION PCRITICAL_SECTION;

typedef PTOP_LEVEL_EXCEPTION_FILTER LPTOP_LEVEL_EXCEPTION_FILTER;

typedef struct _ACTIVATION_CONTEXT _ACTIVATION_CONTEXT, *P_ACTIVATION_CONTEXT;

struct _ACTIVATION_CONTEXT {
};

typedef struct _TP_WORK _TP_WORK, *P_TP_WORK;

struct _TP_WORK {
};

typedef struct _struct_314 _struct_314, *P_struct_314;

struct _struct_314 {
    ULONGLONG Alignment;
    ULONGLONG Region;
};

typedef struct _TP_CLEANUP_GROUP _TP_CLEANUP_GROUP, *P_TP_CLEANUP_GROUP;

struct _TP_CLEANUP_GROUP {
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

typedef struct _RUNTIME_FUNCTION *PRUNTIME_FUNCTION;

typedef union _union_401 _union_401, *P_union_401;

typedef struct _struct_402 _struct_402, *P_struct_402;

struct _struct_402 {
    DWORD LongFunction:1;
    DWORD Persistent:1;
    DWORD Private:30;
};

union _union_401 {
    DWORD Flags;
    struct _struct_402 s;
};

typedef struct _SLIST_ENTRY _SLIST_ENTRY, *P_SLIST_ENTRY;

typedef struct _SLIST_ENTRY *PSLIST_ENTRY;

struct _SLIST_ENTRY {
    PSLIST_ENTRY Next;
};

typedef struct _TP_CALLBACK_INSTANCE _TP_CALLBACK_INSTANCE, *P_TP_CALLBACK_INSTANCE;

struct _TP_CALLBACK_INSTANCE {
};

typedef struct _UNWIND_HISTORY_TABLE_ENTRY _UNWIND_HISTORY_TABLE_ENTRY, *P_UNWIND_HISTORY_TABLE_ENTRY;

typedef struct _UNWIND_HISTORY_TABLE_ENTRY UNWIND_HISTORY_TABLE_ENTRY;

struct _UNWIND_HISTORY_TABLE_ENTRY {
    DWORD64 ImageBase;
    PRUNTIME_FUNCTION FunctionEntry;
};

typedef union _union_61 _union_61, *P_union_61;

typedef struct _M128A *PM128A;

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

typedef struct _struct_64 _struct_64, *P_struct_64;

typedef ulonglong *PDWORD64;

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

typedef union _union_63 _union_63, *P_union_63;

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

typedef enum _EXCEPTION_DISPOSITION {
    ExceptionContinueExecution=0,
    ExceptionContinueSearch=1,
    ExceptionNestedException=2,
    ExceptionCollidedUnwind=3
} _EXCEPTION_DISPOSITION;

typedef enum _EXCEPTION_DISPOSITION EXCEPTION_DISPOSITION;

typedef EXCEPTION_DISPOSITION (EXCEPTION_ROUTINE)(struct _EXCEPTION_RECORD *, PVOID, struct _CONTEXT *, PVOID);

typedef struct _TP_CALLBACK_INSTANCE *PTP_CALLBACK_INSTANCE;

typedef struct _TP_WORK *PTP_WORK;

typedef void (*PTP_WORK_CALLBACK)(PTP_CALLBACK_INSTANCE, PVOID, PTP_WORK);

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

typedef DWORD TP_VERSION;

typedef struct _UNWIND_HISTORY_TABLE _UNWIND_HISTORY_TABLE, *P_UNWIND_HISTORY_TABLE;

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

typedef struct _TP_POOL _TP_POOL, *P_TP_POOL;

struct _TP_POOL {
};

typedef char CHAR;

typedef CHAR *LPCSTR;

typedef CHAR *LPSTR;

typedef struct _TP_CALLBACK_ENVIRON_V3 _TP_CALLBACK_ENVIRON_V3, *P_TP_CALLBACK_ENVIRON_V3;

typedef struct _TP_CALLBACK_ENVIRON_V3 TP_CALLBACK_ENVIRON_V3;

typedef TP_CALLBACK_ENVIRON_V3 *PTP_CALLBACK_ENVIRON;

typedef struct _TP_POOL *PTP_POOL;

typedef struct _TP_CLEANUP_GROUP *PTP_CLEANUP_GROUP;

typedef void (*PTP_CLEANUP_GROUP_CANCEL_CALLBACK)(PVOID, PVOID);

typedef void (*PTP_SIMPLE_CALLBACK)(PTP_CALLBACK_INSTANCE, PVOID);

typedef enum _TP_CALLBACK_PRIORITY {
    TP_CALLBACK_PRIORITY_HIGH=0,
    TP_CALLBACK_PRIORITY_NORMAL=1,
    TP_CALLBACK_PRIORITY_LOW=2,
    TP_CALLBACK_PRIORITY_INVALID=3
} _TP_CALLBACK_PRIORITY;

typedef enum _TP_CALLBACK_PRIORITY TP_CALLBACK_PRIORITY;

union _union_401 {
    DWORD Flags;
    struct _struct_402 s;
};

struct _TP_CALLBACK_ENVIRON_V3 {
    TP_VERSION Version;
    PTP_POOL Pool;
    PTP_CLEANUP_GROUP CleanupGroup;
    PTP_CLEANUP_GROUP_CANCEL_CALLBACK CleanupGroupCancelCallback;
    PVOID RaceDll;
    struct _ACTIVATION_CONTEXT *ActivationContext;
    PTP_SIMPLE_CALLBACK FinalizationCallback;
    union _union_401 u;
    TP_CALLBACK_PRIORITY CallbackPriority;
    DWORD Size;
};

typedef struct _KNONVOLATILE_CONTEXT_POINTERS _KNONVOLATILE_CONTEXT_POINTERS, *P_KNONVOLATILE_CONTEXT_POINTERS;

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

typedef struct _SLIST_ENTRY *PSLIST_ENTRY.conflict;

typedef struct _IMAGE_SECTION_HEADER *PIMAGE_SECTION_HEADER;

typedef WCHAR *LPWCH;

typedef WCHAR *LPCWSTR;

typedef struct _UNWIND_HISTORY_TABLE *PUNWIND_HISTORY_TABLE;

typedef union _SLIST_HEADER *PSLIST_HEADER;

typedef LARGE_INTEGER *PLARGE_INTEGER;

typedef struct _KNONVOLATILE_CONTEXT_POINTERS *PKNONVOLATILE_CONTEXT_POINTERS;

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

typedef longlong INT_PTR;

typedef ULONG_PTR SIZE_T;

typedef struct _FILETIME *LPFILETIME;

typedef ulong ULONG;

typedef INT_PTR (*FARPROC)(void);

typedef DWORD *LPDWORD;

typedef WORD *LPWORD;

typedef struct HINSTANCE__ HINSTANCE__, *PHINSTANCE__;

struct HINSTANCE__ {
    int unused;
};

typedef BYTE *PBYTE;

typedef struct HINSTANCE__ *HINSTANCE;

typedef HINSTANCE HMODULE;

typedef void *LPCVOID;

typedef BOOL *PBOOL;

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

typedef struct IMAGE_BASE_RELOCATION IMAGE_BASE_RELOCATION, *PIMAGE_BASE_RELOCATION;

struct IMAGE_BASE_RELOCATION {
    dword VirtualAddress;
    dword SizeOfBlock;
};

typedef struct IMAGE_THUNK_DATA64 IMAGE_THUNK_DATA64, *PIMAGE_THUNK_DATA64;

struct IMAGE_THUNK_DATA64 {
    qword StartAddressOfRawData;
    qword EndAddressOfRawData;
    qword AddressOfIndex;
    qword AddressOfCallBacks;
    dword SizeOfZeroFill;
    dword Characteristics;
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

typedef struct _CancellationTokenRegistration _CancellationTokenRegistration, *P_CancellationTokenRegistration;

struct _CancellationTokenRegistration { // PlaceHolder Class Structure
};

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

typedef struct __crt_win32_buffer<wchar_t,__crt_win32_buffer_internal_dynamic_resizing> __crt_win32_buffer<wchar_t,__crt_win32_buffer_internal_dynamic_resizing>, *P__crt_win32_buffer<wchar_t,__crt_win32_buffer_internal_dynamic_resizing>;

struct __crt_win32_buffer<wchar_t,__crt_win32_buffer_internal_dynamic_resizing> { // PlaceHolder Structure
};

typedef struct __acrt_ptd __acrt_ptd, *P__acrt_ptd;

struct __acrt_ptd { // PlaceHolder Structure
};

typedef struct <lambda_751a882b2c74d4b022dec766aa51a29a> <lambda_751a882b2c74d4b022dec766aa51a29a>, *P<lambda_751a882b2c74d4b022dec766aa51a29a>;

struct <lambda_751a882b2c74d4b022dec766aa51a29a> { // PlaceHolder Structure
};

typedef struct __crt_multibyte_data __crt_multibyte_data, *P__crt_multibyte_data;

struct __crt_multibyte_data { // PlaceHolder Structure
};

typedef struct __crt_win32_buffer<char,__crt_win32_buffer_internal_dynamic_resizing> __crt_win32_buffer<char,__crt_win32_buffer_internal_dynamic_resizing>, *P__crt_win32_buffer<char,__crt_win32_buffer_internal_dynamic_resizing>;

struct __crt_win32_buffer<char,__crt_win32_buffer_internal_dynamic_resizing> { // PlaceHolder Structure
};

typedef struct <lambda_6e4b09c48022b2350581041d5f6b0c4c> <lambda_6e4b09c48022b2350581041d5f6b0c4c>, *P<lambda_6e4b09c48022b2350581041d5f6b0c4c>;

struct <lambda_6e4b09c48022b2350581041d5f6b0c4c> { // PlaceHolder Structure
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

typedef struct EHExceptionRecord EHExceptionRecord, *PEHExceptionRecord;

struct EHExceptionRecord { // PlaceHolder Structure
};

typedef struct <lambda_7777bce6b2f8c936911f934f8298dc43> <lambda_7777bce6b2f8c936911f934f8298dc43>, *P<lambda_7777bce6b2f8c936911f934f8298dc43>;

struct <lambda_7777bce6b2f8c936911f934f8298dc43> { // PlaceHolder Structure
};

typedef struct <lambda_2358e3775559c9db80273638284d5e45> <lambda_2358e3775559c9db80273638284d5e45>, *P<lambda_2358e3775559c9db80273638284d5e45>;

struct <lambda_2358e3775559c9db80273638284d5e45> { // PlaceHolder Structure
};

typedef struct <lambda_a37b2b86f63e897a80ea819b0eb08c01> <lambda_a37b2b86f63e897a80ea819b0eb08c01>, *P<lambda_a37b2b86f63e897a80ea819b0eb08c01>;

struct <lambda_a37b2b86f63e897a80ea819b0eb08c01> { // PlaceHolder Structure
};

typedef struct _s_ESTypeList _s_ESTypeList, *P_s_ESTypeList;

struct _s_ESTypeList { // PlaceHolder Structure
};

typedef struct <lambda_99fb1378e971ab6e7edea83e3a7a83a2> <lambda_99fb1378e971ab6e7edea83e3a7a83a2>, *P<lambda_99fb1378e971ab6e7edea83e3a7a83a2>;

struct <lambda_99fb1378e971ab6e7edea83e3a7a83a2> { // PlaceHolder Structure
};

typedef struct _Mbstatet _Mbstatet, *P_Mbstatet;

struct _Mbstatet { // PlaceHolder Structure
};

typedef struct <lambda_410d79af7f07d98d83a3f525b3859a53> <lambda_410d79af7f07d98d83a3f525b3859a53>, *P<lambda_410d79af7f07d98d83a3f525b3859a53>;

struct <lambda_410d79af7f07d98d83a3f525b3859a53> { // PlaceHolder Structure
};

typedef struct __crt_locale_data __crt_locale_data, *P__crt_locale_data;

struct __crt_locale_data { // PlaceHolder Structure
};

typedef struct _s_CatchableType _s_CatchableType, *P_s_CatchableType;

struct _s_CatchableType { // PlaceHolder Structure
};

typedef struct <lambda_638799b9deba96c50f710eeac98168cd> <lambda_638799b9deba96c50f710eeac98168cd>, *P<lambda_638799b9deba96c50f710eeac98168cd>;

struct <lambda_638799b9deba96c50f710eeac98168cd> { // PlaceHolder Structure
};

typedef struct <lambda_886d6c58226a84441f68b9f2b8217b83> <lambda_886d6c58226a84441f68b9f2b8217b83>, *P<lambda_886d6c58226a84441f68b9f2b8217b83>;

struct <lambda_886d6c58226a84441f68b9f2b8217b83> { // PlaceHolder Structure
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

typedef struct <lambda_a6f7d7db0129f75315ebf26d50c089f1> <lambda_a6f7d7db0129f75315ebf26d50c089f1>, *P<lambda_a6f7d7db0129f75315ebf26d50c089f1>;

struct <lambda_a6f7d7db0129f75315ebf26d50c089f1> { // PlaceHolder Structure
};

typedef struct <lambda_ab61a845afdef5b7c387490eaf3616ee> <lambda_ab61a845afdef5b7c387490eaf3616ee>, *P<lambda_ab61a845afdef5b7c387490eaf3616ee>;

struct <lambda_ab61a845afdef5b7c387490eaf3616ee> { // PlaceHolder Structure
};

typedef struct <lambda_d80eeec6fff315bfe5c115232f3240e3> <lambda_d80eeec6fff315bfe5c115232f3240e3>, *P<lambda_d80eeec6fff315bfe5c115232f3240e3>;

struct <lambda_d80eeec6fff315bfe5c115232f3240e3> { // PlaceHolder Structure
};

typedef struct <lambda_f03950bc5685219e0bcd2087efbe011e> <lambda_f03950bc5685219e0bcd2087efbe011e>, *P<lambda_f03950bc5685219e0bcd2087efbe011e>;

struct <lambda_f03950bc5685219e0bcd2087efbe011e> { // PlaceHolder Structure
};

typedef struct <lambda_c5860995281e5c4ce005b3de8f5874ee> <lambda_c5860995281e5c4ce005b3de8f5874ee>, *P<lambda_c5860995281e5c4ce005b3de8f5874ee>;

struct <lambda_c5860995281e5c4ce005b3de8f5874ee> { // PlaceHolder Structure
};

typedef enum _crt_exit_return_mode {
} _crt_exit_return_mode;

typedef struct <lambda_842d9ff0dc9ef11c61343bbaebe7f885> <lambda_842d9ff0dc9ef11c61343bbaebe7f885>, *P<lambda_842d9ff0dc9ef11c61343bbaebe7f885>;

struct <lambda_842d9ff0dc9ef11c61343bbaebe7f885> { // PlaceHolder Structure
};

typedef struct __crt_locale_pointers __crt_locale_pointers, *P__crt_locale_pointers;

struct __crt_locale_pointers { // PlaceHolder Structure
};

typedef struct _xDISPATCHER_CONTEXT _xDISPATCHER_CONTEXT, *P_xDISPATCHER_CONTEXT;

struct _xDISPATCHER_CONTEXT { // PlaceHolder Structure
};

typedef enum _crt_exit_cleanup_mode {
} _crt_exit_cleanup_mode;

typedef struct __crt_seh_guarded_call<void> __crt_seh_guarded_call<void>, *P__crt_seh_guarded_call<void>;

struct __crt_seh_guarded_call<void> { // PlaceHolder Structure
};

typedef struct _LocaleUpdate _LocaleUpdate, *P_LocaleUpdate;

struct _LocaleUpdate { // PlaceHolder Structure
};

typedef struct <lambda_38ce7e780aa69e748d6df282ebc68efe> <lambda_38ce7e780aa69e748d6df282ebc68efe>, *P<lambda_38ce7e780aa69e748d6df282ebc68efe>;

struct <lambda_38ce7e780aa69e748d6df282ebc68efe> { // PlaceHolder Structure
};

typedef struct <lambda_3883c3dff614d5e0c5f61bb1ac94921c> <lambda_3883c3dff614d5e0c5f61bb1ac94921c>, *P<lambda_3883c3dff614d5e0c5f61bb1ac94921c>;

struct <lambda_3883c3dff614d5e0c5f61bb1ac94921c> { // PlaceHolder Structure
};

typedef struct <lambda_d90129c13df834fdcbf8d2b88dafcf2d> <lambda_d90129c13df834fdcbf8d2b88dafcf2d>, *P<lambda_d90129c13df834fdcbf8d2b88dafcf2d>;

struct <lambda_d90129c13df834fdcbf8d2b88dafcf2d> { // PlaceHolder Structure
};

typedef struct <lambda_f6c7be5f7998530c34de24c7437d6b54> <lambda_f6c7be5f7998530c34de24c7437d6b54>, *P<lambda_f6c7be5f7998530c34de24c7437d6b54>;

struct <lambda_f6c7be5f7998530c34de24c7437d6b54> { // PlaceHolder Structure
};

typedef struct __crt_stdio_stream __crt_stdio_stream, *P__crt_stdio_stream;

struct __crt_stdio_stream { // PlaceHolder Structure
};

typedef struct __crt_seh_guarded_call<int> __crt_seh_guarded_call<int>, *P__crt_seh_guarded_call<int>;

struct __crt_seh_guarded_call<int> { // PlaceHolder Structure
};

typedef struct shared_ptr<_EXCEPTION_RECORD_const_> shared_ptr<_EXCEPTION_RECORD_const_>, *Pshared_ptr<_EXCEPTION_RECORD_const_>;

struct shared_ptr<_EXCEPTION_RECORD_const_> { // PlaceHolder Structure
};

typedef struct basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_>, *Pbasic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_>;

struct basic_string<char,struct_std::char_traits<char>,class_std::allocator<char>_> { // PlaceHolder Structure
};

typedef struct shared_ptr<struct__EXCEPTION_RECORD_const_> shared_ptr<struct__EXCEPTION_RECORD_const_>, *Pshared_ptr<struct__EXCEPTION_RECORD_const_>;

struct shared_ptr<struct__EXCEPTION_RECORD_const_> { // PlaceHolder Structure
};

typedef struct shared_ptr<__ExceptionPtr> shared_ptr<__ExceptionPtr>, *Pshared_ptr<__ExceptionPtr>;

struct shared_ptr<__ExceptionPtr> { // PlaceHolder Structure
};

typedef struct function<void___cdecl(void)> function<void___cdecl(void)>, *Pfunction<void___cdecl(void)>;

struct function<void___cdecl(void)> { // PlaceHolder Structure
};

typedef struct exception_ptr exception_ptr, *Pexception_ptr;

struct exception_ptr { // PlaceHolder Structure
};

typedef struct basic_string<char,std::char_traits<char>,std::allocator<char>_> basic_string<char,std::char_traits<char>,std::allocator<char>_>, *Pbasic_string<char,std::char_traits<char>,std::allocator<char>_>;

struct basic_string<char,std::char_traits<char>,std::allocator<char>_> { // PlaceHolder Structure
};

typedef struct error_code error_code, *Perror_code;

struct error_code { // PlaceHolder Structure
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

typedef struct _ContextCallback _ContextCallback, *P_ContextCallback;

struct _ContextCallback { // PlaceHolder Structure
};

typedef struct _Threadpool_chore _Threadpool_chore, *P_Threadpool_chore;

struct _Threadpool_chore { // PlaceHolder Structure
};

typedef struct argument_list<char> argument_list<char>, *Pargument_list<char>;

struct argument_list<char> { // PlaceHolder Structure
};

typedef struct write_result write_result, *Pwrite_result;

struct write_result { // PlaceHolder Structure
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

typedef ulonglong size_t;

typedef size_t rsize_t;

typedef int errno_t;

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

typedef longlong intptr_t;

typedef struct localeinfo_struct *_locale_t;




void FUN_180001000(void);
void FUN_180001050(void);
void FUN_1800010a0(void);
void FUN_1800010c0(void);
void FUN_1800010e8(void);
void FUN_180001114(void);
longlong * FUN_180001140(longlong *param_1,longlong param_2);
void FUN_180001180(longlong *param_1);
void FUN_180001190(longlong *param_1);
void FUN_1800020d0(void);
void FUN_1800020f0(undefined8 *param_1,undefined8 param_2);
void FUN_180002120(undefined8 *param_1,undefined8 param_2);
void FUN_180002150(undefined8 *param_1,undefined8 param_2);
void FUN_180002180(undefined8 *param_1,undefined8 param_2);
void FUN_1800021b0(undefined8 *param_1,undefined8 param_2);
void FUN_1800021e0(undefined8 *param_1,undefined8 param_2);
void FUN_180002210(undefined8 *param_1,undefined8 param_2);
void FUN_180002240(undefined8 *param_1,undefined8 param_2);
void FUN_180002270(undefined8 *param_1,undefined8 param_2);
void FUN_1800022a0(undefined8 *param_1,undefined8 param_2);
void FUN_1800022d0(undefined8 *param_1,undefined8 param_2);
void FUN_180002300(undefined8 *param_1,undefined8 param_2);
longlong * FUN_180002330(longlong *param_1,undefined *param_2,undefined8 *param_3);
longlong * FUN_1800025c0(longlong *param_1,undefined *param_2,undefined8 *param_3);
longlong * FUN_180002850(longlong *param_1,undefined *param_2,undefined8 *param_3);
longlong * FUN_180002ae0(longlong *param_1,undefined *param_2,undefined8 *param_3);
longlong * FUN_180002d70(longlong *param_1,undefined *param_2,undefined8 *param_3);
longlong * FUN_180003000(longlong *param_1,undefined *param_2,undefined8 *param_3);
longlong * FUN_180003290(longlong *param_1,undefined *param_2,undefined8 *param_3);
longlong * FUN_180003520(longlong *param_1,undefined *param_2,undefined8 *param_3);
longlong * FUN_1800037b0(longlong *param_1,undefined *param_2,undefined8 *param_3);
longlong * FUN_180003a40(longlong *param_1,undefined *param_2,undefined8 *param_3);
longlong * FUN_180003cd0(longlong *param_1,undefined *param_2,undefined8 *param_3);
longlong * FUN_180003f60(longlong *param_1,undefined *param_2,undefined8 *param_3);
undefined8 FUN_1800041f0(undefined8 *param_1);
undefined8 FUN_180004200(float *param_1,uint *param_2);
void FUN_1800043d0(ulonglong param_1);
void FUN_180004410(undefined8 *param_1,undefined8 *param_2,longlong param_3);
void FUN_180004580(longlong *param_1,longlong *param_2);
undefined8 * FUN_1800045f0(longlong *param_1,longlong *param_2,undefined8 *param_3);
undefined8 * FUN_1800048b0(int param_1,undefined4 *param_2);
undefined8 * FUN_180004ac0(int param_1,undefined8 *param_2);
undefined8 * FUN_180004c90(int param_1,undefined8 *param_2);
undefined8 * FUN_180004e60(int param_1,undefined8 *param_2);
undefined8 * FUN_180005030(int param_1,undefined8 *param_2);
undefined8 * FUN_180005200(int param_1,undefined8 *param_2);
undefined8 * FUN_1800053d0(int param_1,undefined8 *param_2);
undefined8 * FUN_1800055a0(int param_1,undefined8 *param_2);
undefined8 * FUN_180005770(int param_1,undefined8 *param_2);
undefined8 * FUN_180005940(int param_1,undefined8 *param_2);
undefined8 * FUN_180005b10(int param_1,undefined8 *param_2);
undefined8 * FUN_180005ce0(int param_1,undefined8 *param_2);
undefined8 *FUN_180005eb0(undefined8 *param_1,ulonglong param_2,undefined8 param_3,undefined8 *param_4,ulonglong param_5);
undefined8 **FUN_180006040(undefined8 **param_1,int param_2,undefined8 param_3,undefined8 *param_4,undefined4 *param_5,undefined4 *param_6,undefined4 *param_7);
undefined4 FUN_180006150(undefined4 *param_1,undefined4 param_2);
longlong * FUN_180006160(longlong *param_1,undefined *param_2,undefined8 *param_3);
longlong * FUN_1800061f0(longlong *param_1,undefined *param_2,undefined8 *param_3);
longlong * FUN_180006280(longlong *param_1,undefined *param_2,undefined8 *param_3);
longlong * FUN_180006310(longlong *param_1,undefined *param_2,undefined8 *param_3);
longlong * FUN_1800063a0(longlong *param_1,undefined *param_2,undefined8 *param_3);
longlong * FUN_180006430(longlong *param_1,undefined *param_2,undefined8 *param_3);
longlong * FUN_1800064c0(longlong *param_1,undefined *param_2,undefined8 *param_3);
longlong * FUN_180006550(longlong *param_1,undefined *param_2,undefined8 *param_3);
longlong * FUN_1800065e0(longlong *param_1,undefined *param_2,undefined8 *param_3);
longlong * FUN_180006670(longlong *param_1,undefined *param_2,undefined8 *param_3);
longlong * FUN_180006700(longlong *param_1,undefined *param_2,undefined8 *param_3);
longlong * FUN_180006790(longlong *param_1,undefined *param_2,undefined8 *param_3);
undefined8 FUN_180006820(undefined8 param_1);
float FUN_180006830(uint *param_1);
void FUN_1800069e0(int param_1,int param_2,undefined8 param_3);
void FUN_180006c70(int param_1,int param_2,undefined8 param_3);
void FUN_180006f00(int param_1,int param_2,undefined8 param_3);
void FUN_180007190(int param_1,int param_2,undefined8 param_3);
void FUN_180007420(int param_1,int param_2,undefined8 param_3);
void FUN_1800076b0(int param_1,int param_2,undefined8 param_3);
void FUN_180007940(int param_1,int param_2,undefined8 param_3);
void FUN_180007bd0(int param_1,int param_2,undefined8 param_3);
void FUN_180007e60(int param_1,int param_2,undefined8 param_3);
void FUN_1800080f0(int param_1,int param_2,undefined8 param_3);
void FUN_180008380(int param_1,int param_2,undefined8 param_3);
undefined8 * FUN_180008610(undefined8 *param_1,undefined8 param_2);
void FUN_180008690(undefined4 *param_1,undefined8 param_2);
void FUN_1800086c0(undefined8 *param_1,undefined8 param_2);
undefined4 * FUN_1800086e0(undefined4 *param_1,undefined4 *param_2);
undefined8 * FUN_180008710(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_180008720(undefined8 *param_1,ulonglong param_2);
ulonglong * FUN_1800087e0(ulonglong *param_1,ulonglong param_2,int *param_3);
ulonglong * FUN_1800088b0(ulonglong *param_1,ulonglong param_2);
undefined8 **** FUN_180008910(undefined8 ****param_1,undefined8 *param_2,undefined8 *param_3);
undefined8 * FUN_180008bb0(undefined8 *param_1,longlong param_2);
undefined8 * FUN_180008c00(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_180008cc0(undefined8 *param_1,longlong param_2,undefined8 *param_3);
undefined8 * FUN_180008e60(undefined8 *param_1,longlong param_2);
undefined8 * FUN_180008ea0(undefined8 *param_1,longlong param_2);
undefined8 * FUN_180008ee0(undefined8 *param_1);
exception * __thiscall std::exception::exception(exception *this,exception *param_1);
exception_ptr * __thiscall std::exception_ptr::exception_ptr(exception_ptr *this,exception_ptr *param_1);
undefined8 * FUN_180008f70(undefined8 *param_1,longlong param_2);
undefined8 * FUN_180008fb0(undefined8 *param_1,longlong param_2);
undefined8 * FUN_180009000(undefined8 *param_1,longlong param_2);
undefined8 * FUN_180009040(undefined8 *param_1,longlong param_2);
undefined8 * FUN_180009090(undefined8 *param_1,longlong param_2);
undefined8 **** FUN_1800090f0(undefined8 ****param_1,undefined8 *param_2);
undefined8 * FUN_1800091a0(undefined8 *param_1);
void FUN_180009220(longlong *param_1);
void FUN_180009270(longlong param_1);
void FUN_1800092c0(longlong *param_1);
void FUN_1800092e0(undefined8 *param_1);
void FUN_180009360(undefined8 *param_1);
void FUN_1800093e0(undefined8 *param_1);
void FUN_180009460(undefined8 *param_1);
void FUN_1800094e0(undefined8 *param_1);
void FUN_180009560(undefined8 *param_1);
void FUN_1800095e0(undefined8 *param_1);
void FUN_180009660(undefined8 *param_1);
void FUN_1800096e0(undefined8 *param_1);
void FUN_180009760(undefined8 *param_1);
void FUN_1800097e0(undefined8 *param_1);
void FUN_180009860(undefined8 *param_1);
void FUN_1800098e0(undefined8 *param_1);
void FUN_180009960(longlong *param_1);
void FUN_1800099b0(longlong *param_1);
void FUN_180009a20(longlong param_1);
void FUN_180009aa0(undefined8 *param_1);
void FUN_180009ad0(longlong *param_1);
void FUN_180009b30(longlong *param_1);
void FUN_180009ba0(void);
void FUN_180009bb0(int *param_1);
void FUN_180009c50(undefined8 *param_1);
void FUN_180009ca0(longlong *param_1);
void FUN_180009d10(longlong param_1);
void FUN_180009d70(void);
void FUN_180009d90(undefined8 *param_1);
void FUN_180009ea0(longlong *param_1);
void thunk_FUN_18001737c(void *param_1);
void FUN_180009ef0(longlong param_1);
undefined8 FUN_180009fd0(undefined8 param_1,undefined8 param_2);
undefined8 * FUN_180009fe0(undefined8 *param_1,undefined8 *param_2);
undefined4 FUN_18000a060(undefined4 *param_1);
void FUN_18000a070(undefined8 *param_1,int param_2,int param_3);
void FUN_18000a430(undefined8 *param_1,int param_2,int param_3);
void FUN_18000a670(longlong *param_1);
void FUN_18000a810(undefined8 *param_1,int param_2,int param_3);
void FUN_18000a9b0(undefined8 *param_1,int param_2,int param_3);
void FUN_18000acb0(undefined8 *param_1,int param_2,int param_3);
ulonglong * FUN_18000afd0(undefined8 *param_1,ulonglong *param_2,undefined8 param_3);
void FUN_18000b090(undefined8 *param_1,uint param_2,int param_3);
void FUN_18000b760(undefined8 *param_1,longlong *param_2,int param_3,int param_4);
void FUN_18000bf20(undefined8 *param_1,int param_2,int param_3);
float FUN_18000c770(undefined8 *param_1,longlong *param_2);
void FUN_18000cbd0(undefined8 *param_1,int param_2,int param_3);
undefined8 * FUN_18000ce60(undefined8 *param_1,uint param_2);
undefined8 * FUN_18000ced0(undefined8 *param_1,ulonglong param_2);
undefined8 * FUN_18000cf50(undefined8 *param_1,uint param_2);
undefined8 * FUN_18000cf90(undefined8 *param_1,uint param_2);
undefined8 * FUN_18000cfd0(undefined8 *param_1,uint param_2);
undefined8 * FUN_18000d010(undefined8 *param_1,uint param_2);
undefined8 * FUN_18000d050(undefined8 *param_1,uint param_2);
undefined8 * FUN_18000d090(undefined8 *param_1,uint param_2);
undefined8 * FUN_18000d0d0(undefined8 *param_1,uint param_2);
undefined8 * FUN_18000d110(undefined8 *param_1,uint param_2);
undefined8 * FUN_18000d150(undefined8 *param_1,uint param_2);
undefined8 * FUN_18000d190(undefined8 *param_1,uint param_2);
undefined8 * FUN_18000d1d0(undefined8 *param_1,uint param_2);
undefined8 * FUN_18000d210(undefined8 *param_1,uint param_2);
undefined8 * FUN_18000d250(undefined8 *param_1,uint param_2);
undefined8 * FUN_18000d290(undefined8 *param_1,uint param_2);
undefined8 * FUN_18000d2d0(undefined8 *param_1,uint param_2);
undefined8 * FUN_18000d310(undefined8 *param_1,uint param_2);
undefined8 * FUN_18000d350(undefined8 *param_1,uint param_2);
undefined8 * FUN_18000d390(undefined8 *param_1,uint param_2);
undefined8 * FUN_18000d3d0(undefined8 *param_1,uint param_2);
undefined8 * FUN_18000d410(undefined8 *param_1,uint param_2);
undefined8 * FUN_18000d450(undefined8 *param_1,uint param_2);
undefined8 * FUN_18000d490(undefined8 *param_1,uint param_2);
undefined8 * FUN_18000d4d0(undefined8 *param_1,uint param_2);
undefined8 * FUN_18000d510(undefined8 *param_1,uint param_2);
undefined8 * FUN_18000d550(undefined8 *param_1,uint param_2);
undefined8 * FUN_18000d590(undefined8 *param_1,ulonglong param_2);
undefined8 * FUN_18000d5c0(undefined8 *param_1,ulonglong param_2);
undefined8 * FUN_18000d5f0(undefined8 *param_1,uint param_2);
undefined8 * FUN_18000d6a0(undefined8 *param_1,uint param_2);
void * __thiscall Concurrency::details::_CancellationTokenRegistration::`scalar_deleting_destructor'(_CancellationTokenRegistration *this,uint param_1);
LPVOID FUN_18000d7b0(LPVOID param_1,ulonglong param_2);
undefined8 * FUN_18000d7e0(undefined8 *param_1,uint param_2);
undefined8 * FUN_18000d820(undefined8 *param_1,ulonglong param_2);
undefined8 * FUN_18000d850(undefined8 *param_1,uint param_2);
undefined8 * FUN_18000d8a0(undefined8 *param_1,ulonglong param_2);
undefined8 * FUN_18000d8d0(undefined8 *param_1,uint param_2);
undefined8 * FUN_18000d910(undefined8 *param_1,uint param_2);
void FUN_18000d960(longlong param_1,longlong param_2);
void FUN_18000e840(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4,uint param_5,int param_6,char *param_7,uint param_8);
ulonglong FUN_1800112e0(longlong param_1,longlong param_2,longlong param_3);
void FUN_1800128a0(longlong *param_1);
void FUN_180012900(ulonglong *param_1,ulonglong param_2);
void FUN_180012990(longlong *param_1);
void FUN_180012a30(undefined8 *param_1);
ulonglong FUN_180012a70(longlong param_1,char param_2,char param_3,undefined8 param_4,undefined8 *param_5);
undefined1 FUN_180012c30(longlong *param_1,void *param_2);
void FUN_180012d80(longlong *param_1);
undefined8 * FUN_180012dd0(longlong param_1);
undefined8 * FUN_180012e40(longlong param_1,undefined8 *param_2);
undefined8 * FUN_180012e60(longlong param_1,undefined8 *param_2);
undefined8 * FUN_180012e80(longlong param_1,undefined8 *param_2);
undefined8 * FUN_180012ea0(longlong param_1,undefined8 *param_2);
undefined8 * FUN_180012ec0(longlong param_1,undefined8 *param_2);
undefined8 * FUN_180012ee0(longlong param_1,undefined8 *param_2);
undefined8 * FUN_180012f00(longlong param_1,undefined8 *param_2);
undefined8 * FUN_180012f20(longlong param_1,undefined8 *param_2);
undefined8 * FUN_180012f40(longlong param_1,undefined8 *param_2);
undefined8 * FUN_180012f60(longlong param_1,undefined8 *param_2);
undefined8 * FUN_180012f80(longlong param_1,undefined8 *param_2);
undefined8 * FUN_180012fa0(longlong param_1,undefined8 *param_2);
undefined8 * FUN_180012fc0(longlong param_1,undefined8 *param_2);
undefined8 * FUN_180012fe0(longlong param_1,undefined8 *param_2);
undefined8 * FUN_180013020(longlong param_1,undefined8 *param_2);
void FUN_180013040(longlong param_1,undefined8 param_2);
void FUN_180013060(longlong param_1,undefined8 param_2);
void FUN_180013080(longlong param_1,undefined8 param_2);
void FUN_1800130a0(longlong param_1,undefined8 param_2);
void FUN_1800130c0(longlong param_1,undefined8 param_2);
void FUN_1800130e0(longlong param_1,undefined8 param_2);
void FUN_180013100(longlong param_1,undefined8 param_2);
void FUN_180013120(longlong param_1,undefined8 param_2);
void FUN_180013140(longlong param_1,undefined8 param_2);
void FUN_180013160(longlong param_1,undefined8 param_2);
void FUN_180013180(longlong param_1,undefined8 param_2);
void FUN_1800131a0(longlong param_1,undefined8 param_2);
void FUN_1800131c0(longlong *param_1,longlong param_2,undefined8 *param_3);
void FUN_180013510(LPVOID param_1,char param_2);
void FUN_180013570(LPVOID param_1,char param_2);
void FUN_180013580(LPVOID param_1,char param_2);
void FUN_180013600(LPVOID param_1,char param_2);
void FUN_180013610(LPVOID param_1,char param_2);
void FUN_180013620(longlong *param_1);
void FUN_180013640(longlong param_1,longlong *param_2);
void FUN_180013790(longlong param_1);
void FUN_1800137a0(longlong param_1);
void FUN_1800137b0(undefined8 *param_1);
ulonglong FUN_1800137d0(longlong param_1);
void FUN_1800137f0(longlong param_1);
void FUN_180013800(longlong param_1);
void FUN_180013810(longlong param_1);
void FUN_180013820(longlong param_1);
void FUN_180013840(longlong param_1);
void FUN_180013860(longlong param_1);
void FUN_180013870(longlong param_1);
void FUN_180013880(longlong param_1);
void FUN_180013890(longlong param_1);
void FUN_1800138a0(longlong param_1);
void FUN_180013900(longlong param_1);
void FUN_180013a80(longlong param_1);
void FUN_180013a90(longlong param_1);
void FUN_180013aa0(longlong param_1);
void FUN_180013ba0(longlong param_1);
void FUN_180013bb0(longlong param_1);
void FUN_180013bc0(longlong param_1,undefined8 *param_2,char param_3);
void FUN_180013c00(longlong param_1);
void FUN_180013cd0(longlong param_1,undefined1 param_2);
longlong FUN_180013d90(longlong param_1);
undefined8 FUN_180013da0(void);
longlong FUN_180013db0(longlong *param_1,char param_2);
longlong FUN_180013ee0(longlong *param_1,char param_2);
bool FUN_180014020(longlong param_1);
undefined1 FUN_180014030(void);
bool FUN_180014040(longlong param_1);
void FUN_180014050(longlong *param_1);
undefined8 * FUN_1800140e0(longlong param_1,undefined8 *param_2);
undefined8 * FUN_180014120(longlong param_1,undefined8 *param_2);
undefined8 * FUN_180014160(longlong param_1,undefined8 *param_2);
undefined8 * FUN_180014180(longlong param_1,undefined8 *param_2);
undefined8 * FUN_1800141a0(longlong param_1,undefined8 *param_2);
undefined8 * FUN_1800141c0(longlong param_1,undefined8 *param_2);
undefined8 * FUN_1800141e0(longlong param_1,undefined8 *param_2);
undefined8 * FUN_180014200(longlong param_1,undefined8 *param_2);
undefined8 * FUN_180014220(longlong param_1,undefined8 *param_2);
undefined8 * FUN_180014240(longlong param_1,undefined8 *param_2);
undefined8 * FUN_180014260(longlong param_1,undefined8 *param_2);
undefined8 * FUN_180014280(longlong param_1,undefined8 *param_2);
undefined8 * FUN_1800142a0(longlong param_1,undefined8 *param_2);
void FUN_1800142c0(void);
longlong FUN_1800142f0(longlong param_1,longlong *param_2);
void FUN_180014410(longlong param_1,longlong *param_2);
void FUN_1800144d0(undefined4 *param_1);
void FUN_180014510(longlong *param_1);
void FUN_180014550(longlong param_1);
void _guard_check_icall(void);
void FUN_180014670(longlong *param_1,longlong *param_2);
void FUN_1800146f0(longlong param_1,undefined **param_2);
void FUN_1800147b0(longlong param_1,int param_2);
void FUN_1800148f0(longlong param_1,undefined **param_2,int param_3);
void FUN_180014960(longlong *param_1,void *param_2,undefined1 param_3);
TypeDescriptor * FUN_180014a40(void);
TypeDescriptor * FUN_180014a50(void);
TypeDescriptor * FUN_180014a60(void);
TypeDescriptor * FUN_180014a70(void);
TypeDescriptor * FUN_180014a80(void);
TypeDescriptor * FUN_180014a90(void);
TypeDescriptor * FUN_180014aa0(void);
TypeDescriptor * FUN_180014ab0(void);
TypeDescriptor * FUN_180014ac0(void);
TypeDescriptor * FUN_180014ad0(void);
TypeDescriptor * FUN_180014ae0(void);
TypeDescriptor * FUN_180014af0(void);
TypeDescriptor * FUN_180014b00(void);
TypeDescriptor * FUN_180014b10(void);
TypeDescriptor * FUN_180014b20(void);
TypeDescriptor * FUN_180014b30(void);
TypeDescriptor * FUN_180014b40(void);
TypeDescriptor * FUN_180014b50(void);
TypeDescriptor * FUN_180014b60(void);
TypeDescriptor * FUN_180014b70(void);
TypeDescriptor * FUN_180014b80(void);
TypeDescriptor * FUN_180014b90(void);
TypeDescriptor * FUN_180014ba0(void);
TypeDescriptor * FUN_180014bb0(void);
TypeDescriptor * FUN_180014bc0(void);
TypeDescriptor * FUN_180014bd0(void);
TypeDescriptor * FUN_180014be0(void);
TypeDescriptor * FUN_180014bf0(void);
void FUN_180014c00(void);
void FUN_180014c20(undefined4 param_1);
void FUN_180014c60(longlong *param_1);
void FUN_180014cf0(longlong param_1);
void FUN_180014d10(undefined8 *param_1);
ulonglong FUN_180014d70(longlong param_1);
void FUN_180014e40(void);
void FUN_180014e60(void);
ulonglong FUN_180014e80(undefined8 param_1,ulonglong param_2);
longlong * FUN_180014ef0(longlong *param_1,undefined8 *param_2,ulonglong param_3);
void FUN_180014f80(undefined8 param_1,undefined4 param_2,int param_3,float param_4);
longlong * FUN_1800150b0(longlong *param_1,undefined8 *param_2,ulonglong param_3);
float FUN_180015210(float param_1,float param_2,float param_3,float param_4,float param_5);
void __cdecl FID_conflict:_Current_exception(void);
void FUN_180015310(undefined8 param_1,LPVOID param_2,longlong param_3);
undefined4 * FUN_180015360(undefined8 param_1,undefined4 *param_2,undefined4 param_3);
longlong FUN_180015370(longlong param_1,int *param_2,int param_3);
undefined8 FUN_180015390(longlong *param_1,undefined4 param_2,int *param_3);
undefined * FUN_1800153d0(void);
void FUN_180015490(longlong param_1);
void FUN_1800155e0(longlong param_1);
void FUN_180015730(longlong param_1);
void FUN_180015880(longlong param_1);
void FUN_1800159d0(longlong param_1);
void FUN_180015b20(longlong param_1);
void FUN_180015c70(longlong param_1);
void FUN_180015dc0(longlong param_1);
void FUN_180015f10(longlong param_1);
void FUN_180016060(longlong param_1);
void FUN_1800161b0(longlong param_1);
void FUN_180016300(longlong param_1);
undefined4 * FUN_180016450(undefined4 *param_1,undefined4 param_2);
undefined4 * FUN_180016470(undefined4 *param_1,undefined4 param_2);
longlong * FUN_180016490(undefined8 param_1,longlong *param_2,int param_3);
longlong * FUN_180016540(undefined8 param_1,longlong *param_2,int param_3);
char * FUN_1800165e0(void);
char * FUN_1800165f0(void);
void FUN_180016600(longlong *param_1);
void FUN_180016610(undefined8 param_1,undefined8 param_2,undefined8 param_3);
char * FUN_1800166b0(longlong param_1);
ulonglong EffectMain(int param_1,longlong param_2,undefined4 *param_3,undefined8 param_4,undefined8 param_5,longlong *param_6);
void PluginDataEntryFunction2(undefined8 param_1,undefined *param_2);
void FUN_1800169c0(undefined1 param_1);
undefined ** FUN_1800169d0(uint param_1);
undefined8 * FUN_180016a00(undefined8 *param_1);
undefined8 * FUN_180016a30(undefined8 *param_1,longlong param_2);
undefined8 * FUN_180016a74(undefined8 *param_1);
void FUN_180016aa4(longlong param_1);
void FUN_180016ad4(longlong param_1);
void FUN_180016b24(undefined8 *param_1);
shared_ptr<> * __thiscall std::shared_ptr<>::operator=(shared_ptr<> *this,shared_ptr<> *param_1);
undefined8 * FUN_180016bdc(undefined8 *param_1,undefined8 *param_2);
LPVOID FUN_180016c74(LPVOID param_1,uint param_2);
void * __thiscall FID_conflict:`scalar_deleting_destructor'(void *this,uint param_1);
LPVOID FUN_180016d44(LPVOID param_1,ulonglong param_2);
undefined8 * FUN_180016d68(undefined8 *param_1,uint param_2);
void FUN_180016db0(shared_ptr<> *param_1,undefined4 *param_2);
void FUN_180016f88(shared_ptr<> *param_1,undefined4 *param_2,undefined8 *param_3);
void FUN_1800170d8(undefined8 param_1,undefined *UNRECOVERED_JUMPTABLE);
void FUN_1800170dc(undefined8 param_1,undefined *UNRECOVERED_JUMPTABLE,undefined8 param_3);
void __cdecl _CallMemberFunction2(void *param_1,void *param_2,void *param_3,int param_4);
void __cdecl _anon_13FC80AB::_CopyExceptionObject(void *param_1,void *param_2,_s_CatchableType *param_3,__uint64 param_4);
void FUN_180017190(LPVOID param_1);
void FUN_1800171a0(longlong param_1);
void FUN_180017228(undefined4 *param_1,longlong *param_2,byte *param_3);
undefined8 * thunk_FUN_180016bdc(undefined8 *param_1,undefined8 *param_2);
void __cdecl __ExceptionPtrCopy(void *param_1,void *param_2);
void FUN_1800172f0(undefined8 *param_1);
void __cdecl __ExceptionPtrCurrentException(void *param_1);
void FUN_18001737c(void *param_1);
void FUN_180017384(longlong *param_1);
bool FUN_180017508(longlong *param_1);
undefined8 * FUN_180017510(undefined8 *param_1,longlong param_2);
undefined8 * FUN_18001754c(undefined8 *param_1);
undefined8 * FUN_180017564(undefined8 *param_1,longlong param_2);
undefined8 * FUN_1800175a0(undefined8 *param_1,longlong param_2);
undefined8 * FUN_1800175e8(undefined8 *param_1,longlong param_2);
void FUN_180017624(void);
void FUN_180017644(longlong param_1);
char * FUN_180017668(void);
DWORD FUN_180017670(void);
void FUN_180017688(undefined8 *param_1);
void FUN_1800176ec(longlong param_1);
void FUN_1800176f8(longlong param_1);
void FUN_180017704(longlong param_1);
int FUN_180017710(uint *param_1,ulonglong *param_2);
bool FUN_1800178c4(longlong param_1);
bool FUN_1800178dc(void);
void FUN_1800178f4(longlong param_1);
void FUN_180017900(longlong param_1);
void FUN_18001790c(longlong param_1);
void _Mtx_destroy_in_situ(longlong param_1);
void _Mtx_init_in_situ(undefined4 *param_1,undefined4 param_2);
void FUN_180017958(uint *param_1);
void FUN_180017960(longlong param_1);
undefined8 _Mtx_unlock(longlong param_1);
void FUN_1800179a4(longlong param_1);
void FUN_1800179b0(longlong param_1);
void FUN_1800179bc(longlong param_1,longlong param_2);
void FUN_1800179e4(longlong param_1,longlong param_2);
bool FUN_180017a0c(longlong param_1,longlong param_2,DWORD param_3);
bool FUN_180017a28(longlong param_1,longlong param_2,DWORD param_3);
undefined8 FUN_180017a48(longlong *param_1);
void FUN_180017a60(longlong *param_1);
void FUN_180017a70(undefined8 *param_1);
undefined8 FUN_180017abc(undefined8 *param_1,longlong param_2);
void FUN_180017b10(undefined8 param_1,undefined8 param_2,undefined8 param_3);
void _Cnd_unregister_at_thread_exit(longlong param_1);
longlong * FUN_180017c40(longlong *param_1,ulonglong param_2,undefined8 param_3,undefined8 *param_4);
undefined8 ****FUN_180017d4c(undefined8 ****param_1,undefined4 param_2,undefined8 param_3,char *param_4);
void FUN_180017e30(int param_1);
void FUN_180017e6c(int param_1);
basic_string<> * __thiscall std::basic_string<>::assign(basic_string<> *this,char *param_1);
undefined ** FUN_180017f24(void);
char * FUN_180017f2c(int param_1);
void FUN_180017f60(void);
undefined1 Get_STL_host_status(void);
void FUN_180017fac(undefined8 *param_1);
int __cdecl Concurrency::details::_Reschedule_chore(_Threadpool_chore *param_1);
int __cdecl Concurrency::details::_Schedule_chore(_Threadpool_chore *param_1);
void FUN_180018048(PTP_CALLBACK_INSTANCE param_1,longlong param_2);
undefined8 * FUN_180018130(undefined8 *param_1);
DWORD __stdcall GetCurrentThreadId(void);
void __thiscall Concurrency::details::_ContextCallback::_CallInContext(undefined8 param_1,longlong *param_2);
void FUN_180018198(void);
undefined8 * FUN_1800181b8(undefined8 *param_1,longlong param_2);
future_error * __thiscall std::future_error::future_error(future_error *this,undefined8 *param_2);
void __cdecl std::_Rethrow_future_exception(exception_ptr *param_1);
void FUN_180018288(undefined4 *param_1);
char * __thiscall std::future_error::what(future_error *this);
int _Xtime_diff_to_millis2(longlong *param_1,longlong *param_2);
longlong _Xtime_get_ticks(void);
undefined4 xtime_get(ulonglong *param_1,int param_2);
undefined8 FUN_180018450(void);
void __crtGetSystemTimePreciseAsFileTime(LPFILETIME param_1);
void FUN_180018824(void);
_Init_locks * __thiscall std::_Init_locks::_Init_locks(_Init_locks *this);
void FUN_180018878(void);
void FUN_1800188b4(void);
void FUN_1800188c0(void);
void __stdcall DeleteCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
void FUN_1800188d4(undefined8 param_1);
void __stdcall EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
void __stdcall LeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
undefined8 * FUN_1800188f4(undefined8 *param_1,ulonglong param_2);
void FUN_180018920(size_t param_1);
void thunk_FUN_18001f834(LPVOID param_1);
ulonglong __scrt_acquire_startup_lock(void);
undefined8 __scrt_dllmain_after_initialize_c(void);
bool FUN_1800189d4(void);
undefined1 __scrt_dllmain_crt_thread_attach(void);
undefined1 __scrt_dllmain_crt_thread_detach(void);
void __scrt_dllmain_exception_filter(undefined8 param_1,int param_2,undefined8 param_3,undefined *param_4,int param_5,undefined8 param_6);
void __scrt_dllmain_uninitialize_c(void);
void FUN_180018abc(void);
ulonglong __scrt_initialize_crt(int param_1);
undefined8 FUN_180018b1c(uint param_1);
ulonglong FUN_180018ba8(longlong param_1);
void __scrt_release_startup_lock(char param_1);
undefined8 __scrt_uninitialize_crt(bool param_1,char param_2);
_onexit_t __cdecl _onexit(_onexit_t _Func);
int __cdecl atexit(_func_5014 *param_1);
undefined8 FUN_180018ce4(void);
void FUN_180018db4(void);
void _Init_thread_footer(int *param_1);
void _Init_thread_header(int *param_1);
void _Init_thread_notify(void);
void _Init_thread_wait(DWORD param_1);
undefined8 __GSHandlerCheck(undefined8 param_1,undefined8 param_2,undefined8 param_3,longlong param_4);
ulonglong __GSHandlerCheckCommon(undefined8 param_1,longlong param_2);
void __GSHandlerCheck_EH4(EHExceptionRecord *param_1,__uint64 param_2,_CONTEXT *param_3,_xDISPATCHER_CONTEXT *param_4);
void __chkstk(void);
void __cdecl __security_check_cookie(uintptr_t _StackCookie);
ulonglong FUN_1800190e0(undefined8 param_1,int param_2,longlong param_3);
undefined8 FUN_180019130(undefined8 param_1,undefined8 param_2);
byte FUN_180019248(bool param_1);
int __cdecl dllmain_dispatch(HINSTANCE__ *param_1,ulong param_2,void *param_3);
void entry(HINSTANCE__ *param_1,ulong param_2,void *param_3);
void FUN_180019440(void);
void thunk_FUN_18001f834(LPVOID param_1);
undefined8 FUN_180019468(void);
undefined8 FUN_180019614(void);
bool __scrt_is_ucrt_dll_in_use(void);
void FUN_180019628(void);
void FUN_180019630(undefined4 param_1);
void __raise_securityfailure(_EXCEPTION_POINTERS *param_1);
void FUN_1800197b0(void);
void __report_rangecheckfailure(void);
void FUN_180019898(undefined4 param_1);
void capture_current_context(PCONTEXT param_1);
void capture_previous_context(PCONTEXT param_1);
void __cdecl __security_init_cookie(void);
void FUN_180019ac4(void);
void FUN_180019ad4(void);
undefined * FUN_180019ae0(void);
undefined * FUN_180019ae8(void);
void FUN_180019af0(void);
undefined * FUN_180019b0c(void);
void FUN_180019b14(void);
void FUN_180019b50(void);
int __cdecl _CallSETranslator<>(EHExceptionRecord *param_1,__uint64 *param_2,_CONTEXT *param_3,_xDISPATCHER_CONTEXT *param_4,FuncInfo4 *param_5,ulong param_6,__uint64 *param_7,int param_8);
__int64 __cdecl FH4::DecompFuncInfo(uchar *param_1,FuncInfo4 *param_2,__uint64 param_3,int param_4,bool param_5);
byte FUN_180019d08(undefined8 param_1,byte *param_2);
void __cdecl __FrameHandler4::FrameUnwindToEmptyState(__uint64 *param_1,_xDISPATCHER_CONTEXT *param_2,FuncInfo4 *param_3);
__uint64 * __cdecl __FrameHandler4::GetEstablisherFrame(__uint64 *param_1,_xDISPATCHER_CONTEXT *param_2,FuncInfo4 *param_3,__uint64 *param_4);
undefined4 * FUN_180019d68(undefined4 *param_1,int *param_2,int param_3);
void FUN_180019ed4(undefined8 *param_1,ULONG_PTR param_2,ULONG_PTR param_3,ULONG_PTR param_4,ULONG_PTR param_5,undefined8 param_6,int param_7,int param_8,longlong param_9,undefined8 *param_10,byte param_11);
void FUN_180019ff4(longlong param_1,longlong param_2);
undefined8 * _CreateFrameInfo(undefined8 *param_1,undefined8 param_2);
void FUN_18001a1f4(longlong param_1);
undefined8 _GetImageBase(void);
undefined8 _GetThrowImageBase(void);
void FUN_18001a270(undefined8 param_1);
void FUN_18001a288(undefined8 param_1);
void __CxxFrameHandler4(EHExceptionRecord *param_1,__uint64 param_2,_CONTEXT *param_3,_xDISPATCHER_CONTEXT *param_4);
undefined8 FUN_18001a370(undefined1 *param_1,undefined1 param_2,longlong param_3,undefined8 param_4);
undefined1 (*) [32] FUN_18001a390(undefined1 (*param_1) [32],byte param_2,ulonglong param_3);
void __stdcall _CxxThrowException(void *pExceptionObject,ThrowInfo *pThrowInfo);
undefined8 FUN_18001a7b8(void);
void FUN_18001a7c4(void);
void __DestructExceptionObject(int *param_1);
undefined4 _IsExceptionObjectToBeDestroyed(longlong param_1);
longlong __AdjustPointer(longlong param_1,int *param_2);
undefined8 FUN_18001a8a4(undefined8 *param_1);
longlong __current_exception(void);
void FUN_18001a920(void);
void __std_exception_copy(longlong *param_1,longlong *param_2);
void __std_exception_destroy(undefined8 *param_1);
void FUN_18001aa00(undefined1 *param_1,undefined1 *param_2,longlong param_3);
undefined8 * FUN_18001aa20(undefined8 *param_1,undefined8 *param_2,ulonglong param_3);
ulonglong __vcrt_initialize(void);
bool __vcrt_thread_attach(void);
undefined1 __vcrt_thread_detach(void);
undefined8 __vcrt_uninitialize(char param_1);
undefined1 FUN_18001b104(void);
undefined8 FUN_18001b114(PEXCEPTION_RECORD param_1,PVOID param_2,longlong param_3,longlong *param_4);
uint __std_type_info_compare(longlong param_1,longlong param_2);
void FUN_18001b334(PSLIST_HEADER param_1);
void FUN_18001b360(undefined *param_1);
void __vcrt_freeptd(undefined *param_1);
void FUN_18001b3d0(void);
LPVOID __vcrt_getptd_noexit(void);
uint __vcrt_initialize_ptd(void);
undefined1 __vcrt_uninitialize_ptd(void);
void FUN_18001b518(longlong param_1,ulonglong *param_2);
int FUN_18001b520(longlong param_1,longlong param_2,ulonglong param_3);
int __cdecl BuildCatchObjectHelperInternal<>(EHExceptionRecord *param_1,void *param_2,HandlerType4 *param_3,_s_CatchableType *param_4);
void __cdecl BuildCatchObjectInternal<>(EHExceptionRecord *param_1,void *param_2,HandlerType4 *param_3,_s_CatchableType *param_4);
void __cdecl CatchIt<>(EHExceptionRecord *param_1,__uint64 *param_2,_CONTEXT *param_3,_xDISPATCHER_CONTEXT *param_4,FuncInfo4 *param_5,HandlerType4 *param_6,_s_CatchableType *param_7,TryBlockMapEntry4 *param_8,int param_9,__uint64 *param_10,uchar param_11,uchar param_12);
void FUN_18001b9ac(EHExceptionRecord *param_1,__uint64 *param_2,_CONTEXT *param_3,_xDISPATCHER_CONTEXT *param_4,FuncInfo4 *param_5,uchar param_6,int param_7,__uint64 *param_8);
void __cdecl FindHandlerForForeignException<>(EHExceptionRecord *param_1,__uint64 *param_2,_CONTEXT *param_3,_xDISPATCHER_CONTEXT *param_4,FuncInfo4 *param_5,int param_6,int param_7,__uint64 *param_8);
int __cdecl TypeMatchHelper<>(_s_HandlerType *param_1,_s_CatchableType *param_2,_s_ThrowInfo *param_3);
int __cdecl TypeMatchHelper<>(HandlerType4 *param_1,_s_CatchableType *param_2,_s_ThrowInfo *param_3);
undefined8 FUN_18001c41c(EHExceptionRecord *param_1,__uint64 *param_2,_CONTEXT *param_3,_xDISPATCHER_CONTEXT *param_4,FuncInfo4 *param_5,int param_6,__uint64 *param_7,byte param_8);
uint * FUN_18001c6b0(uint *param_1,longlong param_2,longlong param_3,uint param_4);
TryBlockMap4 * __thiscall FH4::TryBlockMap4::TryBlockMap4(TryBlockMap4 *this,FuncInfo4 *param_1,__uint64 param_2);
void * __cdecl __FrameHandler4::CxxCallCatchBlock(_EXCEPTION_RECORD *param_1);
void FUN_18001ca54(longlong param_1);
int __cdecl ExFilterRethrow(_EXCEPTION_POINTERS *param_1,EHExceptionRecord *param_2,int *param_3);
int __cdecl ExFilterRethrowFH4(_EXCEPTION_POINTERS *param_1,EHExceptionRecord *param_2,int param_3,int *param_4);
void FUN_18001cca0(longlong *param_1,ulonglong *param_2,longlong param_3,int param_4);
undefined1 FUN_18001cf9c(longlong param_1,int *param_2);
uchar __cdecl Is_bad_exception_allowed(_s_ESTypeList *param_1);
void FUN_18001d114(longlong param_1,longlong *param_2);
void FUN_18001d1b0(int *param_1,int param_2,int param_3,longlong *param_4,longlong *param_5);
int FUN_18001d280(longlong *param_1,int param_2,longlong param_3,int param_4,longlong *param_5);
undefined8 __vcrt_initialize_locks(void);
undefined8 __vcrt_uninitialize_locks(void);
void _NLG_Notify(void);
void FUN_18001d400(void);
void FUN_18001d410(void);
void __except_validate_context_record(longlong param_1);
FARPROC FUN_18001d44c(uint param_1,LPCSTR param_2,uint *param_3,uint *param_4);
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
void FUN_18001d994(undefined8 param_1);
void FUN_18001d99c(wchar_t *param_1,wchar_t *param_2,wchar_t *param_3,uint param_4,uintptr_t param_5);
void FUN_18001da4c(void);
void FUN_18001da6c(void);
void __cdecl _invoke_watson(wchar_t *param_1,wchar_t *param_2,wchar_t *param_3,uint param_4,uintptr_t param_5);
char * __cdecl strncpy(char *_Dest,char *_Source,size_t _Count);
void __cdecl abort(void);
int ilogbf(undefined4 param_1);
float FID_conflict:scalbnf(undefined4 param_1,int param_2);
void terminate(void);
undefined8 FUN_18001dd90(void);
undefined8 FUN_18001e290(void);
float __cdecl floorf(float _X);
ulonglong FUN_18001e5c0(void);
float roundf(float param_1);
undefined8 FUN_18001ea20(void);
float FUN_18001f118(float param_1);
undefined8 FUN_18001f1c0(void);
void FUN_18001f834(LPVOID param_1);
LPVOID _malloc_base(ulonglong param_1);
undefined8 __acrt_initialize_stdio(void);
void __acrt_uninitialize_stdio(bool param_1);
void FUN_18001f9cc(longlong param_1);
void FUN_18001f9d8(longlong param_1);
LPVOID _calloc_base(ulonglong param_1,ulonglong param_2);
undefined8 __acrt_initialize_locks(void);
void __acrt_lock(int param_1);
undefined8 __acrt_uninitialize_locks(void);
void __acrt_unlock(int param_1);
void FUN_18001faa4(undefined8 param_1);
int __cdecl _callnewh(size_t _Size);
ulonglong _query_new_handler(void);
undefined4 _seh_filter_dll(int param_1,undefined8 param_2);
undefined4 _seh_filter_exe(int param_1,undefined8 param_2);
void __thiscall __crt_seh_guarded_call<void>::operator()<>(__crt_seh_guarded_call<void> *this,<> *param_1,<> *param_2,<> *param_3);
void __thiscall <>::operator()(<> *this);
bool FUN_18001fda4(int param_1);
void __cdecl common_exit(int param_1,_crt_exit_cleanup_mode param_2,_crt_exit_return_mode param_3);
void __cdecl exit_or_terminate_process(uint param_1);
void __cdecl try_cor_exit_process(uint param_1);
void FUN_18001ff14(undefined8 param_1);
void __cdecl _cexit(void);
void FUN_18001ff2c(int param_1);
undefined4 FUN_18001ff38(void);
void __cdecl parse_command_line<char>(char *param_1,char **param_2,char *param_3,__uint64 *param_4,__uint64 *param_5);
LPVOID __acrt_allocate_buffer_for_argv(ulonglong param_1,ulonglong param_2,ulonglong param_3);
int _configure_narrow_argv(int param_1);
int __cdecl common_initialize_environment_nolock<char>(void);
undefined8 * FUN_180020354(char *param_1);
void free_environment<>(undefined8 *param_1);
void uninitialize_environment_internal<>(undefined8 *param_1);
void uninitialize_environment_internal<>(undefined8 *param_1);
void __dcrt_uninitialize_environments_nolock(void);
int __cdecl common_initialize_environment_nolock<char>(void);
int __thiscall __crt_seh_guarded_call<int>::operator()<>(__crt_seh_guarded_call<int> *this,<> *param_1,<> *param_2,<> *param_3);
int __thiscall __crt_seh_guarded_call<int>::operator()<>(__crt_seh_guarded_call<int> *this,<> *param_1,<> *param_2,<> *param_3);
int __thiscall <>::operator()(<> *this);
int __thiscall <>::operator()(<> *this);
void FUN_180020858(undefined8 param_1);
void _execute_onexit_table(undefined8 param_1);
undefined8 _initialize_onexit_table(longlong *param_1);
void _register_onexit_function(undefined8 param_1,undefined8 param_2);
undefined8 FUN_180020928(void);
undefined1 FUN_180020950(void);
undefined1 FUN_180020960(void);
undefined8 FUN_1800209a8(void);
undefined1 FUN_1800209e8(void);
void FUN_180020a44(void);
bool __acrt_thread_attach(void);
undefined1 __acrt_thread_detach(void);
undefined8 __acrt_uninitialize(bool param_1);
undefined1 FUN_180020ab4(void);
void _initterm(undefined8 *param_1,undefined8 *param_2);
undefined8 _initterm_e(undefined8 *param_1,undefined8 *param_2);
errno_t __cdecl strcpy_s(char *_Dst,rsize_t _SizeInBytes,char *_Src);
_LocaleUpdate * __thiscall _LocaleUpdate::_LocaleUpdate(_LocaleUpdate *this,__crt_locale_pointers *param_1);
int __cdecl strncmp(char *_Str1,char *_Str2,size_t _MaxCount);
int __cdecl wcsncmp(wchar_t *_Str1,wchar_t *_Str2,size_t _MaxCount);
void operator()<>(undefined8 param_1,int *param_2,undefined8 *param_3,int *param_4);
void operator()<>(undefined8 param_1,int *param_2,undefined8 *param_3,int *param_4);
void operator()<>(undefined8 param_1,int *param_2,undefined8 *param_3,int *param_4);
void operator()<>(undefined8 param_1,int *param_2,undefined8 *param_3,int *param_4);
void __cdecl construct_ptd_array(__acrt_ptd *param_1);
void FUN_180020f24(__acrt_ptd *param_1);
void __cdecl destroy_ptd_array(__acrt_ptd *param_1);
void __cdecl replace_current_thread_locale_nolock(__acrt_ptd *param_1,__crt_locale_data *param_2);
void __acrt_freeptd(void);
void FUN_1800210e8(void);
__acrt_ptd * FUN_1800211bc(void);
ulonglong __acrt_getptd_noexit(void);
ulonglong __acrt_initialize_ptd(void);
undefined1 __acrt_uninitialize_ptd(void);
ulonglong operator()<>(undefined8 param_1,int *param_2,undefined8 param_3,int *param_4);
void __acrt_get_sigabrt_handler(void);
void FUN_180021408(undefined8 param_1);
int __cdecl raise(int _SigNum);
undefined8 _fdunscale(short *param_1,_Fval *param_2);
ulonglong _fd_int(ushort *param_1,short param_2);
short _fdtest(short *param_1);
void _fperrraise(uint param_1);
undefined8 _fdscale(_Fval *param_1,int param_2);
ulonglong FUN_180021ac0(ulonglong param_1);
ulonglong FUN_180021c10(ulonglong param_1);
uint FUN_180021e00(double param_1);
void FUN_180021e78(float param_1);
float FUN_180021e8c(float param_1,undefined8 param_2,uint param_3);
void FUN_180021f18(float param_1);
void FUN_180021f2c(float param_1);
uint _clrfp(void);
uint _ctrlfp(uint param_1,uint param_2);
void _set_statfp(uint param_1);
uint _statfp(void);
undefined8 __acrt_initialize_fma3(void);
float FUN_180022080(float param_1,float param_2,int param_3);
undefined8 FUN_180022128(int param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4,undefined8 param_5,undefined8 param_6,uint param_7);
bool _exception_enabled(uint param_1,ulonglong param_2);
undefined8 FUN_18002224c(undefined8 param_1,uint param_2,undefined8 param_3,int param_4,uint param_5,uint param_6,undefined8 param_7,undefined8 param_8,int param_9);
float FUN_180022374(undefined8 param_1,uint param_2,float param_3,int param_4,uint param_5,uint param_6,float param_7,float param_8,int param_9);
uint _handle_nanf(uint param_1);
void FUN_1800224c0(undefined8 param_1,undefined8 param_2,int param_3);
undefined8 FUN_1800224e0(undefined8 param_1,undefined8 param_2,int param_3,uint param_4,undefined8 param_5);
void FUN_180022578(float param_1,float param_2,int param_3);
float FUN_180022598(float param_1,float param_2,int param_3,uint param_4,undefined8 param_5);
void __remainder_piby2d2f_forC(ulonglong param_1,double *param_2,uint *param_3);
void _free_base(LPVOID param_1);
LPVOID _malloc_base(ulonglong param_1);
undefined4 __acrt_errno_from_os_error(int param_1);
void __acrt_errno_map_os_error(int param_1);
ulong * __cdecl __doserrno(void);
ulong * __cdecl __doserrno(void);
LPVOID _calloc_base(ulonglong param_1,ulonglong param_2);
void * __cdecl try_get_function(function_id param_1,char *param_2,module_id *param_3,module_id *param_4);
undefined8 __acrt_AppPolicyGetProcessTerminationMethodInternal(undefined8 param_1);
undefined8 __acrt_AreFileApisANSI(void);
void __acrt_FlsAlloc(undefined8 param_1);
void __acrt_FlsFree(undefined4 param_1);
void __acrt_FlsGetValue(undefined4 param_1);
void __acrt_FlsSetValue(DWORD param_1,LPVOID param_2);
void __acrt_InitializeCriticalSectionEx(LPCRITICAL_SECTION param_1,DWORD param_2,undefined4 param_3);
void __acrt_LCMapStringEx(ushort *param_1,DWORD param_2,LPCWSTR param_3,int param_4,LPWSTR param_5,int param_6,undefined8 param_7,undefined8 param_8,undefined8 param_9);
void __acrt_LocaleNameToLCID(ushort *param_1,undefined4 param_2);
undefined8 __acrt_initialize_winapi_thunks(void);
undefined8 __acrt_uninitialize_winapi_thunks(char param_1);
undefined8 __acrt_get_process_end_policy(void);
int __cdecl _fcloseall(void);
void __thiscall __crt_seh_guarded_call<void>::operator()<>(__crt_seh_guarded_call<void> *this,<> *param_1,<> *param_2,<> *param_3);
void __thiscall __crt_seh_guarded_call<void>::operator()<>(__crt_seh_guarded_call<void> *this,<> *param_1,<> *param_2,<> *param_3);
int __cdecl common_flush_all(bool param_1);
undefined8 __acrt_stdio_flush_nolock(FILE *param_1);
int __cdecl _fflush_nolock(FILE *_File);
int __cdecl common_flush_all(bool param_1);
void __acrt_stdio_free_buffer_nolock(undefined8 *param_1);
void __cdecl initialize_inherited_file_handles_nolock(void);
void __cdecl initialize_stdio_handles_nolock(void);
bool __acrt_initialize_lowio(void);
undefined1 __acrt_uninitialize_lowio(void);
int __cdecl _fileno(FILE *_File);
int __cdecl __acrt_mbs_to_wcs_cp<>(char *param_1,__crt_win32_buffer<> *param_2,uint param_3);
int __cdecl __acrt_wcs_to_mbs_cp<>(wchar_t *param_1,__crt_win32_buffer<> *param_2,uint param_3);
int FUN_1800238e8(undefined8 *param_1,undefined8 *param_2);
undefined8 FUN_180023af4(char *param_1,char *param_2,ulonglong param_3,longlong *param_4);
int __cdecl expand_argument_wildcards<char>(char *param_1,char *param_2,argument_list<char> *param_3);
int thunk_FUN_1800238e8(undefined8 *param_1,undefined8 *param_2);
int __cdecl __acrt_wcs_to_mbs_cp<>(wchar_t *param_1,__crt_win32_buffer<> *param_2,uint param_3);
ulonglong __acrt_GetModuleFileNameA(HMODULE param_1,undefined8 param_2,ulonglong param_3);
void FUN_1800241c8(undefined8 param_1,int *param_2,undefined8 *param_3,int *param_4);
int __cdecl getSystemCP(int param_1);
void __cdecl setSBCS(__crt_multibyte_data *param_1);
void __cdecl setSBUpLow(__crt_multibyte_data *param_1);
int FUN_180024678(int param_1,char param_2,__acrt_ptd *param_3,__crt_multibyte_data **param_4);
__crt_multibyte_data * __cdecl update_thread_multibyte_data_internal(__acrt_ptd *param_1,__crt_multibyte_data **param_2);
undefined8 __acrt_initialize_multibyte(void);
void __acrt_update_thread_multibyte_data(void);
undefined8 _setmbcp_nolock(int param_1,__crt_multibyte_data *param_2);
int __cdecl x_ismbbtype_l(__crt_locale_pointers *param_1,uint param_2,int param_3,int param_4);
int __cdecl _ismbblead(uint _C);
undefined1 FUN_180024cb4(void);
void __acrt_MultiByteToWideChar(uint param_1,ulonglong param_2);
void __acrt_WideCharToMultiByte(uint param_1,uint param_2,undefined8 param_3,undefined4 param_4);
LPVOID __dcrt_get_narrow_environment_from_os(void);
LPVOID _recalloc_base(LPVOID param_1,ulonglong param_2,ulonglong param_3);
bool FUN_180024f84(void);
undefined8 __acrt_execute_initializers(undefined8 *param_1,undefined8 *param_2);
undefined8 __acrt_execute_uninitializers(longlong param_1,longlong param_2);
bool FUN_180025068(void);
void FUN_180025088(undefined8 param_1);
undefined8 FUN_180025090(undefined8 param_1);
int __cdecl _mbtowc_l(wchar_t *_DstCh,char *_SrcCh,size_t _SrcSizeInBytes,_locale_t _Locale);
void FUN_180025240(wchar_t *param_1,char *param_2,size_t param_3);
void __acrt_update_locale_info(longlong param_1,longlong *param_2);
void __acrt_update_multibyte_info(longlong param_1,longlong *param_2);
void __thiscall __crt_seh_guarded_call<void>::operator()<>(__crt_seh_guarded_call<void> *this,<> *param_1,<> *param_2,<> *param_3);
undefined4 FUN_180025320(void);
void __acrt_uninitialize_locale(void);
ushort * __cdecl __pctype_func(void);
void __acrt_add_locale_ref(longlong param_1);
void __acrt_free_locale(longlong param_1);
int __acrt_locale_add_lc_time_reference(undefined **param_1);
void __acrt_locale_free_lc_time_if_unreferenced(undefined **param_1);
int __acrt_locale_release_lc_time_reference(undefined **param_1);
void __acrt_release_locale_ref(longlong param_1);
undefined ** __acrt_update_thread_locale_data(void);
undefined ** _updatetlocinfoEx_nolock(longlong *param_1,undefined **param_2);
short __cdecl FID_conflict:_fdnorm(_Fval *param_1);
undefined8 fegetexceptflag(uint *param_1,uint param_2);
undefined8 fesetexceptflag(uint *param_1,uint param_2);
uint __acrt_fenv_get_fp_status_word_from_exception_mask(uint param_1);
uint __acrt_fenv_get_status(void);
void __acrt_fenv_set_status(uint param_1);
undefined4 _get_fpsr(void);
void FUN_180025a20(undefined4 param_1);
void _fclrf(void);
void _raise_exc(uint *param_1,ulonglong *param_2,ulonglong param_3,uint param_4,uint *param_5,uint *param_6);
void _raise_exc_ex(uint *param_1,ulonglong *param_2,ulonglong param_3,uint param_4,uint *param_5,uint *param_6,int param_7);
void _raise_excf(uint *param_1,ulonglong *param_2,ulonglong param_3,uint param_4,uint *param_5,uint *param_6);
void _set_errno_from_matherr(int param_1);
undefined4 FUN_180025df4(void);
undefined4 __acrt_DownlevelLocaleNameToLCID(ushort *param_1);
int __cdecl _fclose_nolock(FILE *_File);
int __cdecl fclose(FILE *_File);
int __thiscall __crt_seh_guarded_call<int>::operator()<>(__crt_seh_guarded_call<int> *this,<> *param_1,<> *param_2,<> *param_3);
int __cdecl _commit(int _FileHandle);
void __cdecl write_double_translated_ansi_nolock(int param_1,char *param_2,uint param_3);
DWORD * __cdecl write_text_ansi_nolock(int param_1,char *param_2,uint param_3);
DWORD * __cdecl write_text_utf16le_nolock(int param_1,char *param_2,uint param_3);
DWORD * __cdecl write_text_utf8_nolock(int param_1,char *param_2,uint param_3);
int __cdecl _write(int _FileHandle,void *_Buf,uint _MaxCharCount);
int __cdecl _write_nolock(int _FileHandle,void *_Buf,uint _MaxCharCount);
undefined8 * __acrt_lowio_create_handle_array(void);
void __acrt_lowio_destroy_handle_array(LPCRITICAL_SECTION param_1);
undefined4 __acrt_lowio_ensure_fh_exists(uint param_1);
void FID_conflict:__acrt_lowio_lock_fh(uint param_1);
void FID_conflict:__acrt_lowio_lock_fh(uint param_1);
int __cdecl _free_osfhnd(int param_1);
intptr_t __cdecl _get_osfhandle(int _FileHandle);
__int64 __cdecl common_lseek_nolock<__int64>(int param_1,__int64 param_2,int param_3);
__int64 __cdecl common_lseek_nolock<__int64>(int param_1,__int64 param_2,int param_3);
int __cdecl _isatty(int _FileHandle);
void __cdecl qsort(void *_Base,size_t _NumOfElements,size_t _SizeOfElements,_PtFuncCompare *_PtFuncCompare);
errno_t __cdecl strncpy_s(char *_Dst,rsize_t _SizeInBytes,char *_Src,rsize_t _MaxCount);
char * __cdecl FID_conflict:fallbackMethod(char *_Str,char *_Control);
char * FUN_180027650(char *param_1,char *param_2);
void FUN_180027a18(uchar *param_1,uchar *param_2);
uchar * __cdecl _mbsdec_l(uchar *_Start,uchar *_Pos,_locale_t _Locale);
BOOL __acrt_GetStringTypeA(__crt_locale_pointers *param_1,DWORD param_2,undefined8 param_3,undefined8 param_4,LPWORD param_5,uint param_6,int param_7);
int __cdecl __acrt_LCMapStringA_stat(__crt_locale_pointers *param_1,wchar_t *param_2,ulong param_3,char *param_4,int param_5,char *param_6,int param_7,int param_8,int param_9);
void __acrt_LCMapStringA(__crt_locale_pointers *param_1,wchar_t *param_2,ulong param_3,char *param_4,int param_5,char *param_6,int param_7,int param_8,int param_9);
bool FUN_180027ff4(void);
undefined8 _msize_base(longlong param_1);
LPVOID _realloc_base(LPVOID param_1,ulonglong param_2);
int __cdecl _isleadbyte_l(int _C,_locale_t _Locale);
__uint64 __cdecl __crt_mbstring::__mbrtowc_utf8(wchar_t *param_1,char *param_2,__uint64 param_3,_Mbstatet *param_4);
__uint64 __cdecl __crt_mbstring::__mbsrtowcs_utf8(wchar_t *param_1,char **param_2,__uint64 param_3,_Mbstatet *param_4);
void __acrt_locale_free_monetary(longlong param_1);
void __acrt_locale_free_numeric(longlong *param_1);
void __cdecl free_crt_array_internal(void **param_1,__uint64 param_2);
void __acrt_locale_free_time(void **param_1);
uint fetestexcept(uint param_1);
int __ascii_wcsnicmp(ushort *param_1,ushort *param_2,longlong param_3);
int __thiscall __crt_seh_guarded_call<int>::operator()<>(__crt_seh_guarded_call<int> *this,<> *param_1,<> *param_2,<> *param_3);
int __cdecl _close(int _FileHandle);
int __cdecl _close_nolock(int _FileHandle);
void __cdecl __acrt_stdio_free_stream(undefined8 *param_1);
wint_t __cdecl _putwch_nolock(wchar_t _WCh);
undefined4 FUN_18002883c(void);
size_t __cdecl __strncnt(char *_String,size_t _Cnt);
__uint64 __cdecl __crt_mbstring::__mbrtoc32_utf8(wchar32 *param_1,char *param_2,__uint64 param_3,_Mbstatet *param_4);
double FUN_180028a40(void);
bool __dcrt_lowio_ensure_console_output_initialized(void);
void FUN_180029040(void);
BOOL __dcrt_write_console(void *param_1,DWORD param_2,LPDWORD param_3);
PIMAGE_SECTION_HEADER __cdecl _FindPESection(PBYTE pImageBase,DWORD_PTR rva);
BOOL __cdecl _IsNonwritableInCurrentImage(PBYTE pTarget);
bool FUN_1800291c0(short *param_1);
void __GSHandlerCheck_SEH(PEXCEPTION_RECORD param_1,PVOID param_2,longlong param_3,longlong *param_4);
int __cdecl memcmp(void *_Buf1,void *_Buf2,size_t _Size);
void _guard_dispatch_icall(void);
void _guard_dispatch_icall(void);
void FUN_1800293c0(undefined8 param_1,longlong param_2);
void FUN_1800293f0(undefined8 param_1,longlong param_2);
void FUN_180029420(undefined8 param_1,longlong param_2);
void FUN_180029470(undefined8 param_1,longlong param_2);
void FUN_180029510(undefined8 param_1,longlong param_2);
void FUN_1800295c0(undefined8 param_1,longlong param_2);
undefined8 FUN_180029610(undefined8 param_1,longlong param_2);
void FUN_180029670(undefined8 param_1,longlong param_2);
void FUN_1800297b0(undefined8 param_1,longlong param_2);
void FUN_1800297e0(undefined8 param_1,longlong param_2);
undefined8 FUN_180029870(undefined8 param_1,longlong param_2);
void FUN_180029910(undefined8 param_1,longlong param_2);
void FUN_180029a20(undefined8 param_1,longlong param_2);
undefined8 FUN_180029a80(void);
undefined8 FUN_180029aa0(undefined8 param_1,longlong param_2);
void FUN_180029b20(undefined8 param_1,longlong param_2);
undefined8 FUN_180029b90(undefined8 param_1,longlong param_2);
undefined8 FUN_180029be0(undefined8 param_1,longlong param_2);
undefined8 FUN_180029c50(void);
void FUN_180029cd5(undefined8 param_1,longlong param_2);
void FUN_180029cfb(undefined8 param_1,longlong param_2);
void FUN_180029d21(undefined8 param_1,longlong param_2);
void FUN_180029d47(undefined8 param_1,longlong param_2);
undefined8 FUN_180029d6d(undefined8 param_1,longlong param_2);
undefined * FUN_180029e26(undefined8 param_1,longlong param_2);
void FUN_18002a125(undefined8 param_1,longlong param_2);
bool FUN_18002a157(undefined8 *param_1);
void FUN_18002a16f(undefined8 param_1,longlong param_2);
void FUN_18002a186(undefined8 param_1,longlong param_2);
void FUN_18002a19f(void);
void FUN_18002a1b3(undefined8 *param_1,longlong param_2);
undefined8 FUN_18002a1e9(undefined8 param_1,longlong param_2);
undefined4 FUN_18002a288(undefined8 param_1,longlong param_2);
void FUN_18002a31e(_EXCEPTION_POINTERS *param_1,longlong param_2);
void FUN_18002a34d(undefined8 param_1,longlong param_2);
void FUN_18002a3de(undefined8 *param_1);
void FUN_18002a3f4(void);
void FUN_18002a417(void);
void FUN_18002a42d(undefined8 param_1,longlong param_2);
void FUN_18002a447(undefined8 *param_1);
void FUN_18002a462(undefined8 param_1,longlong param_2);
undefined8 FUN_18002a483(undefined8 *param_1);
void FUN_18002a4af(void);
void FUN_18002a4c8(undefined8 param_1,longlong param_2);
void FUN_18002a4e3(undefined8 param_1,longlong param_2);
void FUN_18002a500(void);
void FUN_18002a519(void);
void FUN_18002a532(undefined8 param_1,longlong param_2);
void FUN_18002a54c(void);
void FUN_18002a565(undefined8 param_1,longlong param_2);
void FUN_18002a57d(undefined8 param_1,longlong param_2);
void FUN_18002a597(undefined8 param_1,longlong param_2);
bool FUN_18002a5b0(undefined8 *param_1);
void FUN_18002a5d0(void);
void FUN_18002a640(void);

