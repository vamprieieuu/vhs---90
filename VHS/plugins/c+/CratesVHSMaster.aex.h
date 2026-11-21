typedef unsigned char   undefined;

typedef unsigned char    bool;
typedef unsigned char    byte;
typedef unsigned int    dword;
typedef unsigned long long    GUID;
typedef pointer32 ImageBaseOffset32;

typedef pointer64 ImageBaseOffset64;

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

typedef struct _s__RTTICompleteObjectLocator RTTICompleteObjectLocator;

typedef uint64_t __uint64;

typedef ulong DWORD;

typedef DWORD LCTYPE;

typedef void *PVOID;

typedef PVOID BCRYPT_ALG_HANDLE;

typedef long LONG;

typedef LONG NTSTATUS;

typedef struct _SYSTEM_INFO _SYSTEM_INFO, *P_SYSTEM_INFO;

typedef struct _SYSTEM_INFO *LPSYSTEM_INFO;

typedef union _union_552 _union_552, *P_union_552;

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

typedef struct _SECURITY_ATTRIBUTES _SECURITY_ATTRIBUTES, *P_SECURITY_ATTRIBUTES;

typedef int BOOL;

struct _SECURITY_ATTRIBUTES {
    DWORD nLength;
    LPVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
};

typedef struct _WIN32_FIND_DATAW _WIN32_FIND_DATAW, *P_WIN32_FIND_DATAW;

typedef struct _FILETIME _FILETIME, *P_FILETIME;

typedef struct _FILETIME FILETIME;

typedef wchar_t WCHAR;

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

typedef struct _OVERLAPPED _OVERLAPPED, *P_OVERLAPPED;

typedef union _union_540 _union_540, *P_union_540;

typedef void *HANDLE;

typedef struct _struct_541 _struct_541, *P_struct_541;

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

typedef enum _GET_FILEEX_INFO_LEVELS {
    GetFileExInfoStandard=0,
    GetFileExMaxInfoLevel=1
} _GET_FILEEX_INFO_LEVELS;

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

typedef enum _GET_FILEEX_INFO_LEVELS GET_FILEEX_INFO_LEVELS;

typedef enum _FILE_INFO_BY_HANDLE_CLASS {
    FileBasicInfo=0,
    FileStandardInfo=1,
    FileNameInfo=2,
    FileRenameInfo=3,
    FileDispositionInfo=4,
    FileAllocationInfo=5,
    FileEndOfFileInfo=6,
    FileStreamInfo=7,
    FileCompressionInfo=8,
    FileAttributeTagInfo=9,
    FileIdBothDirectoryInfo=10,
    FileIdBothDirectoryRestartInfo=11,
    FileIoPriorityHintInfo=12,
    FileRemoteProtocolInfo=13,
    MaximumFileInfoByHandleClass=14
} _FILE_INFO_BY_HANDLE_CLASS;

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

typedef struct _WIN32_FIND_DATAW *LPWIN32_FIND_DATAW;

typedef enum _FILE_INFO_BY_HANDLE_CLASS FILE_INFO_BY_HANDLE_CLASS;

typedef struct _OVERLAPPED *LPOVERLAPPED;

typedef struct _SECURITY_ATTRIBUTES *LPSECURITY_ATTRIBUTES;

typedef struct _MEMORY_BASIC_INFORMATION _MEMORY_BASIC_INFORMATION, *P_MEMORY_BASIC_INFORMATION;

typedef ULONG_PTR SIZE_T;

struct _MEMORY_BASIC_INFORMATION {
    PVOID BaseAddress;
    PVOID AllocationBase;
    DWORD AllocationProtect;
    SIZE_T RegionSize;
    DWORD State;
    DWORD Protect;
    DWORD Type;
};

typedef char CHAR;

typedef CHAR *LPCSTR;

typedef struct _MEMORY_BASIC_INFORMATION *PMEMORY_BASIC_INFORMATION;

typedef union _LARGE_INTEGER _LARGE_INTEGER, *P_LARGE_INTEGER;

typedef struct _struct_19 _struct_19, *P_struct_19;

typedef struct _struct_20 _struct_20, *P_struct_20;

typedef longlong LONGLONG;

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

typedef CHAR *LPSTR;

typedef WCHAR *LPWSTR;

typedef short SHORT;

typedef WCHAR *LPCWSTR;

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

typedef ulonglong UINT_PTR;

typedef ulong ULONG;

typedef struct _FILETIME *LPFILETIME;

typedef INT_PTR (*FARPROC)(void);

typedef uchar UCHAR;

typedef UCHAR *PUCHAR;

typedef DWORD *LPDWORD;

typedef struct HINSTANCE__ HINSTANCE__, *PHINSTANCE__;

struct HINSTANCE__ {
    int unused;
};

typedef DWORD *PDWORD;

typedef BOOL *LPBOOL;

typedef struct HINSTANCE__ *HINSTANCE;

typedef void *LPCVOID;

typedef HINSTANCE HMODULE;

typedef HANDLE HLOCAL;

typedef uint UINT;

typedef struct IMAGE_RESOURCE_DIRECTORY_ENTRY_NameStruct IMAGE_RESOURCE_DIRECTORY_ENTRY_NameStruct, *PIMAGE_RESOURCE_DIRECTORY_ENTRY_NameStruct;

struct IMAGE_RESOURCE_DIRECTORY_ENTRY_NameStruct {
    dword NameOffset:31;
    dword NameIsString:1;
};

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

typedef struct IMAGE_IMPORT_BY_NAME_16 IMAGE_IMPORT_BY_NAME_16, *PIMAGE_IMPORT_BY_NAME_16;

struct IMAGE_IMPORT_BY_NAME_16 {
    word Hint;
    char Name[16];
};

typedef struct IMAGE_IMPORT_BY_NAME_17 IMAGE_IMPORT_BY_NAME_17, *PIMAGE_IMPORT_BY_NAME_17;

struct IMAGE_IMPORT_BY_NAME_17 {
    word Hint;
    char Name[17];
};

typedef struct IMAGE_SECTION_HEADER IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;

typedef union Misc Misc, *PMisc;

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

typedef struct IMAGE_FILE_HEADER IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

typedef struct IMAGE_OPTIONAL_HEADER64 IMAGE_OPTIONAL_HEADER64, *PIMAGE_OPTIONAL_HEADER64;

typedef struct IMAGE_DATA_DIRECTORY IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;

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

struct IMAGE_DATA_DIRECTORY {
    ImageBaseOffset32 VirtualAddress;
    dword Size;
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

typedef union IMAGE_RESOURCE_DIRECTORY_ENTRY_NameUnion IMAGE_RESOURCE_DIRECTORY_ENTRY_NameUnion, *PIMAGE_RESOURCE_DIRECTORY_ENTRY_NameUnion;

struct IMAGE_RESOURCE_DIRECTORY_ENTRY_NameStruct {
    dword NameOffset:31;
    dword NameIsString:1;
};

union IMAGE_RESOURCE_DIRECTORY_ENTRY_NameUnion {
    struct IMAGE_RESOURCE_DIRECTORY_ENTRY_NameStruct IMAGE_RESOURCE_DIRECTORY_ENTRY_NameStruct;
    dword Name;
    word Id;
};

typedef struct _IMAGE_RUNTIME_FUNCTION_ENTRY _IMAGE_RUNTIME_FUNCTION_ENTRY, *P_IMAGE_RUNTIME_FUNCTION_ENTRY;

struct _IMAGE_RUNTIME_FUNCTION_ENTRY {
    ImageBaseOffset32 BeginAddress;
    dword EndAddress; // Apply ImageBaseOffset32 to see reference
    ImageBaseOffset32 UnwindInfoAddressOrData;
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

union IMAGE_RESOURCE_DIRECTORY_ENTRY_NameUnion {
    struct IMAGE_RESOURCE_DIRECTORY_ENTRY_NameStruct IMAGE_RESOURCE_DIRECTORY_ENTRY_NameStruct;
    dword Name;
    word Id;
};

union IMAGE_RESOURCE_DIRECTORY_ENTRY_DirectoryUnion {
    dword OffsetToData;
    struct IMAGE_RESOURCE_DIRECTORY_ENTRY_DirectoryStruct IMAGE_RESOURCE_DIRECTORY_ENTRY_DirectoryStruct;
};

struct IMAGE_RESOURCE_DIRECTORY_ENTRY {
    union IMAGE_RESOURCE_DIRECTORY_ENTRY_NameUnion NameUnion;
    union IMAGE_RESOURCE_DIRECTORY_ENTRY_DirectoryUnion DirectoryUnion;
};

typedef struct ImgDelayDescr ImgDelayDescr, *PImgDelayDescr;

struct ImgDelayDescr {
    dword grAttrs;
    ImageBaseOffset32 szName;
    ImageBaseOffset32 phmod;
    ImageBaseOffset32 pIAT;
    ImageBaseOffset32 pINT;
    ImageBaseOffset32 pBoundIAT;
    ImageBaseOffset32 pUnloadIAT;
    dword dwTimeStamp;
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

typedef struct _COORD _COORD, *P_COORD;

typedef struct _COORD COORD;

struct _COORD {
    SHORT X;
    SHORT Y;
};

typedef struct _SMALL_RECT _SMALL_RECT, *P_SMALL_RECT;

typedef struct _SMALL_RECT SMALL_RECT;

struct _SMALL_RECT {
    SHORT Left;
    SHORT Top;
    SHORT Right;
    SHORT Bottom;
};

typedef struct _CONSOLE_SCREEN_BUFFER_INFO _CONSOLE_SCREEN_BUFFER_INFO, *P_CONSOLE_SCREEN_BUFFER_INFO;

typedef struct _CONSOLE_SCREEN_BUFFER_INFO *PCONSOLE_SCREEN_BUFFER_INFO;

struct _CONSOLE_SCREEN_BUFFER_INFO {
    COORD dwSize;
    COORD dwCursorPosition;
    WORD wAttributes;
    SMALL_RECT srWindow;
    COORD dwMaximumWindowSize;
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

typedef char *va_list;

typedef ulonglong uintptr_t;

typedef UINT MMRESULT;

typedef struct sockaddr sockaddr, *Psockaddr;

typedef ushort u_short;

struct sockaddr {
    u_short sa_family;
    char sa_data[14];
};

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

typedef WSADATA *LPWSADATA;

typedef struct MRECmpImpl MRECmpImpl, *PMRECmpImpl;

struct MRECmpImpl { // PlaceHolder Structure
};

typedef struct _Cvtvec _Cvtvec, *P_Cvtvec;

struct _Cvtvec { // PlaceHolder Structure
};

typedef struct _Yarn<char> _Yarn<char>, *P_Yarn<char>;

struct _Yarn<char> { // PlaceHolder Structure
};

typedef struct _Facet_base _Facet_base, *P_Facet_base;

struct _Facet_base { // PlaceHolder Structure
};

typedef struct basic_streambuf<char,struct_std::char_traits<char>_> basic_streambuf<char,struct_std::char_traits<char>_>, *Pbasic_streambuf<char,struct_std::char_traits<char>_>;

struct basic_streambuf<char,struct_std::char_traits<char>_> { // PlaceHolder Structure
};

typedef struct basic_istream<char,struct_std::char_traits<char>_> basic_istream<char,struct_std::char_traits<char>_>, *Pbasic_istream<char,struct_std::char_traits<char>_>;

struct basic_istream<char,struct_std::char_traits<char>_> { // PlaceHolder Structure
};

typedef struct locale locale, *Plocale;

struct locale { // PlaceHolder Structure
};

typedef struct basic_istream<char,std::char_traits<char>_> basic_istream<char,std::char_traits<char>_>, *Pbasic_istream<char,std::char_traits<char>_>;

struct basic_istream<char,std::char_traits<char>_> { // PlaceHolder Structure
};

typedef struct basic_streambuf<char,std::char_traits<char>_> basic_streambuf<char,std::char_traits<char>_>, *Pbasic_streambuf<char,std::char_traits<char>_>;

struct basic_streambuf<char,std::char_traits<char>_> { // PlaceHolder Structure
};

typedef struct _Locinfo _Locinfo, *P_Locinfo;

struct _Locinfo { // PlaceHolder Structure
};

typedef struct basic_ios<char,std::char_traits<char>_> basic_ios<char,std::char_traits<char>_>, *Pbasic_ios<char,std::char_traits<char>_>;

struct basic_ios<char,std::char_traits<char>_> { // PlaceHolder Structure
};

typedef struct ctype<char> ctype<char>, *Pctype<char>;

struct ctype<char> { // PlaceHolder Structure
};

typedef struct _Lockit _Lockit, *P_Lockit;

struct _Lockit { // PlaceHolder Structure
};

typedef struct _Locimp _Locimp, *P_Locimp;

struct _Locimp { // PlaceHolder Structure
};

typedef struct facet facet, *Pfacet;

struct facet { // PlaceHolder Structure
};

typedef struct id id, *Pid;

struct id { // PlaceHolder Structure
};

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

typedef longlong __time64_t;

typedef int errno_t;

typedef size_t rsize_t;




void FUN_180001010(void);
void FUN_180001100(void);
void FUN_180001210(void);
void FUN_1800012a0(void);
void FUN_1800017d0(void);
void FUN_180001b60(void);
void FUN_1800039e0(void);
void FUN_180003ad0(void);
void FUN_180003be0(void);
void FUN_180003c70(void);
void FUN_1800041a0(void);
void FUN_180004530(void);
void FUN_1800063b0(void);
void FUN_1800064a0(void);
void FUN_1800065b0(void);
void FUN_180006ae0(void);
void FUN_180006e60(void);
void FUN_180006f50(void);
void FUN_180007060(void);
void FUN_180007590(void);
void FUN_180007910(void);
void FUN_180007a00(void);
void FUN_180007b10(void);
void FUN_180008040(void);
void FUN_1800083d0(void);
void FUN_180008b20(void);
void FUN_180008c10(void);
void FUN_180008d20(void);
void FUN_180009250(void);
void FUN_1800094e0(void);
void FUN_1800095d0(void);
void FUN_1800096e0(void);
void FUN_180009c10(void);
void FUN_180009f90(void);
void FUN_18000a080(void);
void FUN_18000a190(void);
void FUN_18000a6c0(void);
void FUN_18000aa40(void);
void FUN_18000ab30(void);
void FUN_18000ac40(void);
void FUN_18000b170(void);
void FUN_18000b4f0(void);
void FUN_18000b5e0(void);
void FUN_18000b6f0(void);
void FUN_18000bc20(void);
void FUN_18000bfe0(void);
void FUN_18000c0d0(void);
void FUN_18000c1e0(void);
void FUN_18000c710(void);
void FUN_18000ca90(void);
void FUN_18000cb80(void);
void FUN_18000cc90(void);
void FUN_18000d1c0(void);
void FUN_18000d560(void);
void FUN_18000d650(void);
void FUN_18000d760(void);
void FUN_18000dc90(void);
void FUN_18000e030(void);
void FUN_18000e120(void);
void FUN_18000e230(void);
void FUN_18000e760(void);
void FUN_18000ead0(void);
void FUN_18000f520(void);
void FUN_18000f610(void);
void FUN_18000f720(void);
void FUN_18000fc50(void);
void FUN_18000ffc0(void);
void FUN_180010710(void);
void FUN_180010800(void);
void FUN_180010910(void);
void FUN_180010e40(void);
void FUN_180011110(void);
void FUN_180011200(void);
void FUN_180011310(void);
void FUN_180011840(void);
void FUN_180011bc0(void);
void FUN_180011cb0(void);
void FUN_180011dc0(void);
void FUN_1800122f0(void);
void FUN_180012660(void);
void FUN_180012eb0(void);
void FUN_180012fa0(void);
void FUN_1800130b0(void);
void FUN_1800135e0(void);
void FUN_180013960(void);
void FUN_180013a50(void);
void FUN_180013b60(void);
void FUN_180014090(void);
void FUN_180014410(void);
void FUN_180014500(void);
void FUN_180014610(void);
void FUN_180014b40(void);
void FUN_180014ec0(void);
void FUN_180014fb0(void);
void FUN_1800150c0(void);
void FUN_1800155f0(void);
void FUN_180015970(void);
void FUN_180015a60(void);
void FUN_180015b70(void);
void FUN_1800160a0(void);
void FUN_180016420(void);
void FUN_180016510(void);
void FUN_180016620(void);
void FUN_180016b50(void);
void FUN_180016ed0(void);
void FUN_180016fc0(void);
void FUN_1800170d0(void);
void FUN_180017600(void);
void FUN_180017980(void);
void FUN_180017a70(void);
void FUN_180017b80(void);
void FUN_1800180b0(void);
void FUN_180018430(void);
void FUN_180018520(void);
void FUN_180018630(void);
void FUN_180018b60(void);
void FUN_180018ec4(void);
undefined1 (*) [16] FUN_180018f00(undefined1 (*param_1) [16],char *param_2,undefined4 *param_3);
ulonglong FUN_180018f60(size_t param_1);
ulonglong FUN_180018fc0(undefined8 param_1,size_t *param_2);
void FUN_180019040(undefined8 *param_1,void *param_2,size_t param_3);
void FUN_180019140(undefined8 param_1,undefined8 param_2,longlong *param_3);
void FUN_1800191c0(undefined8 param_1,undefined8 param_2,longlong *param_3);
longlong * FUN_180019240(longlong *param_1,longlong *param_2,longlong *param_3,int *param_4);
longlong * FUN_180019480(longlong *param_1,longlong *param_2,longlong *param_3,undefined8 *param_4);
longlong * FUN_180019760(longlong param_1,longlong *param_2,undefined8 *param_3,ulonglong param_4);
void FUN_180019830(float *param_1,longlong *param_2,longlong *param_3);
void FUN_180019b80(longlong param_1,ulonglong param_2);
longlong * FUN_180019ed0(float *param_1,longlong *param_2,undefined8 *param_3);
void FUN_18001a180(undefined8 *param_1,undefined8 *param_2,undefined8 *param_3);
undefined4 * FUN_18001a230(undefined4 *param_1,undefined4 *param_2);
undefined1 (*) [16] FUN_18001a2b0(undefined1 (*param_1) [16],undefined8 *param_2);
undefined1 (*) [16] FUN_18001a3d0(undefined1 (*param_1) [16],char *param_2);
longlong * FUN_18001a420(longlong *param_1,undefined8 *param_2);
float * FUN_18001a550(float *param_1,undefined8 *param_2);
undefined8 * FUN_18001a5a0(undefined8 *param_1,longlong param_2);
undefined8 * FUN_18001a5e0(undefined8 *param_1,longlong param_2);
undefined8 * FUN_18001a620(undefined8 *param_1);
undefined8 * FUN_18001a650(undefined8 *param_1,longlong param_2);
longlong FUN_18001a690(longlong param_1);
void FUN_18001b060(longlong *param_1);
void FUN_18001b0c0(longlong param_1);
void FUN_18001b0e0(undefined8 *param_1);
void FUN_18001b150(longlong param_1);
void FUN_18001b190(longlong *param_1);
void FUN_18001b1c0(longlong *param_1);
void thunk_FUN_18001c360(longlong *param_1);
void FUN_18001b200(longlong *param_1);
ulonglong FUN_18001b2c0(undefined8 param_1,undefined8 *param_2,undefined8 *param_3);
undefined8 * FUN_18001b360(undefined8 *param_1,ulonglong param_2);
ulonglong FUN_18001b3b0(longlong param_1,longlong param_2);
void FUN_18001bc70(ulonglong *param_1,ulonglong param_2,undefined8 param_3);
BOOL __stdcall InitializeCriticalSectionEx(LPCRITICAL_SECTION lpCriticalSection,DWORD dwSpinCount,DWORD Flags);
void FUN_18001bdd0(undefined8 param_1,undefined1 *param_2,longlong param_3);
void FUN_18001be10(longlong param_1,ulonglong param_2);
longlong * FUN_18001c0b0(longlong *param_1,longlong *param_2,longlong *param_3);
void FUN_18001c320(void);
void FUN_18001c340(void);
void FUN_18001c360(longlong *param_1);
void FUN_18001c3d0(void);
ulonglong FUN_18001c3f0(undefined8 param_1,ulonglong param_2);
undefined8 * FUN_18001c470(undefined8 *param_1);
void FUN_18001c480(undefined8 param_1,undefined1 *param_2,ulonglong param_3);
ulonglong FUN_18001c4c0(undefined8 *param_1,longlong param_2,undefined *param_3);
ulonglong FUN_18001cba0(longlong param_1,longlong param_2,longlong param_3);
int FUN_18001ccc0(uint *param_1,longlong param_2,uint *param_3,undefined8 param_4,int *param_5);
char * FUN_18001ceb0(longlong param_1);
ulonglong EffectMain(undefined4 param_1,uint *param_2,undefined8 *param_3,uint *param_4,longlong param_5,longlong *param_6);
void PluginDataEntryFunction2(undefined8 param_1,undefined *param_2);
undefined8 * FUN_18001d650(undefined8 *param_1,ulonglong param_2);
undefined8 * FUN_18001d6e0(undefined8 *param_1,ulonglong param_2);
ulonglong FUN_18001d710(longlong param_1);
void FUN_18001d7e0(longlong param_1);
undefined8 FUN_18001d820(void);
undefined8 FUN_18001d830(longlong param_1,longlong *param_2);
void FUN_18001d960(longlong *param_1);
undefined8 xGPUFilterEntry(undefined8 param_1,int *param_2,int param_3,undefined8 param_4,undefined8 *param_5,undefined4 *param_6);
undefined * FUN_18001da30(int param_1);
undefined8 *FUN_18001da50(undefined8 *param_1,longlong *param_2,undefined8 *param_3,undefined8 *param_4);
undefined8 *FUN_18001dbc0(undefined8 *param_1,longlong *param_2,undefined8 *param_3,undefined8 *param_4);
undefined1 (*) [16]FUN_18001dd30(undefined1 (*param_1) [16],undefined8 *param_2,undefined8 *param_3);
undefined1 (*) [16] FUN_18001dd60(undefined1 (*param_1) [16],longlong *param_2,char *param_3);
undefined1 (*) [16] FUN_18001ddc0(undefined1 (*param_1) [16],char *param_2,char *param_3);
undefined1 (*) [16] FUN_18001dee0(undefined1 (*param_1) [16],char *param_2,undefined8 *param_3);
undefined8 FUN_18001df70(void);
longlong * FUN_18001dfb0(longlong *param_1,uint param_2);
longlong * FUN_18001e030(longlong *param_1,void *param_2,ulonglong param_3);
undefined8 * FUN_18001e0c0(undefined8 *param_1,void *param_2,ulonglong param_3);
undefined8 * FUN_18001e150(undefined8 *param_1,void *param_2,size_t param_3);
longlong * FUN_18001e2c0(longlong *param_1,void *param_2,ulonglong param_3);
void FUN_18001e450(ulonglong *param_1,longlong *param_2,ulonglong param_3);
undefined1 (*) [16] FUN_18001e6d0(undefined1 (*param_1) [16],undefined8 *param_2);
undefined1 (*) [16] FUN_18001e7c0(undefined1 (*param_1) [16],undefined8 *param_2);
undefined1 (*) [16] FUN_18001e8c0(undefined1 (*param_1) [16],UINT param_2,undefined8 *param_3);
void FUN_18001e9b0(longlong param_1,longlong param_2);
undefined8 * FUN_18001ea30(longlong *param_1,undefined8 *param_2,undefined8 *param_3);
undefined8 * FUN_18001ebf0(longlong *param_1,undefined8 *param_2,undefined8 *param_3);
void FUN_18001ee20(undefined8 param_1,undefined8 param_2,longlong *param_3);
void FUN_18001eeb0(undefined8 param_1,undefined8 *param_2);
undefined8 * FUN_18001ef50(undefined8 *param_1,ulonglong param_2,void *param_3,ulonglong param_4);
undefined8 FUN_18001f050(longlong *param_1);
undefined8 *FUN_18001f080(undefined8 *param_1,ulonglong param_2,undefined8 param_3,void *param_4,size_t param_5);
undefined8 *FUN_18001f210(undefined8 *param_1,ulonglong param_2,undefined8 param_3,void *param_4,longlong param_5);
undefined8 *FUN_18001f3d0(undefined8 *param_1,ulonglong param_2,undefined8 param_3,size_t param_4,void *param_5,size_t param_6);
undefined8 *FUN_18001f590(undefined8 *param_1,ulonglong param_2,undefined8 param_3,size_t param_4,char param_5);
undefined8 *FUN_18001f720(undefined8 *param_1,ulonglong param_2,undefined8 param_3,longlong param_4,undefined2 param_5);
undefined8 *FUN_18001f8e0(undefined8 *param_1,ulonglong param_2,undefined8 param_3,undefined2 param_4);
void FUN_18001fa80(undefined8 *param_1,ulonglong param_2,undefined8 *param_3);
undefined8 * FUN_18001fc50(float *param_1,undefined8 *param_2,byte *param_3);
longlong * FUN_18001feb0(float *param_1,longlong *param_2,undefined8 *param_3);
longlong * FUN_1800200f0(float *param_1,longlong *param_2,undefined8 *param_3);
longlong * FUN_180020350(longlong *param_1,longlong param_2,longlong *param_3);
undefined8 * FUN_180020440(undefined8 *param_1,undefined8 *param_2,undefined8 *param_3);
void FUN_1800204a0(longlong *param_1,void *param_2,void *param_3);
void FUN_180020590(uint param_1,longlong *param_2);
void FUN_180020660(uint param_1,longlong *param_2);
void FUN_180020710(ulonglong param_1,longlong *param_2);
undefined8 *FUN_180020790(longlong *param_1,undefined8 *param_2,longlong *param_3,longlong *param_4);
ulonglong FUN_180020970(longlong param_1);
char * FUN_1800209f0(int param_1,longlong param_2,ulonglong param_3,int param_4,char param_5);
longlong FUN_180020a60(longlong param_1,uint param_2,ulonglong param_3);
longlong FUN_180020ad0(longlong param_1,ulonglong param_2,uint param_3);
undefined8 *FUN_180020b70(undefined8 *param_1,longlong *param_2,uint *param_3,uint *param_4,int param_5,uint param_6,longlong param_7);
undefined8 *FUN_180020e00(undefined8 *param_1,longlong *param_2,ulonglong *param_3,uint *param_4,int param_5,uint param_6,longlong param_7);
undefined8 *FUN_1800210b0(undefined8 *param_1,longlong *param_2,undefined8 *param_3,uint *param_4,int param_5,uint param_6,longlong param_7);
undefined8 * FUN_180021320(undefined8 *param_1,longlong *param_2,longlong param_3,uint *param_4);
void FUN_1800213e0(undefined8 *param_1,undefined8 *param_2);
void FUN_1800217e0(undefined8 *param_1,undefined8 *param_2);
undefined8 *FUN_180021bf0(undefined8 *param_1,int param_2,longlong *param_3,ulonglong param_4,uint param_5,char param_6);
undefined8 * FUN_180021d10(undefined8 *param_1,longlong *param_2,uint param_3,ulonglong param_4);
undefined8 *FUN_180021e10(undefined8 *param_1,longlong *param_2,ulonglong param_3,ulonglong param_4);
int FUN_180021f30(undefined8 param_1,uint param_2,uint *param_3,char param_4,ulonglong *param_5);
void FUN_180022500(ulonglong param_1,uint *param_2,longlong *param_3);
undefined4 FUN_180022820(ulonglong *param_1,undefined8 *param_2);
void FUN_1800228d0(int param_1,undefined4 *param_2,ulonglong *param_3,longlong param_4);
void FUN_180022a70(longlong param_1,byte param_2,undefined8 *param_3);
void FUN_180024310(longlong param_1,byte param_2,undefined8 *param_3);
void FUN_180025bb0(longlong *param_1,undefined8 *param_2,int param_3,longlong *param_4,double *param_5);
undefined8 * FUN_180025e30(undefined8 *param_1);
undefined8 * FUN_180025ed0(undefined8 *param_1,undefined8 *param_2,undefined1 (*param_3) [16]);
undefined1 (*) [16]FUN_180025fa0(undefined1 (*param_1) [16],undefined8 *param_2,undefined8 *param_3);
undefined8 * FUN_180025ff0(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_180026070(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_1800260f0(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_180026170(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_1800261f0(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_180026270(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_1800262f0(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_180026370(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_1800263f0(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_180026450(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_1800264b0(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_180026510(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_180026570(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_1800265d0(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_180026630(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_180026690(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_1800266f0(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_180026750(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_1800267b0(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_180026810(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_180026870(undefined8 *param_1,undefined1 *param_2);
undefined8 * FUN_1800268e0(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_180026940(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_1800269a0(undefined8 *param_1,undefined8 *param_2);
void FUN_180026a60(longlong *param_1,ulonglong *param_2);
void FUN_180026c60(uint param_1,longlong *param_2);
void FUN_180026d50(ulonglong param_1,uint param_2,longlong *param_3);
char * FUN_180026e20(char *param_1,char *param_2,longlong *param_3);
char * FUN_180027010(char *param_1,char *param_2,longlong *param_3);
longlong *FUN_1800271e0(longlong *param_1,char *param_2,char *param_3,uint *param_4,int *param_5,longlong param_6);
byte * FUN_1800273d0(byte *param_1,byte *param_2,uint *param_3,longlong param_4,int param_5);
void FUN_180027e30(undefined8 *param_1,longlong *param_2);
ulonglong FUN_180028080(longlong *param_1,char *param_2,uint param_3);
undefined1 (*) [16] FUN_180028100(undefined1 (*param_1) [16],longlong param_2);
void FUN_180028350(longlong param_1,undefined8 *param_2);
undefined8 FUN_180028380(uint param_1);
ulonglong * FUN_180028730(ulonglong *param_1,ulonglong param_2);
longlong FUN_180028a70(longlong *param_1,longlong param_2);
facet * FUN_180028ad0(longlong param_1);
ulonglong FUN_180028c60(ulonglong *param_1,undefined8 *param_2);
void FUN_180028fb0(float *param_1,undefined8 *param_2);
void FUN_180029460(double *param_1,longlong *param_2);
ulonglong FUN_180029a20(ulonglong *param_1);
undefined8 *FUN_180029d10(undefined8 *param_1,longlong *param_2,int param_3,uint *param_4,longlong param_5);
undefined8 * FUN_180029df0(undefined8 *param_1,longlong *param_2,undefined8 *param_3,uint *param_4);
undefined8 * FUN_18002a0a0(undefined8 *param_1,longlong *param_2,float param_3);
undefined8 *FUN_18002a3d0(undefined8 *param_1,longlong *param_2,float param_3,uint *param_4,longlong param_5);
undefined8 * FUN_18002a730(undefined8 *param_1,longlong *param_2,double param_3);
undefined8 *FUN_18002aa70(undefined8 *param_1,longlong *param_2,ulonglong param_3,uint *param_4,longlong param_5);
undefined8 *FUN_18002adc0(undefined8 *param_1,longlong *param_2,ulonglong param_3,uint *param_4,longlong param_5);
undefined8 * FUN_18002b110(undefined8 *param_1,longlong *param_2,byte param_3,uint *param_4);
undefined8 * FUN_18002b250(undefined8 *param_1,longlong *param_2,undefined8 *param_3,uint *param_4);
undefined8 * FUN_18002b330(undefined8 *param_1,longlong *param_2,undefined1 param_3,uint *param_4);
longlong * FUN_18002b380(longlong *param_1,longlong *param_2,undefined1 param_3,uint param_4);
undefined8 * FUN_18002b4b0(undefined8 *param_1,longlong *param_2,undefined1 param_3,uint param_4);
longlong * FUN_18002b570(longlong *param_1,longlong *param_2,undefined8 *param_3);
undefined8 * FUN_18002b910(undefined8 *param_1,longlong *param_2,undefined8 *param_3);
undefined8 * FUN_18002bc20(undefined8 *param_1,uint param_2,longlong *param_3);
undefined8 *FUN_18002bd80(undefined8 *param_1,longlong *param_2,longlong param_3,int param_4,undefined1 param_5,uint *param_6,int param_7,longlong param_8);
undefined8 *FUN_18002c250(undefined8 *param_1,longlong *param_2,longlong param_3,int param_4,undefined1 param_5,uint *param_6,int param_7,longlong param_8);
undefined8 *FUN_18002c720(undefined8 *param_1,longlong *param_2,longlong param_3,int param_4,undefined1 param_5,uint *param_6,int param_7,longlong param_8);
undefined8 *FUN_18002cbf0(undefined8 *param_1,longlong *param_2,longlong param_3,int param_4,undefined1 param_5,uint *param_6,int param_7);
undefined8 *FUN_18002cea0(undefined8 *param_1,longlong *param_2,longlong param_3,int param_4,undefined1 param_5,uint *param_6,int param_7);
undefined8 * FUN_18002d150(undefined8 *param_1,longlong *param_2,ulonglong param_3,uint *param_4);
undefined8 * FUN_18002d490(undefined8 *param_1,longlong *param_2,ulonglong *param_3,uint *param_4);
undefined8 *FUN_18002d830(undefined8 *param_1,longlong *param_2,ulonglong param_3,uint param_4,uint *param_5,longlong *param_6);
undefined8 *FUN_18002dbe0(undefined8 *param_1,longlong *param_2,char param_3,uint *param_4,int param_5);
undefined8 *FUN_18002dd60(undefined8 *param_1,longlong *param_2,uint *param_3,longlong param_4,ulonglong param_5,char *param_6);
undefined8 *FUN_18002df60(undefined8 *param_1,longlong *param_2,uint *param_3,longlong param_4,ulonglong param_5,uint *param_6);
undefined8 *FUN_18002e0e0(undefined8 *param_1,longlong *param_2,uint *param_3,longlong param_4,ulonglong param_5,undefined8 *param_6);
undefined8 *FUN_18002e250(undefined8 *param_1,longlong *param_2,uint *param_3,longlong param_4,ulonglong param_5,undefined8 *param_6);
undefined8 *FUN_18002e3c0(undefined8 *param_1,longlong *param_2,uint *param_3,longlong param_4,longlong *param_5,undefined8 *param_6);
undefined8 *FUN_18002e5a0(undefined8 *param_1,longlong *param_2,uint *param_3,longlong param_4,longlong *param_5,undefined8 *param_6);
undefined8 *FUN_18002e780(undefined8 *param_1,longlong *param_2,uint *param_3,longlong param_4,ulonglong param_5,undefined8 *param_6);
undefined8 * FUN_18002e950(undefined8 *param_1,longlong *param_2,ulonglong param_3,uint *param_4);
undefined8 *FUN_18002eb40(undefined8 *param_1,longlong *param_2,uint param_3,uint param_4,uint param_5,char param_6);
undefined8 *FUN_18002ec70(undefined8 *param_1,longlong *param_2,ulonglong param_3,uint param_4,uint param_5,char param_6);
undefined8 * FUN_18002edc0(undefined8 *param_1);
undefined1 (*) [16] FUN_18002ee80(undefined1 (*param_1) [16],undefined8 *param_2);
undefined1 (*) [16]FUN_18002eeb0(undefined1 (*param_1) [16],undefined8 param_2,undefined8 *param_3,undefined8 *param_4);
undefined1 (*) [16]FUN_18002f090(undefined1 (*param_1) [16],undefined8 param_2,undefined8 param_3,void *param_4,size_t param_5,undefined8 param_6,size_t param_7);
undefined1 (*) [16] FUN_18002f1b0(undefined1 (*param_1) [16]);
undefined1 (*) [16] FUN_18002f1d0(undefined1 (*param_1) [16],undefined8 *param_2);
undefined1 (*) [16] FUN_18002f300(undefined1 (*param_1) [16],longlong *param_2,longlong *param_3);
undefined1 (*) [16] FUN_18002f430(undefined1 (*param_1) [16],longlong param_2,char param_3);
facet * FUN_18002f5b0(facet *param_1,longlong param_2);
undefined8 *FUN_18002f7a0(undefined8 *param_1,longlong *param_2,longlong param_3,ulonglong param_4,char param_5,longlong param_6);
undefined4 * FUN_18002fa60(undefined4 *param_1,undefined4 *param_2);
undefined4 * FUN_18002fb30(undefined4 *param_1);
undefined8 * FUN_18002fbb0(undefined8 *param_1,int param_2);
undefined8 * FUN_18002fd40(undefined8 *param_1,longlong param_2);
undefined8 * FUN_18002fe00(undefined8 *param_1,longlong param_2,undefined8 *param_3);
undefined1 (*) [16]FUN_18002fec0(undefined1 (*param_1) [16],undefined8 *param_2,undefined8 *param_3);
undefined8 * FUN_1800300a0(undefined8 *param_1,longlong param_2);
undefined4 * FUN_180030100(undefined4 *param_1);
undefined8 * FUN_180030170(undefined8 *param_1);
undefined8 * FUN_1800301a0(undefined8 *param_1,longlong param_2);
undefined8 * FUN_1800301e0(undefined8 *param_1);
undefined8 * FUN_180030220(undefined8 *param_1,longlong param_2);
undefined8 * FUN_180030270(undefined8 *param_1,undefined8 param_2,undefined8 *param_3);
undefined8 * FUN_1800302d0(undefined8 *param_1,longlong param_2);
undefined8 * FUN_180030330(undefined8 *param_1,longlong param_2);
undefined1 (*) [16]FUN_1800303c0(undefined1 (*param_1) [16],undefined8 *param_2,undefined8 *param_3,undefined8 *param_4);
undefined8 * FUN_1800304f0(undefined8 *param_1,longlong param_2);
undefined8 * FUN_180030530(undefined8 *param_1,undefined8 param_2);
undefined8 * FUN_180030580(undefined8 *param_1,longlong param_2);
undefined8 * FUN_1800307a0(undefined8 *param_1,ulonglong *param_2,undefined8 *param_3);
undefined8 *FUN_180030960(undefined8 *param_1,longlong *param_2,undefined4 param_3,longlong *param_4,undefined4 *param_5);
undefined8 * FUN_180030b30(undefined8 *param_1,undefined4 param_2,longlong *param_3);
undefined4 * FUN_180030d30(undefined4 *param_1);
undefined8 * FUN_180031260(undefined8 *param_1,longlong param_2);
undefined8 *FUN_1800312a0(undefined8 *param_1,undefined8 param_2,undefined4 param_3,undefined8 param_4,undefined4 param_5);
undefined8 * FUN_1800312c0(undefined8 *param_1,longlong param_2);
undefined8 * FUN_180031320(undefined8 *param_1,char *param_2,undefined4 param_3);
undefined8 * FUN_1800314b0(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_180031510(undefined8 *param_1,longlong param_2);
undefined8 ***** FUN_180031570(undefined8 *****param_1,undefined4 *param_2);
undefined1 (*) [16]FUN_180031670(undefined1 (*param_1) [16],undefined8 *param_2,undefined8 *param_3);
void FUN_1800316e0(longlong *param_1);
void FUN_180031710(longlong param_1);
void FUN_180031760(longlong param_1);
void FUN_180031800(longlong *param_1);
void FUN_180031840(undefined8 *param_1);
void FUN_180031880(longlong *param_1);
void thunk_FUN_180035f50(longlong *param_1);
void FUN_180031920(basic_streambuf<> *param_1);
void FUN_180031a00(undefined8 *param_1);
void FUN_180031b00(longlong *param_1);
void FUN_180031b20(facet *param_1);
void FUN_180031b60(longlong *param_1);
void FUN_180031be0(undefined8 *param_1);
void FUN_180031c10(longlong param_1);
void FUN_180031c60(longlong *param_1);
void FUN_180031c90(longlong param_1);
void FUN_180031cf0(longlong *param_1);
void FUN_180031d10(undefined8 *param_1);
void FUN_180031d30(undefined8 *param_1);
void FUN_180031d90(undefined8 *param_1);
void FUN_180031dc0(undefined8 *param_1);
void FUN_180031e00(undefined8 *param_1);
void FUN_180031e60(undefined8 *param_1);
void FUN_180031ef0(longlong param_1);
void FUN_180031f10(longlong param_1);
void FUN_180031f60(longlong *param_1);
void FUN_180032010(undefined8 *param_1);
void FUN_1800320b0(longlong *param_1);
void FUN_180032150(longlong param_1);
void FUN_180032180(undefined8 *param_1);
void FUN_1800321c0(longlong param_1);
void FUN_1800321f0(longlong param_1);
void FUN_180032260(longlong *param_1);
void FUN_1800322e0(undefined8 *param_1);
void FUN_180032310(longlong param_1);
longlong * FUN_180032330(longlong *param_1,longlong *param_2);
undefined8 * FUN_1800323c0(undefined8 *param_1,undefined8 *param_2);
longlong * FUN_180032440(longlong *param_1,undefined1 param_2);
longlong * FUN_1800324a0(longlong *param_1,longlong *param_2);
longlong * FUN_180032520(longlong *param_1,longlong *param_2);
longlong * FUN_180032590(longlong *param_1);
undefined1 (*) [16] FUN_180032620(undefined1 (*param_1) [16],uint *param_2,uint *param_3);
undefined8 * FUN_1800327a0(int *param_1,undefined8 *param_2,longlong *param_3);
undefined8 * FUN_180032890(int *param_1,undefined8 *param_2,longlong *param_3);
undefined8 * FUN_180032980(int *param_1,undefined8 *param_2,longlong *param_3);
ulonglong FUN_180032aa0(undefined8 *param_1,uint param_2,longlong *param_3);
undefined8 * FUN_180032d10(undefined8 *param_1,undefined8 *param_2,longlong *param_3);
undefined8 * FUN_180032f40(undefined8 *param_1,undefined8 *param_2,longlong *param_3);
undefined8 * FUN_180033170(undefined8 *param_1,undefined8 *param_2,longlong *param_3);
undefined8 * FUN_180033390(undefined8 *param_1,undefined8 *param_2,longlong *param_3);
undefined8 * FUN_1800334c0(undefined8 *param_1,undefined8 *param_2,longlong *param_3);
void FUN_1800335f0(undefined8 *param_1,ulonglong param_2,char *param_3);
undefined8 * FUN_1800336c0(char *param_1,undefined8 *param_2,longlong *param_3);
undefined8 * FUN_1800338a0(undefined8 *param_1,undefined8 *param_2,longlong *param_3);
undefined8 * FUN_180033a50(undefined8 *param_1,undefined8 *param_2,longlong *param_3);
undefined8 * FUN_180033c00(undefined8 *param_1,undefined8 *param_2,longlong *param_3);
void FUN_180033e40(void);
uint * FUN_180033e80(uint *param_1,uint *param_2);
longlong * FUN_180033fc0(longlong *param_1,int param_2);
void FUN_180034080(longlong *param_1);
void FUN_1800340d4(longlong param_1,uint param_2);
void FUN_1800340e0(longlong param_1,ulonglong param_2);
void FUN_1800340ec(longlong param_1,ulonglong param_2);
void * FUN_180034100(void *param_1,ulonglong param_2);
undefined8 * FUN_180034130(undefined8 *param_1,ulonglong param_2);
undefined8 * FUN_180034160(undefined8 *param_1,ulonglong param_2);
undefined8 * FUN_180034190(undefined8 *param_1,ulonglong param_2);
undefined8 * FUN_1800341c0(undefined8 *param_1,ulonglong param_2);
void * FUN_1800341f0(void *param_1,uint param_2);
basic_ios<> * FUN_180034240(basic_ios<> *param_1,uint param_2);
basic_streambuf<> * FUN_1800342e0(basic_streambuf<> *param_1,uint param_2);
void * FUN_180034320(void *param_1,ulonglong param_2);
facet * FUN_180034350(facet *param_1,uint param_2);
void * FUN_1800343b0(void *param_1,uint param_2);
facet * FUN_180034400(facet *param_1,uint param_2);
void * FUN_180034460(void *param_1,uint param_2);
undefined8 * FUN_1800344e0(undefined8 *param_1,uint param_2);
undefined8 * FUN_180034530(undefined8 *param_1,ulonglong param_2);
void * FUN_1800345b0(void *param_1,ulonglong param_2);
void * FUN_1800345e0(void *param_1,ulonglong param_2);
undefined8 * FUN_180034610(undefined8 *param_1,ulonglong param_2);
void * __thiscall MRECmpImpl::`scalar_deleting_destructor'(MRECmpImpl *this,uint param_1);
undefined8 * FUN_1800346a0(undefined8 *param_1,ulonglong param_2);
undefined8 * FUN_1800346d0(undefined8 *param_1,ulonglong param_2);
undefined8 * FUN_180034720(undefined8 *param_1,uint param_2);
void * FUN_180034780(void *param_1,ulonglong param_2);
void * FUN_1800347d0(void *param_1,ulonglong param_2);
void * FUN_180034850(void *param_1,uint param_2);
undefined8 * FUN_1800348c0(undefined8 *param_1,uint param_2);
void FUN_180034910(char *param_1);
void FUN_180034be0(ulonglong *param_1,ulonglong param_2);
void FUN_180034c70(ulonglong *param_1,ulonglong param_2);
void FUN_180034d00(longlong *param_1);
void FUN_180034d20(longlong param_1);
void FUN_180034d30(longlong param_1);
uint * FUN_180034d90(uint *param_1,uint *param_2);
void FUN_180034e90(longlong param_1,ulonglong param_2);
void FUN_180035040(longlong param_1,ulonglong param_2);
undefined8 FUN_1800352e0(void);
void FUN_1800352f0(longlong param_1,ulonglong param_2);
void FUN_1800354f0(longlong param_1,_Locinfo *param_2,char param_3);
undefined4 * FUN_180035680(undefined4 *param_1,undefined4 param_2);
undefined1 (*) [16]FUN_1800356a0(undefined1 (*param_1) [16],undefined8 *param_2,undefined8 *param_3,undefined8 *param_4);
ulonglong FUN_180035b40(longlong param_1,longlong param_2,longlong param_3,longlong param_4);
void FUN_180035bd0(float *param_1);
void FUN_180035c80(void);
void FUN_180035ca0(char *param_1,undefined8 *param_2,undefined8 *param_3);
void FUN_180035d00(undefined4 param_1);
void FUN_180035d40(undefined4 param_1);
void FUN_180035d80(longlong *param_1);
void FUN_180035e10(longlong *param_1);
void FUN_180035e80(longlong *param_1);
void FUN_180035f50(longlong *param_1);
void FUN_180035fc0(void);
void FUN_180035fe0(void);
void FUN_180036000(void);
int FUN_180036020(longlong *param_1,longlong *param_2,longlong *param_3);
undefined8 * thunk_FUN_18001e150(undefined8 *param_1,void *param_2,size_t param_3);
void FUN_180036170(longlong *param_1,uint param_2);
char * FUN_1800362b0(char *param_1);
undefined1 (*) [16]FUN_180036320(undefined1 (*param_1) [16],undefined1 (*param_2) [32],longlong param_3);
undefined8 FUN_1800365c0(uint *param_1);
undefined8 * FUN_180036630(longlong param_1);
undefined8 * FUN_180036790(longlong param_1,undefined8 *param_2,longlong *param_3);
undefined8 * FUN_180036850(longlong param_1,undefined8 *param_2);
void FUN_180036a10(longlong param_1);
undefined8 FUN_180036a80(longlong *param_1,longlong *param_2);
void FUN_180036b20(longlong param_1,byte *param_2);
int FUN_180036e30(char *param_1,int param_2);
undefined8 FUN_180036ec0(undefined8 *param_1);
ulonglong FUN_180037130(uint *param_1,uint *param_2);
int * FUN_180037480(undefined8 param_1,int *param_2,int param_3);
undefined4 * FUN_180037500(undefined8 param_1,undefined4 *param_2,undefined4 param_3);
int FUN_180037510(longlong *param_1,longlong *param_2);
undefined1 FUN_180037710(longlong param_1);
undefined1 (*) [16] FUN_180037720(longlong param_1,undefined1 (*param_2) [16]);
undefined1 (*) [16] FUN_180037770(longlong param_1,undefined1 (*param_2) [16]);
ulonglong FUN_1800377c0(longlong param_1,undefined8 param_2,ulonglong *param_3,undefined8 param_4);
undefined1 FUN_180037870(longlong param_1);
undefined1 (*) [16] FUN_180037880(longlong param_1,undefined1 (*param_2) [16]);
void FUN_1800378d0(longlong param_1,undefined1 param_2,undefined4 param_3,undefined1 (*param_4) [16]);
longlong FUN_180037ae0(longlong param_1,int *param_2,int param_3);
undefined8 FUN_180037b00(longlong *param_1,undefined4 param_2,int *param_3);
longlong * FUN_180037b40(longlong *param_1,ulonglong param_2);
void FUN_180037b80(longlong param_1,undefined8 *param_2);
undefined8 FUN_180037dd0(longlong param_1);
undefined8 * thunk_FUN_18003f450(undefined8 *param_1,undefined8 *param_2,ulonglong param_3);
undefined1 (*) [16] FUN_180037df0(undefined1 (*param_1) [16],undefined8 *param_2);
bool FUN_180037e10(undefined1 (*param_1) [32],longlong param_2);
undefined1 * FUN_180037e40(undefined1 (*param_1) [16],undefined1 (*param_2) [16],ulonglong param_3);
void FUN_180037f40(longlong *param_1);
void _guard_check_icall(void);
void FUN_180037fc0(longlong param_1);
void FUN_180038040(longlong param_1);
void FUN_180038050(longlong param_1);
void FUN_1800380c0(undefined8 param_1,undefined8 param_2,longlong param_3,longlong *param_4);
void FUN_180038100(longlong param_1,undefined8 param_2,longlong param_3,longlong *param_4);
void FUN_180038220(undefined8 param_1,undefined8 param_2,longlong param_3,longlong *param_4);
void FUN_180038260(longlong param_1,undefined8 param_2,longlong param_3,longlong *param_4);
void FUN_180038380(undefined8 param_1,undefined8 param_2,longlong param_3,longlong *param_4);
void FUN_1800383b0(longlong param_1,undefined8 param_2,longlong param_3,longlong *param_4);
void FUN_1800384c0(undefined8 param_1,undefined8 param_2,longlong param_3,longlong *param_4);
void FUN_180038560(longlong param_1,undefined8 param_2,longlong param_3,longlong *param_4);
void FUN_1800386c0(undefined8 param_1,longlong param_2,undefined8 param_3,longlong *param_4);
void FUN_1800386f0(longlong param_1,longlong param_2,undefined8 param_3,longlong *param_4);
void FUN_180038800(undefined8 param_1,longlong param_2,undefined8 param_3,longlong *param_4);
void FUN_180038840(longlong param_1,longlong param_2,undefined8 param_3,longlong *param_4);
void FUN_180038970(undefined8 param_1,undefined8 param_2,longlong param_3,longlong *param_4);
void FUN_180038980(longlong param_1,undefined8 param_2,longlong param_3,longlong *param_4);
void FUN_180038a70(undefined8 param_1,undefined8 param_2,longlong param_3,longlong *param_4);
void FUN_180038a90(longlong param_1,undefined8 param_2,longlong param_3,longlong *param_4);
void FUN_180038b90(undefined8 param_1,undefined8 param_2,longlong param_3,longlong *param_4);
void FUN_180038ba0(longlong param_1,undefined8 param_2,longlong param_3,longlong *param_4);
void FUN_180038c90(undefined8 param_1,undefined8 param_2,longlong param_3,longlong *param_4);
void FUN_180038cf0(longlong param_1,undefined8 param_2,longlong param_3,longlong *param_4);
void FUN_180038e10(undefined8 param_1,undefined8 param_2,uint *param_3,longlong *param_4);
void FUN_180038e20(longlong param_1,undefined8 param_2,uint *param_3,longlong *param_4);
void FUN_180038f10(undefined8 param_1,undefined8 param_2,uint *param_3,longlong *param_4);
void FUN_180038f90(longlong param_1,undefined8 param_2,uint *param_3,longlong *param_4);
void FUN_1800390e0(undefined8 param_1,undefined8 param_2,longlong param_3,longlong *param_4);
void FUN_180039100(longlong param_1,undefined8 param_2,longlong param_3,longlong *param_4);
void FUN_180039200(undefined8 param_1,undefined8 param_2,longlong param_3,longlong *param_4);
void FUN_180039240(longlong param_1,undefined8 param_2,longlong param_3,longlong *param_4);
void FUN_180039360(undefined8 param_1,undefined8 param_2,longlong param_3,longlong *param_4);
void FUN_1800393a0(longlong param_1,undefined8 param_2,longlong param_3,longlong *param_4);
void FUN_1800394c0(undefined8 param_1,undefined8 param_2,uint *param_3,longlong *param_4);
void FUN_180039650(longlong param_1,undefined8 param_2,uint *param_3,longlong *param_4);
void FUN_1800398a0(undefined8 param_1,undefined8 param_2,longlong param_3,longlong *param_4);
void FUN_1800398b0(longlong param_1,undefined8 param_2,longlong param_3,longlong *param_4);
void FUN_1800399a0(undefined8 param_1,longlong param_2,undefined8 param_3,longlong *param_4);
void FUN_180039a00(longlong param_1,longlong param_2,undefined8 param_3,longlong *param_4);
void FUN_180039b40(longlong param_1,longlong param_2,undefined8 param_3,longlong *param_4);
void FUN_180039be0(longlong param_1,longlong param_2,undefined8 param_3,longlong *param_4);
void FUN_180039c70(longlong param_1,longlong param_2,undefined8 param_3,longlong *param_4);
void FUN_180039d10(longlong param_1,longlong param_2,undefined8 param_3,longlong *param_4);
void FUN_180039db0(longlong param_1,longlong param_2,undefined8 param_3,longlong *param_4);
void FUN_180039f60(longlong param_1,longlong param_2,undefined8 param_3,longlong *param_4);
void FUN_18003a0f0(longlong param_1,longlong param_2,undefined8 param_3,longlong *param_4);
void FUN_18003a2a0(longlong param_1,longlong param_2,undefined8 param_3,longlong *param_4);
void FUN_18003a450(undefined8 param_1,longlong param_2,undefined8 param_3,longlong *param_4);
void FUN_18003a4b0(longlong param_1,longlong param_2,undefined8 param_3,longlong *param_4);
void FUN_18003a5f0(undefined8 param_1,longlong param_2,undefined8 param_3,longlong *param_4);
void FUN_18003a620(longlong param_1,longlong param_2,undefined8 param_3,longlong *param_4);
void FUN_18003a730(undefined8 param_1,undefined8 param_2,longlong param_3,longlong *param_4);
void FUN_18003a740(longlong param_1,undefined8 param_2,longlong param_3,longlong *param_4);
void FUN_18003a830(undefined8 param_1,undefined8 param_2,undefined8 param_3,longlong *param_4);
void FUN_18003a870(longlong param_1,undefined8 param_2,undefined8 param_3,longlong *param_4);
void FUN_18003a950(undefined8 param_1,undefined8 *param_2,undefined8 param_3,longlong *param_4);
void FUN_18003a970(longlong param_1,undefined8 *param_2,undefined8 param_3,longlong *param_4);
void FUN_18003aa70(undefined8 param_1,undefined8 param_2,longlong param_3,longlong *param_4);
void FUN_18003aab0(longlong param_1,undefined8 param_2,longlong param_3,longlong *param_4);
void FUN_18003abd0(void);
void FUN_18003abf0(longlong param_1,undefined8 param_2,undefined8 param_3,longlong *param_4);
void FUN_18003ad10(undefined8 param_1,undefined8 param_2,uint *param_3,longlong *param_4);
void FUN_18003ae00(longlong param_1,undefined8 param_2,uint *param_3,longlong *param_4);
void FUN_18003afb0(undefined8 param_1,longlong param_2,undefined8 param_3,longlong *param_4);
void FUN_18003b000(longlong param_1,longlong param_2,undefined8 param_3,longlong *param_4);
void FUN_18003b200(undefined8 param_1,longlong param_2,undefined8 param_3,longlong *param_4);
void FUN_18003b240(longlong param_1,longlong param_2,undefined8 param_3,longlong *param_4);
void FUN_18003b360(undefined8 param_1,longlong param_2,undefined8 param_3,longlong *param_4);
void FUN_18003b390(longlong param_1,longlong param_2,undefined8 param_3,longlong *param_4);
void FUN_18003b530(undefined8 param_1,longlong param_2,undefined8 param_3,longlong *param_4);
void FUN_18003b560(longlong param_1,longlong param_2,undefined8 param_3,longlong *param_4);
void FUN_18003b700(undefined8 param_1,longlong param_2,undefined8 param_3,longlong *param_4);
void FUN_18003b710(longlong param_1,longlong param_2,undefined8 param_3,longlong *param_4);
void FUN_18003b890(undefined8 param_1,longlong param_2,undefined8 param_3,longlong *param_4);
void FUN_18003b910(longlong param_1,longlong param_2,undefined8 param_3,longlong *param_4);
void FUN_18003bb10(undefined8 param_1,longlong param_2,undefined8 param_3,longlong *param_4);
void FUN_18003bb70(longlong param_1,longlong param_2,undefined8 param_3,longlong *param_4);
void FUN_18003bce0(undefined8 param_1,longlong param_2,undefined8 param_3,longlong *param_4);
void FUN_18003bd00(longlong param_1,longlong param_2,undefined8 param_3,longlong *param_4);
void FUN_18003be00(longlong param_1,longlong param_2,undefined8 *param_3,longlong *param_4);
void FUN_18003bf10(longlong param_1,longlong param_2,undefined8 *param_3,longlong *param_4);
void FUN_18003c0e0(longlong param_1,undefined8 param_2,undefined8 param_3,longlong *param_4);
void FUN_18003c100(longlong param_1,undefined8 param_2,undefined8 param_3,longlong *param_4);
void FUN_18003c150(undefined8 param_1,longlong param_2,undefined8 param_3,longlong param_4);
void FUN_18003c160(undefined8 param_1,longlong param_2,undefined8 param_3,longlong param_4);
void FUN_18003c170(longlong param_1,undefined8 *param_2,uint *param_3,longlong *param_4);
void FUN_18003c680(longlong param_1,longlong param_2,longlong *param_3);
void FUN_18003c790(ulonglong *param_1,uint param_2,uint param_3,ulonglong *param_4,uint *param_5);
void FUN_18003dd80(longlong param_1,uint param_2,undefined8 *param_3);
void FUN_18003dee0(undefined8 param_1,longlong *param_2,longlong *param_3);
void FUN_18003e030(longlong param_1,longlong *param_2,longlong *param_3);
void FUN_18003e250(longlong *param_1,undefined4 param_2,char *param_3);
void FUN_18003e430(char *param_1);
ulonglong * FUN_18003e770(ulonglong *param_1,int param_2);
longlong FUN_18003e870(void);
void FUN_18003e900(undefined8 *param_1,ulonglong param_2);
void FUN_18003e990(undefined8 *param_1,ulonglong param_2);
undefined * FUN_18003ea60(void);
undefined * FUN_18003ec10(void);
bool FUN_18003ec90(uint *param_1);
uint FUN_18003ed10(uint param_1);
void FUN_18003ef60(undefined1 (*param_1) [16]);
void FUN_18003efd0(longlong *param_1,undefined8 param_2);
void FUN_18003f050(longlong param_1,longlong param_2);
void FUN_18003f260(longlong *param_1,undefined8 *param_2,char param_3,char param_4);
undefined4 * FUN_18003f430(undefined4 *param_1,undefined4 param_2);
undefined8 * FUN_18003f450(undefined8 *param_1,undefined8 *param_2,ulonglong param_3);
undefined1 (*) [16] FUN_180046970(undefined8 param_1,undefined1 (*param_2) [16],int param_3);
undefined1 (*) [16] FUN_1800469d0(undefined8 param_1,undefined1 (*param_2) [16],DWORD param_3);
void FUN_180046a90(longlong *param_1,longlong *param_2);
char * FUN_180046b20(void);
char * FUN_180046b30(void);
int FUN_180046b40(longlong param_1);
byte * FUN_180046ba0(longlong *param_1,uint param_2,byte *param_3,byte *param_4);
void FUN_180046d90(longlong param_1,uint param_2);
void FUN_180046e20(int *param_1,undefined1 (*param_2) [16],char param_3);
int FUN_180047200(basic_streambuf<> *param_1,int param_2);
void FUN_1800473b0(uint param_1,longlong *param_2);
int FUN_1800474e0(basic_streambuf<> *param_1,int param_2);
void FUN_180047570(undefined8 *param_1,undefined1 param_2);
void FUN_180047710(undefined8 *param_1,undefined2 param_2);
void FUN_180047750(longlong *param_1,undefined8 *param_2);
void FUN_180047780(undefined1 (*param_1) [16]);
void FUN_1800478a0(longlong param_1,longlong *param_2);
uint FUN_1800479f0(ulonglong *param_1);
void FUN_180047ae0(int *param_1,char param_2);
void FUN_180047b20(undefined8 *param_1,ulonglong param_2);
void FUN_180047ca0(undefined8 *param_1,ulonglong param_2,char param_3);
void FUN_180047d40(undefined8 *param_1,ulonglong param_2,undefined2 param_3);
void FUN_180047dd0(longlong param_1);
void FUN_180047e00(longlong param_1);
void FUN_1800480c0(longlong param_1);
void FUN_180048340(char *param_1,char *param_2,rsize_t param_3);
ulonglong *FUN_180048380(basic_streambuf<> *param_1,ulonglong *param_2,longlong param_3,int param_4,byte param_5);
ulonglong *FUN_180048530(basic_streambuf<> *param_1,ulonglong *param_2,longlong *param_3,uint param_4);
void FUN_180048680(longlong *param_1,longlong *param_2);
void FUN_180048720(longlong param_1,longlong *param_2);
void FUN_1800487c0(longlong param_1,longlong *param_2);
void FUN_180048810(longlong *param_1,undefined8 param_2);
void FUN_180048890(longlong param_1,undefined8 *param_2);
void FUN_1800489b0(longlong *param_1,undefined8 *param_2);
void FUN_180048a90(undefined1 (*param_1) [16]);
void FUN_180048b40(longlong param_1,undefined8 param_2);
void FUN_180048c40(longlong param_1,undefined8 param_2);
void FUN_180048d90(longlong *param_1,longlong param_2);
void FUN_180048e70(longlong param_1);
undefined1 (*) [16] FUN_180048f10(undefined1 (*param_1) [16],undefined1 (*param_2) [32]);
void FUN_180049150(longlong *param_1);
undefined1 (*) [16]FUN_1800493b0(undefined8 *param_1,undefined1 (*param_2) [16],ulonglong param_3,ulonglong param_4);
void FUN_180049410(char *param_1,undefined4 param_2);
void FUN_180049440(undefined8 *param_1);
ulonglong FUN_180049480(basic_streambuf<> *param_1);
longlong FUN_180049550(undefined8 *param_1);
undefined1 (*) [16] FUN_1800495f0(undefined1 (*param_1) [16],longlong *param_2,ulonglong *param_3);
longlong * FUN_180049750(longlong param_1);
longlong * FUN_180049770(longlong param_1);
void FUN_180049780(undefined1 param_1,undefined4 param_2,longlong *param_3);
void FUN_180049850(char param_1,undefined4 param_2,undefined8 *param_3);
undefined1 FUN_18004a680(undefined8 param_1,undefined8 *param_2,undefined8 param_3,longlong param_4);
undefined * FUN_18004a950(void);
void FUN_18004a960(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
undefined1 (*) [16] FUN_18004a9b0(undefined1 (*param_1) [16],void *param_2,void *param_3);
undefined1 (*) [16] FUN_18004aae0(undefined1 (*param_1) [16],longlong *param_2,undefined8 *param_3);
undefined4 * FUN_18004ab50(longlong *param_1,void *param_2,undefined4 *param_3);
char * FUN_18004acb0(char *param_1,uint param_2);
ulonglong * FUN_18004ad10(ulonglong *param_1,longlong *param_2);
ulonglong * FUN_18004ad80(ulonglong *param_1,undefined8 *param_2);
void thunk_FUN_18004afc0(undefined8 *param_1);
void FUN_18004ae20(ulonglong *param_1,ulonglong param_2);
void FUN_18004aeb0(ulonglong *param_1,ulonglong param_2);
void FUN_18004af40(longlong *param_1,longlong param_2,longlong param_3,longlong param_4);
void FUN_18004afc0(undefined8 *param_1);
int FUN_18004b030(longlong param_1,longlong *param_2,undefined4 param_3,longlong *param_4,longlong *param_5,ulonglong param_6,ulonglong param_7,undefined4 param_8,undefined4 param_9);
void FUN_18004b610(longlong param_1,undefined8 param_2,undefined4 param_3);
void FUN_18004b6b0(longlong param_1,undefined8 param_2,undefined4 param_3);
int FUN_18004b760(longlong param_1,longlong param_2,longlong param_3,int param_4,char param_5);
undefined4 FUN_18004ba90(undefined8 *param_1,undefined8 param_2,longlong param_3,longlong *param_4);
void FUN_18004c410(undefined8 param_1,undefined1 *param_2,longlong param_3);
longlong * FUN_18004c450(longlong param_1,longlong *param_2,undefined8 *param_3);
ulonglong FUN_18004c550(longlong param_1,undefined8 ****param_2,uint param_3);
void FUN_18004c7d0(longlong param_1,longlong param_2,longlong param_3,longlong param_4);
undefined8 FUN_18004c8e0(longlong param_1,longlong param_2,short param_3);
void FUN_18004c9a0(longlong *param_1,undefined4 *param_2);
undefined1 (*) [16] FUN_18004cb10(undefined1 (*param_1) [16],uint param_2);
undefined4 * FUN_18004cb80(uint *param_1,char *param_2);
undefined8 * FUN_18004cc30(uint *param_1,undefined8 *param_2,uint *param_3);
void FUN_18004cd40(float *param_1,longlong *param_2,longlong *param_3);
ulonglong FUN_18004d080(undefined1 (*param_1) [32],ulonglong param_2,ulonglong param_3,undefined1 (*param_4) [32],ulonglong param_5);
facet * FUN_18004d100(locale *param_1);
undefined1 (*) [16]FUN_18004d220(undefined1 (*param_1) [16],undefined8 param_2,undefined8 param_3,undefined8 param_4);
float * FUN_18004d330(float *param_1,float *param_2);
void FUN_18004d3d0(undefined1 (*param_1) [16]);
void FUN_18004d4b0(longlong param_1);
double FUN_18004d4e0(double *param_1);
undefined8 FUN_18004d540(double *param_1);
ulonglong FUN_18004d5b0(ulonglong param_1);
undefined8 FUN_18004d5e0(uint *param_1,char *param_2);
int FUN_18004d670(longlong param_1,longlong param_2);
int FUN_18004dc30(longlong param_1,undefined8 param_2,char param_3);
ulonglong FUN_18004de40(uint *param_1,longlong param_2,char param_3,uint *param_4,longlong param_5,longlong param_6);
ulonglong FUN_18004e780(uint *param_1,longlong param_2,uint *param_3,longlong param_4,uint *param_5);
ulonglong FUN_180050520(longlong param_1,longlong param_2,undefined8 param_3,int *param_4,longlong param_5,longlong param_6);
ulonglong FUN_180051320(longlong param_1,longlong param_2,uint *param_3,char param_4,int *param_5,longlong param_6,longlong param_7);
ulonglong FUN_180052b80(longlong param_1,undefined8 param_2,longlong param_3,longlong param_4,uint param_5);
undefined8 FUN_180053320(longlong param_1,undefined8 param_2,undefined8 param_3,longlong param_4,longlong param_5);
void FUN_180053420(wchar_t *param_1,wchar_t *param_2);
ulonglong FUN_180053450(longlong param_1,uint param_2,longlong param_3,undefined8 *param_4,uint param_5);
int FUN_180053690(longlong param_1,undefined8 param_2,char param_3,undefined1 *param_4);
ulonglong FUN_180053d90(int param_1,undefined8 *param_2,longlong param_3,int *param_4);
undefined1 (*) [16] FUN_180054200(undefined1 (*param_1) [16],undefined1 (*param_2) [32]);
undefined8 * FUN_180054450(longlong *param_1,undefined8 *param_2,undefined8 *param_3);
ulonglong FUN_180054650(uint *param_1,longlong param_2,uint *param_3,longlong *param_4,int param_5,int param_6,int param_7);
uint FUN_180054800(longlong param_1,int param_2);
int FUN_180054850(longlong param_1,longlong param_2,longlong param_3,longlong *param_4,int param_5);
int FUN_1800549d0(longlong param_1,longlong param_2,longlong param_3,longlong param_4,longlong param_5,uint *param_6,undefined1 *param_7);
ulonglong FUN_180054ed0(longlong param_1,longlong param_2,uint *param_3,int *param_4,longlong param_5,longlong param_6);
ulonglong FUN_180055d50(undefined8 *param_1,longlong param_2,longlong param_3,int param_4,int param_5,char param_6,char *param_7);
void FUN_180056330(int *param_1,int *param_2,undefined8 *param_3);
void FUN_1800563e0(int *param_1,int *param_2,int *param_3);
void FUN_180056460(ulonglong *param_1);
void FUN_1800564d0(ulonglong *param_1);
void FUN_180056540(ulonglong *param_1);
void FUN_1800565d0(ulonglong *param_1);
void FUN_180056660(undefined8 param_1,int *param_2,int *param_3,undefined8 *param_4);
void FUN_1800567f0(undefined8 param_1,int *param_2,int *param_3,int *param_4);
void FUN_180056980(undefined8 param_1,int *param_2,int *param_3,int *param_4);
void FUN_180056b40(undefined8 param_1,int *param_2,int *param_3,int *param_4);
undefined8 * FUN_180056d00(undefined8 *param_1,longlong param_2);
undefined8 * FUN_180056d40(undefined8 *param_1);
void FUN_180056da0(void);
void FUN_180056de0(longlong param_1);
int FUN_180056e60(longlong param_1,longlong param_2,longlong param_3);
undefined8 FUN_1800575d0(longlong param_1,longlong param_2,longlong param_3);
undefined4 FUN_1800576e0(longlong param_1,longlong param_2,longlong param_3);
ulonglong FUN_180057bd0(longlong param_1,longlong param_2,undefined1 *param_3);
undefined8 FUN_180058200(undefined8 param_1,longlong *param_2);
undefined8 FUN_180058470(undefined8 param_1,undefined8 param_2,longlong param_3);
undefined4 FUN_1800589e0(undefined8 param_1,undefined8 param_2,longlong param_3,longlong *param_4);
undefined8 FUN_180058c90(longlong param_1);
void FUN_180058ce0(int param_1,int param_2,longlong param_3,longlong param_4);
void FUN_180059290(int param_1,int param_2,undefined8 param_3,int param_4,int param_5,undefined8 param_6,int param_7,longlong param_8,longlong param_9);
void FUN_180059570(int param_1,int param_2,longlong param_3,longlong param_4);
undefined8 FUN_180059900(longlong param_1,undefined8 param_2,undefined8 param_3);
char * FUN_180059960(void);
ulonglong FUN_180059970(undefined8 param_1,ulonglong *param_2);
undefined8 * FUN_1800599f0(longlong *param_1,void *param_2,undefined8 *param_3);
char * FUN_180059ba0(char *param_1,ulonglong param_2);
void FUN_180059c30(ulonglong *param_1,void *param_2,longlong param_3);
undefined4 * FUN_180059da0(undefined4 *param_1);
undefined8 * FUN_180059fe0(undefined8 *param_1);
undefined8 * FUN_18005a0f0(undefined8 *param_1);
void FUN_18005a240(undefined8 *param_1);
void FUN_18005a2c0(undefined8 *param_1);
void FUN_18005a340(longlong param_1);
undefined8 * FUN_18005a490(undefined8 *param_1,ulonglong param_2);
undefined8 * FUN_18005a4c0(undefined8 *param_1,ulonglong param_2);
undefined8 * FUN_18005a4f0(undefined8 *param_1,ulonglong param_2);
int FUN_18005a520(longlong param_1,undefined8 param_2,undefined4 param_3,int param_4,int param_5,int *param_6);
int FUN_18005ad60(longlong param_1,undefined8 param_2,undefined4 param_3,int param_4,undefined8 param_5);
int FUN_18005b370(longlong param_1,undefined8 param_2,char param_3,undefined4 *param_4);
int FUN_18005b6f0(longlong param_1,undefined8 param_2,int param_3,undefined8 param_4);
int FUN_18005ba70(undefined8 *param_1,longlong param_2,undefined8 param_3,longlong *param_4,longlong param_5,longlong param_6,int *param_7,longlong *param_8);
int FUN_18005c900(undefined8 *param_1,longlong param_2,longlong *param_3,longlong *param_4,undefined8 param_5,undefined8 param_6,longlong param_7,longlong param_8,longlong *param_9);
uint FUN_18005cb40(longlong *param_1,undefined8 *param_2,longlong param_3,ulonglong param_4,char *param_5,char param_6,undefined8 param_7,longlong *param_8);
int FUN_18005e340(undefined8 *param_1,undefined8 *param_2,char param_3,char param_4,char param_5,char param_6,undefined8 param_7,longlong *param_8);
uint FUN_18005e580(longlong *param_1,undefined8 *param_2,undefined1 param_3,longlong *param_4);
ulonglong FUN_18005e5e0(undefined8 *param_1,undefined8 param_2,undefined8 *param_3,longlong param_4,ulonglong *param_5,ulonglong param_6);
void FUN_18005e910(undefined8 *param_1,undefined8 param_2,undefined8 *param_3,longlong param_4,ulonglong *param_5,ulonglong param_6);
void FUN_18005e950(longlong param_1,longlong param_2);
ulonglong FUN_18005e9b0(longlong param_1,undefined8 param_2,int param_3,int param_4,int param_5,undefined4 param_6,longlong param_7,undefined8 param_8,undefined8 param_9,int *param_10,longlong *param_11);
void FUN_18005ebd0(ulonglong *param_1,ulonglong param_2);
void FUN_18005ec60(longlong *param_1,longlong param_2,longlong param_3,longlong param_4);
void FUN_18005ece0(longlong param_1);
void FUN_18005ed00(longlong param_1);
int FUN_18005ed80(longlong param_1,longlong param_2,longlong param_3,longlong param_4);
int FUN_180060770(longlong param_1,longlong param_2,longlong param_3,longlong *param_4,longlong *param_5);
int FUN_180060b30(longlong param_1,undefined8 param_2);
int FUN_180061290(longlong *param_1,uint param_2);
int FUN_180061a60(longlong param_1,undefined8 *param_2);
int FUN_180061be0(longlong param_1,double *param_2);
int FUN_1800621a0(longlong param_1,undefined8 *param_2,longlong param_3,longlong param_4,undefined1 *param_5);
int FUN_180062880(longlong param_1,longlong param_2,char param_3,undefined1 param_4,undefined8 param_5,undefined8 *param_6,undefined8 param_7,int *param_8,undefined1 *param_9,longlong param_10);
bool FUN_180062dd0(undefined8 *param_1,longlong param_2);
bool FUN_180062f10(longlong param_1);
int FUN_180063100(longlong param_1,undefined8 param_2);
ulonglong FUN_180063310(longlong param_1,longlong param_2,longlong *param_3,ulonglong param_4,undefined8 param_5,undefined8 param_6,char param_7,char param_8,char param_9,char param_10,undefined8 *param_11,undefined8 *param_12,int *param_13,undefined8 param_14,longlong param_15);
int FUN_180063ec0(undefined8 *param_1,undefined8 *param_2,longlong *param_3,undefined8 *param_4,longlong *param_5,double *param_6,longlong param_7,longlong *param_8,undefined8 *param_9);
undefined8 FUN_1800641a0(undefined8 param_1,longlong param_2);
undefined1 (*) [16] FUN_1800641e0(undefined1 (*param_1) [16],uint param_2);
undefined1 (*) [16] FUN_180064290(undefined1 (*param_1) [16],ulonglong param_2);
void FUN_1800642f0(longlong param_1,undefined8 *param_2,longlong param_3,longlong param_4);
int FUN_180064460(undefined8 *param_1,longlong param_2,longlong param_3,longlong *param_4,undefined8 param_5,longlong *param_6);
int FUN_180064c50(longlong param_1,longlong param_2,int *param_3);
int FUN_180064d50(longlong param_1,longlong param_2,longlong param_3,undefined1 *param_4);
uint FUN_180064f70(longlong *param_1,undefined8 *param_2,longlong *param_3,longlong param_4,longlong *param_5,longlong *param_6,char param_7,longlong param_8,longlong *param_9);
uint FUN_180065700(longlong *param_1,undefined8 *param_2,longlong *param_3,longlong param_4);
ulonglong FUN_1800657b0(undefined8 *param_1,longlong param_2,undefined4 param_3,int param_4,longlong *param_5);
undefined8 FUN_180065870(int *param_1,undefined4 param_2,undefined8 param_3);
undefined8 * FUN_180065910(undefined8 *param_1,undefined8 param_2);
void FUN_180065990(undefined8 *param_1,void *param_2,ulonglong param_3);
void FUN_180065b40(ulonglong *param_1,void *param_2,ulonglong param_3);
undefined8 * FUN_180065ce0(longlong *param_1,void *param_2,undefined8 *param_3);
undefined4 * FUN_180065ec0(longlong *param_1,undefined4 *param_2,undefined4 *param_3);
void FUN_1800660f0(longlong *param_1,ulonglong param_2);
undefined4 * FUN_1800662f0(undefined4 *param_1,undefined4 *param_2,undefined4 *param_3);
undefined8 * FUN_180066360(undefined8 *param_1,longlong param_2);
undefined8 * FUN_1800663b0(undefined8 *param_1);
undefined8 * FUN_180066440(undefined8 *param_1,longlong *param_2);
undefined4 * FUN_180066580(undefined4 *param_1,undefined4 *param_2);
undefined4 * FUN_180066820(undefined4 *param_1,undefined4 *param_2);
undefined8 * FUN_180066ad0(undefined8 *param_1);
void FUN_180066be0(undefined8 *param_1);
void FUN_180066c20(longlong *param_1);
void FUN_180066c70(undefined8 *param_1);
void FUN_180066cf0(longlong *param_1);
void FUN_180066d60(longlong param_1);
void FUN_180066da0(longlong param_1);
undefined8 * FUN_180066f80(undefined8 *param_1,ulonglong param_2);
void FUN_180066fb0(ulonglong *param_1,ulonglong param_2);
void FUN_180067040(longlong *param_1,longlong param_2,longlong param_3,longlong param_4);
void FUN_1800670e0(longlong param_1);
void FUN_180067110(longlong param_1);
void FUN_180067140(longlong *param_1);
undefined4 * FUN_1800671d0(undefined4 *param_1,undefined4 *param_2);
ulonglong FUN_180067700(ulonglong param_1,longlong param_2,longlong param_3,longlong param_4,longlong param_5,longlong *param_6,longlong *param_7,longlong *param_8,longlong *param_9);
int FUN_180067a10(longlong *param_1,undefined8 *param_2,longlong *param_3,longlong param_4,undefined8 param_5,undefined8 *param_6,longlong *param_7,int param_8,undefined8 *param_9,undefined8 param_10);
int FUN_180068650(undefined8 *param_1,undefined4 param_2,undefined4 *param_3,undefined4 *param_4);
ulonglong FUN_180068c70(longlong param_1,longlong param_2,char param_3,longlong param_4,longlong param_5);
int FUN_180068db0(undefined8 *param_1,undefined8 param_2,undefined1 (*param_3) [16],undefined1 (*param_4) [16],undefined8 *param_5);
undefined4 FUN_180069cc0(longlong param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4,undefined4 *param_5,undefined4 param_6,undefined8 param_7,undefined8 param_8,undefined8 param_9,undefined8 param_10,undefined8 param_11);
undefined4 FUN_18006a210(undefined8 *param_1,undefined8 *param_2,undefined8 param_3,undefined4 *param_4);
undefined8 FUN_18006a4f0(longlong param_1,longlong param_2,undefined8 param_3);
longlong * FUN_18006a5f0(longlong *param_1,longlong param_2,uint *param_3);
undefined4 FUN_18006a750(longlong *param_1,undefined8 *param_2,longlong *param_3,longlong *param_4,longlong *param_5,char param_6,int param_7,undefined1 (*param_8) [32],longlong param_9,longlong *param_10);
void FUN_18006b370(longlong *param_1,undefined8 *param_2);
void FUN_18006b3e0(longlong param_1,longlong param_2);
void FUN_18006b650(longlong *param_1,ulonglong param_2);
bool FUN_18006b710(int param_1);
undefined8 FUN_18006b720(int param_1);
uint * FUN_18006b750(uint *param_1,undefined8 *param_2,undefined8 *param_3,ulonglong *param_4);
longlong FUN_18006b8c0(ulonglong *param_1,longlong param_2);
void * FUN_18006b990(ulonglong *param_1,void *param_2,longlong param_3,longlong param_4);
ulonglong FUN_18006ba60(longlong param_1,longlong param_2,undefined8 param_3,undefined8 param_4,undefined8 *param_5);
ulonglong FUN_18006bef0(longlong param_1,longlong param_2,undefined8 param_3,undefined8 param_4,char param_5,undefined8 *param_6);
ulonglong FUN_18006c260(longlong param_1,longlong param_2,undefined8 param_3,undefined8 param_4,char param_5,undefined8 *param_6);
undefined8 FUN_18006c550(longlong param_1,longlong param_2,undefined8 param_3,ulonglong *param_4);
undefined8 FUN_18006cc60(longlong param_1,longlong param_2);
ulonglong FUN_18006cc80(longlong param_1,longlong param_2);
undefined8 FUN_18006cdc0(longlong param_1,longlong param_2);
ulonglong FUN_18006d2b0(longlong param_1,longlong param_2);
ulonglong FUN_18006d6c0(longlong param_1,longlong param_2);
undefined8 * FUN_18006d940(longlong *param_1,undefined8 *param_2,undefined8 *param_3);
longlong * FUN_18006db20(longlong param_1,longlong *param_2,undefined8 *param_3,ulonglong param_4);
void FUN_18006dbf0(undefined8 param_1,undefined8 *param_2);
void FUN_18006dc60(void **param_1,void **param_2,void **param_3);
void FUN_18006de30(longlong param_1,longlong param_2);
void FUN_18006e0d0(ulonglong *param_1,undefined1 (*param_2) [32],undefined1 (*param_3) [32],undefined1 (*param_4) [32]);
void FUN_18006ea60(longlong param_1,longlong param_2,ulonglong param_3,longlong *param_4);
void FUN_18006ebc0(undefined1 (*param_1) [32],undefined1 (*param_2) [32],longlong param_3,undefined1 param_4);
undefined8 * FUN_18006f000(float *param_1,undefined8 *param_2,byte *param_3);
longlong * FUN_18006f290(float *param_1,longlong *param_2,undefined8 *param_3);
longlong * FUN_18006f520(float *param_1,longlong *param_2,undefined8 *param_3);
undefined1 (*) [16]FUN_18006f7d0(undefined8 *param_1,undefined8 *param_2,undefined1 (*param_3) [16]);
void FUN_18006f830(undefined8 *param_1,undefined8 *param_2,undefined8 *param_3);
void FUN_18006f8e0(void **param_1,void **param_2);
ulonglong * FUN_18006fa30(ulonglong *param_1,undefined8 *param_2);
void FUN_18006fb00(undefined8 *param_1);
void FUN_18006fb70(longlong param_1);
void FUN_18006fbb0(longlong param_1);
void FUN_18006fc20(longlong *param_1);
void FUN_18006fc90(undefined8 *param_1);
void FUN_18006fcc0(longlong param_1);
void FUN_18006fce0(longlong param_1);
void FUN_18006fd20(longlong *param_1);
void FUN_18006fd80(longlong *param_1);
void FUN_18006fe40(longlong param_1);
ulonglong * FUN_18006fe70(ulonglong *param_1,ulonglong *param_2);
void FUN_180070000(ulonglong *param_1,ulonglong param_2,undefined8 param_3);
void FUN_180070150(longlong *param_1,longlong param_2,longlong param_3,longlong param_4);
void FUN_180070240(longlong param_1,ulonglong param_2);
void FUN_180070410(longlong param_1,ulonglong param_2);
void FUN_1800706b0(float *param_1);
void FUN_180070760(void);
void FUN_180070780(longlong param_1,undefined4 param_2);
undefined1 (*) [16] FUN_180070800(undefined1 (*param_1) [16],uint *param_2);
float * FUN_180070a30(float *param_1,uint *param_2);
float * FUN_180071e40(float *param_1,uint *param_2);
undefined1 (*) [32] FUN_180072480(undefined1 (*param_1) [32],longlong param_2);
ulonglong FUN_180072aa0(undefined8 param_1,undefined8 param_2,longlong param_3,char *param_4,undefined1 param_5);
undefined4 FUN_180073510(longlong param_1,undefined8 *param_2);
double FUN_180073610(double *param_1);
undefined1 (*) [16] FUN_180073690(undefined1 (*param_1) [16],char *param_2);
undefined8 FUN_1800739c0(char *param_1);
char FUN_180073bd0(undefined1 (*param_1) [32]);
ulonglong FUN_180073e00(longlong param_1,uint *param_2,longlong param_3);
ulonglong FUN_180075aa0(longlong param_1,uint *param_2,longlong param_3);
int FUN_180076810(longlong param_1,uint param_2,ulonglong param_3);
ulonglong FUN_1800782e0(undefined1 *param_1,ulonglong param_2,uint *param_3,ulonglong param_4,char param_5);
ulonglong FUN_1800795b0(undefined1 *param_1,uint param_2,undefined1 (*param_3) [16],ulonglong param_4);
int FUN_18007aa40(longlong param_1,uint *param_2,longlong param_3,longlong param_4);
ulonglong *FUN_18007c720(ulonglong *param_1,uint *param_2,int param_3,undefined8 *param_4,longlong param_5,double *param_6,double *param_7,longlong *param_8,double *param_9);
longlong FUN_18007d730(longlong *param_1);
undefined8 * FUN_18007d760(undefined8 *param_1);
int FUN_18007d860(longlong param_1,longlong param_2,longlong param_3,undefined8 param_4,longlong *param_5,undefined4 param_6,longlong *param_7,longlong param_8,undefined4 param_9,char *param_10,char param_11,longlong *param_12);
int FUN_18007db90(longlong param_1,longlong param_2,undefined8 param_3,uint param_4,longlong *param_5,longlong param_6,uint param_7,char *param_8,undefined8 *param_9);
int FUN_18007e060(longlong *param_1,undefined8 *param_2);
void FUN_18007e320(longlong *param_1);
undefined4 FUN_18007e440(longlong param_1,longlong param_2,longlong param_3,longlong param_4,longlong *param_5,undefined8 *param_6);
int FUN_18007eb60(longlong param_1,undefined8 *param_2,longlong param_3,longlong *param_4,longlong param_5,undefined4 param_6,char *param_7);
int FUN_18007ef50(longlong param_1,longlong param_2,longlong param_3);
int FUN_18007f2c0(longlong param_1,longlong param_2,char *param_3);
int FUN_18007f4d0(longlong param_1,undefined8 *param_2,longlong param_3,longlong param_4,longlong param_5,char *param_6,undefined8 *param_7);
int snprintf(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
ulonglong FUN_18007f6c0(longlong param_1,undefined8 param_2,__int64 *param_3);
undefined8 * FUN_18007f8b0(longlong *param_1,undefined8 *param_2,undefined8 *param_3);
undefined8 * FUN_18007fac0(longlong *param_1,undefined8 *param_2,undefined4 *param_3);
undefined8 * FUN_18007fc30(undefined8 *param_1,undefined8 *param_2);
void FUN_18007fda0(longlong *param_1);
void FUN_18007fdd0(longlong *param_1);
void FUN_18007fe30(undefined8 *param_1);
void FUN_18007ff10(undefined8 *param_1);
void FUN_18007ff50(longlong param_1);
ulonglong * FUN_18007ff90(ulonglong *param_1,ulonglong *param_2);
undefined8 * FUN_180080120(undefined8 *param_1,ulonglong param_2);
undefined8 FUN_180080150(longlong param_1,undefined8 *param_2,int *param_3,undefined4 *param_4);
undefined8 FUN_180080340(longlong param_1,longlong *param_2);
ulonglong FUN_1800805e0(longlong param_1,longlong *param_2,undefined8 *param_3,undefined8 **param_4,undefined8 *param_5);
void FUN_180081800(longlong *param_1,longlong param_2,longlong param_3,longlong param_4);
void FUN_180081880(longlong *param_1,longlong param_2,longlong param_3,longlong param_4);
undefined8 * FUN_180081990(longlong param_1,undefined8 *param_2);
void FUN_1800819b0(void *param_1,char param_2);
void FUN_1800819c0(longlong param_1);
undefined8 * FUN_1800819d0(longlong param_1,undefined8 *param_2,int *param_3);
longlong FUN_180081a30(longlong param_1);
TypeDescriptor * FUN_180081a40(void);
undefined8 FUN_180081a50(longlong param_1,longlong *param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,ulonglong param_6);
longlong * FUN_180081e30(longlong param_1,longlong *param_2,undefined8 *param_3);
undefined8 FUN_180082390(longlong param_1,undefined8 *param_2,longlong param_3,longlong param_4,ulonglong param_5);
undefined4 *FUN_180082fe0(longlong param_1,undefined4 *param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,undefined8 param_6,undefined8 param_7,undefined8 param_8,undefined8 param_9,undefined8 param_10,undefined4 param_11);
undefined4 FUN_180083230(undefined8 param_1,longlong param_2,longlong param_3,longlong *param_4,undefined8 *param_5);
undefined8 FUN_180083560(longlong param_1);
void FUN_180083750(longlong *param_1,undefined8 *param_2);
void FUN_1800838c0(undefined8 param_1,undefined8 *param_2);
ulonglong FUN_1800839a0(double *param_1,longlong *param_2);
void FUN_1800840b0(undefined8 *param_1,uint param_2);
void FUN_180084240(longlong *param_1,longlong param_2);
void FUN_1800842f0(longlong *param_1,longlong param_2);
void FUN_1800843a0(longlong param_1,longlong *param_2,longlong param_3);
int FUN_180084560(longlong param_1,longlong *param_2,undefined8 param_3);
void FUN_1800845e0(longlong param_1,undefined8 *param_2,longlong param_3);
undefined8 * FUN_180084ae0(undefined8 *param_1,longlong *param_2);
void FUN_180084cc0(longlong *param_1,longlong *param_2,longlong param_3);
void FUN_180084f80(longlong *param_1,longlong *param_2,longlong param_3);
void FUN_180085440(longlong param_1);
void FUN_180085470(longlong *param_1);
undefined8 FUN_1800854b0(longlong param_1,byte param_2);
void FUN_180085510(longlong *param_1,ulonglong *param_2,ulonglong param_3,longlong param_4,int *param_5,int *param_6);
undefined8 FUN_1800858a0(longlong param_1,undefined8 param_2);
undefined8 FUN_180085900(longlong param_1,uint param_2);
undefined8 FUN_180085980(longlong param_1,uint param_2);
void FUN_180085a00(ulonglong param_1,longlong param_2,int *param_3,int *param_4);
undefined8 FUN_180085be0(longlong param_1,ulonglong param_2);
undefined8 FUN_180085c90(longlong param_1,int param_2);
undefined8 FUN_180085d00(longlong param_1);
void FUN_180085d50(undefined8 *param_1);
char * FUN_180085de0(undefined2 *param_1,undefined8 param_2,int param_3,int param_4);
void FUN_180085fa0(longlong *param_1,undefined1 param_2);
void FUN_180085ff0(undefined4 *param_1,undefined8 *param_2,ulonglong *param_3);
undefined8 FUN_180086080(longlong *param_1,byte *param_2,uint param_3);
double FUN_1800861e0(double param_1,int param_2);
undefined8 FUN_180086250(longlong param_1,ulonglong param_2);
undefined8 FUN_1800862f0(longlong param_1,uint param_2);
char * FUN_180086360(int param_1,char *param_2);
void FUN_1800863f0(uint *param_1,char *param_2,char *param_3);
undefined8 FUN_180086580(longlong param_1,undefined8 *param_2);
undefined1 (*) [16] FUN_180086680(undefined1 (*param_1) [16],double *param_2);
char * FUN_1800867f0(uint param_1,char *param_2);
char * FUN_180086b00(ulonglong param_1,char *param_2);
undefined8 FUN_1800873f0(undefined8 *param_1);
undefined8 * FUN_180087420(undefined8 *param_1);
void FUN_1800874e0(undefined8 *param_1);
void FUN_180087570(longlong param_1);
longlong * FUN_1800875b0(longlong *param_1,longlong param_2);
ulonglong * FUN_180087720(ulonglong *param_1,undefined8 *param_2);
undefined8 * FUN_1800878a0(undefined8 *param_1,ulonglong param_2);
undefined8 * FUN_1800878d0(undefined8 *param_1,ulonglong param_2);
undefined8 * FUN_180087900(undefined8 *param_1,ulonglong param_2);
undefined8 * FUN_180087930(undefined8 *param_1,ulonglong param_2);
undefined8 * FUN_180087960(undefined8 *param_1,ulonglong param_2);
void FUN_180087990(longlong param_1,longlong param_2,longlong param_3,double param_4,undefined4 param_5,longlong param_6);
void FUN_180087a30(longlong param_1);
void FUN_180087a40(longlong param_1);
void FUN_180087b00(longlong *param_1);
void FUN_180087be0(undefined8 *param_1);
void FUN_180087c70(void);
void FUN_180087c90(longlong param_1);
int FUN_180088040(longlong param_1,undefined8 param_2,longlong param_3,longlong param_4);
int FUN_180088200(longlong param_1,undefined8 param_2,longlong param_3,longlong param_4,longlong param_5,longlong *param_6);
int FUN_180088410(undefined8 *param_1,longlong param_2,longlong param_3,char param_4,longlong *param_5);
ulonglong FUN_180088720(int *param_1,longlong param_2,longlong param_3);
void FUN_1800890b0(longlong param_1,undefined4 *param_2);
float * FUN_180089160(float *param_1,longlong param_2);
int FUN_1800892a0(undefined8 *param_1,undefined8 *param_2,undefined8 param_3,undefined8 *param_4,byte param_5,undefined1 (*param_6) [16],ulonglong param_7);
ulonglong FUN_180089510(uint *param_1,char param_2,ulonglong param_3);
ulonglong FUN_180089ad0(undefined8 *param_1,undefined8 param_2,undefined8 param_3,longlong param_4,uint *param_5,int *param_6);
void FUN_180089ed0(undefined8 *param_1,undefined8 param_2,longlong param_3,undefined4 *param_4);
void FUN_180089f30(longlong param_1,undefined8 param_2,longlong param_3,undefined4 *param_4);
void FUN_18008a430(longlong param_1,undefined8 param_2,longlong param_3,undefined4 *param_4);
undefined4 *FUN_18008a990(undefined4 *param_1,int param_2,longlong param_3,undefined8 param_4,undefined8 param_5,undefined8 param_6,undefined8 *param_7);
ulonglong FUN_18008ab60(undefined8 *param_1,undefined8 param_2,longlong param_3,longlong param_4,longlong param_5);
void FUN_18008ade0(longlong *param_1);
int FUN_18008c210(longlong param_1,undefined8 param_2,longlong param_3,longlong param_4,longlong param_5);
undefined4 FUN_18008c6d0(undefined8 *param_1);
void FUN_18008c800(FILE *param_1,long param_2);
undefined4 FUN_18008c840(FILE *param_1);
void __cdecl free(void *_Memory);
void * FUN_18008c890(longlong *param_1,undefined4 *param_2,undefined4 *param_3,undefined4 *param_4,uint param_5);
void FUN_18008d1c0(longlong param_1,int param_2,uint *param_3,uint *param_4,uint *param_5,uint param_6);
undefined8 FUN_18008d260(undefined8 param_1,undefined8 param_2);
void FUN_18008d270(undefined1 *param_1,longlong param_2,longlong param_3,longlong param_4,int param_5,int param_6);
void FUN_18008d380(undefined1 (*param_1) [16],longlong param_2,longlong param_3,longlong param_4,int param_5,int param_6);
bool FUN_18008d600(int param_1,int param_2);
void * FUN_18008d650(uint *param_1,uint *param_2,uint *param_3,uint *param_4,uint param_5);
undefined8 FUN_18008e780(int *param_1,int *param_2);
undefined8 FUN_18008eba0(int *param_1,int param_2);
undefined8 FUN_18008ec10(void *param_1,longlong param_2);
undefined8 FUN_18008edf0(longlong param_1);
bool FUN_18008eeb0(ulonglong *param_1);
void * FUN_18008f1c0(void *param_1,int param_2,int param_3,int param_4,int param_5);
void * FUN_18008f6b0(void *param_1,int param_2,int param_3,uint param_4,uint param_5);
undefined8 FUN_18008fbb0(longlong *param_1,byte *param_2,uint param_3,uint param_4,uint param_5,int param_6,int param_7);
undefined8 FUN_18008ffe0(longlong *param_1,byte *param_2,uint param_3,uint param_4,uint param_5,uint param_6,uint param_7,int param_8);
bool FUN_1800910f0(longlong *param_1,int param_2);
undefined8 FUN_180091260(longlong *param_1);
void FUN_1800914e0(undefined8 *param_1);
undefined4 FUN_180091540(longlong param_1,int param_2,undefined4 param_3);
undefined2 FUN_180091600(longlong param_1);
undefined2 FUN_1800916a0(longlong param_1);
int FUN_180091740(longlong param_1);
int FUN_180091770(longlong param_1);
ulonglong FUN_1800917a0(longlong *param_1);
bool FUN_180091870(longlong param_1,void *param_2,int param_3);
undefined8 FUN_180091930(longlong param_1,int *param_2,undefined4 *param_3,int param_4);
void * FUN_180091cf0(void *param_1,uint *param_2,uint *param_3,undefined4 *param_4,uint param_5);
longlong FUN_180091df0(longlong param_1,int *param_2,undefined4 *param_3,undefined8 param_4,longlong param_5);
void FUN_180092530(longlong param_1,longlong param_2,int param_3,int param_4);
undefined8 FUN_180092680(longlong param_1);
void FUN_180092830(longlong *param_1);
void FUN_180092980(float *param_1,byte *param_2,int param_3);
longlong FUN_180092ab0(longlong param_1,longlong param_2);
float * FUN_180092c00(longlong param_1,uint *param_2,uint *param_3,undefined4 *param_4,uint param_5);
void FUN_180093430(longlong param_1);
undefined8 FUN_1800934a0(longlong param_1,char *param_2);
uint FUN_180093560(uint param_1);
void FUN_1800935f0(undefined1 *param_1,int param_2,short *param_3);
void FUN_180093cb0(undefined8 *param_1,int param_2,undefined1 (*param_3) [16]);
undefined8 FUN_180094410(longlong *param_1,short *param_2,longlong param_3,longlong param_4,longlong param_5,int param_6,short *param_7);
undefined8 FUN_180094960(longlong *param_1,longlong param_2,longlong param_3,longlong param_4);
undefined8 FUN_180095080(longlong *param_1,short *param_2,longlong param_3,int param_4);
void FUN_1800953b0(longlong *param_1);
undefined1 * FUN_1800955a0(uint *param_1,uint *param_2,uint *param_3,uint *param_4,uint param_5);
undefined1 *FUN_180095800(uint *param_1,uint *param_2,uint *param_3,uint *param_4,uint param_5,int *param_6,int param_7);
undefined8 FUN_1800960f0(int param_1,uint param_2,uint param_3,uint param_4,int param_5);
void * FUN_180096170(int param_1,uint param_2,uint param_3,uint param_4,int param_5);
bool FUN_1800961d0(undefined8 param_1,short param_2);
void FUN_180096230(longlong param_1,ushort param_2);
undefined8 FUN_180096360(longlong *param_1);
undefined8 FUN_180096ba0(ulonglong *param_1);
undefined8 FUN_180096e50(longlong *param_1,int param_2,uint param_3);
undefined8 FUN_180097df0(longlong *param_1);
undefined8 FUN_180097f70(ulonglong *param_1,int param_2);
undefined8 FUN_180098150(longlong param_1,longlong param_2);
void * FUN_1800981f0(longlong param_1,uint *param_2,uint *param_3,int *param_4,int param_5);
longlong FUN_180098470(longlong param_1,uint param_2,int param_3,int *param_4,longlong param_5);
int FUN_180098a50(longlong param_1,char *param_2);
undefined8 FUN_180098b60(longlong param_1,int *param_2,int *param_3,int *param_4);
void * FUN_180098da0(uint *param_1,uint *param_2,uint *param_3,uint *param_4,uint param_5,int *param_6);
void FUN_180098fc0(longlong param_1,char *param_2);
undefined4 FUN_180099140(undefined8 *param_1,int param_2);
undefined8 FUN_180099920(longlong param_1,longlong param_2);
bool FUN_180099dc0(longlong *param_1,int param_2);
undefined8 FUN_18009a820(longlong *param_1);
undefined8 FUN_18009ac20(longlong param_1,byte *param_2,uint param_3);
void * FUN_18009ada0(longlong param_1,uint *param_2,uint *param_3,undefined4 *param_4,uint param_5,int *param_6,int param_7);
longlong FUN_18009ba20(longlong param_1,uint param_2,longlong param_3);
void FUN_18009bb10(longlong param_1);
longlong FUN_18009bb90(longlong param_1,longlong param_2,undefined8 param_3,int param_4,int param_5);
byte * FUN_18009bbf0(byte *param_1,byte *param_2,undefined8 param_3,int param_4);
undefined1 * FUN_18009bcc0(undefined1 *param_1,byte *param_2,byte *param_3,int param_4);
undefined1 * FUN_18009bd70(undefined1 *param_1,byte *param_2,byte *param_3,int param_4);
ulonglong FUN_18009bf30(ulonglong param_1,ulonglong param_2,ulonglong param_3,uint param_4);
undefined * FUN_18009c0c0(longlong param_1);
void FUN_18009c140(longlong param_1,int param_2);
undefined1 * FUN_18009c1a0(longlong param_1,uint *param_2,uint *param_3,uint *param_4,uint param_5);
void FUN_18009ca80(longlong param_1,undefined1 *param_2);
undefined8 FUN_18009cb10(longlong param_1);
void FUN_18009cdb0(longlong param_1,int param_2,int param_3,int param_4);
undefined8 FUN_18009cee0(void *param_1,longlong param_2,int param_3);
undefined8 FUN_18009d180(longlong param_1,undefined8 param_2,uint param_3);
ulonglong FUN_18009d2a0(longlong param_1,longlong param_2);
void FUN_18009d3a0(longlong *param_1,ulonglong param_2,byte *param_3);
ulonglong FUN_18009d530(ulonglong param_1,longlong param_2,longlong *param_3,uint param_4,uint param_5,int param_6,uint param_7,uint param_8,longlong param_9);
undefined8 FUN_18009e280(undefined4 param_1);
undefined4 FUN_18009e2e0(longlong param_1,longlong param_2,longlong *param_3);
ulonglong FUN_18009e9c0(undefined8 *param_1,longlong param_2,longlong param_3,ulonglong param_4,undefined8 *param_5,char param_6,undefined4 param_7);
ulonglong FUN_18009ef90(longlong param_1,longlong param_2,longlong *param_3);
undefined8 FUN_18009f580(longlong *param_1);
ulonglong FUN_18009f810(longlong param_1,longlong param_2,char param_3,longlong *param_4,longlong param_5);
ulonglong FUN_18009f840(longlong param_1,longlong param_2,char param_3,longlong *param_4,longlong *param_5,longlong param_6);
ulonglong FUN_18009f940(longlong param_1,longlong param_2,int *param_3,longlong *param_4,longlong *param_5,undefined8 *param_6);
ulonglong FUN_18009fc70(int param_1,longlong param_2,longlong param_3,int param_4,int param_5,int param_6,int param_7,int param_8,int *param_9,undefined8 *param_10);
ulonglong FUN_1800a0440(ulonglong param_1,longlong param_2,longlong *param_3,uint param_4,uint param_5,int param_6,uint param_7,uint param_8,longlong param_9);
int FUN_1800a0820(longlong param_1,longlong param_2,longlong *param_3,uint param_4,uint param_5);
int FUN_1800a1600(longlong param_1,longlong param_2,longlong *param_3,uint param_4,uint param_5);
undefined8 FUN_1800a23e0(longlong param_1,longlong param_2,undefined8 param_3,undefined4 param_4,undefined8 *param_5);
uint FUN_1800a2560(longlong param_1,longlong param_2,longlong *param_3,longlong param_4,int param_5,undefined1 param_6);
uint FUN_1800a29d0(longlong param_1,longlong param_2,longlong *param_3,longlong param_4,longlong *param_5,undefined1 param_6);
int FUN_1800a2a80(longlong param_1,longlong param_2,longlong param_3,longlong *param_4);
undefined8 * FUN_1800a2c50(undefined8 *param_1,longlong param_2,longlong param_3);
int FUN_1800a2d00(longlong param_1,longlong param_2,longlong param_3,undefined4 *param_4);
undefined8 * FUN_1800a2f10(undefined8 *param_1,longlong param_2,longlong param_3);
bool FUN_1800a2fc0(longlong *param_1);
undefined8 FUN_1800a3000(undefined4 param_1);
undefined4 FUN_1800a3060(int param_1,longlong param_2,void *param_3,longlong param_4,longlong param_5,longlong param_6,undefined8 *param_7);
undefined8 FUN_1800a3360(int param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4,longlong param_5);
undefined4 FUN_1800a34d0(int param_1,void *param_2,undefined8 param_3,size_t param_4,undefined8 *param_5);
void cuCtxSynchronize(void);
void FUN_1800a37d0(longlong *param_1);
undefined8 *FUN_1800a38c0(undefined8 *param_1,undefined8 param_2,longlong *param_3,undefined4 param_4,undefined4 param_5,char param_6,undefined1 param_7);
void FUN_1800a3a90(longlong *param_1,char *param_2,u_short param_3,undefined8 param_4);
void FUN_1800a3d90(longlong *param_1,char *param_2,u_short param_3,undefined8 param_4);
void FUN_1800a3df0(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void * FUN_1800a3e50(longlong *param_1,void *param_2,undefined8 *param_3);
void FUN_1800a4060(longlong *param_1,ulonglong param_2,int *param_3);
void FUN_1800a4200(longlong *param_1,ulonglong param_2,undefined8 *param_3);
void FUN_1800a43a0(longlong *param_1,ulonglong param_2,undefined8 *param_3);
int * FUN_1800a4540(int *param_1,longlong param_2,int *param_3);
int * FUN_1800a45a0(int *param_1,ulonglong param_2,int *param_3);
int FUN_1800a4630(undefined8 *param_1,undefined8 *param_2,undefined8 *param_3,undefined1 param_4,longlong param_5);
ulonglong FUN_1800a5e60(undefined8 *param_1,undefined8 *param_2,undefined8 *param_3,undefined8 param_4,undefined8 param_5,longlong param_6);
ulonglong FUN_1800a7340(undefined8 *param_1,undefined8 *param_2,undefined8 *param_3,undefined8 param_4,undefined8 param_5,longlong param_6);
void FUN_1800a8a10(undefined8 *param_1,undefined8 param_2,longlong param_3,longlong param_4);
void FUN_1800a97a0(longlong *param_1,undefined8 param_2,longlong param_3,longlong param_4);
void FUN_1800aa480(longlong param_1,undefined8 param_2,longlong param_3,undefined4 *param_4);
void FUN_1800aac50(longlong *param_1,undefined4 *param_2);
void FUN_1800aadc0(longlong *param_1,ulonglong param_2,int *param_3);
undefined8 * FUN_1800aaf40(undefined8 *param_1,undefined8 *param_2,float *param_3);
undefined8 * FUN_1800ab270(longlong *param_1,void *param_2,undefined8 *param_3);
int FUN_1800ab4a0(longlong param_1,undefined8 param_2,float *param_3,undefined4 *param_4,undefined4 *param_5);
void FUN_1800ac290(longlong param_1,undefined8 *param_2,undefined8 param_3,undefined8 param_4);
int FUN_1800ac480(longlong param_1,undefined8 param_2,undefined8 param_3,char param_4,longlong *param_5);
undefined8 *FUN_1800ad440(undefined8 *param_1,float param_2,float param_3,float param_4,float param_5);
void FUN_1800ad6f0(longlong param_1,longlong param_2,void *param_3,undefined8 param_4,longlong param_5);
int FUN_1800ad790(longlong param_1,longlong param_2,void *param_3,undefined8 param_4,longlong param_5);
undefined8 FUN_1800adb20(longlong param_1,int param_2,int param_3,undefined4 *param_4,undefined4 *param_5);
undefined8 FUN_1800adc30(longlong param_1,int param_2,int param_3,undefined1 *param_4,undefined1 *param_5);
undefined8 FUN_1800add40(longlong param_1,int param_2,int param_3,undefined2 *param_4,undefined2 *param_5);
undefined8 * FUN_1800ade60(longlong *param_1,undefined8 *param_2,char *param_3,longlong *param_4);
void FUN_1800ae050(undefined8 param_1,undefined8 param_2,longlong *param_3);
longlong * FUN_1800ae0d0(longlong *param_1,longlong *param_2,undefined8 *param_3);
longlong * FUN_1800ae1b0(longlong *param_1,longlong *param_2,undefined8 *param_3,longlong *param_4);
longlong * FUN_1800ae2a0(longlong *param_1);
void FUN_1800ae350(longlong param_1);
void FUN_1800ae390(longlong *param_1);
void FUN_1800ae400(longlong *param_1);
void FUN_1800ae460(longlong *param_1);
longlong FUN_1800ae4e0(longlong *param_1,longlong *param_2,longlong param_3,longlong param_4);
longlong * FUN_1800ae5d0(longlong param_1,longlong *param_2,longlong *param_3);
undefined8 * FUN_1800ae6e0(undefined8 *param_1);
undefined8 * FUN_1800ae760(void);
void FUN_1800aed20(void);
longlong * FUN_1800aed40(longlong *param_1,longlong param_2);
void FUN_1800aed80(longlong *param_1);
void FUN_1800aed90(longlong *param_1);
void FUN_1800afd00(int *param_1,int *param_2);
undefined8 FUN_1800afd80(longlong param_1,longlong param_2,longlong *param_3);
undefined8 FUN_1800aff10(longlong param_1,longlong param_2,undefined8 param_3,undefined4 param_4,char *param_5,longlong *param_6);
undefined8 FUN_1800affa0(longlong param_1,longlong param_2);
undefined8 FUN_1800b00d0(longlong param_1,longlong param_2,undefined8 param_3,undefined4 param_4,char *param_5);
longlong * FUN_1800b0130(longlong *param_1);
void FUN_1800b0160(longlong *param_1);
void FUN_1800b0200(longlong param_1);
ulonglong FUN_1800b02a0(longlong *param_1,longlong *param_2,longlong *param_3);
void cuCtxSynchronize(void);
void FUN_1800b0490(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_cuCtxSynchronize(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void cuMemcpyHtoD_v2(void);
void DelayLoad_cuMemcpyHtoD_v2(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void cuMemcpyDtoH_v2(void);
void DelayLoad_cuMemcpyDtoH_v2(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void cuMemcpyDtoD_v2(void);
void DelayLoad_cuMemcpyDtoD_v2(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void FUN_1800b054c(void);
bool FUN_1800b0608(void);
void FUN_1800b06a8(LPCVOID param_1,ulonglong param_2);
longlong FUN_1800b0740(dword *param_1,SectionFlags *param_2);
void FUN_1800b07d4(LPCVOID param_1,SIZE_T param_2,DWORD param_3,PDWORD param_4);
void FUN_1800b08b8(void);
FARPROC FUN_1800b0964(uint *param_1,longlong *param_2);
undefined4 * FUN_1800b0cf4(undefined4 *param_1);
void FUN_1800b0d9c(longlong param_1);
void __cdecl std::_Xbad_alloc(void);
void __cdecl std::_Xlength_error(char *param_1);
void _Mtx_init_in_situ(void);
void _Xtime_get_ticks(void);
ulonglong FUN_1800b0e00(undefined1 (*param_1) [16],ulonglong param_2,undefined1 (*param_3) [16],ulonglong param_4);
undefined1 (*) [32]FUN_1800b0f70(undefined1 (*param_1) [32],undefined1 (*param_2) [32],undefined1 (*param_3) [32],ulonglong param_4);
undefined1 (*) [32]FUN_1800b1310(undefined1 (*param_1) [32],undefined1 (*param_2) [32],undefined1 (*param_3) [32],ulonglong param_4);
undefined1 (*) [32]FUN_1800b16d0(undefined1 (*param_1) [32],undefined1 (*param_2) [32],byte param_3);
undefined1 (*) [32]FUN_1800b17d0(undefined1 (*param_1) [32],undefined1 (*param_2) [32],byte param_3);
undefined1 *FUN_1800b18f0(undefined1 (*param_1) [16],ulonglong param_2,undefined1 (*param_3) [16],ulonglong param_4);
longlong FUN_1800b1b10(longlong param_1,ulonglong param_2,byte *param_3,ulonglong param_4);
ulonglong FUN_1800b1d40(void *param_1,ulonglong param_2,byte *param_3,ulonglong param_4);
undefined1 (*) [16]FUN_1800b1f70(undefined1 (*param_1) [16],ulonglong param_2,undefined1 (*param_3) [16],ulonglong param_4);
ulonglong FUN_1800b2150(longlong param_1,longlong param_2,ulonglong param_3);
char FUN_1800b2250(ulonglong param_1,ulonglong param_2,char param_3);
undefined1 (*) [32]FUN_1800b22e0(undefined1 (*param_1) [32],undefined1 (*param_2) [32],undefined1 (*param_3) [16],ulonglong param_4);
ulonglong FUN_1800b24d0(undefined8 *param_1,uint param_2);
undefined1 (*) [32]thunk_FUN_1800b1310(undefined1 (*param_1) [32],undefined1 (*param_2) [32],undefined1 (*param_3) [32],ulonglong param_4);
ulonglong thunk_FUN_1800b0e00(undefined1 (*param_1) [16],ulonglong param_2,undefined1 (*param_3) [16],ulonglong param_4);
undefined1 *FUN_1800b2580(undefined1 (*param_1) [16],undefined1 *param_2,undefined1 (*param_3) [16],ulonglong param_4);
undefined1 (*) [32]thunk_FUN_1800b17d0(undefined1 (*param_1) [32],undefined1 (*param_2) [32],byte param_3);
undefined1 (*) [32]thunk_FUN_1800b16d0(undefined1 (*param_1) [32],undefined1 (*param_2) [32],byte param_3);
ulonglong thunk_FUN_1800b2150(longlong param_1,longlong param_2,ulonglong param_3);
undefined1 (*) [32]thunk_FUN_1800b22e0(undefined1 (*param_1) [32],undefined1 (*param_2) [32],undefined1 (*param_3) [16],ulonglong param_4);
void __cdecl std::_Xout_of_range(char *param_1);
void _Thrd_join(void);
void _Thrd_id(void);
void _Mtx_lock(void);
void _Mtx_unlock(void);
void _Cnd_init_in_situ(void);
void _Cnd_wait(void);
void _Cnd_signal(void);
void _Cnd_do_broadcast_at_thread_exit(void);
void __cdecl std::_Throw_Cpp_error(int param_1);
void FUN_1800b2770(DWORD param_1,longlong *param_2);
HLOCAL __stdcall LocalFree(HLOCAL hMem);
char * __cdecl std::_Syserror_map(int param_1);
int __cdecl std::_Winerror_map(int param_1);
uint FUN_1800b2870(void);
ulonglong FUN_1800b28a0(UINT param_1,LPCSTR param_2,int param_3,LPWSTR param_4,int param_5);
undefined8 FUN_1800b28f0(UINT param_1,LPCWSTR param_2,int param_3,LPSTR param_4,int param_5);
undefined8 FUN_1800b2a10(UINT param_1,LPCWSTR param_2,int param_3,LPSTR param_4,int param_5);
ulonglong FUN_1800b2ac0(LPCWSTR param_1);
DWORD FUN_1800b2b40(LPCWSTR param_1,ulonglong *param_2,uint param_3,uint param_4);
DWORD FUN_1800b2e70(longlong *param_1,undefined8 param_2,undefined4 param_3,uint param_4);
void FUN_1800b2ef0(undefined8 param_1);
_Locimp * __cdecl std::locale::_Init(bool param_1);
_Locimp * __cdecl std::locale::_Getgloballocale(void);
void __cdecl std::_Xbad_function_call(void);
void __thiscall std::locale::facet::_Incref(facet *this);
_Facet_base * __thiscall std::locale::facet::_Decref(facet *this);
void __thiscall std::basic_streambuf<>::imbue(basic_streambuf<> *this,locale *param_1);
int __thiscall std::basic_streambuf<>::sync(basic_streambuf<> *this);
basic_streambuf<> * __thiscall std::basic_streambuf<>::setbuf(basic_streambuf<> *this,char *param_1,__int64 param_2);
__int64 __thiscall std::basic_streambuf<>::xsputn(basic_streambuf<> *this,char *param_1,__int64 param_2);
__int64 __thiscall std::basic_streambuf<>::xsgetn(basic_streambuf<> *this,char *param_1,__int64 param_2);
int __thiscall std::basic_streambuf<>::uflow(basic_streambuf<> *this);
__int64 __thiscall std::basic_streambuf<>::showmanyc(basic_streambuf<> *this);
void __thiscall std::basic_streambuf<>::_Unlock(basic_streambuf<> *this);
void __thiscall std::basic_streambuf<>::_Lock(basic_streambuf<> *this);
void __std_parallel_algorithms_hw_threads(void);
void __std_create_threadpool_work(void);
void __std_bulk_submit_threadpool_work(void);
void __std_close_threadpool_work(void);
void __std_wait_for_threadpool_work_callbacks(void);
void _Thrd_hardware_concurrency(void);
NTSTATUS __stdcall BCryptOpenAlgorithmProvider(BCRYPT_ALG_HANDLE *phAlgorithm,LPCWSTR pszAlgId,LPCWSTR pszImplementation,ULONG dwFlags);
NTSTATUS __stdcall BCryptCloseAlgorithmProvider(BCRYPT_ALG_HANDLE hAlgorithm,ULONG dwFlags);
NTSTATUS __stdcall BCryptGenRandom(BCRYPT_ALG_HANDLE hAlgorithm,PUCHAR pbBuffer,ULONG cbBuffer,ULONG dwFlags);
void FUN_1800b2fd0(longlong param_1,longlong param_2,longlong param_3,undefined *param_4);
void FUN_1800b3050(longlong param_1,longlong param_2,longlong param_3,undefined *param_4);
void FUN_1800b30b0(size_t param_1);
void __cdecl free(void *_Memory);
void thunk_FUN_1800b30b0(size_t param_1);
ulonglong FUN_1800b3120(void);
undefined8 FUN_1800b3170(void);
uint FUN_1800b31b0(void);
undefined1 FUN_1800b3230(void);
undefined1 FUN_1800b3260(void);
void FUN_1800b3280(undefined8 param_1,int param_2,undefined8 param_3,undefined *param_4,undefined4 param_5,undefined8 param_6);
void FUN_1800b32d0(void);
void FUN_1800b3310(void);
longlong FUN_1800b3330(int param_1);
undefined * FUN_1800b3370(longlong param_1);
undefined8 FUN_1800b3410(char param_1);
undefined1 FUN_1800b3440(undefined8 param_1,char param_2);
int FUN_1800b3470(longlong param_1);
void FUN_1800b34b0(void);
undefined8 FUN_1800b34c0(undefined8 param_1,undefined8 param_2,undefined8 param_3,longlong param_4);
void FUN_1800b34e0(undefined8 param_1,longlong param_2);
undefined8 FUN_1800b36c0(longlong param_1,undefined8 param_2,undefined8 param_3,longlong param_4);
void __chkstk(void);
void __cdecl __security_check_cookie(uintptr_t _StackCookie);
undefined8 * FUN_1800b37c0(undefined8 *param_1,ulonglong param_2);
void __cdecl free(void *_Memory);
ulonglong FUN_1800b3800(undefined8 param_1,int param_2,longlong param_3);
undefined8 FUN_1800b3860(undefined8 param_1,undefined8 param_2);
ulonglong FUN_1800b3970(byte param_1);
ulonglong FUN_1800b3a10(HMODULE param_1,int param_2,longlong param_3);
void entry(HMODULE param_1,int param_2,longlong param_3);
void FUN_1800b3b90(undefined4 *param_1);
void _Init_thread_footer(int *param_1);
void FUN_1800b3c40(int *param_1);
void tls_callback_1(undefined8 param_1,int param_2);
undefined8 FUN_1800b3d80(undefined8 param_1);
void tls_callback_0(undefined8 param_1,int param_2);
void FUN_1800b3e80(size_t param_1);
undefined8 FUN_1800b3ea0(void);
void FUN_1800b4150(void);
undefined8 FUN_1800b4170(void);
bool FUN_1800b4180(void);
void FUN_1800b4190(undefined4 param_1);
void FUN_1800b41a0(void);
undefined8 FUN_1800b4260(HMODULE param_1,int param_2);
void FUN_1800b4290(void);
void FUN_1800b42a0(void);
undefined * FUN_1800b42b0(void);
void FUN_1800b42c0(void);
undefined ** FUN_1800b42e0(void);
void FUN_1800b42f0(void);
void FUN_1800b4340(void);
void __std_exception_copy(void);
void __std_exception_destroy(void);
void __stdcall _CxxThrowException(void *pExceptionObject,ThrowInfo *pThrowInfo);
void __CxxFrameHandler4(void);
int __cdecl memcmp(void *_Buf1,void *_Buf2,size_t _Size);
void * __cdecl memcpy(void *_Dst,void *_Src,size_t _Size);
void * __cdecl memset(void *_Dst,int _Val,size_t _Size);
void _purecall(void);
void * __cdecl memmove(void *_Dst,void *_Src,size_t _Size);
void __current_exception(void);
void __current_exception_context(void);
void __std_type_info_destroy_list(void);
void __cdecl free(void *_Memory);
size_t __cdecl strlen(char *_Str);
size_t __cdecl wcslen(wchar_t *_Str);
void * __cdecl malloc(size_t _Size);
void terminate(void);
int __cdecl strcmp(char *_Str1,char *_Str2);
void log2(void);
float __cdecl powf(float _X,float _Y);
int __cdecl strncmp(char *_Str1,char *_Str2,size_t _MaxCount);
double __cdecl atan(double _X);
float __cdecl sqrtf(float _X);
float __cdecl tanf(float _X);
int __cdecl _callnewh(size_t _Size);
void _seh_filter_dll(void);
void _configure_narrow_argv(void);
void _initialize_narrow_environment(void);
void _initialize_onexit_table(void);
void _register_onexit_function(void);
void _execute_onexit_table(void);
void _crt_atexit(void);
void __cdecl _cexit(void);
void _initterm(void);
void _initterm_e(void);
undefined1 FUN_1800b4470(void);
void _guard_dispatch_icall(void);
void _guard_dispatch_icall(void);
void FUN_1800b45f0(undefined8 param_1,longlong param_2);
void FUN_1800b4670(undefined8 param_1,longlong param_2);
void FUN_1800b5110(undefined8 param_1,longlong param_2);
void FUN_1800b5160(undefined8 param_1,longlong param_2);
undefined8 FUN_1800b51e0(undefined8 param_1,longlong param_2);
void FUN_1800b5290(undefined8 param_1,longlong param_2);
void FUN_1800b52c0(undefined8 param_1,longlong param_2);
void FUN_1800b52f0(undefined8 param_1,longlong param_2);
undefined8 FUN_1800b56a0(undefined8 param_1,longlong param_2);
void FUN_1800b57c0(undefined8 param_1,longlong param_2);
void FUN_1800b5810(undefined8 param_1,longlong param_2);
void FUN_1800b5830(undefined8 param_1,longlong param_2);
void FUN_1800b58b0(undefined8 param_1,longlong param_2);
void FUN_1800b58d0(undefined8 param_1,longlong param_2);
void FUN_1800b5970(undefined8 param_1,longlong param_2);
void FUN_1800b5ad0(undefined8 param_1,longlong param_2);
void FUN_1800b5be0(undefined8 param_1,longlong param_2);
void FUN_1800b5d50(undefined8 param_1,longlong param_2);
void FUN_1800b5e60(undefined8 param_1,longlong param_2);
void FUN_1800b5e80(undefined8 param_1,longlong param_2);
void FUN_1800b5ed0(undefined8 param_1,longlong param_2);
void FUN_1800b5f00(undefined8 param_1,longlong param_2);
void FUN_1800b5f70(undefined8 param_1,longlong param_2);
void FUN_1800b5fb0(undefined8 param_1,longlong param_2);
void FUN_1800b6020(undefined8 param_1,longlong param_2);
undefined8 FUN_1800b6100(undefined8 param_1,longlong param_2);
void FUN_1800b6190(undefined8 param_1,longlong param_2);
undefined8 FUN_1800b6250(void);
void FUN_1800b62b0(void);
void FUN_1800b6490(undefined8 param_1,longlong param_2);
undefined8 FUN_1800b6510(undefined8 param_1,longlong param_2);
void FUN_1800b6630(undefined8 param_1,longlong param_2);
void FUN_1800b6760(undefined8 param_1,longlong param_2);
void FUN_1800b6780(undefined8 param_1,longlong param_2);
undefined8 FUN_1800b67b0(undefined8 param_1,longlong param_2);
void FUN_1800b6a10(undefined8 param_1,longlong param_2);
void FUN_1800b6a50(undefined8 param_1,longlong param_2);
void FUN_1800b6b00(undefined8 param_1,longlong param_2);
void FUN_1800b6d50(undefined8 param_1,longlong param_2);
undefined8 FUN_1800b6eb0(undefined8 param_1,longlong param_2);
void FUN_1800b70a0(undefined8 param_1,longlong param_2);
void FUN_1800b71b0(undefined8 param_1,longlong param_2);
void FUN_1800b7430(undefined8 param_1,longlong param_2);
void FUN_1800b7460(undefined8 param_1,longlong param_2);
void FUN_1800b7490(undefined8 param_1,longlong param_2);
void FUN_1800b74c0(undefined8 param_1,longlong param_2);
void FUN_1800b74f0(undefined8 param_1,longlong param_2);
void FUN_1800b7520(undefined8 param_1,longlong param_2);
void FUN_1800b7550(undefined8 param_1,longlong param_2);
void FUN_1800b7580(undefined8 param_1,longlong param_2);
void FUN_1800b75b0(undefined8 param_1,longlong param_2);
void FUN_1800b75f0(undefined8 param_1,longlong param_2);
void FUN_1800b7630(undefined8 param_1,longlong param_2);
void FUN_1800b7870(undefined8 param_1,longlong param_2);
void FUN_1800b78c0(undefined8 param_1,longlong param_2);
void FUN_1800b78f0(undefined8 param_1,longlong param_2);
void FUN_1800b7970(undefined8 param_1,longlong param_2);
void FUN_1800b79d0(undefined8 param_1,longlong param_2);
void FUN_1800b7a60(undefined8 param_1,longlong param_2);
undefined8 FUN_1800b7b40(void);
void FUN_1800b7b60(undefined8 param_1,longlong param_2);
void FUN_1800b7cd0(undefined8 param_1,longlong param_2);
void FUN_1800b7d00(undefined8 param_1,longlong param_2);
undefined8 FUN_1800b7d80(undefined8 param_1,longlong param_2);
void FUN_1800b7e50(undefined8 param_1,longlong param_2);
void FUN_1800b7f80(undefined8 param_1,longlong param_2);
void FUN_1800b8100(undefined8 param_1,longlong param_2);
void FUN_1800b8120(undefined8 param_1,longlong param_2);
void FUN_1800b8150(undefined8 param_1,longlong param_2);
void FUN_1800b8180(undefined8 param_1,longlong param_2);
void FUN_1800b81b0(undefined8 param_1,longlong param_2);
void FUN_1800b81e0(undefined8 param_1,longlong param_2);
void FUN_1800b8210(undefined8 param_1,longlong param_2);
void FUN_1800b8240(undefined8 param_1,longlong param_2);
void FUN_1800b8270(undefined8 param_1,longlong param_2);
void FUN_1800b82a0(undefined8 param_1,longlong param_2);
void FUN_1800b82d0(undefined8 param_1,longlong param_2);
void FUN_1800b8300(undefined8 param_1,longlong param_2);
void FUN_1800b8330(undefined8 param_1,longlong param_2);
void FUN_1800b8360(undefined8 param_1,longlong param_2);
void FUN_1800b8390(undefined8 param_1,longlong param_2);
void FUN_1800b83c0(undefined8 param_1,longlong param_2);
void FUN_1800b8490(undefined8 param_1,longlong param_2);
undefined8 FUN_1800b8650(undefined8 param_1,longlong param_2);
void FUN_1800b8760(undefined8 param_1,longlong param_2);
void FUN_1800b88b0(undefined8 param_1,longlong param_2);
undefined4 FUN_1800b88e0(undefined8 param_1,longlong param_2);
bool FUN_1800b8940(undefined8 *param_1);
void FUN_1800b8960(undefined8 param_1,longlong param_2);
void FUN_1800b8980(undefined8 param_1,longlong param_2);
void FUN_1800b899a(void);
void FUN_1800b89b0(undefined8 *param_1,longlong param_2);
void FUN_1800b89f0(void);
void FUN_1800b8a60(void);
void FUN_1800b8ad0(void);
void FUN_1800b8b40(void);
void FUN_1800b8bb0(void);
void FUN_1800b8c30(void);
void FUN_1800b8cb0(void);
void FUN_1800b8cd0(void);
void FUN_1800b8d40(void);
void FUN_1800b8db0(void);
void FUN_1800b8e20(void);
void FUN_1800b8e90(void);
void FUN_1800b8f10(void);
void FUN_1800b8f90(void);
void FUN_1800b8fb0(void);
void FUN_1800b9020(void);
void FUN_1800b9090(void);
void FUN_1800b9100(void);
void FUN_1800b9180(void);
void FUN_1800b9200(void);
void FUN_1800b9270(void);
void FUN_1800b92e0(void);
void FUN_1800b9350(void);
void FUN_1800b93d0(void);
void FUN_1800b9450(void);
void FUN_1800b94c0(void);
void FUN_1800b9530(void);
void FUN_1800b95a0(void);
void FUN_1800b95f0(void);
void FUN_1800b9670(void);
void FUN_1800b96f0(void);
void FUN_1800b9730(void);
void FUN_1800b9850(void);
void FUN_1800b98d0(void);
void FUN_1800b9940(void);
void FUN_1800b99b0(void);
void FUN_1800b9a20(void);
void FUN_1800b9a90(void);
void FUN_1800b9b10(void);
void FUN_1800b9b90(void);
void FUN_1800b9c00(void);
void FUN_1800b9c70(void);
void FUN_1800b9ce0(void);
void FUN_1800b9d60(void);
void FUN_1800b9de0(void);
void FUN_1800b9e50(void);
void FUN_1800b9ec0(void);
void FUN_1800b9f30(void);
void FUN_1800b9fb0(void);
void FUN_1800ba030(void);
void FUN_1800ba0a0(void);
void FUN_1800ba110(void);
void FUN_1800ba180(void);
void FUN_1800ba200(void);
void FUN_1800ba280(void);
void FUN_1800ba2f0(void);
void FUN_1800ba360(void);
void FUN_1800ba3d0(void);
void FUN_1800ba450(void);
void FUN_1800ba4d0(void);
void FUN_1800ba540(void);
void FUN_1800ba5b0(void);
void FUN_1800ba620(void);
void FUN_1800ba6a0(void);
void FUN_1800ba720(void);
void FUN_1800ba790(void);
void FUN_1800ba800(void);
void FUN_1800ba870(void);
void FUN_1800ba8f0(void);
void FUN_1800ba970(void);
void FUN_1800ba9e0(void);
void FUN_1800baa50(void);
void FUN_1800baac0(void);
void FUN_1800bab40(void);
void FUN_1800babc0(void);
void FUN_1800bac30(void);
void FUN_1800baca0(void);
void FUN_1800bad10(void);
void FUN_1800bad90(void);
void FUN_1800bae10(void);
void FUN_1800baeb0(void);
void FUN_1800baf20(void);
void FUN_1800baf90(void);
void FUN_1800bb000(void);
void FUN_1800bb080(void);
void FUN_1800bb100(void);
void FUN_1800bb180(void);
void FUN_1800bb1f0(void);
void FUN_1800bb260(void);
void FUN_1800bb2d0(void);
void FUN_1800bb340(void);
void FUN_1800bb3c0(void);
void FUN_1800bb440(void);
void FUN_1800bb4b0(void);
void FUN_1800bb520(void);
void FUN_1800bb590(void);
void FUN_1800bb610(void);
void FUN_1800bb690(void);
void FUN_1800bb700(void);
void FUN_1800bb770(void);
void FUN_1800bb7e0(void);
void FUN_1800bb860(void);
void FUN_1800bb8e0(void);
void FUN_1800bb960(void);
void FUN_1800bb9d0(void);
void FUN_1800bba40(void);
void FUN_1800bbab0(void);
void FUN_1800bbb20(void);
void FUN_1800bbba0(void);
void FUN_1800bbc20(void);
void FUN_1800bbc90(void);
void FUN_1800bbd00(void);
void FUN_1800bbd70(void);
void FUN_1800bbdf0(void);
void FUN_1800bbe70(void);
void FUN_1800bbee0(void);
void FUN_1800bbf50(void);
void FUN_1800bbfc0(void);
void FUN_1800bc040(void);
void FUN_1800bc0c0(void);
void FUN_1800bc130(void);
void FUN_1800bc1a0(void);
void FUN_1800bc210(void);
void FUN_1800bc290(void);
void FUN_1800bc310(void);
void FUN_1800bc380(void);
void FUN_1800bc3f0(void);
void FUN_1800bc460(void);
void FUN_1800bc4e0(void);
void FUN_1800bc560(void);
void FUN_1800bc5d0(void);
void FUN_1800bc640(void);
void FUN_1800bc6b0(void);
void FUN_1800bc730(void);
void FUN_1800bc7b0(void);
void FUN_1800bc820(void);
void FUN_1800bc890(void);
void FUN_1800bc900(void);
void FUN_1800bc980(void);
void FUN_1800bca00(void);
void FUN_1800bca70(void);
void FUN_1800bcae0(void);
void FUN_1800bcb50(void);
void FUN_1800bcbd0(void);
void FUN_1800bcc50(void);
void FUN_1800bccc0(void);
void FUN_1800bcd30(void);
void FUN_1800bcda0(void);
void FUN_1800bce20(void);
void FUN_1800bcea0(void);
void FUN_1800bcf30(void);
void FUN_1800bcfb0(void);

