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

typedef struct _SECURITY_ATTRIBUTES _SECURITY_ATTRIBUTES, *P_SECURITY_ATTRIBUTES;

typedef void *LPVOID;

typedef int BOOL;

struct _SECURITY_ATTRIBUTES {
    DWORD nLength;
    LPVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
};

typedef struct _SYSTEMTIME _SYSTEMTIME, *P_SYSTEMTIME;

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

typedef struct _SYSTEMTIME SYSTEMTIME;

typedef union _RTL_RUN_ONCE _RTL_RUN_ONCE, *P_RTL_RUN_ONCE;

typedef union _RTL_RUN_ONCE *PRTL_RUN_ONCE;

typedef PRTL_RUN_ONCE LPINIT_ONCE;

typedef void *PVOID;

union _RTL_RUN_ONCE {
    PVOID Ptr;
};

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

typedef struct _SECURITY_ATTRIBUTES *LPSECURITY_ATTRIBUTES;

typedef struct _SYSTEMTIME *LPSYSTEMTIME;

typedef wchar_t WCHAR;

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

typedef WCHAR *LPCWSTR;

typedef void *HANDLE;

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

typedef longlong fpos_t;

typedef struct _FILETIME _FILETIME, *P_FILETIME;

typedef struct _FILETIME *LPFILETIME;

struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
};

typedef ulong ULONG;

typedef INT_PTR (*FARPROC)(void);

typedef struct tagRECT tagRECT, *PtagRECT;

struct tagRECT {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
};

typedef int INT;

typedef DWORD *LPDWORD;

typedef WORD ATOM;

typedef struct HMENU__ HMENU__, *PHMENU__;

struct HMENU__ {
    int unused;
};

typedef struct _FILETIME FILETIME;

typedef struct tagRECT *LPRECT;

typedef void *HGDIOBJ;

typedef HINSTANCE HMODULE;

typedef BOOL *PBOOL;

typedef struct HMENU__ *HMENU;

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

typedef struct _Mbstatet _Mbstatet, *P_Mbstatet;

struct _Mbstatet { // PlaceHolder Structure
};

typedef struct aescriptsLicenseData aescriptsLicenseData, *PaescriptsLicenseData;

struct aescriptsLicenseData { // PlaceHolder Structure
};

typedef struct basic_istream<char,struct_std::char_traits<char>_> basic_istream<char,struct_std::char_traits<char>_>, *Pbasic_istream<char,struct_std::char_traits<char>_>;

struct basic_istream<char,struct_std::char_traits<char>_> { // PlaceHolder Structure
};

typedef struct codecvt_base codecvt_base, *Pcodecvt_base;

struct codecvt_base { // PlaceHolder Structure
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

typedef struct basic_iostream<char,std::char_traits<char>_> basic_iostream<char,std::char_traits<char>_>, *Pbasic_iostream<char,std::char_traits<char>_>;

struct basic_iostream<char,std::char_traits<char>_> { // PlaceHolder Structure
};

typedef struct basic_streambuf<char,struct_std::char_traits<char>_> basic_streambuf<char,struct_std::char_traits<char>_>, *Pbasic_streambuf<char,struct_std::char_traits<char>_>;

struct basic_streambuf<char,struct_std::char_traits<char>_> { // PlaceHolder Structure
};

typedef struct ios_base ios_base, *Pios_base;

struct ios_base { // PlaceHolder Structure
};

typedef struct basic_ios<char,std::char_traits<char>_> basic_ios<char,std::char_traits<char>_>, *Pbasic_ios<char,std::char_traits<char>_>;

struct basic_ios<char,std::char_traits<char>_> { // PlaceHolder Structure
};

typedef struct _Smanip<__int64> _Smanip<__int64>, *P_Smanip<__int64>;

struct _Smanip<__int64> { // PlaceHolder Structure
};

typedef struct basic_ostream<char,struct_std::char_traits<char>_> basic_ostream<char,struct_std::char_traits<char>_>, *Pbasic_ostream<char,struct_std::char_traits<char>_>;

struct basic_ostream<char,struct_std::char_traits<char>_> { // PlaceHolder Structure
};

typedef struct basic_ostream<char,std::char_traits<char>_> basic_ostream<char,std::char_traits<char>_>, *Pbasic_ostream<char,std::char_traits<char>_>;

struct basic_ostream<char,std::char_traits<char>_> { // PlaceHolder Structure
};

typedef struct codecvt<char,char,_Mbstatet> codecvt<char,char,_Mbstatet>, *Pcodecvt<char,char,_Mbstatet>;

struct codecvt<char,char,_Mbstatet> { // PlaceHolder Structure
};

typedef struct fpos<_Mbstatet> fpos<_Mbstatet>, *Pfpos<_Mbstatet>;

struct fpos<_Mbstatet> { // PlaceHolder Structure
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

typedef ulonglong size_t;

typedef longlong __time64_t;

typedef int errno_t;

typedef size_t rsize_t;




void FUN_180001010(void);
void FUN_180001220(void);
void FUN_180001290(void);
void FUN_1800012b0(void);
longlong * FUN_1800013f0(longlong *param_1,longlong param_2);
void FUN_180001430(longlong *param_1);
void FUN_180001440(longlong *param_1);
void FUN_1800023b0(void);
undefined8 * FUN_1800023d0(longlong *param_1,undefined8 *param_2,undefined8 *param_3);
undefined8 FUN_180002570(undefined4 *param_1);
undefined8 * FUN_1800025a0(undefined8 *param_1,longlong param_2);
undefined8 * FUN_1800025e0(undefined8 *param_1,longlong param_2);
undefined8 * FUN_180002620(undefined8 *param_1);
exception * __thiscall std::exception::exception(exception *this,exception *param_1);
void FUN_180002690(longlong *param_1);
void FUN_1800026e0(longlong *param_1);
void FUN_180002790(longlong param_1);
undefined8 * FUN_1800027b0(undefined8 *param_1,ulonglong param_2);
undefined8 FUN_180002800(undefined8 param_1,undefined8 param_2,longlong param_3);
undefined8 FUN_180002ec0(undefined8 param_1,undefined8 param_2,longlong param_3);
undefined8 FUN_180003590(undefined8 param_1,undefined8 param_2,longlong param_3);
float * FUN_180003c60(float *param_1,longlong param_2,float *param_3);
undefined8 FUN_180003ee0(undefined8 param_1,undefined8 param_2,longlong param_3);
undefined8 FUN_1800045b0(undefined8 param_1,undefined8 param_2,longlong param_3);
undefined8 FUN_180004c80(undefined8 param_1,undefined8 param_2,longlong param_3);
undefined8 FUN_180005340(undefined8 param_1,undefined8 param_2,longlong param_3);
undefined8 FUN_180005a10(undefined8 param_1,undefined8 param_2,longlong param_3);
float * FUN_1800060f0(float *param_1,longlong param_2,float *param_3,int param_4,int param_5);
void FUN_1800062e0(longlong param_1,undefined4 *param_2);
float * FUN_1800066e0(float *param_1,longlong param_2,float param_3,float param_4);
void FUN_180008300(longlong param_1,int param_2,int param_3);
float FUN_180008410(longlong param_1,float param_2,float param_3);
void FUN_1800089e0(longlong param_1,int param_2,int param_3);
float * FUN_180008ac0(float *param_1,longlong param_2,float param_3,float param_4);
void FUN_180008f30(longlong param_1,int param_2,int param_3);
void FUN_180009030(longlong param_1,int param_2,int param_3);
void FUN_180009130(longlong param_1,int param_2,int param_3);
void FUN_180009230(longlong param_1,int param_2,int param_3);
void FUN_180009370(longlong param_1,longlong param_2);
int FUN_18000bfb0(undefined8 *param_1,undefined8 param_2,longlong *param_3);
void FUN_18000c2d0(longlong param_1);
float FUN_18000c370(ulonglong param_1);
void FUN_18000c3c0(undefined4 *param_1,longlong param_2);
undefined8 FUN_18000cc60(longlong param_1,undefined8 param_2,longlong *param_3,longlong param_4);
void FUN_18000cf60(longlong param_1,undefined *param_2);
void FUN_18000d310(longlong param_1,longlong param_2);
undefined8 FUN_18000d750(longlong param_1,int param_2,longlong param_3);
float * FUN_18000dbb0(float *param_1,longlong param_2,float param_3,float param_4);
float * FUN_18000dda0(float *param_1,longlong param_2,int param_3,int param_4);
float FUN_18000e070(longlong param_1,float param_2,float param_3);
float * FUN_18000e1b0(float *param_1,longlong param_2,float param_3,float param_4);
void FUN_18000e3a0(longlong param_1,int param_2,float param_3,float param_4,float param_5);
int FUN_18000e480(longlong param_1,longlong param_2,longlong *param_3);
int FUN_18000e830(longlong param_1,longlong param_2,longlong *param_3);
int FUN_18000ee20(longlong param_1,longlong param_2);
int FUN_18000f060(int param_1,float param_2,float param_3,float param_4,int param_5);
float FUN_18000f100(float param_1,float param_2,float param_3);
float FUN_18000f170(float param_1,float param_2,float param_3,int param_4,float param_5,float param_6);
float FUN_18000f260(float param_1,float param_2);
float FUN_18000f310(float param_1,float param_2,float param_3);
float * FUN_18000f3f0(float *param_1,longlong param_2,float param_3,float param_4,float param_5,float param_6,float param_7,float param_8);
float * FUN_18000f8d0(float *param_1,longlong param_2,float param_3,float param_4,float param_5);
float FUN_18000fb40(float param_1,float param_2,float param_3);
void FUN_1800100d0(longlong param_1,int param_2,int param_3,float *param_4);
void FUN_1800101f0(longlong param_1,int param_2,int param_3,float *param_4);
void FUN_180010300(longlong param_1,int param_2,int param_3,float *param_4);
void FUN_180010410(longlong *param_1,longlong param_2,longlong param_3,longlong param_4);
void FUN_1800104c0(void);
void FUN_1800104e0(void);
float * FUN_180010500(float *param_1,undefined8 param_2,float *param_3);
void FUN_180010740(float *param_1,longlong param_2,float param_3,float param_4);
float * FUN_180010a40(float *param_1,longlong param_2,float param_3,float param_4);
float * FUN_180011c40(float *param_1,float *param_2,longlong param_3,int param_4);
char * FUN_180011e80(longlong param_1);
ulonglong EffectMain(uint param_1,undefined8 ****param_2,undefined4 *param_3,longlong *param_4,longlong param_5,longlong *param_6);
undefined * FUN_180012320(void);
int snprintf(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
ulonglong FUN_180012390(undefined8 *param_1,char *param_2);
undefined8 * FUN_180012400(undefined8 *param_1,undefined8 *param_2,undefined8 *param_3);
longlong * FUN_180012430(longlong *param_1,longlong *param_2,undefined8 *param_3);
longlong * FUN_1800124a0(longlong *param_1,longlong *param_2,char *param_3);
undefined8 * FUN_180012510(undefined8 *param_1,char *param_2,char *param_3);
undefined8 * FUN_180012640(undefined8 *param_1,char *param_2,undefined8 *param_3);
ulonglong FUN_1800126d0(undefined8 param_1,size_t *param_2);
longlong * FUN_180012750(longlong *param_1,void *param_2,ulonglong param_3);
longlong * FUN_1800127e0(longlong *param_1,void *param_2,size_t param_3);
void FUN_180012950(undefined8 *param_1,void *param_2,size_t param_3);
undefined8 FUN_180012a50(longlong *param_1,undefined8 *param_2,undefined8 *param_3);
basic_ostream<> * FUN_180012cd0(basic_ostream<> *param_1,char *param_2,ulonglong param_3);
undefined8 *FUN_180012f40(undefined8 *param_1,ulonglong param_2,undefined8 param_3,size_t param_4,longlong param_5,void *param_6,size_t param_7);
undefined8 *FUN_180013110(undefined8 *param_1,ulonglong param_2,undefined8 param_3,void *param_4,size_t param_5);
undefined8 *FUN_1800132a0(undefined8 *param_1,ulonglong param_2,undefined8 param_3,size_t param_4,void *param_5,size_t param_6);
longlong *FUN_180013460(longlong *param_1,ulonglong param_2,ulonglong param_3,void *param_4,ulonglong param_5);
facet * FUN_180013610(locale *param_1);
undefined8 * FUN_180013730(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_180013840(undefined8 *param_1,char *param_2);
undefined8 *FUN_180013890(undefined8 *param_1,undefined8 param_2,undefined8 *param_3,undefined8 *param_4);
undefined8 *FUN_180013a80(undefined8 *param_1,undefined8 param_2,undefined8 param_3,void *param_4,size_t param_5,undefined8 param_6,size_t param_7);
undefined8 * FUN_180013ba0(undefined8 *param_1);
longlong *FUN_180013bc0(longlong *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6,undefined4 param_7,HINSTANCE param_8);
undefined8 * FUN_180013e50(undefined8 *param_1,longlong param_2);
undefined8 * FUN_180013e90(undefined8 *param_1);
void FUN_180013ec0(basic_streambuf<> *param_1);
void thunk_FUN_1800156d0(longlong *param_1);
void FUN_180013f40(basic_streambuf<> *param_1);
void FUN_180014020(longlong *param_1);
void FUN_180014040(longlong *param_1);
void FUN_1800140d0(undefined8 *param_1);
void FUN_180014110(longlong param_1);
void FUN_1800141a0(longlong *param_1);
void FUN_1800141d0(longlong param_1);
void FUN_180014200(longlong *param_1);
longlong * FUN_180014250(longlong *param_1,longlong *param_2);
longlong * FUN_1800142f0(longlong *param_1,longlong *param_2);
void FUN_180014330(longlong *param_1,char *param_2);
longlong FUN_180014360(longlong *param_1,longlong param_2);
void FUN_180014380(longlong *param_1,undefined8 *param_2);
void FUN_1800143b0(longlong *param_1,char *param_2);
void FUN_1800143e0(longlong *param_1);
void FUN_180014440(longlong *param_1);
void FUN_180014494(longlong param_1,uint param_2);
void FUN_1800144a0(longlong param_1,uint param_2);
basic_streambuf<> * FUN_1800144b0(basic_streambuf<> *param_1,uint param_2);
basic_ios<> * FUN_1800144f0(basic_ios<> *param_1,uint param_2);
basic_streambuf<> * FUN_180014590(basic_streambuf<> *param_1,uint param_2);
basic_ios<> * FUN_1800145d0(basic_ios<> *param_1,uint param_2);
bool FUN_180014670(undefined8 *param_1,char *param_2,char *param_3,char *param_4,char *param_5,char *param_6,undefined1 param_7,char *param_8,undefined8 param_9);
void FUN_180014900(HWND param_1,int param_2,undefined8 param_3,longlong *param_4);
undefined4 FUN_1800149b0(longlong param_1);
void FUN_180014a40(int param_1,longlong param_2,longlong param_3);
undefined8 FUN_180014d30(void);
undefined8 FUN_180014dc0(longlong param_1,longlong param_2,undefined8 param_3);
uint * FUN_180014e90(uint *param_1);
void FUN_180015520(undefined8 param_1,void *param_2,longlong param_3);
bool FUN_180015560(longlong *param_1);
void FUN_180015650(longlong param_1);
void FUN_180015670(basic_streambuf<> *param_1);
void FUN_1800156b0(void);
void FUN_1800156d0(longlong *param_1);
void FUN_180015740(longlong param_1);
void FUN_180015760(void);
void FUN_180015780(void);
undefined8 * FUN_1800157a0(undefined8 *param_1);
ulonglong FUN_1800157b0(undefined8 *param_1,longlong param_2,longlong *param_3);
basic_streambuf<> * FUN_180015d10(basic_streambuf<> *param_1);
void FUN_180015db0(longlong param_1,undefined8 param_2);
void FUN_1800160d0(undefined8 param_1,void *param_2,ulonglong param_3);
char * FUN_180016110(undefined4 param_1);
void FUN_180016210(longlong param_1,undefined8 *param_2,undefined8 param_3,undefined8 param_4,undefined4 param_5);
longlong * FUN_1800163a0(longlong *param_1,char *param_2);
undefined4 aescriptsGetLicenseServerConfig(char *param_1,char *param_2,char *param_3,char *param_4);
void FUN_180016eb0(basic_streambuf<> *param_1,locale *param_2);
void ** FUN_180016f00(void **param_1,int param_2);
bool FUN_1800171d0(char *param_1);
char * FUN_180017230(char *param_1);
void FUN_1800173e0(LPCSTR param_1);
int FUN_180017410(basic_streambuf<> *param_1,int param_2);
int FUN_1800175d0(basic_streambuf<> *param_1,int param_2);
uint FUN_180017780(basic_streambuf<> *param_1,uint param_2);
int FUN_1800178a0(basic_streambuf<> *param_1,int param_2);
char * FUN_180017930(undefined4 param_1);
void aescriptsSaveLicenseToFile(char *param_1,char *param_2,char *param_3,char *param_4);
fpos_t * FUN_180017b20(basic_streambuf<> *param_1,fpos_t *param_2,longlong param_3,int param_4);
ulonglong *FUN_180017bf0(basic_streambuf<> *param_1,ulonglong *param_2,longlong param_3,int param_4,byte param_5);
longlong * FUN_180017da0(basic_streambuf<> *param_1,longlong *param_2,longlong *param_3);
ulonglong *FUN_180017e50(basic_streambuf<> *param_1,ulonglong *param_2,longlong *param_3,uint param_4);
basic_streambuf<> * FUN_180017fa0(basic_streambuf<> *param_1,char *param_2,size_t param_3);
undefined8 FUN_180018080(undefined8 *param_1,undefined8 *param_2,undefined8 param_3,char *param_4);
longlong *FUN_18001bad0(longlong *param_1,longlong *param_2,undefined1 (*param_3) [16],longlong *param_4);
longlong * FUN_18001bbf0(longlong *param_1,longlong *param_2,undefined1 (*param_3) [16]);
void FUN_18001c060(undefined8 *param_1,byte param_2,longlong *param_3);
void FUN_18001c2a0(longlong param_1);
undefined8 *FUN_18001c330(undefined8 *param_1,undefined8 *param_2,ulonglong param_3,ulonglong param_4);
int FUN_18001c390(longlong *param_1);
uint FUN_18001c3e0(basic_streambuf<> *param_1);
ulonglong FUN_18001c7c0(basic_streambuf<> *param_1);
ulonglong FUN_18001c840(basic_streambuf<> *param_1);
void FUN_18001c910(char *param_1,char ************param_2,__int64 param_3,undefined8 *******param_4,char param_5);
longlong FUN_18001c920(basic_streambuf<> *param_1,char *param_2,ulonglong param_3);
longlong FUN_18001ca50(basic_streambuf<> *param_1,char *param_2,size_t param_3);
undefined8 FUN_18001cb30(longlong param_1,undefined8 param_2);
undefined8 FUN_18001cb60(longlong param_1,undefined8 param_2);
longlong * FUN_18001cba0(longlong *param_1,longlong *param_2);
undefined8 * FUN_18001cbe0(undefined8 *param_1,ulonglong param_2);
undefined8 * FUN_18001cc10(undefined8 *param_1,uint param_2);
void FUN_18001ccc0(longlong param_1,longlong *param_2,int param_3,int param_4,float *param_5);
ulonglong FUN_18001d8c0(longlong *param_1);
void FUN_18001d9d0(longlong param_1);
undefined8 FUN_18001da00(void);
undefined8 FUN_18001da10(longlong param_1,longlong *param_2);
undefined8 FUN_18001db20(longlong param_1,longlong *param_2);
double FUN_18001dda0(longlong param_1,int param_2,undefined8 param_3);
undefined8 FUN_18001dea0(longlong param_1,longlong *param_2,longlong *param_3,longlong param_4,longlong *param_5);
undefined8 xGPUFilterEntry(undefined8 param_1,int *param_2,int param_3,undefined8 param_4,undefined8 *param_5,undefined4 *param_6);
undefined8 FUN_18001e250(longlong param_1,undefined8 param_2);
undefined8 FUN_18001e2b0(longlong param_1,undefined8 param_2);
void FUN_18001e2f0(undefined8 *param_1);
longlong * FUN_18001e370(longlong *param_1,uint param_2);
undefined1 FUN_18001e400(longlong param_1,int param_2,int param_3);
ulonglong FUN_18001e420(longlong param_1,longlong param_2,int *param_3,undefined8 param_4,longlong *param_5);
undefined8 FUN_18001e670(longlong param_1,longlong *param_2);
undefined8 FUN_18001e6d0(longlong param_1,longlong param_2,longlong *param_3);
longlong * FUN_18001e9e0(longlong param_1,undefined8 param_2);
longlong * FUN_18001ea30(longlong param_1,undefined8 param_2);
void FUN_18001ea70(undefined8 param_1,undefined8 param_2,longlong *param_3);
undefined8 * FUN_18001eb10(undefined8 *param_1,undefined8 *param_2);
void FUN_18001ed70(longlong *param_1);
void FUN_18001edc0(longlong param_1);
void FUN_18001ee10(undefined8 *param_1);
void FUN_18001ee40(undefined8 *param_1);
void FUN_18001eef0(undefined8 *param_1);
undefined8 * FUN_18001f0d0(undefined8 *param_1,ulonglong param_2);
undefined8 * FUN_18001f100(undefined8 *param_1,uint param_2);
void * FUN_18001f140(void *param_1,uint param_2);
longlong ** FUN_18001f1f0(longlong param_1,longlong **param_2,undefined8 param_3,int param_4);
undefined8 * FUN_18001f310(undefined8 *param_1,longlong *param_2);
longlong * FUN_18001f3e0(longlong *param_1,longlong *param_2);
longlong ** FUN_18001fdb0(longlong **param_1);
undefined8 FUN_18001fff0(longlong param_1);
bool FUN_180020080(longlong param_1);
bool FUN_180020350(longlong param_1);
ulonglong FUN_1800204e0(longlong param_1,undefined8 *param_2,undefined8 *param_3);
undefined8 FUN_180020560(longlong param_1,undefined8 *param_2,void *param_3,int param_4,int param_5,void *param_6);
ulonglong FUN_180020870(longlong param_1,undefined8 *param_2,undefined8 *param_3,char param_4);
bool FUN_1800217f0(longlong param_1,longlong param_2);
undefined8 FUN_180021870(longlong *param_1,undefined8 *param_2);
undefined8 * FUN_180021930(undefined8 *param_1);
undefined8 * FUN_180021950(undefined8 *param_1,undefined8 *param_2);
void FUN_1800219e0(longlong *param_1);
void FUN_180021a00(longlong param_1);
longlong * FUN_180021a60(longlong *param_1,longlong *param_2);
undefined8 FUN_180021df0(void);
longlong * FUN_180021e00(longlong *param_1,longlong *param_2,longlong *param_3);
void FUN_180022070(longlong *param_1,longlong param_2);
void FUN_1800220d0(longlong *param_1,longlong *param_2);
void FUN_180022130(void);
undefined8 * FUN_180022150(undefined8 *param_1);
undefined8 * FUN_180022360(undefined8 *param_1,uint param_2);
ulonglong FUN_1800224d0(longlong param_1,int param_2,longlong param_3);
ulonglong FUN_180022580(longlong param_1);
undefined8 FUN_180022750(longlong param_1,undefined8 *param_2,longlong *param_3);
undefined8 * FUN_180023080(undefined8 *param_1,undefined8 *param_2);
void FUN_180023350(longlong *param_1);
double FUN_1800233c0(void);
void FUN_180023460(undefined8 *param_1);
void FUN_180023490(void);
void FUN_180023850(void);
bool FUN_180023950(void);
void FUN_1800239f0(void);
undefined1 FUN_180023a60(void);
void FUN_180023d50(void);
undefined8 FUN_180023e20(undefined8 *param_1,longlong param_2,int param_3,int param_4,longlong *param_5,longlong param_6);
void FUN_180024740(longlong *param_1);
bool FUN_1800247b0(int param_1);
undefined8 FUN_180024850(int param_1,undefined8 *param_2,longlong param_3,int param_4,int param_5,longlong *param_6,int *param_7);
undefined * FUN_1800249f0(int param_1);
void D3D12SerializeRootSignature(void);
void D3D12CreateDevice(void);
void CreateDXGIFactory1(void);
basic_istream<> * FUN_180024a20(basic_istream<> *param_1,undefined8 *param_2);
basic_ostream<> * FUN_180024c30(basic_ostream<> *param_1,char param_2);
bool FUN_180024e90(undefined8 *param_1,char *param_2);
ulonglong FUN_180024f00(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_180024f60(undefined8 *param_1,undefined8 *param_2,char *param_3);
ulonglong FUN_180024ff0(size_t param_1);
void FUN_180025060(undefined8 *param_1,void *param_2,size_t param_3);
void FUN_180025120(longlong *param_1,longlong *param_2);
undefined8 * FUN_180025190(longlong *param_1,undefined8 *param_2);
undefined8 * FUN_180025300(longlong *param_1,undefined8 *param_2);
undefined8 * FUN_1800254d0(longlong *param_1,void *param_2,undefined8 *param_3);
undefined8 * FUN_1800256b0(longlong *param_1,undefined8 *param_2,undefined8 *param_3);
undefined8 * FUN_1800258e0(longlong *param_1,undefined8 *param_2,undefined8 *param_3);
longlong * FUN_180025aa0(longlong *param_1,longlong *param_2,undefined8 *param_3);
void FUN_180025cc0(undefined8 param_1,undefined8 param_2,longlong *param_3);
undefined8 ****** FUN_180025d40(undefined8 ******param_1,undefined8 ******param_2);
undefined8 FUN_180026110(undefined8 *param_1);
void FUN_180026140(longlong param_1,longlong param_2);
void FUN_1800263f0(undefined8 *param_1,undefined8 *param_2,undefined8 *param_3);
void FUN_180026570(ulonglong *param_1,undefined8 *param_2,undefined8 *param_3);
void FUN_180026ac0(longlong param_1,longlong param_2,ulonglong param_3,longlong *param_4);
void FUN_180026c90(longlong param_1,longlong param_2,longlong param_3,longlong *param_4);
undefined8 *FUN_180026e00(undefined8 *param_1,ulonglong param_2,undefined8 param_3,size_t param_4,longlong param_5,void *param_6,size_t param_7);
undefined8 *FUN_180026fa0(undefined8 *param_1,ulonglong param_2,undefined8 param_3,size_t param_4,void *param_5,size_t param_6);
void FUN_180027120(undefined8 ******param_1,undefined8 ******param_2,longlong param_3,undefined1 param_4);
void FUN_180027310(undefined8 *param_1,undefined8 *param_2,undefined8 *param_3);
undefined8 * FUN_180027380(undefined8 *param_1,undefined8 *param_2,undefined8 *param_3);
undefined8 * FUN_180027440(undefined8 *param_1,undefined8 *param_2,undefined8 *param_3);
longlong * FUN_1800274f0(longlong *param_1,longlong *param_2,longlong *param_3);
void FUN_180027710(undefined8 param_1,undefined8 *param_2,undefined8 *param_3);
void FUN_180027810(undefined8 param_1,undefined8 *param_2,undefined8 *param_3);
facet * FUN_1800278c0(locale *param_1);
basic_istream<> *FUN_1800279e0(basic_istream<> *param_1,char *param_2,uint param_3,int param_4,int param_5);
basic_istream<> * FUN_180027b00(basic_istream<> *param_1,int param_2);
basic_ostream<> *FUN_180027be0(basic_ostream<> *param_1,char *param_2,uint param_3,int param_4,int param_5);
undefined8 * FUN_180027d00(undefined8 *param_1,ulonglong param_2,char param_3);
basic_iostream<> * FUN_180027de0(basic_iostream<> *param_1,int param_2);
longlong * FUN_180027ea0(longlong *param_1);
undefined4 * FUN_180027ee0(undefined4 *param_1);
undefined8 * FUN_1800280d0(undefined8 *param_1,undefined8 *param_2);
void FUN_1800281f0(undefined8 *param_1);
void FUN_180028230(undefined8 *param_1);
void FUN_180028270(longlong *param_1);
void FUN_1800282c0(longlong *param_1);
void FUN_180028370(longlong *param_1);
void FUN_180028410(longlong param_1);
void FUN_180028570(longlong *param_1);
void FUN_180028590(longlong *param_1);
void FUN_1800285b0(longlong *param_1);
void FUN_180028610(longlong param_1);
void FUN_1800286b0(longlong param_1);
void FUN_180028760(longlong param_1);
void FUN_180028810(longlong param_1);
void FUN_180028890(longlong *param_1);
void FUN_1800288c0(longlong *param_1);
void FUN_180028910(longlong *param_1);
longlong * FUN_180028940(longlong *param_1,byte *param_2);
undefined8 * FUN_180028a50(longlong *param_1,undefined8 *param_2,int *param_3);
void FUN_180028ac0(longlong *param_1);
void FUN_180028b20(longlong param_1,uint param_2);
longlong * FUN_180028b30(longlong *param_1,uint param_2);
basic_ios<> * FUN_180028bc0(basic_ios<> *param_1,uint param_2);
undefined8 FUN_180028c50(void);
undefined8 *FUN_180028ca0(undefined8 *param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
int FUN_1800290b0(longlong *param_1,uint param_2,int param_3,longlong *param_4,longlong *param_5);
undefined8 * FUN_180029380(undefined8 *param_1,undefined8 *param_2);
void FUN_180029540(longlong *param_1,longlong param_2,longlong param_3,longlong param_4);
longlong * FUN_180029610(longlong *param_1,longlong *param_2,longlong *param_3);
void FUN_180029870(longlong *param_1);
void FUN_180029910(longlong *param_1);
void FUN_1800299e0(longlong *param_1,int *param_2,ulonglong param_3);
int __cdecl aescriptsGetLicLibVersionString(char *param_1);
int __cdecl aescriptsSaveLicenseToFileEx(char *param_1,char *param_2,char *param_3,char *param_4,aescriptsLicenseData *param_5);
undefined8 * FUN_18002af40(undefined8 *param_1,void *param_2,size_t param_3);
longlong * FUN_18002b0c0(longlong *param_1,void *param_2,size_t param_3);
longlong * FUN_18002b1e0(longlong *param_1,longlong param_2,uint param_3);
undefined4 FUN_18002b380(longlong param_1);
ulonglong FUN_18002b5a0(longlong *param_1,int *param_2,int *param_3);
undefined8 * FUN_18002c2c0(undefined8 *param_1,int param_2);
void FUN_18002c930(longlong *param_1);
void FUN_18002c980(longlong *param_1);
undefined8 FUN_18002c9c0(int param_1,sockaddr *param_2,int param_3,int param_4);
void FUN_18002cae0(longlong *param_1,char *******param_2,undefined8 param_3,undefined8 param_4);
int FUN_18002d250(int param_1,uint param_2,int param_3);
void FUN_18002d300(undefined8 param_1,void *param_2,longlong param_3);
undefined8 * FUN_18002d360(undefined8 *param_1,longlong *param_2);
void FUN_18002d9b0(char *param_1);
undefined8 FUN_18002df30(longlong param_1);
ulonglong FUN_18002e1c0(int param_1,char *param_2,char *param_3,longlong *param_4,longlong *param_5,char param_6);
DWORD FUN_180030d50(longlong *param_1,longlong *param_2);
longlong * FUN_180032520(longlong *param_1,longlong *param_2,byte *param_3);
longlong * FUN_180032600(longlong param_1,longlong *param_2,longlong param_3);
longlong * FUN_1800326f0(longlong param_1,longlong *param_2,longlong param_3);
undefined1 FUN_1800327c0(FILE *param_1);
ulonglong FUN_180032830(undefined8 *param_1,undefined1 (*param_2) [16],ulonglong param_3);
int FUN_1800328c0(longlong *param_1,longlong *param_2);
longlong * FUN_1800329c0(longlong *param_1,longlong *param_2);
undefined8 FUN_180032ba0(longlong *param_1,longlong *param_2,longlong *param_3,longlong *param_4,undefined1 (*param_5) [32]);
undefined4 FUN_1800331a0(undefined1 (*param_1) [32],longlong *param_2);
undefined8 * FUN_180033a30(longlong *param_1,undefined8 *param_2);
void FUN_180033d20(longlong *param_1,undefined1 *param_2);
undefined8 *FUN_180033f10(undefined8 *param_1,longlong *param_2,byte *******param_3,char *param_4,int param_5,longlong *param_6);
int FUN_180034e40(char *param_1,char *param_2,__int64 param_3,char param_4,char *param_5);
int FUN_180034ef0(char *param_1,char *param_2,__uint64 param_3,char param_4,char *param_5);
undefined4 FUN_180035a60(char *param_1,char *param_2,char *param_3,char *param_4,char *********param_5);
undefined8 FUN_1800372d0(undefined8 *param_1);
undefined8 FUN_180037780(longlong *param_1);
undefined8 FUN_180038210(longlong *param_1,longlong *param_2,uint param_3);
__int64 FUN_1800389f0(__int64 param_1);
undefined8 FUN_180038cb0(longlong *param_1,char param_2,char ****param_3,undefined8 param_4,char param_5);
undefined8 * FUN_1800391e0(undefined8 *param_1,longlong *param_2,int param_3,int param_4);
void ** FUN_180039280(void **param_1,longlong *param_2);
int FUN_180039710(longlong *param_1);
ios_base * FUN_180039af0(ios_base *param_1);
void FUN_180039b20(char *param_1,uint param_2,longlong param_3);
undefined8 * FUN_180039cd0(undefined8 *param_1,longlong *param_2);
void FUN_18003a280(char *param_1,longlong *param_2);
undefined8 * FUN_18003b260(undefined8 *param_1,ulonglong param_2,void *param_3,ulonglong param_4);
longlong * FUN_18003b370(longlong *param_1,int param_2);
undefined8 * FUN_18003b4a0(undefined8 *param_1,uint param_2,undefined8 param_3,undefined8 param_4);
bool FUN_18003b520(char *param_1);
bool FUN_18003b590(char *param_1);
longlong * FUN_18003b630(longlong *param_1,uint param_2);
undefined8 * FUN_18003b9d0(undefined8 *param_1,uint param_2,undefined8 *param_3,ulonglong param_4);
undefined8 * FUN_18003bda0(undefined8 *param_1,uint param_2,undefined8 *param_3,undefined8 param_4);
undefined8 *FUN_18003c0c0(undefined8 *param_1,uint param_2,undefined4 param_3,undefined8 param_4,longlong param_5,longlong param_6,char param_7,uint param_8);
undefined8 *FUN_18003c6a0(undefined8 *param_1,uint param_2,uint param_3,undefined8 param_4,char param_5);
ulonglong FUN_18003cec0(byte *param_1,char *param_2,char ************param_3,undefined8 *******param_4,char param_5,undefined1 (*param_6) [32],longlong *param_7,byte ***********param_8,char *******param_9,undefined1 (*param_10) [32],char param_11);
ulonglong FUN_1800404b0(byte *param_1,char *param_2,__int64 param_3,undefined8 *******param_4,char param_5,char param_6);
void ** FUN_180041210(void **param_1,longlong *param_2,char param_3);
void FUN_1800417a0(longlong param_1);
longlong * FUN_1800417e0(longlong *param_1,__uint64 param_2);
undefined8 FUN_180041910(char *param_1);
ulonglong * FUN_180043070(void **param_1,uint param_2);
int FUN_1800434d0(longlong *param_1,longlong param_2,int param_3);
undefined8 * FUN_180043590(undefined8 *param_1,uint param_2,undefined8 param_3,undefined8 param_4);
undefined8 * FUN_180043610(undefined8 *param_1,longlong *param_2,byte param_3);
basic_streambuf<> * FUN_180043fa0(basic_streambuf<> *param_1,char *param_2,int param_3,int param_4);
void FUN_1800440f0(longlong *param_1,char *param_2,uint param_3,int param_4);
ulonglong * FUN_180044140(ulonglong *param_1,ulonglong *param_2,int param_3);
undefined8 FUN_180044250(undefined1 (*param_1) [32],char *param_2);
void FUN_180045130(longlong param_1,int *param_2);
undefined8 FUN_180045910(longlong param_1,longlong *param_2);
void FUN_1800475f0(undefined8 *param_1,undefined1 param_2);
ulonglong FUN_180047750(int param_1,longlong *param_2);
longlong *FUN_180047810(longlong *param_1,ulonglong param_2,ulonglong param_3,void *param_4,ulonglong param_5);
longlong * FUN_1800479b0(longlong *param_1,longlong *param_2);
int FUN_180047a50(char *param_1,longlong *param_2,char *param_3,char *param_4,char *param_5,char param_6,longlong *param_7,undefined8 param_8,char param_9);
undefined8 FUN_1800499e0(longlong param_1,longlong *param_2);
int FUN_180049d60(longlong *param_1,longlong *param_2,longlong *param_3,longlong *param_4,longlong *param_5,longlong *param_6,__uint64 param_7,byte *param_8,undefined8 *******param_9);
int FUN_18004b460(int param_1,longlong *param_2);
void FUN_18004b680(longlong *param_1,DWORD param_2,char param_3,byte param_4);
void FUN_18004c840(undefined1 (*param_1) [32],undefined1 (*param_2) [32],longlong *param_3,undefined1 (*param_4) [32],longlong *param_5);
undefined8 FUN_18004d1f0(char *param_1);
longlong * FUN_18004d210(longlong param_1,longlong *param_2);
undefined8 *FUN_18004d300(undefined8 *param_1,undefined1 (*param_2) [32],longlong *param_3,char param_4);
longlong * FUN_18004d750(longlong *param_1,undefined8 *param_2,longlong *param_3,char param_4);
char * FUN_18004da70(char *param_1,undefined8 *param_2);
longlong *FUN_18004db00(longlong *param_1,longlong *param_2,undefined1 (*param_3) [16],longlong *param_4,char param_5,char param_6);
undefined8 * FUN_18004de50(undefined8 *param_1,longlong *param_2,longlong *param_3);
longlong * FUN_18004df90(longlong *param_1,longlong *param_2,longlong *param_3);
void FUN_18004e110(undefined8 *param_1,byte param_2,longlong *param_3,char param_4);
undefined8 * FUN_18004e760(undefined8 *param_1,byte *param_2,byte *param_3);
char * FUN_18004e930(char *param_1,undefined8 *param_2);
undefined8 FUN_18004e9c0(longlong *param_1,longlong *param_2,int *param_3);
undefined8 FUN_18004ec30(char *param_1,char *param_2,__uint64 param_3,char *param_4,char param_5);
undefined8 FUN_18004f590(char *******param_1,__uint64 *param_2);
ulonglong FUN_180051880(char *param_1,char ************param_2,char *param_3,char ************param_4,undefined8 *******param_5,char param_6,char param_7);
undefined8 FUN_180053710(char *******param_1,undefined1 (*param_2) [32],byte *param_3,undefined8 param_4,byte param_5,char *param_6);
undefined8 aescriptsAddBlockedSerial(__int64 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5);
undefined4 aescriptsCheckOveruse(longlong param_1);
undefined8 aescriptsConfigure(int param_1,int param_2);
void aescriptsDropLicenseFromLicenseServer(char *param_1,__int64 param_2);
undefined8 aescriptsGetLicLibVersion(void);
ulonglong aescriptsGetLicenseDataFromFile(byte *param_1,__int64 param_2,undefined8 *******param_3);
undefined4 aescriptsGetLicenseServerConfig(char *param_1,char *param_2,char *param_3,char *param_4);
ulonglong aescriptsGetMachineId(undefined8 *param_1);
ulonglong aescriptsGetRawLicenseDataFromFile(byte *param_1,void *param_2);
undefined8 aescriptsInitLicenseData(void *param_1);
undefined8 aescriptsLoadLicenseFromFile(void);
void aescriptsRequestLicenseFromLicenseServer(char *param_1,char *param_2,__int64 param_3,char *param_4);
void aescriptsSaveLicenseToFile(char *param_1,char *param_2,char *param_3,char *param_4);
ulonglong aescriptsValidateLicense(char *param_1,char ************param_2,__int64 param_3,undefined8 *******param_4,char param_5);
int FUN_1800576a0(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void __cdecl std::_Xlength_error(char *param_1);
void _Thrd_join(void);
void _Thrd_hardware_concurrency(void);
void _Thrd_id(void);
void _Cnd_do_broadcast_at_thread_exit(void);
void __cdecl std::_Throw_Cpp_error(int param_1);
bool __cdecl std::uncaught_exception(void);
ulonglong FUN_180057730(undefined1 (*param_1) [16],ulonglong param_2,undefined1 (*param_3) [16],ulonglong param_4);
undefined1 (*) [32]FUN_1800578a0(undefined1 (*param_1) [32],undefined1 (*param_2) [32],undefined1 (*param_3) [32],ulonglong param_4);
undefined1 (*) [32]FUN_180057c40(undefined1 (*param_1) [32],undefined1 (*param_2) [32],undefined1 (*param_3) [32],ulonglong param_4);
undefined1 (*) [32]FUN_180058000(undefined1 (*param_1) [32],undefined1 (*param_2) [32],byte param_3);
undefined1 (*) [32]FUN_180058100(undefined1 (*param_1) [32],undefined1 (*param_2) [32],byte param_3);
longlong FUN_180058220(longlong param_1,ulonglong param_2,byte *param_3,ulonglong param_4);
undefined1 (*) [16]FUN_180058450(undefined1 (*param_1) [16],ulonglong param_2,undefined1 (*param_3) [16],ulonglong param_4);
char FUN_180058630(ulonglong param_1,ulonglong param_2,char param_3);
undefined1 (*) [32]FUN_1800586c0(undefined1 (*param_1) [32],undefined1 (*param_2) [32],undefined1 (*param_3) [16],ulonglong param_4);
ulonglong FUN_1800588b0(undefined8 *param_1,uint param_2);
undefined1 (*) [32]thunk_FUN_180057c40(undefined1 (*param_1) [32],undefined1 (*param_2) [32],undefined1 (*param_3) [32],ulonglong param_4);
ulonglong thunk_FUN_180057730(undefined1 (*param_1) [16],ulonglong param_2,undefined1 (*param_3) [16],ulonglong param_4);
undefined1 (*) [32]thunk_FUN_180058000(undefined1 (*param_1) [32],undefined1 (*param_2) [32],byte param_3);
undefined1 (*) [32]thunk_FUN_1800586c0(undefined1 (*param_1) [32],undefined1 (*param_2) [32],undefined1 (*param_3) [16],ulonglong param_4);
void __cdecl std::_Xout_of_range(char *param_1);
void _Mtx_lock(void);
void _Mtx_unlock(void);
void FUN_180058990(undefined8 param_1);
_Locimp * __cdecl std::locale::_Getgloballocale(void);
__int64 __thiscall std::basic_streambuf<>::xsgetn(basic_streambuf<> *this,char *param_1,__int64 param_2);
__int64 __thiscall std::basic_streambuf<>::xsputn(basic_streambuf<> *this,char *param_1,__int64 param_2);
_iobuf * __cdecl std::_Fiopen(char *param_1,int param_2,int param_3);
void __thiscall std::basic_streambuf<>::imbue(basic_streambuf<> *this,locale *param_1);
int __thiscall std::basic_streambuf<>::sync(basic_streambuf<> *this);
basic_streambuf<> * __thiscall std::basic_streambuf<>::setbuf(basic_streambuf<> *this,char *param_1,__int64 param_2);
int __thiscall std::basic_streambuf<>::uflow(basic_streambuf<> *this);
__int64 __thiscall std::basic_streambuf<>::showmanyc(basic_streambuf<> *this);
void __thiscall std::basic_streambuf<>::_Unlock(basic_streambuf<> *this);
void __thiscall std::basic_streambuf<>::_Lock(basic_streambuf<> *this);
void _Query_perf_counter(void);
void _Query_perf_frequency(void);
void FUN_180058a20(void);
void __cdecl std::_Xbad_alloc(void);
void _Thrd_detach(void);
void _Mtx_init_in_situ(void);
void __cdecl std::setw(__int64 param_1);
undefined8 * FUN_180058a50(undefined8 *param_1,ulonglong param_2);
void FUN_180058a80(size_t param_1);
void __cdecl free(void *_Memory);
void thunk_FUN_180058a80(size_t param_1);
void FUN_180058af0(size_t param_1);
void FUN_180058b10(void);
undefined8 FUN_180058b20(undefined8 param_1,ulonglong param_2,undefined8 param_3,longlong param_4);
uint FUN_180058b40(ulonglong param_1,longlong param_2,uint *param_3,undefined8 param_4,undefined1 param_5,longlong *param_6);
undefined8 FUN_180058d20(longlong param_1,ulonglong param_2,undefined8 param_3,longlong param_4);
void __chkstk(void);
void __cdecl __security_check_cookie(uintptr_t _StackCookie);
void FUN_180058e20(longlong param_1,longlong param_2,longlong param_3,undefined *param_4);
void FUN_180058ea0(longlong param_1,longlong param_2,longlong param_3,undefined *param_4);
void FUN_180058f20(longlong param_1,longlong param_2,longlong param_3,undefined *param_4);
ulonglong FUN_180058f80(void);
undefined8 FUN_180058fd0(void);
uint FUN_180059010(void);
undefined1 FUN_180059090(void);
undefined1 FUN_1800590c0(void);
void FUN_1800590e0(undefined8 param_1,int param_2,undefined8 param_3,undefined *param_4,undefined4 param_5,undefined8 param_6);
void FUN_180059130(void);
void FUN_180059170(void);
longlong FUN_180059190(int param_1);
ulonglong FUN_1800591d0(longlong param_1);
undefined8 FUN_180059270(char param_1);
undefined1 FUN_1800592a0(undefined8 param_1,char param_2);
int FUN_1800592d0(longlong param_1);
void FUN_180059310(undefined4 *param_1);
void _Init_thread_footer(int *param_1);
void FUN_1800593c0(int *param_1);
ulonglong FUN_180059460(undefined8 param_1,int param_2,longlong param_3);
undefined8 FUN_1800594c0(undefined8 param_1,undefined8 param_2);
ulonglong FUN_1800595d0(byte param_1);
ulonglong FUN_180059670(HMODULE param_1,int param_2,longlong param_3);
void entry(HMODULE param_1,int param_2,longlong param_3);
void __cdecl free(void *_Memory);
void FUN_180059800(void);
undefined8 FUN_180059810(void);
void _guard_check_icall(void);
undefined8 * FUN_180059ad0(undefined8 *param_1);
void FUN_180059b00(void);
void __cdecl free(void *_Memory);
bool FUN_180059b30(void);
void FUN_180059b40(undefined4 param_1);
void FUN_180059b50(void);
undefined8 FUN_180059c10(HMODULE param_1,int param_2);
void FUN_180059c40(void);
void FUN_180059c50(void);
undefined * FUN_180059c60(void);
void FUN_180059c70(void);
undefined * FUN_180059c90(void);
void FUN_180059ca0(void);
void FUN_180059cf0(void);
void __CxxFrameHandler4(void);
void * __cdecl memset(void *_Dst,int _Val,size_t _Size);
void __stdcall _CxxThrowException(void *pExceptionObject,ThrowInfo *pThrowInfo);
void __std_exception_copy(void);
void __std_exception_destroy(void);
int __cdecl memcmp(void *_Buf1,void *_Buf2,size_t _Size);
void * __cdecl memcpy(void *_Dst,void *_Src,size_t _Size);
void * __cdecl memmove(void *_Dst,void *_Src,size_t _Size);
void _purecall(void);
void __current_exception(void);
void __current_exception_context(void);
void __std_type_info_destroy_list(void);
void __cdecl free(void *_Memory);
void terminate(void);
float __cdecl cosf(float _X);
float __cdecl expf(float _X);
float __cdecl floorf(float _X);
float __cdecl fmodf(float _X,float _Y);
float __cdecl powf(float _X,float _Y);
float __cdecl sinf(float _X);
float __cdecl sqrtf(float _X);
char * __cdecl strcpy(char *_Dest,char *_Source);
size_t __cdecl strlen(char *_Str);
int __cdecl strncmp(char *_Str1,char *_Str2,size_t _MaxCount);
char * __cdecl strncpy(char *_Dest,char *_Source,size_t _Count);
void * __cdecl malloc(size_t _Size);
int __cdecl strcmp(char *_Str1,char *_Str2);
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
int __stdcall __WSAFDIsSet(SOCKET param_1,fd_set *param_2);
void inet_ntop(void);
undefined1 FUN_180059e40(void);
void _guard_dispatch_icall(void);
void _guard_dispatch_icall(void);
undefined8 FUN_180059f20(undefined8 param_1,longlong param_2);
undefined8 FUN_180059f50(undefined8 param_1,longlong param_2);
undefined8 FUN_180059fa0(undefined8 param_1,longlong param_2);
undefined8 FUN_18005a000(undefined8 param_1,longlong param_2);
undefined8 FUN_18005a060(void);
undefined8 FUN_18005a140(void);
void FUN_18005a1d0(undefined8 param_1,longlong param_2);
void FUN_18005a200(undefined8 param_1,longlong param_2);
void FUN_18005a250(undefined8 param_1,longlong param_2);
undefined8 FUN_18005a3d0(void);
void FUN_18005a420(undefined8 param_1,longlong param_2);
void FUN_18005a470(undefined8 param_1,longlong param_2);
void FUN_18005a4d0(undefined8 param_1,longlong param_2);
undefined8 FUN_18005a520(undefined8 param_1,longlong param_2);
void FUN_18005a5d0(undefined8 param_1,longlong param_2);
void FUN_18005a630(undefined8 param_1,longlong param_2);
undefined8 FUN_18005a650(undefined8 param_1,longlong param_2);
undefined8 FUN_18005a710(undefined8 param_1,longlong param_2);
void FUN_18005a810(undefined8 param_1,longlong param_2);
void FUN_18005a870(undefined8 param_1,longlong param_2);
void FUN_18005a8c0(undefined8 param_1,longlong param_2);
void FUN_18005a920(undefined8 param_1,longlong param_2);
undefined8 FUN_18005a9d0(void);
void FUN_18005aac0(undefined8 param_1,longlong param_2);
void FUN_18005ab40(undefined8 param_1,longlong param_2);
void FUN_18005abc0(undefined8 param_1,longlong param_2);
undefined8 FUN_18005abf0(undefined8 param_1,longlong param_2);
void FUN_18005ad10(undefined8 param_1,longlong param_2);
void FUN_18005ad40(undefined8 param_1,longlong param_2);
void FUN_18005ad70(undefined8 param_1,longlong param_2);
void FUN_18005aeb0(undefined8 param_1,longlong param_2);
void FUN_18005aee0(undefined8 param_1,longlong param_2);
undefined8 FUN_18005af30(void);
void FUN_18005af60(undefined8 param_1,longlong param_2);
void FUN_18005afc0(undefined8 param_1,longlong param_2);
undefined8 FUN_18005b000(undefined8 param_1,longlong param_2);
void FUN_18005b120(undefined8 param_1,longlong param_2);
void FUN_18005b190(undefined8 param_1,longlong param_2);
void FUN_18005b1c0(undefined8 param_1,longlong param_2);
void FUN_18005b200(undefined8 param_1,longlong param_2);
void FUN_18005b260(undefined8 param_1,longlong param_2);
void FUN_18005b2b0(undefined8 param_1,longlong param_2);
void FUN_18005b2e0(undefined8 param_1,longlong param_2);
void FUN_18005b320(undefined8 param_1,longlong param_2);
void FUN_18005b350(undefined8 param_1,longlong param_2);
void FUN_18005b3d0(undefined8 param_1,longlong param_2);
void FUN_18005b400(undefined8 param_1,longlong param_2);
void FUN_18005b440(undefined8 param_1,longlong param_2);
void FUN_18005b480(undefined8 param_1,longlong param_2);
void FUN_18005b4b0(undefined8 param_1,longlong param_2);
void FUN_18005b4e0(undefined8 param_1,longlong param_2);
void FUN_18005b510(undefined8 param_1,longlong param_2);
void FUN_18005b540(undefined8 param_1,longlong param_2);
void FUN_18005b570(undefined8 param_1,longlong param_2);
undefined8 FUN_18005b5f0(undefined8 param_1,longlong param_2);
undefined8 FUN_18005b620(undefined8 param_1,longlong param_2);
undefined8 FUN_18005b650(void);
void FUN_18005b680(undefined8 param_1,longlong param_2);
void FUN_18005b6c0(undefined8 param_1,longlong param_2);
undefined8 FUN_18005b730(undefined8 param_1,longlong param_2);
void FUN_18005b780(undefined8 param_1,longlong param_2);
void FUN_18005b8d0(undefined8 param_1,longlong param_2);
void FUN_18005ba70(undefined8 param_1,longlong param_2);
void FUN_18005baa0(undefined8 param_1,longlong param_2);
void FUN_18005bad0(undefined8 param_1,longlong param_2);
void FUN_18005bb10(undefined8 param_1,longlong param_2);
void FUN_18005bb60(undefined8 param_1,longlong param_2);
void FUN_18005bbc0(undefined8 param_1,longlong param_2);
void FUN_18005bbf0(undefined8 param_1,longlong param_2);
undefined8 FUN_18005bc20(undefined8 param_1,longlong param_2);
void FUN_18005bc50(undefined8 param_1,longlong param_2);
void FUN_18005bc90(undefined8 param_1,longlong param_2);
void FUN_18005bd90(undefined8 param_1,longlong param_2);
undefined8 FUN_18005bdc0(undefined8 param_1,longlong param_2);
undefined8 FUN_18005bdf0(undefined8 param_1,longlong param_2);
undefined8 FUN_18005be20(undefined8 param_1,longlong param_2);
void FUN_18005be50(undefined8 param_1,longlong param_2);
undefined8 FUN_18005be80(undefined8 param_1,longlong param_2);
undefined8 FUN_18005beb0(undefined8 param_1,longlong param_2);
undefined8 FUN_18005bee0(undefined8 param_1,longlong param_2);
undefined8 FUN_18005bf10(undefined8 param_1,longlong param_2);
undefined8 FUN_18005bf40(void);
void FUN_18005bf60(undefined8 param_1,longlong param_2);
void FUN_18005bf90(undefined8 param_1,longlong param_2);
undefined4 FUN_18005bfc0(undefined8 param_1,longlong param_2);
bool FUN_18005c020(undefined8 *param_1);
void FUN_18005c040(undefined8 param_1,longlong param_2);
void FUN_18005c060(undefined8 param_1,longlong param_2);
void FUN_18005c07a(void);
void FUN_18005c090(undefined8 *param_1,longlong param_2);
void FUN_18005c0d0(void);
void FUN_18005c1a0(void);
void FUN_18005c250(void);
void FUN_18005c340(void);

