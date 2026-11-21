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

typedef union _LARGE_INTEGER _LARGE_INTEGER, *P_LARGE_INTEGER;

typedef union _LARGE_INTEGER LARGE_INTEGER;

typedef DWORD (*LPPROGRESS_ROUTINE)(LARGE_INTEGER, LARGE_INTEGER, LARGE_INTEGER, LARGE_INTEGER, DWORD, DWORD, HANDLE, HANDLE, LPVOID);

typedef struct _struct_19 _struct_19, *P_struct_19;

typedef struct _struct_20 _struct_20, *P_struct_20;

typedef longlong LONGLONG;

typedef long LONG;

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

typedef struct _OVERLAPPED *LPOVERLAPPED;

typedef struct _BY_HANDLE_FILE_INFORMATION _BY_HANDLE_FILE_INFORMATION, *P_BY_HANDLE_FILE_INFORMATION;

typedef struct _FILETIME _FILETIME, *P_FILETIME;

typedef struct _FILETIME FILETIME;

struct _BY_HANDLE_FILE_INFORMATION {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD dwVolumeSerialNumber;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD nNumberOfLinks;
    DWORD nFileIndexHigh;
    DWORD nFileIndexLow;
};

struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
};

typedef struct _SECURITY_ATTRIBUTES *LPSECURITY_ATTRIBUTES;

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

typedef struct _BY_HANDLE_FILE_INFORMATION *LPBY_HANDLE_FILE_INFORMATION;

typedef enum _GET_FILEEX_INFO_LEVELS GET_FILEEX_INFO_LEVELS;

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

typedef uchar BYTE;

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

typedef PTOP_LEVEL_EXCEPTION_FILTER LPTOP_LEVEL_EXCEPTION_FILTER;

typedef struct IMalloc IMalloc, *PIMalloc;

typedef struct IMallocVtbl IMallocVtbl, *PIMallocVtbl;

struct IMalloc {
    struct IMallocVtbl *lpVtbl;
};

struct IMallocVtbl {
    HRESULT (*QueryInterface)(struct IMalloc *, IID *, void **);
    ULONG (*AddRef)(struct IMalloc *);
    ULONG (*Release)(struct IMalloc *);
    void * (*Alloc)(struct IMalloc *, SIZE_T);
    void * (*Realloc)(struct IMalloc *, void *, SIZE_T);
    void (*Free)(struct IMalloc *, void *);
    SIZE_T (*GetSize)(struct IMalloc *, void *);
    int (*DidAlloc)(struct IMalloc *, void *);
    void (*HeapMinimize)(struct IMalloc *);
};

typedef enum _EXCEPTION_DISPOSITION {
    ExceptionContinueExecution=0,
    ExceptionContinueSearch=1,
    ExceptionNestedException=2,
    ExceptionCollidedUnwind=3
} _EXCEPTION_DISPOSITION;

typedef enum _EXCEPTION_DISPOSITION EXCEPTION_DISPOSITION;

typedef DWORD ULONG;

typedef longlong fpos_t;

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

typedef char *va_list;

typedef ulonglong uintptr_t;

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

typedef struct _ITEMIDLIST _ITEMIDLIST, *P_ITEMIDLIST;

typedef struct _SHITEMID _SHITEMID, *P_SHITEMID;

typedef struct _SHITEMID SHITEMID;

typedef ushort USHORT;

struct _ITEMIDLIST {
    SHITEMID mkid;
};

struct _SHITEMID {
    USHORT cb;
    BYTE abID[1];
};

typedef struct _ITEMIDLIST ITEMIDLIST;

typedef ITEMIDLIST *LPCITEMIDLIST;

typedef ITEMIDLIST *LPITEMIDLIST;

typedef struct exception exception, *Pexception;

struct exception { // PlaceHolder Class Structure
};

typedef struct basic_filebuf<char,std::char_traits<char>_> basic_filebuf<char,std::char_traits<char>_>, *Pbasic_filebuf<char,std::char_traits<char>_>;

struct basic_filebuf<char,std::char_traits<char>_> { // PlaceHolder Class Structure
};


// WARNING! conflicting data type names: /guiddef.h/GUID - /GUID

typedef GUID IID;

typedef struct _GUID _GUID, *P_GUID;

struct _GUID {
    ulong Data1;
    ushort Data2;
    ushort Data3;
    uchar Data4[8];
};

typedef PVOID BCRYPT_KEY_HANDLE;

typedef PVOID BCRYPT_HANDLE;

typedef PVOID BCRYPT_ALG_HANDLE;

typedef LONG NTSTATUS;

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

typedef wchar_t WCHAR;

typedef WCHAR *LPWSTR;

typedef struct _RUNTIME_FUNCTION _RUNTIME_FUNCTION, *P_RUNTIME_FUNCTION;

struct _RUNTIME_FUNCTION {
    DWORD BeginAddress;
    DWORD EndAddress;
    DWORD UnwindData;
};

typedef CHAR *LPSTR;

typedef struct _RUNTIME_FUNCTION *PRUNTIME_FUNCTION;

typedef EXCEPTION_DISPOSITION (EXCEPTION_ROUTINE)(struct _EXCEPTION_RECORD *, PVOID, struct _CONTEXT *, PVOID);

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

typedef long HRESULT;

typedef CHAR *LPCSTR;

typedef struct _MEMORY_BASIC_INFORMATION *PMEMORY_BASIC_INFORMATION;

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

typedef struct _CRYPTOAPI_BLOB _CRYPTOAPI_BLOB, *P_CRYPTOAPI_BLOB;

typedef struct _CRYPTOAPI_BLOB CRYPT_OBJID_BLOB;

struct _CRYPTOAPI_BLOB {
    DWORD cbData;
    BYTE *pbData;
};

typedef struct _CRYPT_BIT_BLOB _CRYPT_BIT_BLOB, *P_CRYPT_BIT_BLOB;

typedef struct _CRYPT_BIT_BLOB CRYPT_BIT_BLOB;

struct _CRYPT_BIT_BLOB {
    DWORD cbData;
    BYTE *pbData;
    DWORD cUnusedBits;
};

typedef ULONG_PTR HCRYPTKEY;

typedef struct _CERT_PUBLIC_KEY_INFO _CERT_PUBLIC_KEY_INFO, *P_CERT_PUBLIC_KEY_INFO;

typedef struct _CRYPT_ALGORITHM_IDENTIFIER _CRYPT_ALGORITHM_IDENTIFIER, *P_CRYPT_ALGORITHM_IDENTIFIER;

typedef struct _CRYPT_ALGORITHM_IDENTIFIER CRYPT_ALGORITHM_IDENTIFIER;

struct _CERT_PUBLIC_KEY_INFO {
    CRYPT_ALGORITHM_IDENTIFIER Algorithm;
    CRYPT_BIT_BLOB PublicKey;
};

struct _CRYPT_ALGORITHM_IDENTIFIER {
    LPSTR pszObjId;
    CRYPT_OBJID_BLOB Parameters;
};

typedef struct _CERT_PUBLIC_KEY_INFO *PCERT_PUBLIC_KEY_INFO;

typedef uint ALG_ID;

typedef struct _CRYPT_DECODE_PARA _CRYPT_DECODE_PARA, *P_CRYPT_DECODE_PARA;

typedef LPVOID (*PFN_CRYPT_ALLOC)(size_t);

typedef void (*PFN_CRYPT_FREE)(LPVOID);

struct _CRYPT_DECODE_PARA {
    DWORD cbSize;
    PFN_CRYPT_ALLOC pfnAlloc;
    PFN_CRYPT_FREE pfnFree;
};

typedef ULONG_PTR HCRYPTPROV;

typedef struct _CRYPT_DECODE_PARA *PCRYPT_DECODE_PARA;

typedef ULONG_PTR HCRYPTHASH;

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

typedef struct HGLRC__ HGLRC__, *PHGLRC__;

struct HGLRC__ {
    int unused;
};


// WARNING! conflicting data type names: /WinDef.h/ULONG - /wtypes.h/ULONG

typedef struct _FILETIME *LPFILETIME;

typedef INT_PTR (*FARPROC)(void);

typedef struct HGLRC__ *HGLRC;

typedef struct HDC__ HDC__, *PHDC__;

typedef struct HDC__ *HDC;

struct HDC__ {
    int unused;
};

typedef uchar UCHAR;

typedef UCHAR *PUCHAR;

typedef DWORD *LPDWORD;

typedef int INT;

typedef struct HINSTANCE__ HINSTANCE__, *PHINSTANCE__;

struct HINSTANCE__ {
    int unused;
};

typedef DWORD *PDWORD;

typedef BOOL *LPBOOL;

typedef uint *PUINT;

typedef void *LPCVOID;

typedef struct HINSTANCE__ *HINSTANCE;

typedef struct HWND__ HWND__, *PHWND__;

typedef struct HWND__ *HWND;

struct HWND__ {
    int unused;
};

typedef HINSTANCE HMODULE;

typedef HANDLE HLOCAL;

typedef long *LPLONG;

typedef uint UINT;

typedef struct Var Var, *PVar;

struct Var {
    word wLength;
    word wValueLength;
    word wType;
};

typedef struct IMAGE_RESOURCE_DIRECTORY_ENTRY_NameStruct IMAGE_RESOURCE_DIRECTORY_ENTRY_NameStruct, *PIMAGE_RESOURCE_DIRECTORY_ENTRY_NameStruct;

struct IMAGE_RESOURCE_DIRECTORY_ENTRY_NameStruct {
    dword NameOffset:31;
    dword NameIsString:1;
};

typedef struct IMAGE_IMPORT_BY_NAME_97 IMAGE_IMPORT_BY_NAME_97, *PIMAGE_IMPORT_BY_NAME_97;

struct IMAGE_IMPORT_BY_NAME_97 {
    word Hint;
    char Name[97];
};

typedef struct IMAGE_LOAD_CONFIG_CODE_INTEGRITY IMAGE_LOAD_CONFIG_CODE_INTEGRITY, *PIMAGE_LOAD_CONFIG_CODE_INTEGRITY;

struct IMAGE_LOAD_CONFIG_CODE_INTEGRITY {
    word Flags;
    word Catalog;
    dword CatalogOffset;
    dword Reserved;
};

typedef struct IMAGE_IMPORT_BY_NAME_11 IMAGE_IMPORT_BY_NAME_11, *PIMAGE_IMPORT_BY_NAME_11;

struct IMAGE_IMPORT_BY_NAME_11 {
    word Hint;
    char Name[11];
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

typedef struct IMAGE_IMPORT_BY_NAME_13 IMAGE_IMPORT_BY_NAME_13, *PIMAGE_IMPORT_BY_NAME_13;

struct IMAGE_IMPORT_BY_NAME_13 {
    word Hint;
    char Name[13];
};

typedef struct IMAGE_IMPORT_BY_NAME_17 IMAGE_IMPORT_BY_NAME_17, *PIMAGE_IMPORT_BY_NAME_17;

struct IMAGE_IMPORT_BY_NAME_17 {
    word Hint;
    char Name[17];
};

typedef struct IMAGE_IMPORT_BY_NAME_19 IMAGE_IMPORT_BY_NAME_19, *PIMAGE_IMPORT_BY_NAME_19;

struct IMAGE_IMPORT_BY_NAME_19 {
    word Hint;
    char Name[19];
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

typedef struct IMAGE_IMPORT_BY_NAME_23 IMAGE_IMPORT_BY_NAME_23, *PIMAGE_IMPORT_BY_NAME_23;

struct IMAGE_IMPORT_BY_NAME_23 {
    word Hint;
    char Name[23];
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

typedef struct IMAGE_IMPORT_BY_NAME_22 IMAGE_IMPORT_BY_NAME_22, *PIMAGE_IMPORT_BY_NAME_22;

struct IMAGE_IMPORT_BY_NAME_22 {
    word Hint;
    char Name[22];
};

typedef struct IMAGE_IMPORT_BY_NAME_25 IMAGE_IMPORT_BY_NAME_25, *PIMAGE_IMPORT_BY_NAME_25;

struct IMAGE_IMPORT_BY_NAME_25 {
    word Hint;
    char Name[25];
};

typedef struct IMAGE_IMPORT_BY_NAME_24 IMAGE_IMPORT_BY_NAME_24, *PIMAGE_IMPORT_BY_NAME_24;

struct IMAGE_IMPORT_BY_NAME_24 {
    word Hint;
    char Name[24];
};

typedef struct IMAGE_IMPORT_BY_NAME_27 IMAGE_IMPORT_BY_NAME_27, *PIMAGE_IMPORT_BY_NAME_27;

struct IMAGE_IMPORT_BY_NAME_27 {
    word Hint;
    char Name[27];
};

typedef struct IMAGE_IMPORT_BY_NAME_26 IMAGE_IMPORT_BY_NAME_26, *PIMAGE_IMPORT_BY_NAME_26;

struct IMAGE_IMPORT_BY_NAME_26 {
    word Hint;
    char Name[26];
};

typedef struct IMAGE_IMPORT_BY_NAME_28 IMAGE_IMPORT_BY_NAME_28, *PIMAGE_IMPORT_BY_NAME_28;

struct IMAGE_IMPORT_BY_NAME_28 {
    word Hint;
    char Name[28];
};

typedef struct _IMAGE_RUNTIME_FUNCTION_ENTRY _IMAGE_RUNTIME_FUNCTION_ENTRY, *P_IMAGE_RUNTIME_FUNCTION_ENTRY;

struct _IMAGE_RUNTIME_FUNCTION_ENTRY {
    ImageBaseOffset32 BeginAddress;
    dword EndAddress; // Apply ImageBaseOffset32 to see reference
    ImageBaseOffset32 UnwindInfoAddressOrData;
};

typedef struct IMAGE_IMPORT_BY_NAME_82 IMAGE_IMPORT_BY_NAME_82, *PIMAGE_IMPORT_BY_NAME_82;

struct IMAGE_IMPORT_BY_NAME_82 {
    word Hint;
    char Name[82];
};

typedef struct IMAGE_IMPORT_BY_NAME_85 IMAGE_IMPORT_BY_NAME_85, *PIMAGE_IMPORT_BY_NAME_85;

struct IMAGE_IMPORT_BY_NAME_85 {
    word Hint;
    char Name[85];
};

typedef struct IMAGE_IMPORT_BY_NAME_76 IMAGE_IMPORT_BY_NAME_76, *PIMAGE_IMPORT_BY_NAME_76;

struct IMAGE_IMPORT_BY_NAME_76 {
    word Hint;
    char Name[76];
};

typedef struct IMAGE_IMPORT_BY_NAME_75 IMAGE_IMPORT_BY_NAME_75, *PIMAGE_IMPORT_BY_NAME_75;

struct IMAGE_IMPORT_BY_NAME_75 {
    word Hint;
    char Name[75];
};

typedef struct IMAGE_IMPORT_BY_NAME_78 IMAGE_IMPORT_BY_NAME_78, *PIMAGE_IMPORT_BY_NAME_78;

struct IMAGE_IMPORT_BY_NAME_78 {
    word Hint;
    char Name[78];
};

typedef struct IMAGE_IMPORT_BY_NAME_79 IMAGE_IMPORT_BY_NAME_79, *PIMAGE_IMPORT_BY_NAME_79;

struct IMAGE_IMPORT_BY_NAME_79 {
    word Hint;
    char Name[79];
};

typedef struct IMAGE_IMPORT_BY_NAME_9 IMAGE_IMPORT_BY_NAME_9, *PIMAGE_IMPORT_BY_NAME_9;

struct IMAGE_IMPORT_BY_NAME_9 {
    word Hint;
    char Name[9];
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

typedef struct IMAGE_IMPORT_BY_NAME_90 IMAGE_IMPORT_BY_NAME_90, *PIMAGE_IMPORT_BY_NAME_90;

struct IMAGE_IMPORT_BY_NAME_90 {
    word Hint;
    char Name[90];
};

typedef struct IMAGE_IMPORT_BY_NAME_92 IMAGE_IMPORT_BY_NAME_92, *PIMAGE_IMPORT_BY_NAME_92;

struct IMAGE_IMPORT_BY_NAME_92 {
    word Hint;
    char Name[92];
};

typedef struct VarFileInfo VarFileInfo, *PVarFileInfo;

struct VarFileInfo {
    word wLength;
    word wValueLength;
    word wType;
};

typedef struct StringInfo StringInfo, *PStringInfo;

struct StringInfo {
    word wLength;
    word wValueLength;
    word wType;
};

typedef struct IMAGE_IMPORT_BY_NAME_61 IMAGE_IMPORT_BY_NAME_61, *PIMAGE_IMPORT_BY_NAME_61;

struct IMAGE_IMPORT_BY_NAME_61 {
    word Hint;
    char Name[61];
};

typedef struct IMAGE_IMPORT_BY_NAME_60 IMAGE_IMPORT_BY_NAME_60, *PIMAGE_IMPORT_BY_NAME_60;

struct IMAGE_IMPORT_BY_NAME_60 {
    word Hint;
    char Name[60];
};

typedef struct IMAGE_IMPORT_BY_NAME_63 IMAGE_IMPORT_BY_NAME_63, *PIMAGE_IMPORT_BY_NAME_63;

struct IMAGE_IMPORT_BY_NAME_63 {
    word Hint;
    char Name[63];
};

typedef struct IMAGE_IMPORT_BY_NAME_62 IMAGE_IMPORT_BY_NAME_62, *PIMAGE_IMPORT_BY_NAME_62;

struct IMAGE_IMPORT_BY_NAME_62 {
    word Hint;
    char Name[62];
};

typedef struct IMAGE_IMPORT_BY_NAME_54 IMAGE_IMPORT_BY_NAME_54, *PIMAGE_IMPORT_BY_NAME_54;

struct IMAGE_IMPORT_BY_NAME_54 {
    word Hint;
    char Name[54];
};

typedef struct IMAGE_IMPORT_BY_NAME_53 IMAGE_IMPORT_BY_NAME_53, *PIMAGE_IMPORT_BY_NAME_53;

struct IMAGE_IMPORT_BY_NAME_53 {
    word Hint;
    char Name[53];
};

typedef struct IMAGE_IMPORT_BY_NAME_56 IMAGE_IMPORT_BY_NAME_56, *PIMAGE_IMPORT_BY_NAME_56;

struct IMAGE_IMPORT_BY_NAME_56 {
    word Hint;
    char Name[56];
};

typedef struct IMAGE_IMPORT_BY_NAME_55 IMAGE_IMPORT_BY_NAME_55, *PIMAGE_IMPORT_BY_NAME_55;

struct IMAGE_IMPORT_BY_NAME_55 {
    word Hint;
    char Name[55];
};

typedef struct IMAGE_IMPORT_BY_NAME_58 IMAGE_IMPORT_BY_NAME_58, *PIMAGE_IMPORT_BY_NAME_58;

struct IMAGE_IMPORT_BY_NAME_58 {
    word Hint;
    char Name[58];
};

typedef struct IMAGE_IMPORT_BY_NAME_57 IMAGE_IMPORT_BY_NAME_57, *PIMAGE_IMPORT_BY_NAME_57;

struct IMAGE_IMPORT_BY_NAME_57 {
    word Hint;
    char Name[57];
};

typedef struct IMAGE_IMPORT_BY_NAME_59 IMAGE_IMPORT_BY_NAME_59, *PIMAGE_IMPORT_BY_NAME_59;

struct IMAGE_IMPORT_BY_NAME_59 {
    word Hint;
    char Name[59];
};

typedef struct StringTable StringTable, *PStringTable;

struct StringTable {
    word wLength;
    word wValueLength;
    word wType;
};

typedef struct IMAGE_RESOURCE_DIR_STRING_U_8 IMAGE_RESOURCE_DIR_STRING_U_8, *PIMAGE_RESOURCE_DIR_STRING_U_8;

struct IMAGE_RESOURCE_DIR_STRING_U_8 {
    word Length;
    wchar16 NameString[4];
};

typedef struct IMAGE_IMPORT_BY_NAME_70 IMAGE_IMPORT_BY_NAME_70, *PIMAGE_IMPORT_BY_NAME_70;

struct IMAGE_IMPORT_BY_NAME_70 {
    word Hint;
    char Name[70];
};

typedef struct IMAGE_IMPORT_BY_NAME_72 IMAGE_IMPORT_BY_NAME_72, *PIMAGE_IMPORT_BY_NAME_72;

struct IMAGE_IMPORT_BY_NAME_72 {
    word Hint;
    char Name[72];
};

typedef struct IMAGE_IMPORT_BY_NAME_71 IMAGE_IMPORT_BY_NAME_71, *PIMAGE_IMPORT_BY_NAME_71;

struct IMAGE_IMPORT_BY_NAME_71 {
    word Hint;
    char Name[71];
};

typedef struct IMAGE_IMPORT_BY_NAME_73 IMAGE_IMPORT_BY_NAME_73, *PIMAGE_IMPORT_BY_NAME_73;

struct IMAGE_IMPORT_BY_NAME_73 {
    word Hint;
    char Name[73];
};

typedef struct IMAGE_IMPORT_BY_NAME_65 IMAGE_IMPORT_BY_NAME_65, *PIMAGE_IMPORT_BY_NAME_65;

struct IMAGE_IMPORT_BY_NAME_65 {
    word Hint;
    char Name[65];
};

typedef struct IMAGE_IMPORT_BY_NAME_64 IMAGE_IMPORT_BY_NAME_64, *PIMAGE_IMPORT_BY_NAME_64;

struct IMAGE_IMPORT_BY_NAME_64 {
    word Hint;
    char Name[64];
};

typedef struct IMAGE_IMPORT_BY_NAME_67 IMAGE_IMPORT_BY_NAME_67, *PIMAGE_IMPORT_BY_NAME_67;

struct IMAGE_IMPORT_BY_NAME_67 {
    word Hint;
    char Name[67];
};

typedef struct IMAGE_IMPORT_BY_NAME_66 IMAGE_IMPORT_BY_NAME_66, *PIMAGE_IMPORT_BY_NAME_66;

struct IMAGE_IMPORT_BY_NAME_66 {
    word Hint;
    char Name[66];
};

typedef struct IMAGE_IMPORT_BY_NAME_69 IMAGE_IMPORT_BY_NAME_69, *PIMAGE_IMPORT_BY_NAME_69;

struct IMAGE_IMPORT_BY_NAME_69 {
    word Hint;
    char Name[69];
};

typedef struct IMAGE_IMPORT_BY_NAME_68 IMAGE_IMPORT_BY_NAME_68, *PIMAGE_IMPORT_BY_NAME_68;

struct IMAGE_IMPORT_BY_NAME_68 {
    word Hint;
    char Name[68];
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

typedef struct IMAGE_IMPORT_BY_NAME_102 IMAGE_IMPORT_BY_NAME_102, *PIMAGE_IMPORT_BY_NAME_102;

struct IMAGE_IMPORT_BY_NAME_102 {
    word Hint;
    char Name[102];
};

typedef struct IMAGE_IMPORT_BY_NAME_101 IMAGE_IMPORT_BY_NAME_101, *PIMAGE_IMPORT_BY_NAME_101;

struct IMAGE_IMPORT_BY_NAME_101 {
    word Hint;
    char Name[101];
};

typedef struct IMAGE_IMPORT_BY_NAME_41 IMAGE_IMPORT_BY_NAME_41, *PIMAGE_IMPORT_BY_NAME_41;

struct IMAGE_IMPORT_BY_NAME_41 {
    word Hint;
    char Name[41];
};

typedef struct IMAGE_IMPORT_BY_NAME_40 IMAGE_IMPORT_BY_NAME_40, *PIMAGE_IMPORT_BY_NAME_40;

struct IMAGE_IMPORT_BY_NAME_40 {
    word Hint;
    char Name[40];
};

typedef struct IMAGE_IMPORT_BY_NAME_31 IMAGE_IMPORT_BY_NAME_31, *PIMAGE_IMPORT_BY_NAME_31;

struct IMAGE_IMPORT_BY_NAME_31 {
    word Hint;
    char Name[31];
};

typedef struct IMAGE_IMPORT_BY_NAME_38 IMAGE_IMPORT_BY_NAME_38, *PIMAGE_IMPORT_BY_NAME_38;

struct IMAGE_IMPORT_BY_NAME_38 {
    word Hint;
    char Name[38];
};

typedef struct IMAGE_IMPORT_BY_NAME_100 IMAGE_IMPORT_BY_NAME_100, *PIMAGE_IMPORT_BY_NAME_100;

struct IMAGE_IMPORT_BY_NAME_100 {
    word Hint;
    char Name[100];
};

typedef struct IMAGE_IMPORT_BY_NAME_37 IMAGE_IMPORT_BY_NAME_37, *PIMAGE_IMPORT_BY_NAME_37;

struct IMAGE_IMPORT_BY_NAME_37 {
    word Hint;
    char Name[37];
};

typedef struct IMAGE_IMPORT_BY_NAME_39 IMAGE_IMPORT_BY_NAME_39, *PIMAGE_IMPORT_BY_NAME_39;

struct IMAGE_IMPORT_BY_NAME_39 {
    word Hint;
    char Name[39];
};

typedef struct StringFileInfo StringFileInfo, *PStringFileInfo;

struct StringFileInfo {
    word wLength;
    word wValueLength;
    word wType;
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

typedef struct VS_VERSION_INFO VS_VERSION_INFO, *PVS_VERSION_INFO;

struct VS_VERSION_INFO {
    word StructLength;
    word ValueLength;
    word StructType;
    wchar16 Info[16];
    byte Padding[2];
    dword Signature;
    word StructVersion[2];
    word FileVersion[4];
    word ProductVersion[4];
    dword FileFlagsMask[2];
    dword FileFlags;
    dword FileOS;
    dword FileType;
    dword FileSubtype;
    dword FileTimestamp;
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

typedef struct IMAGE_IMPORT_BY_NAME_50 IMAGE_IMPORT_BY_NAME_50, *PIMAGE_IMPORT_BY_NAME_50;

struct IMAGE_IMPORT_BY_NAME_50 {
    word Hint;
    char Name[50];
};

typedef struct IMAGE_IMPORT_BY_NAME_116 IMAGE_IMPORT_BY_NAME_116, *PIMAGE_IMPORT_BY_NAME_116;

struct IMAGE_IMPORT_BY_NAME_116 {
    word Hint;
    char Name[116];
};

typedef struct IMAGE_IMPORT_BY_NAME_52 IMAGE_IMPORT_BY_NAME_52, *PIMAGE_IMPORT_BY_NAME_52;

struct IMAGE_IMPORT_BY_NAME_52 {
    word Hint;
    char Name[52];
};

typedef struct IMAGE_IMPORT_BY_NAME_51 IMAGE_IMPORT_BY_NAME_51, *PIMAGE_IMPORT_BY_NAME_51;

struct IMAGE_IMPORT_BY_NAME_51 {
    word Hint;
    char Name[51];
};

typedef struct IMAGE_IMPORT_BY_NAME_43 IMAGE_IMPORT_BY_NAME_43, *PIMAGE_IMPORT_BY_NAME_43;

struct IMAGE_IMPORT_BY_NAME_43 {
    word Hint;
    char Name[43];
};

typedef struct IMAGE_IMPORT_BY_NAME_42 IMAGE_IMPORT_BY_NAME_42, *PIMAGE_IMPORT_BY_NAME_42;

struct IMAGE_IMPORT_BY_NAME_42 {
    word Hint;
    char Name[42];
};

typedef struct IMAGE_IMPORT_BY_NAME_45 IMAGE_IMPORT_BY_NAME_45, *PIMAGE_IMPORT_BY_NAME_45;

struct IMAGE_IMPORT_BY_NAME_45 {
    word Hint;
    char Name[45];
};

typedef struct IMAGE_IMPORT_BY_NAME_44 IMAGE_IMPORT_BY_NAME_44, *PIMAGE_IMPORT_BY_NAME_44;

struct IMAGE_IMPORT_BY_NAME_44 {
    word Hint;
    char Name[44];
};

typedef struct IMAGE_IMPORT_BY_NAME_47 IMAGE_IMPORT_BY_NAME_47, *PIMAGE_IMPORT_BY_NAME_47;

struct IMAGE_IMPORT_BY_NAME_47 {
    word Hint;
    char Name[47];
};

typedef struct IMAGE_IMPORT_BY_NAME_46 IMAGE_IMPORT_BY_NAME_46, *PIMAGE_IMPORT_BY_NAME_46;

struct IMAGE_IMPORT_BY_NAME_46 {
    word Hint;
    char Name[46];
};

typedef struct IMAGE_IMPORT_BY_NAME_111 IMAGE_IMPORT_BY_NAME_111, *PIMAGE_IMPORT_BY_NAME_111;

struct IMAGE_IMPORT_BY_NAME_111 {
    word Hint;
    char Name[111];
};

typedef struct IMAGE_IMPORT_BY_NAME_49 IMAGE_IMPORT_BY_NAME_49, *PIMAGE_IMPORT_BY_NAME_49;

struct IMAGE_IMPORT_BY_NAME_49 {
    word Hint;
    char Name[49];
};

typedef struct IMAGE_IMPORT_BY_NAME_48 IMAGE_IMPORT_BY_NAME_48, *PIMAGE_IMPORT_BY_NAME_48;

struct IMAGE_IMPORT_BY_NAME_48 {
    word Hint;
    char Name[48];
};

typedef struct _Mbstatet _Mbstatet, *P_Mbstatet;

struct _Mbstatet { // PlaceHolder Structure
};

typedef struct CAtlBaseModule CAtlBaseModule, *PCAtlBaseModule;

struct CAtlBaseModule { // PlaceHolder Structure
};

typedef struct _Yarn<char> _Yarn<char>, *P_Yarn<char>;

struct _Yarn<char> { // PlaceHolder Structure
};

typedef struct codecvt<wchar_t,char,_Mbstatet> codecvt<wchar_t,char,_Mbstatet>, *Pcodecvt<wchar_t,char,_Mbstatet>;

struct codecvt<wchar_t,char,_Mbstatet> { // PlaceHolder Structure
};

typedef struct basic_istream<char,struct_std::char_traits<char>_> basic_istream<char,struct_std::char_traits<char>_>, *Pbasic_istream<char,struct_std::char_traits<char>_>;

struct basic_istream<char,struct_std::char_traits<char>_> { // PlaceHolder Structure
};

typedef struct codecvt_base codecvt_base, *Pcodecvt_base;

struct codecvt_base { // PlaceHolder Structure
};

typedef struct optional<float> optional<float>, *Poptional<float>;

struct optional<float> { // PlaceHolder Structure
};

typedef struct basic_string<char,std::char_traits<char>,std::allocator<char>_> basic_string<char,std::char_traits<char>,std::allocator<char>_>, *Pbasic_string<char,std::char_traits<char>,std::allocator<char>_>;

struct basic_string<char,std::char_traits<char>,std::allocator<char>_> { // PlaceHolder Structure
};

typedef struct locale locale, *Plocale;

struct locale { // PlaceHolder Structure
};

typedef struct basic_istream<char,std::char_traits<char>_> basic_istream<char,std::char_traits<char>_>, *Pbasic_istream<char,std::char_traits<char>_>;

struct basic_istream<char,std::char_traits<char>_> { // PlaceHolder Structure
};

typedef struct _Fac_tidy_reg_t _Fac_tidy_reg_t, *P_Fac_tidy_reg_t;

struct _Fac_tidy_reg_t { // PlaceHolder Structure
};

typedef struct vector<CHN*___ptr64,std::allocator<CHN*___ptr64>_> vector<CHN*___ptr64,std::allocator<CHN*___ptr64>_>, *Pvector<CHN*___ptr64,std::allocator<CHN*___ptr64>_>;

struct vector<CHN*___ptr64,std::allocator<CHN*___ptr64>_> { // PlaceHolder Structure
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

typedef struct vector<unsigned___int64,std::allocator<unsigned___int64>_> vector<unsigned___int64,std::allocator<unsigned___int64>_>, *Pvector<unsigned___int64,std::allocator<unsigned___int64>_>;

struct vector<unsigned___int64,std::allocator<unsigned___int64>_> { // PlaceHolder Structure
};

typedef struct vector<char_const*___ptr64,std::allocator<char_const*___ptr64>_> vector<char_const*___ptr64,std::allocator<char_const*___ptr64>_>, *Pvector<char_const*___ptr64,std::allocator<char_const*___ptr64>_>;

struct vector<char_const*___ptr64,std::allocator<char_const*___ptr64>_> { // PlaceHolder Structure
};

typedef struct ios_base ios_base, *Pios_base;

struct ios_base { // PlaceHolder Structure
};

typedef struct basic_ios<char,std::char_traits<char>_> basic_ios<char,std::char_traits<char>_>, *Pbasic_ios<char,std::char_traits<char>_>;

struct basic_ios<char,std::char_traits<char>_> { // PlaceHolder Structure
};

typedef struct vector<unsigned___int64,std::_Parallelism_allocator<unsigned___int64>_> vector<unsigned___int64,std::_Parallelism_allocator<unsigned___int64>_>, *Pvector<unsigned___int64,std::_Parallelism_allocator<unsigned___int64>_>;

struct vector<unsigned___int64,std::_Parallelism_allocator<unsigned___int64>_> { // PlaceHolder Structure
};

typedef struct _Facet_base _Facet_base, *P_Facet_base;

struct _Facet_base { // PlaceHolder Structure
};

typedef struct priority_queue<unsigned___int64,std::vector<unsigned___int64,std::_Parallelism_allocator<unsigned___int64>_>,std::greater<void>_> priority_queue<unsigned___int64,std::vector<unsigned___int64,std::_Parallelism_allocator<unsigned___int64>_>,std::greater<void>_>, *Ppriority_queue<unsigned___int64,std::vector<unsigned___int64,std::_Parallelism_allocator<unsigned___int64>_>,std::greater<void>_>;

struct priority_queue<unsigned___int64,std::vector<unsigned___int64,std::_Parallelism_allocator<unsigned___int64>_>,std::greater<void>_> { // PlaceHolder Structure
};

typedef struct basic_ostream<char,struct_std::char_traits<char>_> basic_ostream<char,struct_std::char_traits<char>_>, *Pbasic_ostream<char,struct_std::char_traits<char>_>;

struct basic_ostream<char,struct_std::char_traits<char>_> { // PlaceHolder Structure
};

typedef struct lock_guard<std::recursive_mutex> lock_guard<std::recursive_mutex>, *Plock_guard<std::recursive_mutex>;

struct lock_guard<std::recursive_mutex> { // PlaceHolder Structure
};

typedef struct basic_ostream<char,std::char_traits<char>_> basic_ostream<char,std::char_traits<char>_>, *Pbasic_ostream<char,std::char_traits<char>_>;

struct basic_ostream<char,std::char_traits<char>_> { // PlaceHolder Structure
};

typedef struct codecvt<char,char,_Mbstatet> codecvt<char,char,_Mbstatet>, *Pcodecvt<char,char,_Mbstatet>;

struct codecvt<char,char,_Mbstatet> { // PlaceHolder Structure
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

typedef struct vec<4,int,0> vec<4,int,0>, *Pvec<4,int,0>;

struct vec<4,int,0> { // PlaceHolder Structure
};

typedef struct vec<3,float,0> vec<3,float,0>, *Pvec<3,float,0>;

struct vec<3,float,0> { // PlaceHolder Structure
};

typedef struct mat<4,4,float,0> mat<4,4,float,0>, *Pmat<4,4,float,0>;

struct mat<4,4,float,0> { // PlaceHolder Structure
};

typedef struct String String, *PString;

struct String { // PlaceHolder Structure
};

typedef struct StackBaseString<char> StackBaseString<char>, *PStackBaseString<char>;

struct StackBaseString<char> { // PlaceHolder Structure
};

typedef struct Renderer Renderer, *PRenderer;

struct Renderer { // PlaceHolder Structure
};

typedef struct Gpu_Engine Gpu_Engine, *PGpu_Engine;

struct Gpu_Engine { // PlaceHolder Structure
};

typedef struct Gpu_ShaderContext_API Gpu_ShaderContext_API, *PGpu_ShaderContext_API;

struct Gpu_ShaderContext_API { // PlaceHolder Structure
};

typedef struct Gpu_ImageBuffer Gpu_ImageBuffer, *PGpu_ImageBuffer;

struct Gpu_ImageBuffer { // PlaceHolder Structure
};

typedef enum MatrixMode {
} MatrixMode;

typedef struct uiDialogPreset uiDialogPreset, *PuiDialogPreset;

struct uiDialogPreset { // PlaceHolder Structure
};

typedef struct Vector3 Vector3, *PVector3;

struct Vector3 { // PlaceHolder Structure
};

typedef struct Vector2 Vector2, *PVector2;

struct Vector2 { // PlaceHolder Structure
};

typedef enum DataType {
} DataType;

typedef int (*_onexit_t)(void);




void FUN_180001000(void);
void FUN_180001030(void);
void FUN_180001070(void);
void FUN_1800010a0(void);
void FUN_1800010f0(void);
void FUN_1800014a0(void);
void FUN_1800014e0(void);
void FUN_1800018f0(void);
void FUN_180001960(void);
void FUN_1800019a0(void);
void FUN_180001a00(void);
void FUN_180001a90(void);
void FUN_180001b20(void);
void FUN_180001c00(void);
void FUN_180001c60(void);
void FUN_180001cc0(void);
void FUN_180001d40(void);
void FUN_180001db0(void);
void FUN_180001e30(void);
void FUN_180001e90(void);
void FUN_180001ef0(void);
void FUN_180001f70(void);
void FUN_180002010(void);
void FUN_180002080(void);
void FUN_1800020d0(void);
void FUN_180002150(void);
void FUN_1800021b0(void);
void FUN_180002220(void);
void FUN_180002280(void);
void FUN_1800022e0(void);
void FUN_180002340(void);
void FUN_1800023b0(void);
void FUN_180002420(void);
void FUN_180002480(void);
void FUN_1800024f0(void);
void FUN_180002550(void);
void FUN_1800025e0(void);
void FUN_180002650(void);
void FUN_1800026d0(void);
void FUN_180002790(void);
void FUN_1800027f0(void);
void FUN_180002820(void);
void FUN_180002bf8(void);
void FUN_180002c20(longlong *param_1);
undefined8 FUN_180002c70(void);
int FUN_180002c80(int *param_1);
void FUN_180002cf0(longlong param_1,undefined8 *param_2);
bool FUN_180002d60(undefined8 param_1,Gpu_ImageBuffer *param_2,undefined8 param_3,uint param_4);
bool FUN_180003730(undefined8 param_1,Gpu_ImageBuffer *param_2,uint param_3);
undefined8 * FUN_180003960(undefined8 *param_1,undefined8 *param_2);
void FUN_1800039f0(longlong param_1);
void FUN_180003a40(longlong *param_1);
void FUN_180003aa0(undefined8 param_1,void *param_2,ulonglong param_3);
void FUN_180003ae0(undefined8 param_1,void *param_2,longlong param_3);
void FUN_180003b20(longlong *param_1);
void FUN_180003b40(longlong param_1);
undefined8 * FUN_180003b50(undefined8 *param_1,ulonglong param_2);
bool FUN_180003b80(void);
void FUN_180003c30(LPCVOID param_1,ulonglong param_2);
void FUN_180003cd0(DWORD param_1,PDWORD param_2);
void FUN_180003dd0(void);
void FUN_180003e80(void);
undefined8 * FUN_180003f20(undefined8 *param_1,longlong param_2);
char * FUN_180003f80(longlong param_1);
undefined8 * FUN_180003fa0(undefined8 *param_1,ulonglong param_2);
undefined8 * FUN_180003ff0(undefined8 *param_1);
void FUN_180004020(void);
undefined8 * FUN_180004040(undefined8 *param_1,longlong param_2);
undefined8 * FUN_180004080(undefined8 *param_1,longlong param_2);
void FUN_1800040c0(void);
undefined8 * FUN_1800040e0(undefined8 *param_1,void *param_2);
void FUN_180004120(undefined8 *param_1,void *param_2,size_t param_3);
undefined8 * FUN_180004230(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_180004290(undefined8 *param_1,longlong param_2);
undefined4 * FUN_1800042d0(undefined8 param_1,undefined4 *param_2,undefined4 param_3);
undefined8 FUN_1800042e0(longlong *param_1,undefined4 param_2,int *param_3);
longlong FUN_180004320(longlong param_1,int *param_2,int param_3);
undefined4 * FUN_180004340(undefined4 *param_1);
undefined8 * FUN_180004360(undefined8 *param_1,ulonglong param_2);
undefined8 *** FUN_1800043b0(undefined8 ***param_1,ulonglong *param_2);
void FUN_1800044b0(void);
undefined8 * FUN_1800044f0(undefined8 *param_1,longlong param_2);
undefined8 * FUN_180004550(undefined8 *param_1,longlong param_2);
char * FUN_1800045b0(void);
undefined8 * FUN_1800045c0(undefined8 param_1,undefined8 *param_2,int param_3);
void * FUN_180004610(void *param_1,ulonglong param_2);
undefined1 * FUN_180004640(void);
undefined8 * FUN_180004650(undefined8 param_1,undefined8 *param_2,DWORD param_3);
int * FUN_1800046f0(undefined8 param_1,int *param_2,int param_3);
undefined4 * FUN_180004770(undefined4 *param_1,undefined4 param_2);
void FUN_180004790(undefined4 param_1);
uint * FUN_1800047d0(uint *param_1,uint *param_2);
uint * FUN_1800048d0(uint *param_1,uint *param_2,ulonglong param_3);
LPSTR FUN_180004af0(LPCWSTR param_1,LPSTR param_2);
uint * FUN_180004be0(uint *param_1,undefined8 *param_2,uint *param_3);
longlong * FUN_180004c30(longlong *param_1,longlong *param_2,undefined8 param_3);
void FUN_180004cb0(longlong *param_1);
undefined8 *** FUN_180004d20(undefined8 ***param_1);
char * FUN_1800058b0(char *param_1);
ImgDelayDescr * FUN_1800058d0(void);
FARPROC FUN_180005990(uint *param_1,longlong *param_2);
undefined8 FUN_180005da0(void);
void thunk_FUN_180006250(longlong *param_1);
ulonglong * FUN_180005e40(ulonglong *param_1,longlong *param_2);
void FUN_180005f10(undefined8 *param_1,ulonglong param_2);
void FUN_180005f90(undefined8 *param_1,undefined8 param_2);
longlong * FUN_180005fd0(longlong *param_1,void *param_2,ulonglong param_3);
undefined8 * FUN_180006040(undefined8 *param_1,void *param_2,ulonglong param_3);
undefined8 * FUN_1800060d0(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_180006200(undefined8 *param_1);
undefined8 * FUN_180006210(undefined8 *param_1,undefined8 *param_2);
void FUN_180006250(longlong *param_1);
void FUN_1800062c0(undefined8 *param_1,ulonglong param_2,char param_3);
void FUN_180006360(undefined8 param_1,void *param_2,longlong param_3);
void FUN_1800063b0(void);
ulonglong FUN_1800063d0(undefined8 param_1,size_t *param_2);
undefined8 *FUN_180006450(undefined8 *param_1,undefined8 param_2,undefined8 param_3,undefined2 param_4);
longlong * FUN_180006600(longlong *param_1,ulonglong param_2,undefined8 param_3,void *param_4);
undefined8 *FUN_180006780(undefined8 *param_1,ulonglong param_2,undefined8 param_3,void *param_4,longlong param_5);
void FUN_180006960(undefined8 *param_1,undefined8 *param_2,ulonglong param_3);
void FUN_180006a60(longlong *param_1,longlong *param_2);
undefined8 *FUN_180006af0(undefined8 *param_1,ulonglong param_2,undefined8 param_3,longlong param_4);
undefined8 *FUN_180006cc0(undefined8 *param_1,ulonglong param_2,undefined8 param_3,size_t param_4,char param_5);
ulonglong FUN_180006e60(undefined8 param_1,ulonglong param_2);
void FUN_180006ec0(void);
void FUN_180006ee0(ulonglong *param_1,ulonglong param_2);
undefined8 * FUN_180006f70(longlong *param_1,longlong *param_2,undefined8 *param_3);
void FUN_180007110(longlong *param_1,longlong param_2,longlong param_3,longlong param_4);
LPWSTR FUN_1800071b0(LPWSTR param_1,undefined8 *param_2);
longlong * FUN_1800072b0(longlong *param_1,longlong *param_2,longlong *param_3);
longlong * FUN_180007330(longlong *param_1,uint param_2);
undefined * FUN_1800073b0(void);
void FUN_1800073c0(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
undefined8 * FUN_180007410(undefined8 *param_1);
undefined8 * FUN_180007430(undefined8 *param_1,uint param_2);
undefined8 * FUN_180007520(undefined8 *param_1,undefined8 param_2);
undefined8 * FUN_180007570(undefined8 *param_1);
undefined8 * FUN_1800075c0(undefined8 *param_1,longlong param_2);
void FUN_180007600(void);
void FUN_180007620(longlong param_1);
void FUN_180007640(longlong *param_1);
void FUN_1800076a0(longlong *param_1);
undefined8 FUN_180007700(longlong param_1,longlong *param_2);
undefined8 FUN_180007850(void);
undefined8 * FUN_180007860(undefined8 *param_1,ulonglong param_2);
undefined8 * FUN_1800078a0(undefined8 *param_1,ulonglong param_2);
void FUN_1800078d0(char *param_1);
undefined1 * FUN_180007960(undefined1 *param_1,undefined1 *param_2);
longlong FUN_180007ae0(longlong param_1,undefined8 *param_2,undefined8 *param_3);
undefined1 * FUN_180007c00(undefined1 *param_1,longlong *param_2);
void FUN_180007cf0(longlong *param_1);
undefined4 * FUN_180007da0(undefined4 *param_1,undefined8 *param_2);
undefined4 * FUN_180007e10(undefined4 *param_1,longlong *param_2);
void FUN_180007e90(int *param_1);
void FUN_180007f30(longlong *param_1);
int * FUN_180007f60(int *param_1,int *param_2,undefined8 param_3,undefined8 param_4);
int * FUN_180008070(int *param_1);
undefined8 FUN_1800080b0(int *param_1);
undefined8 FUN_1800080f0(int *param_1);
int * FUN_180008130(int *param_1);
undefined * FUN_180008170(int *param_1,undefined8 *param_2);
bool FUN_180008260(int *param_1,undefined8 *param_2);
longlong * FUN_1800082f0(undefined4 *param_1,longlong *param_2);
undefined8 FUN_1800086f0(undefined8 *param_1,undefined1 param_2);
longlong * FUN_1800087a0(longlong *param_1,undefined8 param_2,byte *param_3);
undefined8 FUN_180008920(longlong *param_1,int *param_2);
void _guard_check_icall(void);
void FUN_180008d60(undefined8 param_1,undefined8 param_2,LPCSTR param_3);
undefined8 * FUN_180008da0(undefined8 *param_1);
void FUN_180008f60(longlong *param_1);
int FUN_180008f90(int param_1,undefined8 *param_2,undefined8 *param_3,ulonglong *param_4);
undefined8 * FUN_180009410(undefined8 *param_1,LPCSTR param_2);
undefined1 FUN_1800095e0(void);
undefined8 FUN_1800095f0(longlong *param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
longlong * FUN_1800097e0(longlong *param_1,longlong *param_2,undefined8 param_3,undefined8 param_4);
undefined8 * FUN_1800099d0(undefined8 *param_1,ulonglong param_2);
undefined1 FUN_180009a00(longlong param_1,longlong *param_2);
longlong FUN_180009a60(longlong param_1,undefined8 *param_2,undefined8 *param_3);
longlong FUN_180009b70(longlong param_1,undefined8 *param_2,longlong *param_3,undefined8 param_4);
undefined8 * FUN_180009d30(undefined8 *param_1);
void FUN_180009d90(undefined8 *param_1);
undefined8 * FUN_180009dd0(undefined8 *param_1,uint param_2);
undefined8 FUN_180009e40(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void FUN_180009ea0(undefined8 *param_1);
longlong * FUN_180009fc0(longlong param_1);
void FUN_180009fd0(longlong param_1,longlong *param_2);
undefined8 * FUN_180009ff0(undefined8 *param_1,undefined8 *param_2,undefined2 param_3);
undefined8 * FUN_18000a020(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_18000a050(undefined8 *param_1,undefined8 *param_2,undefined2 param_3);
undefined8 * FUN_18000a080(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_18000a0b0(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_18000a0e0(undefined8 *param_1,undefined8 *param_2,undefined2 param_3);
undefined8 * FUN_18000a110(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_18000a140(undefined8 *param_1,undefined8 *param_2,undefined2 param_3);
undefined8 * FUN_18000a170(undefined8 *param_1,undefined8 *param_2,undefined2 param_3);
undefined8 * FUN_18000a1a0(undefined8 *param_1,undefined8 *param_2,undefined2 param_3);
undefined8 * FUN_18000a1d0(undefined8 *param_1,undefined8 *param_2,undefined2 param_3);
undefined8 *FUN_18000a200(undefined8 *param_1,undefined4 param_2,undefined8 *param_3,undefined2 param_4);
undefined8 * FUN_18000a4c0(undefined8 *param_1,uint param_2);
undefined8 * FUN_18000a500(undefined8 *param_1,longlong param_2);
void FUN_18000a5f0(undefined8 *param_1);
ulonglong FUN_18000a690(longlong param_1,longlong *param_2,undefined1 *param_3);
undefined8 * FUN_18000aaf0(undefined8 *param_1,uint param_2);
bool FUN_18000ab30(undefined8 param_1,longlong param_2,undefined8 *param_3,longlong *param_4);
bool FUN_18000ad50(undefined8 param_1,longlong param_2,undefined8 *param_3,longlong *param_4);
void FUN_18000af70(longlong *param_1);
longlong * FUN_18000afa0(longlong *param_1);
void FUN_18000b010(longlong param_1);
undefined8 FUN_18000b040(longlong param_1,longlong *param_2);
undefined8 FUN_18000b330(longlong param_1,int *param_2);
undefined8 FUN_18000b490(longlong param_1,longlong *param_2,undefined8 param_3,undefined8 param_4);
undefined8 FUN_18000bac0(longlong param_1,int *param_2,undefined8 param_3,undefined8 param_4);
void FUN_18000c2e0(undefined8 *param_1);
undefined8 * FUN_18000c3a0(undefined8 *param_1,uint param_2);
void FUN_18000c3e0(undefined8 *param_1);
undefined8 FUN_18000c600(longlong param_1,longlong *param_2);
undefined8 FUN_18000d150(longlong param_1,int *param_2);
undefined8 * FUN_18000dc20(undefined8 *param_1,uint param_2);
undefined8 FUN_18000dc60(longlong param_1,longlong *param_2);
undefined8 FUN_18000e1b0(longlong param_1,int *param_2);
undefined8 FUN_18000e940(longlong param_1,longlong *param_2);
undefined8 FUN_18000edb0(longlong param_1,int *param_2);
undefined8 FUN_18000efe0(longlong param_1,longlong *param_2);
undefined8 FUN_18000f5c0(longlong param_1,int *param_2);
void FUN_18000fda0(undefined8 *param_1);
void FUN_18000fde0(undefined8 *param_1);
void FUN_18000fea0(undefined8 *param_1);
void FUN_18000fee0(undefined8 *param_1);
undefined8 * FUN_18000fff0(undefined8 *param_1);
undefined1 FUN_180010120(char *param_1);
__time64_t * FUN_1800101a0(__time64_t *param_1,longlong *param_2,longlong param_3);
void FUN_1800103b0(char *param_1);
void FUN_180010470(undefined2 *param_1,undefined8 *param_2,longlong *param_3,undefined8 ****param_4);
void FUN_1800108f0(undefined8 *param_1);
void FUN_180010cf0(undefined1 *param_1);
undefined8 FUN_180010e90(longlong param_1,undefined8 *param_2,longlong *param_3);
undefined8 FUN_180011180(longlong param_1,char *param_2,undefined8 param_3);
void FUN_180011630(longlong param_1,undefined1 param_2);
void FUN_180011660(longlong param_1,undefined4 param_2);
undefined8 FUN_180011680(longlong param_1,undefined1 *param_2);
undefined8 FUN_180011890(longlong param_1,undefined8 param_2);
undefined8 FUN_1800118a0(longlong param_1,int param_2,undefined8 *param_3);
undefined8 FUN_180011a30(longlong param_1,undefined8 *param_2);
undefined8 FUN_180011bb0(longlong param_1,longlong *param_2);
undefined8 FUN_180011be0(undefined8 param_1,undefined8 ****param_2,LPSTR *****param_3,undefined8 param_4);
undefined8 * FUN_180011d60(undefined8 *param_1,uint param_2);
void FUN_180011e00(longlong param_1);
undefined8 * FUN_180011e70(void);
undefined8 * FUN_180011fb0(undefined8 *param_1,void *param_2,undefined8 *param_3);
undefined8 * FUN_180012020(undefined8 *param_1,ulonglong param_2,void *param_3,ulonglong param_4);
undefined8 *FUN_180012120(undefined8 *param_1,ulonglong param_2,undefined8 param_3,size_t param_4,void *param_5,size_t param_6);
longlong * FUN_1800122d0(longlong *param_1,longlong *param_2,void *param_3);
longlong * FUN_180012340(longlong *param_1,void *param_2,ulonglong param_3);
undefined8 *FUN_1800123d0(undefined8 *param_1,ulonglong param_2,undefined8 param_3,void *param_4,size_t param_5);
void FUN_180012560(undefined4 *param_1);
void FUN_180012650(longlong param_1);
undefined4 * FUN_180012720(undefined4 *param_1,undefined4 *param_2);
undefined8 * FUN_1800128c0(undefined8 param_1,undefined8 *param_2);
longlong FUN_180012900(undefined8 param_1,longlong param_2);
undefined1 FUN_180012910(void);
undefined8 FUN_180012930(void);
undefined8 * FUN_180012950(undefined8 *param_1,ulonglong param_2);
undefined8 * FUN_180012990(undefined8 *param_1,ulonglong param_2);
undefined8 FUN_180012a00(void);
undefined8 FUN_180012a10(void);
undefined8 FUN_180012a40(void);
longlong FUN_180012a50(longlong param_1,char *param_2);
undefined8 FUN_180012b20(undefined8 param_1,longlong param_2);
bool FUN_180012b50(undefined8 param_1,byte *param_2);
char * FUN_180012b80(undefined8 param_1,char *param_2);
undefined8 * FUN_180012c70(undefined8 *param_1,uint param_2);
undefined8 FUN_180012d00(undefined8 param_1,undefined1 *param_2,undefined1 *param_3,undefined1 *param_4,undefined1 *param_5,undefined1 *param_6,undefined1 *param_7);
undefined8 FUN_180012d30(longlong param_1,undefined8 param_2,undefined8 param_3,undefined4 *param_4);
double FUN_180012da0(longlong param_1);
double FUN_180012dc0(longlong param_1);
undefined8 FUN_180012de0(longlong param_1);
undefined8 FUN_180012df0(longlong param_1);
undefined8 FUN_180012e00(void);
undefined8 FUN_180012e10(longlong param_1,undefined8 param_2,undefined4 *param_3);
undefined8 FUN_180012e30(longlong param_1,undefined8 param_2,undefined4 *param_3);
undefined8 FUN_180012e80(longlong param_1,undefined4 *param_2,undefined4 *param_3);
undefined8 * FUN_180012ea0(undefined8 *param_1,ulonglong param_2);
undefined4 FUN_180012f30(longlong param_1);
void FUN_180012f40(longlong param_1,undefined8 param_2);
undefined8 FUN_180012f60(longlong param_1,undefined8 param_2,int *param_3);
undefined8 FUN_180013050(longlong param_1,undefined8 param_2,float *param_3);
undefined8 FUN_180013130(longlong param_1,undefined8 param_2,undefined1 *param_3);
undefined8 FUN_1800131f0(longlong param_1,undefined8 param_2,float *param_3,float *param_4);
undefined8 FUN_1800132c0(longlong param_1,undefined8 param_2,float *param_3,float *param_4,float *param_5);
undefined8 FUN_1800133d0(longlong param_1);
undefined8 FUN_1800133e0(longlong param_1,undefined8 param_2,undefined4 *param_3);
undefined8 FUN_1800133f0(longlong param_1,undefined4 param_2,undefined8 param_3);
undefined8 FUN_180013480(longlong param_1,undefined8 param_2,longlong *param_3,uint *param_4,undefined8 param_5,longlong *param_6);
void FUN_180013610(longlong *param_1,double param_2);
undefined8 FUN_180013660(longlong *param_1,float *param_2);
longlong FUN_1800136d0(longlong param_1,char *param_2);
void FUN_180013880(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
char * FUN_1800138b0(longlong param_1,char *param_2);
double FUN_1800139c0(longlong param_1);
undefined8 FUN_1800139e0(longlong param_1);
double FUN_1800139f0(longlong param_1);
undefined8 FUN_180013a10(void);
undefined4 FUN_180013a20(longlong param_1);
undefined4 FUN_180013a30(longlong param_1);
double FUN_180013a40(longlong param_1);
undefined8 * FUN_180013a60(undefined8 *param_1,uint param_2);
undefined8 * FUN_180013af0(undefined8 *param_1,void *param_2);
void FUN_180013ca0(undefined8 *param_1);
void FUN_180013d10(undefined8 *param_1);
undefined8 FUN_180014640(longlong param_1,longlong *param_2);
ulonglong FUN_180014870(longlong param_1,longlong *param_2,int *param_3,undefined4 *param_4);
ulonglong FUN_180014d20(longlong param_1,undefined8 *param_2,undefined **param_3,longlong param_4,longlong *param_5);
ulonglong FUN_180015870(longlong param_1,undefined4 param_2,int param_3);
void FUN_1800159c0(longlong param_1,undefined4 param_2);
longlong * FUN_180015a60(longlong param_1,longlong *param_2,longlong param_3,longlong param_4);
void FUN_180015d30(longlong param_1,undefined4 *param_2,undefined8 *param_3);
undefined8 * FUN_180016090(undefined8 *param_1,ulonglong param_2);
void FUN_1800160c0(longlong param_1);
undefined8 xGPUFilterEntry(undefined8 param_1,int *param_2,int param_3,longlong param_4,undefined8 *param_5,undefined4 *param_6);
void FUN_180016440(longlong param_1);
ulonglong FUN_180016470(longlong *param_1);
void FUN_180016550(longlong *param_1);
void FUN_180016600(undefined8 *param_1);
void FUN_1800166c0(undefined8 *param_1);
undefined8 *FUN_1800167a0(longlong param_1,undefined8 *param_2,LPSTR *****param_3,undefined8 param_4);
undefined8 * FUN_180017680(longlong param_1,undefined8 *param_2,void **param_3,undefined1 *param_4);
undefined8 *FUN_1800181f0(longlong param_1,undefined8 *param_2,longlong *param_3,longlong *param_4,longlong *******param_5);
undefined8 *FUN_180019110(longlong param_1,undefined8 *param_2,LPSTR *****param_3,longlong *param_4,undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8,longlong *param_9,longlong *param_10,longlong *param_11);
longlong *FUN_180019ed0(longlong param_1,void *param_2,longlong *param_3,undefined8 param_4,undefined8 *param_5,char param_6,undefined8 *param_7);
void FUN_18001aba0(longlong param_1);
undefined8 * FUN_18001ade0(undefined8 *param_1,longlong param_2);
void FUN_18001ae70(undefined8 *param_1);
longlong * FUN_18001aea0(longlong *param_1,undefined8 *param_2);
longlong * FUN_18001afa0(longlong *param_1,longlong *param_2);
longlong * FUN_18001b090(longlong *param_1,longlong *param_2);
longlong * FUN_18001b0f0(longlong *param_1,longlong *param_2);
longlong * FUN_18001b150(longlong *param_1);
ulonglong *FUN_18001b1a0(ulonglong *param_1,longlong *param_2,undefined8 param_3,undefined8 param_4);
void FUN_18001b2b0(longlong *param_1);
basic_iostream<> * FUN_18001b320(basic_iostream<> *param_1);
longlong * FUN_18001b3e0(longlong *param_1);
ulonglong * FUN_18001b420(longlong param_1,ulonglong *param_2,longlong *param_3,byte param_4);
ulonglong *FUN_18001b550(longlong param_1,ulonglong *param_2,longlong param_3,int param_4,byte param_5);
ulonglong FUN_18001b6e0(longlong param_1);
int FUN_18001b750(longlong param_1,int param_2);
int FUN_18001b7b0(longlong param_1,int param_2);
void FUN_18001b9a0(basic_streambuf<> *param_1);
bool FUN_18001ba90(longlong param_1);
longlong * FUN_18001baa0(longlong *param_1,longlong *param_2);
longlong * FUN_18001bad0(longlong *param_1,longlong *param_2);
undefined8 * FUN_18001bb60(undefined8 *param_1);
longlong FUN_18001bb80(longlong param_1,longlong param_2);
longlong FUN_18001bc30(longlong param_1,longlong param_2);
void FUN_18001bcd0(longlong param_1);
void FUN_18001bd70(longlong *param_1);
undefined8 * FUN_18001be30(undefined8 *param_1);
void FUN_18001bf20(longlong *param_1);
undefined8 * FUN_18001c0d0(undefined8 *param_1,uint param_2);
void FUN_18001c110(undefined8 *param_1);
undefined8 * FUN_18001c180(undefined8 *param_1,uint param_2);
void FUN_18001c1c0(undefined8 *param_1);
undefined8 * FUN_18001c290(undefined8 *param_1,uint param_2);
basic_ios<> * FUN_18001c3a0(basic_ios<> *param_1,uint param_2);
basic_streambuf<> * FUN_18001c440(basic_streambuf<> *param_1,uint param_2);
basic_ios<> * FUN_18001c480(basic_ios<> *param_1,uint param_2);
undefined8 * FUN_18001c520(undefined8 *param_1,uint param_2);
undefined8 * FUN_18001c560(undefined8 *param_1,uint param_2);
void FUN_18001c5a0(longlong *param_1);
LPSTR ****** FUN_18001c5e0(undefined8 param_1,LPSTR ******param_2);
undefined8 * FUN_18001c9a0(undefined8 param_1,undefined8 *param_2,longlong *param_3);
undefined8 * FUN_18001cc60(longlong param_1,undefined8 *param_2);
void FUN_18001ce80(longlong *param_1);
undefined8 FUN_18001cf40(undefined8 param_1);
void FUN_18001cf50(longlong *param_1);
longlong * FUN_18001cfe0(longlong param_1,longlong *param_2);
longlong * FUN_18001d090(longlong *param_1,void *param_2,size_t param_3);
undefined8 * FUN_18001d200(undefined8 *param_1,undefined8 *param_2);
longlong * FUN_18001d320(longlong *param_1,longlong *param_2);
void FUN_18001d3f0(longlong *param_1);
void FUN_18001d420(undefined8 param_1,void *param_2,longlong param_3);
undefined8 * FUN_18001d470(undefined8 param_1,undefined8 *param_2);
undefined8 * FUN_18001d750(undefined8 *param_1,undefined8 *param_2);
void deallocate(undefined8 param_1,void *param_2,longlong param_3);
basic_ostream<> * FUN_18001d820(basic_ostream<> *param_1,char *param_2);
basic_ostream<> * FUN_18001d9f0(basic_ostream<> *param_1,char param_2);
int FUN_18001dbc0(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void FUN_18001dc30(int *param_1,longlong *param_2,int param_3);
undefined8 * FUN_18001e1a0(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_18001e240(undefined8 *param_1,longlong param_2);
undefined8 * FUN_18001e330(undefined8 *param_1,undefined8 *param_2);
undefined1 * FUN_18001e3e0(undefined1 *param_1,undefined8 *param_2,undefined8 *param_3);
longlong FUN_18001e550(longlong param_1,undefined8 param_2,longlong param_3,longlong *param_4,int *param_5);
longlong FUN_18001e820(longlong param_1,undefined8 param_2,longlong param_3,longlong *param_4,int *param_5,undefined8 param_6,undefined8 param_7,undefined8 param_8);
void FUN_18001eb10(char *param_1,char *param_2);
void FUN_18001eb50(ulonglong *param_1,longlong param_2,ulonglong param_3);
undefined8 *FUN_18001ed60(undefined8 *param_1,ulonglong param_2,undefined8 param_3,undefined1 param_4);
void FUN_18001eed0(longlong *param_1,longlong *param_2);
longlong * FUN_18001ef80(longlong *param_1,ulonglong param_2,undefined8 param_3,void *param_4);
void FUN_18001f0c0(undefined8 *param_1,undefined8 *param_2);
void FUN_18001f100(undefined8 param_1,undefined8 param_2,longlong *param_3);
undefined8 * FUN_18001f180(undefined8 *param_1,longlong param_2);
undefined8 * FUN_18001f220(undefined8 *param_1,longlong param_2);
void FUN_18001f2e0(longlong param_1);
void FUN_18001f350(undefined8 *param_1);
void FUN_18001f390(longlong param_1);
void FUN_18001f3a0(longlong param_1);
void FUN_18001f580(ulonglong *param_1,ulonglong param_2);
longlong * FUN_18001f6f0(longlong *param_1,longlong *param_2,longlong *param_3);
basic_istream<> * FUN_18001f950(basic_istream<> *param_1,undefined8 *param_2);
undefined8 *FUN_18001fa90(undefined8 *param_1,undefined8 param_2,undefined8 param_3,void *param_4,size_t param_5,void *param_6,size_t param_7);
undefined8 * FUN_18001fbe0(undefined8 *param_1,ulonglong param_2);
void FUN_18001fc10(void);
basic_ostream<> * FUN_18001fc30(basic_ostream<> *param_1,char *param_2,ulonglong param_3);
void FUN_18001fdf0(byte *param_1,longlong *param_2);
void FUN_18001fee0(longlong *param_1,int param_2);
void FUN_18001ffa0(undefined8 *param_1,longlong *param_2);
void FUN_180020040(int *param_1,longlong *param_2,int param_3);
void FUN_180020600(longlong param_1,undefined8 param_2,Gpu_ImageBuffer *param_3,undefined8 *param_4);
void FUN_1800206e0(longlong param_1,undefined8 param_2,Gpu_ImageBuffer *param_3,undefined8 *param_4,undefined8 param_5,undefined8 param_6,undefined8 param_7);
undefined8 * FUN_1800207f0(undefined8 param_1,undefined8 *param_2,undefined8 *param_3);
undefined8 * FUN_1800209a0(longlong *param_1,void *param_2,undefined8 *param_3);
undefined8 * FUN_180020b40(longlong *param_1,undefined8 *param_2,longlong param_3);
longlong * FUN_180020d70(longlong *param_1,longlong *param_2,undefined8 *param_3);
undefined4 * FUN_180020e40(longlong *param_1,undefined8 *param_2,undefined4 *param_3);
void FUN_1800210a0(longlong param_1,longlong *param_2,longlong *param_3);
void FUN_1800211f0(undefined8 param_1,undefined8 param_2,longlong *param_3);
undefined4 * FUN_180021270(longlong *param_1,void *param_2,undefined4 *param_3);
longlong *FUN_180021410(undefined8 *param_1,undefined8 *param_2,longlong param_3,undefined8 param_4);
undefined8 * FUN_180021510(undefined8 *param_1,undefined8 *param_2);
void FUN_180021620(undefined8 *param_1);
void FUN_180021660(undefined8 *param_1);
void FUN_1800216c0(void *param_1,char param_2);
longlong FUN_1800216d0(longlong param_1);
TypeDescriptor * FUN_1800216e0(void);
void FUN_1800216f0(longlong param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
undefined8 * FUN_180021710(longlong param_1,undefined8 *param_2);
ulonglong FUN_180021730(undefined8 *param_1);
longlong FUN_180021770(undefined8 param_1,undefined8 *param_2,undefined8 *param_3);
ulonglong FUN_1800217f0(undefined8 *param_1,longlong *param_2);
int FUN_180021e20(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
undefined8 FUN_180021e90(undefined8 *param_1);
void FUN_180021ec0(undefined8 *param_1,undefined8 *param_2,undefined8 *param_3);
void FUN_180021f70(undefined8 param_1,undefined8 *param_2,undefined8 *param_3);
void FUN_180022000(longlong *param_1);
undefined4 * FUN_180022070(longlong *param_1,undefined4 *param_2,undefined4 *param_3);
void FUN_1800222c0(undefined8 *param_1,byte param_2);
int FUN_180022a60(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void FUN_180022ad0(undefined8 *param_1,undefined8 *param_2);
longlong FUN_180022c60(undefined8 *param_1,undefined8 *param_2);
longlong FUN_180022f40(undefined8 *param_1,longlong *param_2);
longlong * FUN_180023650(longlong *param_1,undefined8 *param_2);
undefined8 * FUN_180023780(undefined8 *param_1);
undefined8 * FUN_180023900(undefined8 *param_1,longlong param_2);
void FUN_180023990(undefined8 *param_1,byte param_2);
longlong FUN_180024080(undefined8 *param_1,int param_2);
undefined8 * FUN_1800240f0(undefined8 *param_1,ulonglong param_2);
ulonglong FUN_180024120(longlong *param_1,undefined8 *param_2);
undefined4 * FUN_180024500(undefined4 *param_1,undefined4 *param_2);
undefined8 FUN_180024650(longlong *param_1,undefined8 *param_2);
int FUN_180024870(undefined8 *param_1);
void FUN_1800248e8(longlong param_1,uint param_2);
void FUN_1800248f4(longlong param_1,uint param_2);
void FUN_180024900(longlong param_1,uint param_2);
void FUN_18002490c(longlong param_1,uint param_2);
void FUN_180024918(longlong param_1,uint param_2);
bool FUN_180024930(undefined8 param_1,char param_2,undefined8 param_3,uint param_4,Gpu_ImageBuffer *param_5,undefined8 param_6,undefined8 param_7,undefined8 param_8,undefined8 param_9);
void FUN_180024e80(longlong param_1);
void FUN_180024f00(void);
void __thiscall FID_conflict:~vector<>(void *this);
void FUN_180026c10(undefined8 *param_1);
longlong * FUN_180026c40(longlong *param_1,longlong param_2);
void FUN_180026dd0(ulonglong *param_1,ulonglong param_2,undefined8 param_3);
void FUN_180026f10(undefined8 param_1,ulonglong *param_2,ulonglong *param_3);
void FUN_180027250(undefined8 param_1,undefined8 *param_2);
void FUN_1800272f0(undefined8 *param_1,undefined8 *param_2,undefined8 *param_3);
void FUN_180027380(longlong param_1);
TypeDescriptor * FUN_180027410(void);
void FUN_180027420(longlong param_1,undefined8 param_2,undefined8 param_3);
undefined8 * FUN_180027430(longlong param_1,undefined8 *param_2);
TypeDescriptor * FUN_180027450(void);
void FUN_180027460(longlong param_1,undefined8 param_2);
undefined8 * FUN_180027470(longlong param_1,undefined8 *param_2);
TypeDescriptor * FUN_180027490(void);
undefined8 * FUN_1800274a0(longlong param_1,undefined8 *param_2);
TypeDescriptor * FUN_1800274c0(void);
undefined8 * FUN_1800274d0(longlong param_1,undefined8 *param_2);
TypeDescriptor * FUN_1800274f0(void);
undefined8 * FUN_180027500(longlong param_1,undefined8 *param_2);
TypeDescriptor * FUN_180027520(void);
undefined8 * FUN_180027530(longlong param_1,undefined8 *param_2);
void FUN_180027570(undefined8 param_1,ulonglong param_2);
int FUN_180027720(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
undefined8 * FUN_1800277a0(undefined8 *param_1,uint param_2);
void FUN_1800277e0(longlong param_1);
undefined8 * FUN_1800277f0(undefined8 *param_1,undefined8 param_2,undefined8 param_3);
undefined8 * FUN_180027920(undefined8 *param_1,uint param_2);
void FUN_180027970(longlong param_1,int param_2,double param_3,char param_4);
void FUN_180027a20(longlong param_1,int param_2,double *param_3);
ulonglong FUN_180027ad0(longlong param_1,int param_2,longlong *param_3);
void FUN_180028170(longlong *param_1,int param_2,longlong *param_3);
void FUN_1800287b0(longlong *param_1);
longlong * FUN_180028810(longlong *param_1,longlong *param_2,undefined8 *param_3);
void FUN_180028880(longlong *param_1,undefined8 param_2);
ulonglong FUN_180028900(longlong param_1,ulonglong param_2,ulonglong param_3,char *param_4,ulonglong param_5);
void FUN_1800289d0(undefined8 param_1,undefined8 param_2,longlong *param_3);
undefined8 *FUN_180028a30(undefined8 *param_1,undefined8 param_2,undefined8 *param_3,undefined8 *param_4);
void FUN_180028c60(undefined8 *param_1);
undefined8 FUN_180028d60(longlong *param_1,uint param_2,int param_3,char param_4);
longlong * FUN_1800290a0(undefined8 param_1,longlong *param_2,longlong *param_3);
int FUN_180029170(longlong param_1,int param_2);
void FUN_1800292b0(undefined8 param_1,longlong *param_2,undefined8 *param_3);
void FUN_1800294d0(longlong param_1,longlong *param_2,undefined8 param_3,undefined8 param_4);
longlong * FUN_1800295a0(longlong *param_1,longlong *param_2,undefined8 param_3,undefined8 param_4);
longlong * FUN_180029670(undefined8 param_1,int *param_2);
undefined8 * FUN_180029770(longlong *param_1,undefined8 *param_2,uint *param_3);
void FUN_180029870(undefined8 param_1,undefined8 param_2,longlong *param_3);
void FUN_1800298f0(undefined8 param_1,undefined8 param_2,longlong *param_3);
undefined4 * FUN_180029990(undefined8 param_1,undefined4 *param_2,undefined4 *param_3);
longlong *FUN_180029b60(undefined8 *param_1,undefined8 *param_2,longlong param_3,undefined8 param_4);
void FUN_180029c10(longlong *param_1);
int snprintf(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void FUN_18002ab00(double *param_1,double *param_2,undefined8 *param_3,double *param_4,double *param_5,double *param_6,double *param_7,undefined8 *param_8);
void FUN_18002aea0(longlong *param_1,undefined4 param_2,undefined4 param_3,undefined8 param_4,undefined1 param_5);
undefined8 * FUN_18002af00(undefined8 *param_1,ulonglong param_2);
int FUN_18002af30(longlong *param_1);
int FUN_18002af50(longlong *param_1);
void FUN_18002af70(longlong *param_1,int param_2);
undefined4 FUN_18002afb0(longlong *param_1);
undefined8 FUN_18002afe0(longlong *param_1,undefined8 param_2,short *param_3);
void FUN_18002b0c0(longlong *param_1);
undefined8 * FUN_18002b0d0(undefined8 param_1,undefined8 *param_2);
void FUN_18002b100(longlong *param_1);
undefined8 FUN_18002b120(undefined8 param_1,undefined8 param_2,undefined4 *param_3,undefined4 *param_4);
undefined8 FUN_18002b130(longlong *param_1,float *param_2);
void FUN_18002b190(undefined8 param_1,undefined4 *param_2,undefined4 *param_3);
void FUN_18002b1a0(longlong param_1,void *param_2,longlong param_3);
undefined8 FUN_18002b3f0(longlong param_1,void *param_2,undefined8 param_3,undefined8 param_4);
void FUN_18002b580(longlong *param_1,longlong *param_2,int param_3,int param_4,int param_5,int param_6);
void FUN_18002baf0(longlong *param_1);
undefined4 FUN_18002bc80(longlong param_1);
undefined4 FUN_18002bc90(longlong param_1);
undefined4 FUN_18002bca0(longlong param_1);
void FUN_18002bcb0(longlong param_1,longlong *param_2);
undefined4 FUN_18002bcd0(longlong param_1);
undefined4 FUN_18002bce0(longlong param_1);
undefined4 FUN_18002bcf0(longlong param_1);
undefined4 FUN_18002bd00(longlong param_1);
undefined8 * FUN_18002bd10(undefined8 *param_1,ulonglong param_2);
undefined8 FUN_18002bd40(longlong param_1);
undefined4 FUN_18002bd50(longlong param_1);
undefined4 FUN_18002bd70(longlong param_1);
undefined4 FUN_18002bd90(longlong param_1);
undefined4 FUN_18002bdb0(longlong param_1);
void FUN_18002bdd0(longlong param_1,int param_2);
void FUN_18002be30(longlong param_1);
undefined8 FUN_18002be70(longlong param_1);
double FUN_18002be80(longlong param_1);
double FUN_18002beb0(longlong param_1);
double FUN_18002bee0(longlong param_1);
void FUN_18002bf10(longlong param_1,undefined4 param_2);
float FUN_18002bf30(longlong param_1,float param_2);
void FUN_18002bf60(longlong param_1,undefined4 param_2);
float FUN_18002bf80(longlong param_1,float param_2);
void FUN_18002bfb0(longlong param_1,undefined4 param_2);
float FUN_18002bfd0(longlong param_1,float param_2);
void FUN_18002c000(longlong param_1,undefined4 param_2,undefined4 param_3);
float FUN_18002c020(longlong param_1,float param_2,float param_3);
void FUN_18002c050(longlong param_1,undefined4 param_2);
float FUN_18002c070(longlong param_1,float param_2);
void FUN_18002c0a0(longlong param_1,undefined4 param_2);
float FUN_18002c0c0(longlong param_1,float param_2);
void FUN_18002c0f0(longlong param_1,undefined4 param_2);
float FUN_18002c110(longlong param_1,float param_2);
void FUN_18002c140(longlong param_1,undefined4 param_2,undefined4 param_3);
float FUN_18002c160(longlong param_1,float param_2,float param_3);
void FUN_18002c190(longlong param_1,undefined4 param_2,undefined4 param_3);
float FUN_18002c1b0(longlong param_1,float param_2,float param_3);
float FUN_18002c1e0(longlong param_1,float param_2);
void FUN_18002c210(longlong param_1,undefined4 param_2);
float FUN_18002c230(longlong param_1,float param_2);
void FUN_18002c260(longlong param_1,undefined4 param_2);
ulonglong FUN_18002c280(longlong param_1,longlong *param_2);
void FUN_18002c2c0(longlong param_1,int param_2);
void FUN_18002c2e0(longlong param_1,undefined8 param_2);
void FUN_18002c300(longlong param_1,undefined8 param_2);
void FUN_18002c320(longlong param_1,undefined8 param_2);
void FUN_18002c340(longlong param_1);
void FUN_18002c370(longlong param_1,int param_2,int param_3,short param_4,short param_5);
longlong FUN_18002c3c0(longlong param_1);
undefined8 FUN_18002c3e0(longlong *param_1,int *param_2,int *param_3);
undefined4 FUN_18002c460(longlong param_1);
undefined4 FUN_18002c470(longlong param_1);
int FUN_18002c480(longlong param_1);
undefined4 FUN_18002c4b0(longlong param_1);
int FUN_18002c4c0(longlong param_1);
undefined8 FUN_18002c4f0(longlong param_1,int param_2);
undefined8 FUN_18002c500(longlong param_1);
void FUN_18002c510(longlong param_1,undefined8 param_2);
void FUN_18002c520(longlong param_1,undefined4 *param_2,undefined4 *param_3);
void FUN_18002c530(longlong param_1,undefined4 param_2,undefined4 param_3);
undefined4 FUN_18002c540(longlong param_1,undefined8 param_2,undefined4 param_3,char *param_4,longlong *param_5);
undefined8 FUN_18002c5e0(longlong param_1,undefined8 param_2,undefined4 param_3,char *param_4);
void FUN_18002c650(void);
void FUN_18002c670(void);
void FUN_18002c690(longlong *param_1,undefined8 param_2);
void FUN_18002c6b0(longlong param_1);
undefined8 FUN_18002c6d0(longlong param_1);
void FUN_18002c6e0(longlong param_1,undefined8 param_2);
void FUN_18002c6f0(undefined8 param_1,undefined8 param_2,undefined4 param_3,undefined4 *param_4,undefined4 *param_5,undefined4 *param_6);
void FUN_18002c710(longlong param_1);
undefined8 FUN_18002c730(longlong param_1,int *param_2,int *param_3);
void FUN_18002c750(longlong param_1,longlong *param_2,undefined8 param_3,short *param_4,longlong *param_5);
void FUN_18002c860(longlong param_1,longlong *param_2,longlong *param_3,undefined4 param_4,double param_5,double param_6,double param_7,double param_8,double param_9,double param_10);
undefined8 FUN_18002ca60(longlong param_1,undefined8 param_2,undefined8 param_3,double *param_4,double *param_5,double *param_6,undefined4 *param_7);
undefined8 FUN_18002cc20(longlong param_1,undefined4 param_2,undefined4 param_3);
char FUN_18002cc30(longlong param_1);
undefined8 FUN_18002cc50(longlong param_1,float *param_2);
void FUN_18002cc80(longlong param_1);
undefined8 * FUN_18002cc90(longlong param_1,undefined8 param_2);
void FUN_18002cd80(longlong *param_1);
undefined8 *FUN_18002cd90(undefined8 *param_1,longlong param_2,longlong param_3,undefined8 param_4,undefined8 param_5);
undefined8 * FUN_18002cf20(undefined8 *param_1,uint param_2);
void FUN_18002cf60(longlong param_1);
void * FUN_18002cf90(void *param_1,uint param_2);
void FUN_18002cff0(undefined8 *param_1);
void FUN_18002d0c0(longlong param_1,longlong param_2,undefined8 param_3,undefined8 param_4,undefined8 param_5,longlong param_6,char param_7,undefined8 *param_8);
void FUN_18002d220(longlong param_1,int param_2,undefined8 param_3);
void FUN_18002d250(longlong param_1,undefined8 *param_2,undefined8 param_3,undefined8 param_4);
void FUN_18002d280(longlong param_1,longlong *param_2,undefined8 *param_3,undefined8 param_4);
undefined8 FUN_18002d310(longlong param_1,LPCSTR param_2);
undefined8 FUN_18002d4f0(longlong param_1);
undefined8 FUN_18002d660(longlong param_1);
void FUN_18002d740(longlong param_1,ulonglong param_2,int param_3,uint *param_4,double *param_5,char param_6);
void FUN_18002d810(longlong param_1,ulonglong param_2,undefined8 param_3,uint *param_4,double *param_5,char param_6);
void FUN_18002d8d0(longlong param_1,ulonglong param_2,undefined8 param_3,uint *param_4,undefined4 *param_5,char param_6);
void FUN_18002d980(longlong param_1,ulonglong param_2,undefined8 param_3,uint *param_4,int *param_5,char param_6);
void FUN_18002da30(longlong param_1,int param_2,undefined8 param_3,undefined8 param_4,undefined1 *param_5,char param_6);
void FUN_18002da80(longlong *param_1,ulonglong param_2,undefined8 param_3,uint *param_4,double *param_5,double *param_6,char param_7);
void FUN_18002dc40(longlong *param_1,ulonglong param_2,undefined8 param_3,uint *param_4,double *param_5,double *param_6,undefined8 *param_7,char param_8);
void FUN_18002ddc0(longlong param_1,ulonglong param_2,undefined8 param_3,uint *param_4,double *param_5,double *param_6,double *param_7,char param_8);
void FUN_18002ded0(longlong param_1,longlong param_2,undefined8 param_3,undefined4 param_4,undefined8 *param_5);
void FUN_18002df40(longlong param_1,undefined8 *param_2,undefined8 param_3,undefined8 param_4);
void FUN_18002df80(longlong param_1,longlong param_2,undefined8 param_3,undefined4 *param_4);
void FUN_18002dfc0(longlong param_1,undefined8 *param_2,undefined4 *param_3);
void FUN_18002e000(longlong param_1,longlong param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6,undefined8 param_7);
void FUN_18002e040(longlong param_1,longlong param_2,undefined4 param_3,undefined1 param_4,undefined8 param_5);
void FUN_18002e070(longlong param_1,longlong param_2,undefined4 param_3,undefined8 param_4);
undefined8 *FUN_18002e090(longlong param_1,undefined4 param_2,undefined4 param_3,int *param_4,undefined4 *param_5,uint param_6);
undefined4 FUN_18002e2c0(longlong param_1,longlong *param_2);
undefined8 * FUN_18002e3d0(longlong param_1,undefined8 *param_2,longlong param_3);
undefined4 * FUN_18002e520(longlong param_1,undefined4 *param_2);
undefined8 FUN_18002e650(longlong *param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
undefined8 FUN_18002e7c0(longlong *param_1,double param_2,undefined8 param_3,char param_4);
undefined8 FUN_18002e8e0(longlong *param_1,int param_2);
undefined8 FUN_18002f0c0(longlong *param_1,undefined8 *param_2);
int FUN_18002f0f0(longlong *param_1,undefined4 param_2,double param_3,double param_4,longlong *param_5,longlong *param_6);
void FUN_18002f5d0(longlong param_1,undefined4 param_2,longlong *param_3,longlong *param_4,short *param_5,short *param_6,double param_7);
void FUN_18002f740(longlong *param_1,undefined8 param_2,undefined4 param_3);
void FUN_18002f760(longlong *param_1,undefined4 param_2,double param_3,undefined8 param_4);
longlong *FUN_18002f780(longlong *param_1,undefined4 param_2,double param_3,undefined8 param_4,longlong param_5);
undefined8 * FUN_18002ff70(undefined8 *param_1,longlong param_2);
undefined8 FUN_180030080(longlong *param_1);
int FUN_1800300c0(longlong param_1,int param_2);
undefined8 FUN_1800301d0(longlong param_1);
int FUN_180030200(longlong param_1,undefined4 param_2,uint *param_3,uint *param_4,uint *param_5);
int FUN_1800303e0(longlong param_1,undefined4 *param_2,undefined4 *param_3);
undefined8 * FUN_1800304e0(longlong *param_1,undefined8 *param_2);
void FUN_180030610(longlong param_1,longlong *param_2);
int FUN_180030640(longlong param_1,undefined4 param_2,float *param_3,float *param_4);
undefined4 FUN_1800309b0(longlong param_1,undefined8 param_2,undefined4 param_3);
undefined4 FUN_180030c80(longlong param_1,undefined8 param_2);
int FUN_180030e80(longlong param_1,int param_2,undefined4 *param_3);
void FUN_180031170(longlong param_1);
void FUN_180031200(longlong param_1,longlong *param_2);
void FUN_180031250(longlong param_1,longlong *param_2,longlong *param_3);
void FUN_1800312d0(longlong param_1);
void FUN_180031300(longlong param_1);
undefined8 FUN_180031340(longlong param_1,longlong *param_2,longlong *param_3,short *param_4,short *param_5);
void FUN_180031420(longlong param_1,int param_2,undefined8 param_3,int *param_4);
void FUN_1800314c0(longlong param_1,int param_2,undefined8 param_3,double *param_4);
void FUN_1800315c0(longlong param_1,int param_2,undefined8 param_3,double *param_4);
void FUN_180031690(longlong *param_1,int param_2,undefined8 param_3,double param_4,double param_5);
void FUN_1800317d0(longlong *param_1,int param_2,undefined8 param_3,double param_4,double param_5,double param_6);
undefined8 FUN_180031910(longlong param_1,int param_2);
void FUN_180031920(longlong *param_1,undefined4 param_2,undefined8 param_3);
int FUN_180031980(longlong *param_1);
bool FUN_180031b80(longlong param_1);
undefined8 * FUN_180031c90(undefined8 *param_1,uint param_2);
bool FUN_180031e30(longlong param_1);
void FUN_180031e40(longlong param_1,int param_2,int param_3,double param_4);
void FUN_180031e60(longlong param_1,int param_2,undefined8 param_3,double param_4);
void FUN_180031ed0(longlong param_1,int param_2,undefined8 param_3,int param_4);
void FUN_180031f20(longlong param_1,int param_2,undefined8 param_3,byte param_4);
void FUN_180031f70(longlong param_1,int param_2,undefined8 param_3,int param_4);
void FUN_180031fc0(longlong param_1,int param_2,undefined8 param_3,double param_4,undefined8 param_5);
void FUN_180031ff0(longlong param_1,int param_2,undefined8 param_3,double param_4,double param_5,double param_6);
void FUN_180032090(longlong param_1,int param_2,undefined8 param_3,double param_4,double param_5,double param_6);
undefined8 FUN_180032190(longlong param_1,int param_2,undefined8 param_3,char param_4);
void FUN_180032240(longlong param_1,int param_2,undefined8 param_3,char param_4);
undefined4 FUN_180032290(longlong param_1,int param_2,undefined8 param_3,void *param_4);
undefined4 FUN_1800323e0(longlong param_1,int param_2,undefined8 param_3,void *param_4,char param_5);
int FUN_180032550(longlong param_1,int param_2,undefined8 param_3,byte param_4);
longlong * FUN_1800328d0(longlong *param_1,longlong param_2);
void FUN_180032940(longlong *param_1);
void FUN_180032950(undefined8 param_1);
void FUN_180032980(longlong *param_1);
undefined8 *FUN_1800329a0(undefined8 *param_1,longlong param_2,undefined8 param_3,undefined4 param_4,longlong param_5);
void FUN_180032a70(undefined8 param_1,undefined8 param_2,longlong *param_3);
void FUN_180032b30(longlong param_1,uint param_2);
void FUN_180032b40(longlong *param_1,longlong *param_2,undefined8 param_3,undefined8 param_4);
void FUN_180032b70(longlong *param_1,longlong *param_2,undefined8 *param_3,undefined8 param_4);
int * FUN_180032bc0(int *param_1,int *param_2,undefined8 param_3,undefined8 param_4);
void FUN_180032c10(undefined8 *param_1);
void FUN_180032c50(longlong *param_1);
void FUN_180032c70(longlong param_1);
undefined1 FUN_180032c80(longlong param_1);
undefined4 FUN_180032c90(longlong param_1);
undefined8 * FUN_180032ca0(undefined8 *param_1,ulonglong param_2);
undefined8 * FUN_180032cd0(undefined8 *param_1,ulonglong param_2);
longlong * FUN_180032d40(longlong *param_1,longlong *param_2,undefined8 param_3,undefined8 param_4);
undefined8 FUN_180032f80(longlong *param_1);
ulonglong FUN_180032fc0(longlong *param_1,void *param_2,int *param_3);
ulonglong FUN_1800330b0(longlong *param_1,undefined8 *param_2,undefined4 *param_3);
undefined8 FUN_180033190(longlong *param_1,uint param_2,undefined4 param_3);
undefined8 FUN_180033260(longlong *param_1,uint param_2,undefined4 *param_3);
undefined8 FUN_180033330(longlong *param_1,undefined4 param_2);
ulonglong FUN_1800333c0(longlong *param_1,void *param_2,int *param_3);
ulonglong FUN_1800334a0(longlong *param_1,undefined8 *param_2,undefined4 *param_3);
undefined8 FUN_180033580(longlong *param_1,char param_2);
void FUN_180033610(longlong *param_1,undefined8 *param_2);
undefined1 FUN_180033630(longlong *param_1,longlong *param_2);
longlong * FUN_180033860(longlong *param_1,uint param_2);
undefined8 *FUN_1800338a0(undefined8 *param_1,undefined4 param_2,undefined8 param_3,undefined8 param_4);
void FUN_180033930(longlong *param_1);
bool FUN_180033a30(longlong *param_1,longlong param_2);
undefined1 FUN_180033b70(longlong *param_1,void *param_2,int param_3);
undefined1 FUN_180033d40(longlong *param_1,void *param_2,int param_3);
undefined8 FUN_180033ea0(longlong param_1);
void FUN_180033f50(longlong param_1);
longlong FUN_180033fa0(longlong *param_1);
void FUN_180034000(longlong *param_1);
void FUN_180034060(longlong param_1);
longlong * FUN_180034080(longlong *param_1,uint param_2);
undefined8 FUN_1800341a0(longlong param_1);
void FUN_180034240(longlong param_1);
undefined8 FUN_180034280(longlong *param_1,longlong param_2,undefined8 param_3,undefined8 param_4,undefined8 param_5,longlong *param_6);
undefined8 FUN_1800343f0(undefined8 param_1,longlong param_2);
undefined4 FUN_180034470(undefined8 param_1,longlong param_2);
undefined8 FUN_180034620(undefined8 param_1,longlong param_2);
undefined8 FUN_180034690(undefined8 param_1,longlong param_2);
undefined8 FUN_180034770(undefined8 param_1,longlong param_2);
undefined4 FUN_1800348d0(longlong *param_1,longlong param_2,undefined8 param_3,undefined8 param_4,undefined8 param_5,longlong param_6);
undefined8 FUN_180034c00(longlong *param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4,undefined8 param_5,longlong param_6);
undefined8 FUN_180034d90(longlong *param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4,undefined8 param_5,longlong param_6);
undefined8 FUN_180034ef0(longlong *param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4,undefined8 param_5,longlong param_6);
undefined8 FUN_1800350d0(longlong *param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4,undefined8 param_5,longlong param_6);
undefined8 FUN_180035410(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4,undefined4 *param_5);
void FUN_1800356f0(undefined8 param_1,undefined4 param_2,undefined8 param_3);
void FUN_1800357a0(undefined8 param_1,undefined4 param_2,undefined8 param_3);
longlong * FUN_1800357f0(longlong *param_1);
void FUN_1800358d0(undefined8 *param_1,undefined8 param_2,undefined2 *param_3,undefined8 *param_4,undefined8 param_5,undefined8 param_6,undefined8 *param_7);
void FUN_180035b40(longlong *param_1);
void FUN_180035c40(undefined8 *param_1,undefined8 *param_2);
void FUN_180035e20(longlong param_1);
void FUN_180036a20(longlong param_1);
void FUN_1800371a0(void);
void FUN_180037700(void);
bool FUN_1800378c0(longlong param_1,longlong param_2);
undefined8 FUN_180037980(longlong param_1,longlong param_2);
undefined8 FUN_180037d10(longlong param_1);
undefined8 FUN_180037e00(longlong param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
ulonglong FUN_180038700(longlong param_1,longlong param_2,char *param_3,undefined8 param_4);
void FUN_180038ac0(undefined8 *param_1,undefined8 *param_2,undefined8 *param_3);
undefined1 *FUN_180038d70(undefined1 *param_1,undefined8 *param_2,undefined8 *param_3,undefined8 *param_4,undefined8 *param_5,undefined8 *param_6,undefined8 *param_7);
undefined1 *FUN_180039260(undefined1 *param_1,undefined8 *param_2,undefined8 *param_3,undefined8 *param_4,undefined8 *param_5);
undefined1 *FUN_180039590(undefined1 *param_1,undefined8 *param_2,undefined8 *param_3,undefined8 *param_4,undefined8 *param_5,undefined8 *param_6);
undefined1 *FUN_1800399a0(undefined1 *param_1,undefined8 *param_2,undefined8 *param_3,undefined8 *param_4);
undefined8 FUN_180039be0(undefined8 *param_1);
char * FUN_180039c10(undefined8 param_1,char *param_2,undefined1 *param_3);
undefined1 * FUN_180039e40(undefined1 *param_1,undefined1 *param_2,undefined1 *param_3);
void FUN_180039ea0(undefined8 *param_1);
int FUN_180039ee0(LPCSTR param_1,LPCSTR param_2);
void FUN_18003a010(longlong *param_1);
longlong * FUN_18003a110(longlong *param_1);
longlong * FUN_18003a1c0(longlong *param_1,LPCSTR param_2);
longlong * FUN_18003a380(longlong *param_1,longlong *param_2,longlong *param_3);
bool FUN_18003aa10(void);
undefined * FUN_18003b060(void);
int __cdecl sscanf(char *_Src,char *_Format,...);
short * FUN_18003b0d0(short *param_1);
undefined8 * FUN_18003b140(short *param_1,undefined8 *param_2);
ulonglong FUN_18003b330(char *param_1,undefined8 *param_2);
void FUN_18003b6b0(short *param_1);
void FUN_18003b720(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
undefined8 * FUN_18003b780(undefined8 *param_1);
undefined4 * FUN_18003b7d0(int *param_1,undefined8 *param_2);
ulonglong FUN_18003b8d0(longlong *param_1,byte *param_2);
undefined8 * FUN_18003ba00(undefined8 *param_1,ulonglong param_2);
int FUN_18003ba30(LPCSTR param_1,BYTE *param_2,LPCSTR param_3);
undefined4 * FUN_18003bd80(byte *param_1);
undefined8 *FUN_18003be40(undefined8 *param_1,longlong *param_2,longlong *param_3,longlong *param_4);
undefined8 FUN_18003c030(longlong param_1,longlong *param_2);
undefined8 * FUN_18003c700(undefined8 *param_1,uint param_2);
void FUN_18003c740(undefined8 *param_1);
undefined8 FUN_18003c8a0(longlong *param_1,int *param_2);
longlong * FUN_18003cff0(longlong *param_1,longlong param_2,longlong *param_3,undefined8 param_4);
void FUN_18003d460(longlong param_1);
undefined8 FUN_18003d610(longlong param_1,longlong *param_2);
undefined8 FUN_18003d910(longlong param_1,int *param_2);
void FUN_18003da90(undefined8 *param_1);
void FUN_18003db50(undefined8 *param_1);
undefined8 FUN_18003dc10(longlong param_1,longlong *param_2);
undefined8 FUN_18003df20(longlong param_1,int *param_2);
void FUN_18003e0c0(undefined8 *param_1);
ulonglong FUN_18003e180(longlong param_1,longlong *param_2,undefined8 param_3,undefined8 param_4);
ulonglong FUN_18003ea00(longlong param_1,int *param_2,size_t param_3);
undefined8 * FUN_18003f1d0(undefined8 *param_1,uint param_2);
undefined8 * FUN_18003f210(undefined8 *param_1,uint param_2);
undefined8 * FUN_18003f250(undefined8 *param_1,uint param_2);
undefined8 FUN_18003f290(longlong *param_1,int *param_2);
bool FUN_18003f4a0(undefined8 *param_1,__time64_t *param_2);
undefined8 FUN_18003f590(longlong param_1,int *param_2);
undefined8 FUN_18003ff80(longlong param_1,longlong *param_2);
undefined8 FUN_1800403f0(longlong param_1,int *param_2);
void FUN_180040640(undefined8 *param_1);
undefined8 * FUN_180040750(undefined8 *param_1,uint param_2);
void FUN_180040790(undefined8 *param_1);
undefined8 FUN_1800407d0(longlong param_1,longlong *param_2);
undefined8 FUN_180041350(longlong param_1,int *param_2);
void FUN_180041f20(undefined8 *param_1);
undefined8 * FUN_180042190(undefined8 *param_1,uint param_2);
void FUN_1800421d0(undefined8 *param_1);
undefined8 * FUN_180042210(undefined8 *param_1);
void FUN_180042340(char *param_1);
void FUN_180042490(longlong *param_1);
longlong * FUN_180042590(longlong *param_1,undefined8 *param_2,longlong param_3);
void FUN_1800428d0(undefined8 *param_1,byte ******param_2,longlong *param_3,longlong *param_4);
undefined8 * FUN_180042dc0(undefined8 *param_1,longlong *param_2);
longlong * FUN_180042fb0(longlong param_1,longlong *param_2,longlong *param_3,longlong param_4);
void * FUN_1800430a0(void *param_1,uint param_2);
void FUN_1800430e0(longlong param_1);
void thunk_FUN_180043270(longlong param_1);
void FUN_180043270(longlong param_1);
undefined8 * FUN_1800432e0(undefined8 *param_1,longlong param_2);
int FUN_180043320(undefined8 param_1,undefined8 param_2,undefined1 *param_3,undefined8 param_4);
undefined8 FUN_180043510(void);
void FUN_1800436c0(undefined8 ****param_1);
undefined8 * FUN_180043d00(undefined8 *param_1,longlong *param_2);
int FUN_180044470(longlong *param_1,char param_2,longlong *param_3);
undefined4 FUN_1800448d0(longlong *param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void FUN_180044f60(longlong *param_1);
undefined4 FUN_180045150(longlong *param_1);
undefined4 FUN_180045430(longlong *param_1);
int FUN_180045750(longlong *param_1);
undefined8 * FUN_180045de0(undefined8 *param_1);
void FUN_180045ee0(undefined8 *param_1);
void FUN_180045f20(undefined8 *param_1);
undefined8 * FUN_180045fe0(undefined8 *param_1,uint param_2);
void FUN_180046020(undefined8 *param_1);
undefined4 FUN_1800462e0(longlong *param_1);
void FUN_180046330(longlong *param_1,undefined8 param_2,undefined8 param_3);
longlong * FUN_1800464a0(longlong *param_1,longlong *param_2);
void FUN_180046c40(longlong param_1);
void FUN_180046d90(longlong *param_1);
ulonglong FUN_180046f60(void);
int FUN_1800471a0(longlong *param_1);
undefined8 FUN_180047470(longlong param_1,int *param_2,undefined8 param_3,undefined8 param_4);
undefined8 FUN_180048490(longlong param_1,int *param_2,undefined8 param_3,undefined8 param_4);
void FUN_1800492b0(longlong param_1,undefined8 *param_2,undefined8 *param_3);
undefined8 *FUN_1800497f0(longlong param_1,undefined8 *param_2,undefined8 param_3,undefined8 param_4);
undefined1 *FUN_18004a270(longlong param_1,undefined1 *param_2,undefined8 *param_3,undefined8 *param_4);
undefined8 *FUN_18004ac90(longlong param_1,undefined8 *param_2,undefined8 *param_3,undefined8 *param_4);
longlong FUN_18004b990(byte *param_1,byte *param_2);
longlong FUN_18004ba90(longlong param_1,longlong param_2);
void FUN_18004bc00(undefined8 *param_1);
void FUN_18004bcb0(undefined8 *param_1);
undefined8 * FUN_18004bcf0(undefined8 *param_1);
undefined1 FUN_18004be00(longlong param_1);
ulonglong FUN_18004be70(longlong param_1,longlong *param_2);
void FUN_18004bf00(longlong *param_1);
void FUN_18004bfb0(longlong *param_1,longlong param_2);
longlong * FUN_18004c050(longlong param_1,longlong *param_2,undefined8 *param_3,undefined8 *param_4);
undefined4 * FUN_18004c170(undefined4 *param_1,undefined4 *param_2);
longlong *FUN_18004c1f0(longlong *param_1,ulonglong param_2,ulonglong param_3,void *param_4,ulonglong param_5);
void FUN_18004c390(undefined8 param_1,void *param_2,longlong param_3);
void FUN_18004c3e0(longlong param_1);
void FUN_18004ccb0(longlong param_1);
undefined8 * FUN_18004cd80(undefined8 *param_1,longlong param_2);
void FUN_18004cf60(undefined8 *param_1);
undefined8 * FUN_18004cff0(undefined8 *param_1,uint param_2);
longlong * FUN_18004d030(longlong param_1,longlong *param_2,longlong *param_3);
ulonglong FUN_18004d240(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_18004d280(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_18004d330(undefined8 *param_1,undefined8 *param_2);
undefined8 *FUN_18004d3b0(longlong param_1,undefined8 *param_2,undefined8 *param_3,undefined8 *param_4);
void FUN_18004df20(longlong param_1,longlong param_2);
longlong * FUN_18004ed00(float *param_1,longlong *param_2,undefined8 *param_3);
void FUN_18004efc0(undefined8 *param_1,undefined8 *param_2);
void FUN_18004f000(undefined8 param_1,undefined8 *param_2);
undefined8 *FUN_18004f050(undefined8 *param_1,ulonglong param_2,undefined8 param_3,size_t param_4,longlong param_5,void *param_6,size_t param_7);
undefined8 * FUN_18004f220(undefined8 *param_1,longlong param_2);
undefined8 * FUN_18004f2e0(undefined8 *param_1,longlong param_2);
undefined8 * FUN_18004f490(undefined8 *param_1,uint param_2);
void FUN_18004f4d0(longlong param_1);
void FUN_18004f580(longlong param_1);
void FUN_18004f5c0(float *param_1);
undefined8 * FUN_18004f670(undefined8 *param_1,ulonglong param_2);
undefined8 * FUN_18004f6a0(undefined8 *param_1,ulonglong param_2);
void FUN_18004f6d0(longlong param_1,ulonglong param_2);
undefined8 * FUN_18004f950(undefined8 *param_1,undefined8 *param_2,LPCSTR param_3);
undefined8 * FUN_180050210(undefined8 *param_1,undefined8 *param_2);
longlong * FUN_1800502c0(longlong param_1,longlong *param_2,undefined8 *param_3,ulonglong param_4);
undefined8 * FUN_180050390(longlong *param_1,undefined8 *param_2,longlong param_3);
undefined8 * FUN_1800505d0(undefined8 *param_1,longlong param_2);
undefined8 * FUN_180050610(undefined8 *param_1,longlong param_2);
TypeDescriptor * FUN_180050770(void);
undefined8 FUN_180050780(undefined8 param_1,longlong param_2);
undefined8 * FUN_1800507f0(undefined8 param_1,undefined8 *param_2);
void FUN_180050800(undefined8 param_1,undefined8 *param_2,longlong param_3);
undefined8 * FUN_180050870(undefined8 *param_1,undefined8 *param_2,undefined8 *param_3);
TypeDescriptor * FUN_180050940(void);
undefined8 FUN_180050950(undefined8 param_1,longlong param_2);
undefined8 * FUN_1800509c0(undefined8 param_1,undefined8 *param_2);
void FUN_1800509d0(undefined8 *param_1);
longlong * FUN_180050a10(longlong *param_1,longlong *param_2,undefined8 param_3,undefined8 param_4);
longlong **** FUN_180050be0(longlong *param_1,longlong ****param_2,undefined8 *param_3);
void FUN_1800518e0(longlong *param_1,undefined8 *param_2,byte *param_3);
bool FUN_180051d50(longlong *param_1);
ulong FUN_180051de0(undefined8 param_1,undefined8 *param_2,undefined8 ******param_3);
basic_iostream<> * FUN_180052070(basic_iostream<> *param_1,undefined8 *param_2);
longlong * FUN_180052140(longlong *param_1,undefined8 *param_2);
void FUN_180052260(longlong param_1,void *param_2,ulonglong param_3,uint param_4);
void FUN_1800523a0(ulonglong *param_1,longlong *param_2,ulonglong param_3);
basic_istream<> * FUN_180052580(basic_istream<> *param_1,longlong *param_2);
longlong * FUN_180052730(undefined8 *param_1,longlong param_2,longlong *param_3);
undefined8 * FUN_1800527d0(longlong *param_1,longlong *param_2,undefined8 *param_3);
longlong *FUN_180052970(undefined8 *param_1,undefined8 *param_2,longlong param_3,undefined8 param_4);
void FUN_180052a30(longlong *param_1);
undefined8 * FUN_180052a60(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_180052ae0(undefined8 *param_1);
void FUN_180052b10(void);
undefined8 * FUN_180052b30(undefined8 *param_1,longlong param_2);
void FUN_180052b70(longlong param_1);
undefined8 FUN_180052ba0(undefined8 *param_1,undefined8 *param_2);
undefined8 FUN_180052ca0(undefined8 *param_1,undefined8 *param_2);
bool FUN_180052db0(undefined8 *param_1,undefined8 *param_2);
undefined8 FUN_180053040(undefined8 *param_1,undefined8 *param_2);
longlong * FUN_180053250(longlong *param_1,undefined8 *param_2);
undefined8 * FUN_180053610(undefined8 *param_1,uint param_2);
undefined8 * FUN_1800536b0(undefined8 *param_1,uint param_2);
ulonglong FUN_180053750(byte *param_1,undefined1 *param_2,char *param_3);
uint FUN_180053990(char *param_1,long *param_2,char *param_3);
uint FUN_180053a40(char *param_1,long *param_2,char *param_3);
ulonglong FUN_180053af0(char *param_1,ulong *param_2,char *param_3);
void FUN_180053bb0(longlong *param_1);
undefined8 FUN_180053c20(undefined8 *param_1,undefined8 *param_2,uint param_3);
void FUN_180054de0(longlong *param_1);
undefined8 FUN_180054e50(undefined8 *param_1,undefined8 ****param_2,uint param_3);
basic_istream<> * FUN_180055470(basic_istream<> *param_1);
void FUN_180055540(basic_streambuf<> *param_1,locale *param_2);
int FUN_180055590(longlong *param_1);
basic_streambuf<> * FUN_1800555e0(basic_streambuf<> *param_1,char *param_2,size_t param_3);
longlong * FUN_1800556c0(longlong *param_1,longlong *param_2,longlong *param_3);
fpos_t * FUN_180055780(basic_filebuf<> *param_1,fpos_t *param_2,longlong param_3,int param_4);
longlong FUN_180055860(basic_streambuf<> *param_1,char *param_2,size_t param_3);
longlong FUN_180055930(basic_streambuf<> *param_1,char *param_2,ulonglong param_3);
uint FUN_180055a90(longlong param_1);
ulonglong FUN_180055d80(longlong *param_1);
uint FUN_180055df0(longlong param_1,uint param_2);
int FUN_180055ee0(longlong param_1,int param_2);
void FUN_1800560a0(longlong param_1);
void FUN_1800560c0(longlong param_1);
void FUN_1800560e0(basic_streambuf<> *param_1);
basic_ostream<> * FUN_180056150(basic_ostream<> *param_1);
undefined8 * FUN_180056220(undefined8 *param_1,ulonglong param_2,char param_3);
basic_ios<> * FUN_180056350(basic_ios<> *param_1,uint param_2);
basic_streambuf<> * FUN_1800563f0(basic_streambuf<> *param_1,uint param_2);
basic_ios<> * FUN_180056430(basic_ios<> *param_1,uint param_2);
void __thiscall std::basic_filebuf<>::_Reset_back(basic_filebuf<> *this);
bool FUN_180056500(longlong *param_1);
basic_streambuf<> * FUN_1800565f0(basic_streambuf<> *param_1);
basic_streambuf<> * FUN_1800566b0(basic_streambuf<> *param_1,undefined8 param_2,int param_3);
basic_ostream<> * FUN_180056800(basic_ostream<> *param_1);
basic_ostream<> * FUN_180056840(basic_ostream<> *param_1);
_Facet_base * FUN_180056860(locale *param_1);
void FUN_180056950(undefined8 *param_1,void *param_2,ulonglong param_3);
undefined8 * FUN_180056a80(undefined8 *param_1,ulonglong param_2);
void FUN_180056be4(longlong param_1,uint param_2);
void FUN_180056bf0(longlong param_1,uint param_2);
int snprintf(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
undefined8 * FUN_180056c60(undefined8 *param_1,ulonglong param_2);
undefined8 * FUN_180056c90(undefined8 *param_1,longlong param_2);
void FUN_180056cd0(longlong *param_1);
undefined8 * FUN_180056d50(undefined8 *param_1,undefined8 *param_2);
undefined8 * FUN_180056dc0(undefined8 *param_1,ulonglong param_2);
undefined8 * FUN_180056df0(longlong *param_1,undefined8 *param_2,undefined8 *param_3);
longlong * FUN_180056ef0(longlong *param_1,longlong *param_2,undefined8 *param_3,longlong param_4);
int FUN_180057050(int *param_1);
void FUN_1800570a0(undefined8 *param_1);
BOOL __stdcall InitializeCriticalSectionEx(LPCRITICAL_SECTION lpCriticalSection,DWORD dwSpinCount,DWORD Flags);
undefined8 * FUN_1800570f0(undefined8 *param_1,LPCSTR param_2);
bool FUN_1800571b0(LPCSTR param_1);
bool FUN_180057250(longlong *param_1,LPCSTR param_2);
bool FUN_180057330(LPCSTR param_1);
undefined8 FUN_1800573d0(LPCSTR param_1,longlong *param_2);
longlong FUN_180057690(LPCSTR param_1,longlong *param_2);
ulonglong FUN_180057920(byte *param_1,char param_2,int *param_3);
undefined1 FUN_180057b90(LPCSTR param_1,byte *param_2);
undefined1 FUN_180057d60(LPCSTR param_1);
undefined8 FUN_180057e10(byte *param_1,byte *param_2,char param_3);
undefined8 FUN_180058010(byte *param_1,longlong *param_2);
undefined1 FUN_180058120(byte *param_1,longlong *param_2);
void FUN_180058280(byte *param_1);
undefined8 FUN_180058350(LPCSTR param_1);
bool FUN_180058400(undefined8 *param_1);
ulonglong FUN_180058600(longlong *param_1);
void FUN_1800588f0(undefined8 *param_1);
ulonglong FUN_180058920(longlong param_1,longlong *param_2);
void FUN_1800589a0(longlong param_1,longlong *param_2);
undefined8 * FUN_1800589c0(longlong param_1);
undefined8 FUN_180058a30(longlong param_1,longlong *param_2);
void FUN_180058a90(longlong param_1,longlong *param_2);
void FUN_180058ab0(longlong param_1);
bool FUN_180058ae0(undefined8 param_1,longlong *param_2);
void FUN_180058b20(longlong param_1,longlong *param_2);
void FUN_180058b40(void);
void FUN_180058b60(longlong *param_1);
longlong * FUN_180058bd0(longlong *param_1,longlong *param_2);
undefined8 * FUN_180058c80(undefined8 *param_1,uint param_2);
undefined8 * FUN_180058d10(undefined8 *param_1,ulonglong param_2);
undefined8 * FUN_180058d40(undefined8 *param_1,ulonglong param_2);
void FUN_180058d70(longlong param_1,undefined8 param_2);
basic_streambuf<> * FUN_180058d80(basic_streambuf<> *param_1,wchar_t *param_2,int param_3);
undefined8 * FUN_180058ec0(undefined8 *param_1,ulonglong param_2);
longlong * FUN_180058ef0(longlong *param_1,longlong *param_2,longlong *param_3);
void FUN_180059030(undefined8 param_1,undefined8 param_2,longlong *param_3);
void FUN_1800590f0(undefined8 param_1,undefined8 *param_2);
void FUN_180059190(longlong param_1);
void FUN_180059210(longlong param_1,longlong *param_2,longlong *param_3);
longlong * FUN_180059340(longlong *param_1,longlong *param_2,undefined8 *param_3);
void FUN_180059470(undefined8 param_1,undefined8 *param_2,undefined8 *param_3);
uint FUN_1800594d0(undefined8 param_1,undefined8 *param_2,undefined8 *param_3);
void FUN_180059590(undefined8 param_1,basic_ostream<> *param_2);
void FUN_180059610(longlong param_1,char *param_2,int param_3);
basic_ostream<> * FUN_1800596d0(longlong param_1,basic_ostream<> *param_2);
basic_ostream<> * FUN_180059730(longlong param_1,basic_ostream<> *param_2);
basic_ostream<> * FUN_180059790(longlong param_1,basic_ostream<> *param_2);
basic_ostream<> * FUN_1800597f0(longlong param_1,basic_ostream<> *param_2);
basic_ostream<> * FUN_180059850(longlong param_1,basic_ostream<> *param_2);
void FUN_1800598b0(longlong param_1,undefined4 param_2);
undefined4 FUN_180059930(longlong param_1);
undefined8 * FUN_1800599b0(void);
undefined8 * FUN_180059ae0(undefined8 *param_1,uint param_2);
void FUN_180059b20(longlong *param_1);
void FUN_180059b50(undefined8 *param_1);
longlong * FUN_180059d70(longlong param_1,void *param_2);
ulonglong FUN_18005a060(longlong *param_1,longlong param_2);
undefined8 FUN_18005a140(longlong param_1,void *param_2);
void FUN_18005a370(longlong param_1,void *param_2);
void FUN_18005a660(longlong param_1,void *param_2);
void FUN_18005a950(longlong param_1);
undefined8 FUN_18005aae0(longlong param_1,void *param_2,undefined8 *param_3,longlong *param_4);
undefined8 * FUN_18005b5d0(undefined8 *param_1,undefined8 param_2,void *param_3);
undefined8 * FUN_18005b640(undefined8 *param_1,uint param_2);
void FUN_18005b730(longlong param_1,undefined8 param_2,undefined8 param_3);
longlong * FUN_18005b7d0(longlong *param_1,longlong *param_2);
void FUN_18005b8c0(undefined8 param_1,undefined8 param_2,longlong *param_3);
longlong * FUN_18005b980(longlong *param_1,longlong *param_2);
void FUN_18005bd10(longlong *param_1,longlong *param_2);
void FUN_18005bd70(longlong *param_1,longlong param_2);
void FUN_18005bdd0(undefined8 param_1,void *param_2);
ulonglong FUN_18005be40(undefined4 param_1,longlong *param_2);
undefined8 FUN_18005bf00(byte *param_1);
void ** FUN_18005c150(void **param_1,LPCWSTR param_2);
void ** FUN_18005c320(void **param_1,LPCSTR param_2);
longlong * FUN_18005c4f0(longlong *param_1,undefined8 *param_2);
undefined8 FUN_18005c5f0(longlong param_1,longlong *param_2);
void FUN_18005c650(longlong param_1,longlong *param_2);
void FUN_18005c670(longlong param_1);
bool FUN_18005c6a0(undefined8 param_1,int param_2);
char * FUN_18005c6b0(undefined8 param_1,int param_2,char *param_3,longlong param_4);
undefined8 * FUN_18005c720(undefined8 param_1,undefined8 *param_2,int param_3);
void FUN_18005c780(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
undefined ** FUN_18005c7e0(void);
undefined8 * FUN_18005c800(undefined8 *param_1,uint param_2);
undefined8 * FUN_18005c890(undefined8 *param_1,DWORD param_2);
undefined8 FUN_18005cab0(uint param_1);
uint * FUN_18005ce80(undefined8 param_1,uint *param_2,uint param_3);
undefined8 * FUN_18005cee0(undefined8 param_1,undefined8 *param_2,DWORD param_3);
LPSTR FUN_18005cf00(undefined8 param_1,uint param_2,LPSTR param_3,longlong param_4);
void FUN_18005cff0(longlong param_1);
undefined8 FUN_18005d000(longlong param_1,undefined8 param_2);
undefined4 * FUN_18005d020(longlong param_1,undefined4 *param_2);
void * FUN_18005d070(void *param_1,ulonglong param_2);
int * FUN_18005d0a0(int *param_1,int param_2,longlong *param_3);
undefined1 FUN_18005d110(undefined4 *param_1,undefined4 *param_2);
ulonglong FUN_18005d220(longlong *param_1,undefined4 param_2,int *param_3);
ulonglong FUN_18005d2b0(longlong param_1,int *param_2,int param_3);
undefined8 FUN_18005d340(longlong *param_1,ulonglong param_2,int *param_3);
ulonglong FUN_18005d450(longlong param_1,int *param_2,undefined4 param_3);
undefined ** FUN_18005d5c0(longlong param_1);
longlong * FUN_18005d6b0(longlong *param_1,undefined8 *param_2,undefined8 *param_3);
ulonglong FUN_18005d810(char *param_1,uint param_2);
void FUN_18005e430(longlong *param_1);
void FUN_18005e460(longlong *param_1);
longlong * FUN_18005e4c0(longlong *param_1,longlong *param_2,longlong *param_3);
ulonglong FUN_18005e5f0(char ****param_1,undefined8 *param_2);
void FUN_18005e8e0(longlong *param_1,longlong *param_2,longlong *param_3);
ulonglong FUN_18005eab0(undefined8 *param_1,char *param_2,uint param_3);
longlong *** FUN_18005f3c0(void);
int __cdecl _snwprintf_s(wchar_t *_DstBuf,size_t _SizeInWords,size_t _MaxCount,wchar_t *_Format,...);
undefined8 * FUN_180060690(undefined8 *param_1,uint param_2);
void FUN_180060740(undefined8 *param_1);
HMODULE FUN_180060790(longlong *param_1);
bool FUN_1800608e0(longlong param_1,undefined8 *param_2);
undefined8 * FUN_180060970(longlong param_1,undefined8 *param_2,longlong *param_3);
bool FUN_180060a60(int *param_1,int *param_2);
undefined8 FUN_180060d40(longlong param_1,LPCSTR param_2,undefined8 param_3,undefined8 param_4);
undefined4 FUN_180060e90(LPCWSTR param_1);
undefined8 * FUN_1800613b0(undefined8 *param_1,uint param_2);
void FUN_180061410(undefined8 *param_1);
LPWSTR * FUN_180061450(LPWSTR *param_1);
undefined8 *FUN_180061620(undefined8 *param_1,undefined8 param_2,undefined8 *param_3,undefined8 *param_4,undefined8 *param_5);
void FUN_180061890(longlong param_1);
undefined8 FUN_180061990(undefined8 param_1,undefined8 param_2,undefined8 *param_3,undefined8 *param_4,undefined8 *param_5);
ulonglong FUN_180061dd0(undefined8 *param_1,undefined8 param_2);
undefined8 * FUN_180061fd0(undefined8 *param_1,longlong *param_2,longlong param_3);
undefined8 * FUN_1800621b0(undefined8 *param_1);
void FUN_180062260(longlong *param_1,int *param_2);
ulonglong ******* FUN_1800625c0(longlong *param_1,ulonglong *******param_2,longlong *param_3);
ulonglong * FUN_1800632e0(longlong *param_1,ulonglong *param_2,longlong *param_3);
undefined8 *FUN_180063db0(undefined8 *param_1,LPCSTR param_2,undefined8 *param_3,ulonglong param_4,longlong *param_5);
undefined8 * FUN_180064300(undefined8 *param_1,ulonglong param_2);
void FUN_180064330(longlong *param_1);
undefined8 FUN_180064380(longlong param_1);
undefined4 FUN_180064390(longlong param_1);
int FUN_1800643a0(longlong *param_1);
void FUN_1800643d0(longlong *param_1);
void FUN_180064450(longlong param_1);
undefined4 FUN_180064550(longlong *param_1,longlong *param_2,undefined8 param_3,undefined8 param_4);
undefined8 FUN_180064a70(longlong param_1,ulonglong *param_2,undefined8 param_3,undefined8 param_4);
undefined1 FUN_180065590(longlong param_1,longlong *param_2,size_t param_3,undefined8 param_4);
undefined8 * FUN_180065cf0(undefined8 *param_1,uint param_2);
void FUN_180065d40(longlong param_1);
void FUN_180065d50(longlong param_1);
void FUN_180065d60(longlong param_1);
undefined8 * FUN_180065d80(undefined8 *param_1,longlong param_2);
undefined8 * FUN_180065f00(undefined8 *param_1,longlong param_2);
undefined8 *** FUN_180065fc0(undefined8 ***param_1);
bool FUN_180066b50(void);
ulonglong FUN_180067630(void);
undefined8 FUN_1800677d0(double *param_1,double *param_2);
int FUN_180068230(longlong param_1,float *param_2,undefined4 *param_3,undefined8 *param_4,undefined1 *param_5);
int FUN_180068750(longlong param_1,float *param_2,undefined8 param_3,int *param_4,double *param_5,double *param_6,double *param_7,int *param_8);
int FUN_180068f40(longlong param_1,longlong param_2,undefined8 param_3,int *param_4);
undefined8 FUN_1800696a0(longlong *param_1,undefined8 param_2,longlong param_3,Gpu_ImageBuffer *param_4);
void FUN_1800699b0(longlong *param_1);
longlong FUN_180069b50(longlong param_1);
void FUN_180069c00(longlong param_1);
undefined8 FUN_180069c60(undefined8 param_1,longlong *param_2);
undefined8 FUN_180069dd0(longlong param_1,longlong param_2);
undefined8 * FUN_18006aaf0(undefined8 *param_1,longlong param_2,longlong param_3);
void FUN_18006ad60(longlong *param_1);
void FUN_18006ae70(longlong param_1);
void FUN_18006ae90(longlong param_1);
longlong FUN_18006af10(longlong *param_1);
undefined8 FUN_18006b0c0(longlong *param_1,longlong *param_2);
undefined8 *FUN_18006bb70(undefined8 *param_1,longlong param_2,undefined8 param_3,undefined8 param_4);
void FUN_18006bd40(undefined8 *param_1);
undefined8 FUN_18006bf70(longlong param_1,undefined8 param_2,undefined8 param_3);
void FUN_18006c060(longlong param_1);
undefined8 FUN_18006c070(longlong *param_1,undefined8 param_2,int *param_3);
undefined8 FUN_18006c150(longlong param_1);
undefined8 FUN_18006c160(longlong *param_1,undefined8 param_2,float *param_3);
undefined8 FUN_18006c270(longlong *param_1,undefined8 param_2,undefined4 *param_3);
undefined8 FUN_18006c340(longlong param_1,undefined8 param_2,undefined1 *param_3);
undefined8 FUN_18006c3a0(longlong *param_1,undefined8 param_2,float *param_3,float *param_4);
undefined8 FUN_18006c780(longlong *param_1,undefined8 param_2,float *param_3,float *param_4,float *param_5);
undefined8 FUN_18006c980(longlong *param_1,undefined8 param_2,float *param_3);
undefined8 FUN_18006ca70(longlong param_1,undefined8 param_2,int *param_3);
undefined8 FUN_18006ca90(longlong param_1);
int FUN_18006cab0(longlong param_1);
void FUN_18006cad0(longlong *param_1,double param_2);
uint FUN_18006cb20(longlong *param_1,float *param_2);
void FUN_18006cc50(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
longlong FUN_18006cc80(longlong param_1,char *param_2);
char * FUN_18006cce0(longlong param_1,char *param_2);
undefined8 FUN_18006cde0(longlong param_1,void *param_2,undefined4 *param_3);
undefined8 FUN_18006d0b0(longlong param_1,void *param_2,float *param_3);
undefined8 FUN_18006d380(longlong param_1,void *param_2,undefined8 param_3);
undefined8 FUN_18006d630(longlong param_1,void *param_2,undefined8 param_3);
undefined8 FUN_18006d8e0(longlong param_1,void *param_2,float *param_3,float *param_4);
undefined8 FUN_18006dbd0(longlong param_1,void *param_2,float *param_3,float *param_4,float *param_5);
undefined8 FUN_18006def0(longlong param_1,void *param_2,float *param_3);
longlong FUN_18006e1c0(longlong param_1,char *param_2);
undefined8 FUN_18006e3e0(longlong param_1,undefined4 param_2,longlong param_3);
undefined8 FUN_18006e4d0(longlong param_1);
undefined8 FUN_18006e4f0(longlong param_1);
undefined8 FUN_18006e510(longlong param_1);
double FUN_18006e530(longlong param_1);
double FUN_18006e580(longlong param_1);
undefined8 FUN_18006e5d0(longlong param_1);
undefined4 FUN_18006e5e0(longlong param_1);
undefined4 FUN_18006e5f0(longlong param_1);
undefined4 FUN_18006e600(longlong param_1);
double FUN_18006e610(longlong param_1);
undefined8 FUN_18006e650(longlong param_1,undefined8 param_2,longlong *param_3,int *param_4,undefined8 param_5,longlong *param_6);
undefined8 FUN_18006e7a0(longlong param_1,longlong *param_2);
undefined8 FUN_18006e8d0(undefined8 param_1,longlong *param_2);
ulonglong FUN_18006e950(longlong param_1,undefined8 *param_2,undefined8 param_3,undefined8 param_4);
void FUN_18006eb60(longlong param_1,longlong *param_2);
void FUN_18006ebd0(undefined8 param_1,void *param_2,uint param_3,undefined8 *param_4,longlong param_5);
undefined8 * FUN_18006f1e0(undefined8 *param_1,uint param_2);
void FUN_18006f220(undefined8 *param_1);
Gpu_ImageBuffer *FUN_18006f270(longlong param_1,Gpu_ImageBuffer *param_2,longlong param_3,int *param_4);
void FUN_18006f6b0(longlong param_1);
undefined4 FUN_18006f8b0(longlong *param_1,int param_2,double param_3);
ulonglong FUN_1800701a0(longlong *param_1,int param_2,double param_3,Gpu_ImageBuffer *param_4);
void FUN_180070510(longlong *param_1,double param_2,Gpu_ImageBuffer *param_3);
void FUN_180070540(longlong *param_1,double param_2,Gpu_ImageBuffer *param_3);
int FUN_180070570(longlong param_1);
double FUN_180070590(longlong *param_1);
double FUN_1800705e0(longlong *param_1);
undefined8 FUN_180070630(longlong param_1);
void FUN_180070640(longlong param_1);
undefined8 FUN_180070650(longlong param_1,longlong *param_2);
float * FUN_1800706a0(longlong param_1,float *param_2);
undefined8 * FUN_180070800(longlong param_1,undefined8 *param_2);
ulonglong FUN_180070ac0(longlong param_1,double param_2,double param_3);
ulonglong FUN_1800710e0(longlong param_1,longlong param_2);
bool FUN_180071600(longlong param_1);
void FUN_180071610(longlong param_1,float *param_2,float *param_3);
longlong FUN_180071690(longlong param_1,longlong param_2);
ulonglong FUN_1800716d0(longlong param_1,longlong param_2);
ulonglong FUN_180071740(longlong param_1,longlong param_2,undefined4 *param_3,undefined4 *param_4,uint *param_5,undefined4 *param_6);
ulonglong FUN_180071810(longlong param_1,longlong param_2,undefined8 *param_3);
undefined8 * FUN_1800718d0(undefined8 *param_1,uint param_2);
undefined8 FUN_180071950(longlong param_1,undefined4 *param_2,undefined8 param_3,undefined8 param_4);
undefined8 FUN_180072170(longlong param_1,longlong *param_2,undefined8 param_3,undefined8 param_4);
int FUN_180072520(longlong param_1,undefined8 *param_2,undefined4 *param_3,undefined4 *param_4,undefined4 *param_5,double *param_6,longlong param_7);
ulonglong FUN_180072a30(longlong param_1,longlong param_2,longlong *param_3,longlong param_4,longlong param_5,longlong *param_6,longlong **param_7,double *param_8,char param_9,longlong *param_10,longlong param_11,undefined8 param_12);
undefined8 * FUN_1800753c0(undefined8 *param_1);
void FUN_1800756c0(undefined8 *param_1);
ulonglong FUN_180075790(undefined8 *param_1,undefined8 *param_2,longlong *param_3,undefined8 param_4);
ulonglong FUN_180076540(longlong param_1,longlong param_2,undefined8 param_3,undefined8 param_4,longlong *param_5,double *param_6,char param_7);
void FUN_180076760(longlong *param_1);
void FUN_180076830(longlong param_1);
undefined8 FUN_180076840(undefined8 param_1,undefined1 *param_2,undefined1 *param_3,undefined1 *param_4,undefined1 *param_5,undefined1 *param_6);
undefined8 FUN_180076b70(undefined8 param_1,void *param_2,undefined4 *param_3);
undefined8 * FUN_180076c80(longlong param_1,undefined8 *param_2,undefined1 *param_3);
undefined8 FUN_180076e60(longlong param_1,void *param_2,short *param_3,undefined2 param_4,undefined8 param_5,int param_6);
undefined8 FUN_180077350(longlong param_1,undefined8 param_2,undefined1 *param_3,void *param_4,int param_5);
void FUN_1800777f0(longlong param_1,int param_2);
undefined8 FUN_180077800(longlong param_1,undefined8 param_2,void *param_3,float *param_4,float param_5,float param_6,float param_7,float param_8,undefined2 param_9,int param_10);
ulonglong FUN_180077be0(longlong param_1,undefined1 *param_2,float *param_3,float param_4,float param_5,float param_6,float param_7,undefined2 param_8,int param_9);
int FUN_180077cf0(longlong param_1,undefined1 *param_2,undefined4 *param_3,undefined4 param_4,undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined8 param_8,int param_9);
int FUN_180078000(longlong param_1,undefined1 *param_2,byte *param_3,int param_4);
int FUN_1800782f0(longlong param_1,undefined1 *param_2,float *param_3,float *param_4,undefined1 param_5,int param_6);
int FUN_180078610(longlong param_1,undefined1 *param_2,float *param_3,float *param_4,float *param_5,int param_6);
int FUN_180078940(longlong param_1,undefined1 *param_2,float *param_3,int param_4);
undefined8 FUN_180078c30(longlong param_1,void *param_2,int param_3);
undefined8 FUN_1800790f0(longlong param_1,int param_2);
undefined8 FUN_180079220(longlong param_1,undefined8 param_2,void *param_3,undefined4 *param_4,int param_5);
ulonglong FUN_1800794d0(longlong param_1,undefined1 *param_2,undefined4 *param_3,int param_4);
void FUN_180079590(longlong param_1,char *param_2,int param_3);
ulonglong FUN_180079760(longlong param_1,float *param_2);
longlong FUN_18007a300(longlong param_1,char *param_2);
longlong FUN_18007a3d0(longlong param_1,char *param_2);
char * FUN_18007a430(longlong param_1,char *param_2);
int FUN_18007a530(longlong param_1,void *param_2,undefined8 param_3,undefined8 param_4,int param_5,longlong *param_6);
longlong * FUN_18007a9d0(longlong *param_1,uint param_2);
undefined8 *FUN_18007aa10(undefined8 *param_1,longlong param_2,undefined4 param_3,undefined8 param_4,undefined1 param_5,undefined1 param_6);
void FUN_18007b4b0(longlong *param_1);
undefined8 FUN_18007bb80(longlong param_1);
void FUN_18007bb90(undefined8 param_1,longlong *param_2,int *param_3,int *param_4,int *param_5);
void FUN_18007bde0(longlong param_1,int *param_2,int *param_3,int *param_4);
void FUN_18007c2a0(longlong param_1,undefined4 *param_2,undefined4 *param_3,undefined4 *param_4);
bool FUN_18007c3b0(longlong param_1);
undefined8 FUN_18007c7c0(longlong param_1,void *param_2);
void FUN_18007cbb0(longlong param_1,longlong param_2);
void FUN_18007cbc0(longlong param_1,longlong param_2);
void FUN_18007cbd0(longlong param_1,longlong param_2);
void FUN_18007cbf0(longlong param_1,undefined8 *param_2,undefined8 param_3);
undefined4 FUN_18007cd80(undefined8 *param_1,void *param_2,undefined8 param_3,char ****param_4);
undefined8 FUN_18007d490(longlong *param_1,void *param_2,int *param_3,char ****param_4);
undefined8 FUN_18007d7e0(longlong param_1);
undefined8 FUN_18007d800(longlong *param_1,void *param_2,float *param_3,char ****param_4);
undefined8 FUN_18007dbd0(longlong *param_1,void *param_2,int *param_3,char ****param_4);
undefined8 FUN_18007df10(undefined8 *param_1,void *param_2,byte *param_3,char ****param_4);
undefined8 FUN_18007e1e0(longlong *param_1,void *param_2,float *param_3,char ****param_4,undefined1 param_5,undefined4 param_6);
undefined8 FUN_18007e540(longlong *param_1,void *param_2,float *param_3,char ****param_4,float *param_5);
undefined8 FUN_18007e8d0(longlong *param_1,void *param_2,float *param_3,char ****param_4);
undefined8 FUN_18007ec60(undefined8 *param_1,void *param_2,undefined8 param_3,char ****param_4);
undefined8 FUN_18007ee90(undefined8 *param_1,void *param_2,int *param_3,char ****param_4);
void FUN_18007f0b0(longlong param_1,longlong param_2,int param_3);
undefined1 FUN_18007f1f0(longlong param_1);
ulonglong FUN_18007f220(longlong param_1,void *param_2);
ulonglong FUN_18007f400(longlong param_1,void *param_2);
longlong FUN_18007f5e0(longlong param_1,char *param_2);
undefined8 FUN_18007f830(longlong param_1,void *param_2,undefined8 param_3,char ****param_4);
undefined8 FUN_18007fb10(undefined8 *param_1,void *param_2,longlong *param_3,char ****param_4,undefined8 param_5,longlong *param_6);
longlong * FUN_18007fe40(longlong *param_1,uint param_2);
undefined8 * FUN_18007fe80(undefined8 *param_1,ulonglong param_2);
undefined8 * FUN_18007feb0(undefined8 *param_1,ulonglong param_2);
undefined8 * FUN_18007fef0(undefined8 *param_1,ulonglong param_2);
undefined8 * FUN_18007ff20(undefined8 *param_1,ulonglong param_2);
ulonglong FUN_18007ff50(longlong param_1,longlong param_2,longlong param_3,char param_4,char param_5,int *param_6);
undefined8 FUN_1800805d0(longlong param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
ulonglong FUN_180080630(longlong param_1,longlong param_2,undefined8 param_3,undefined8 param_4);
ulonglong FUN_180081130(longlong param_1,longlong param_2,undefined8 param_3,undefined8 param_4,longlong *param_5,byte param_6);
undefined8 FUN_1800811a0(longlong param_1,longlong param_2,longlong *param_3);
ulonglong FUN_180081370(longlong param_1,longlong param_2);
ulonglong EntryPointFunc(uint param_1,undefined8 *param_2,longlong *param_3,longlong *param_4,longlong param_5,longlong *param_6);
undefined8 FUN_180082700(HMODULE param_1,int param_2);
void FUN_180082720(undefined8 *param_1);
longlong * FUN_180082760(longlong *param_1,longlong *param_2);
void FUN_180082880(longlong *param_1);
void FUN_1800828f0(longlong *param_1);
longlong * FUN_180082950(longlong *param_1,longlong *param_2);
ulonglong FUN_180082a40(LPCSTR param_1,longlong *param_2,size_t *param_3,undefined8 param_4,longlong *param_5);
ulonglong FUN_180082d50(LPCSTR param_1,longlong *param_2,ulonglong *param_3,undefined8 param_4,longlong *param_5);
float * FUN_180083040(float *param_1,float *param_2,float *param_3);
undefined8 * FUN_180083160(undefined8 *param_1,float *param_2,float *param_3);
float * FUN_1800832e0(float *param_1,float *param_2,float *param_3);
undefined8 FUN_180083820(longlong param_1,longlong param_2);
void FUN_180084270(undefined8 param_1,undefined8 param_2,longlong *param_3);
void FUN_1800842d0(longlong *param_1,undefined8 param_2);
void FUN_180084350(longlong *param_1,longlong *param_2,undefined8 param_3,undefined8 param_4);
void FUN_180084400(float *param_1,longlong *param_2,longlong *param_3);
void FUN_1800846c0(longlong param_1);
undefined8 * FUN_1800846d0(undefined8 *param_1,ulonglong param_2);
ulonglong * FUN_180084720(ulonglong *param_1,char *param_2,char *param_3);
longlong *FUN_180084890(undefined8 *param_1,undefined8 *param_2,longlong param_3,undefined8 param_4);
void FUN_180084948(longlong param_1,uint param_2);
void FUN_180084954(longlong param_1,uint param_2);
void FUN_180084960(longlong param_1,uint param_2);
void FUN_18008496c(longlong param_1,uint param_2);
void FUN_180084978(longlong param_1,uint param_2);
void FUN_180084984(longlong param_1,uint param_2);
void FUN_180084990(longlong param_1,uint param_2);
void FUN_18008499c(longlong param_1,uint param_2);
void FUN_1800849b0(longlong *param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void FUN_1800849f0(longlong *param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
undefined8 * FUN_180084a30(undefined8 *param_1,ulonglong param_2);
void FUN_180084a60(undefined8 param_1,undefined8 param_2,undefined8 param_3);
undefined8 * FUN_180084ab0(undefined8 *param_1,uint param_2);
undefined8 FUN_180084b60(longlong param_1,undefined1 *param_2,undefined1 *param_3,undefined1 *param_4,undefined1 *param_5,undefined1 *param_6);
undefined8 FUN_180084ee0(longlong param_1);
void FUN_180084ef0(longlong param_1,char *param_2,int param_3);
ulonglong FUN_180085120(longlong param_1);
ulonglong FUN_180085130(longlong param_1);
ulonglong FUN_180085140(longlong param_1);
longlong FUN_180085150(longlong param_1,char *param_2);
longlong FUN_180085240(longlong param_1,char *param_2);
char * FUN_1800852a0(longlong param_1,char *param_2);
void FUN_180085394(longlong param_1,uint param_2);
float * FUN_1800853a0(float *param_1,longlong param_2,longlong param_3,float param_4,float param_5);
undefined8 FUN_1800855b0(longlong *param_1,char param_2,longlong *param_3,Gpu_ImageBuffer *param_4);
undefined8 *FUN_180085920(longlong param_1,undefined8 *param_2,undefined8 *param_3,undefined8 *param_4);
uint FUN_180085b60(longlong *param_1,byte param_2);
uint FUN_180087110(longlong *param_1,undefined8 *param_2,undefined8 param_3,int param_4);
void FUN_1800872d0(longlong *param_1);
void FUN_180087310(longlong *param_1);
void FUN_180087510(longlong *param_1);
void FUN_180087590(void);
void FUN_18008a040(undefined8 *param_1);
void FUN_18008a110(undefined8 *param_1,float param_2,undefined4 param_3);
undefined8 *FUN_18008a4e0(undefined8 *param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4,undefined8 param_5,undefined8 param_6,undefined8 param_7,undefined8 param_8,undefined8 param_9,undefined8 param_10,undefined8 param_11,undefined8 param_12,undefined8 param_13,undefined8 param_14,undefined8 param_15,undefined8 param_16,undefined8 param_17,undefined8 param_18);
void FUN_18008a580(undefined8 *param_1,int param_2,int param_3);
undefined8 *FUN_18008acd0(undefined8 *param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4,undefined8 param_5,undefined8 param_6,undefined8 param_7,undefined8 param_8,undefined8 param_9,undefined8 param_10,undefined8 param_11,undefined8 param_12,undefined8 param_13,undefined8 param_14);
void FUN_18008ad40(undefined8 *param_1);
undefined8 *FUN_18008b520(undefined8 *param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4,undefined8 param_5,undefined8 param_6,undefined8 param_7,undefined8 param_8,undefined8 param_9,undefined8 param_10,undefined8 param_11,undefined8 param_12,undefined8 param_13,undefined8 param_14,undefined8 param_15,undefined8 param_16,undefined8 param_17,undefined8 param_18,undefined8 param_19);
void FUN_18008b5d0(undefined8 *param_1);
undefined8 * FUN_18008b5f0(undefined8 *param_1,undefined8 *param_2);
void FUN_18008b6e0(longlong param_1);
float FUN_18008b710(float param_1,float param_2,float param_3);
float * FUN_18008b720(float *param_1,longlong *param_2);
undefined4 * FUN_18008b770(undefined4 *param_1,longlong *param_2);
float * FUN_18008b7a0(float *param_1,longlong *param_2);
undefined8 * FUN_18008b7f0(undefined8 *param_1,uint param_2);
undefined8 * FUN_18008b870(undefined8 *param_1,uint param_2);
void FUN_18008b8f0(longlong param_1);
longlong * FUN_18008b950(longlong *param_1,uint param_2);
undefined8 * FUN_18008ba50(undefined8 *param_1,undefined1 *param_2);
ulonglong FUN_18008baa0(longlong *param_1,char *param_2,undefined8 *param_3);
void FUN_18008bb80(char *param_1);
void FUN_18008bc00(undefined8 *param_1);
void FUN_18008bc60(longlong param_1,undefined4 param_2,undefined4 *param_3,undefined4 *param_4,undefined4 *param_5,undefined8 param_6,undefined8 param_7);
void FUN_18008bd20(longlong param_1,undefined8 param_2,undefined4 param_3,undefined4 param_4,int param_5,int param_6,int *param_7,uint *param_8,undefined4 *param_9);
undefined4 * FUN_18008bef0(longlong param_1,undefined4 *param_2);
void FUN_18008c290(longlong param_1,undefined4 param_2,int *param_3);
undefined8 *FUN_18008c360(undefined8 *param_1,longlong *param_2,longlong *param_3,longlong *param_4,undefined8 param_5,longlong *param_6);
void * FUN_18008c590(void *param_1,uint param_2);
undefined4 * FUN_18008c5d0(longlong param_1,undefined4 *param_2,undefined8 **********param_3);
longlong * FUN_18008c870(longlong *param_1,char *param_2,char *param_3);
longlong * FUN_18008cd10(longlong *param_1,char *param_2,char *param_3);
undefined8 *FUN_18008d1b0(undefined8 *param_1,longlong *param_2,undefined8 *param_3,undefined8 *******param_4);
undefined8 *FUN_18008e690(undefined8 *param_1,longlong *param_2,longlong *param_3,longlong *param_4);
void * FUN_18008e830(void *param_1,uint param_2);
undefined4 * FUN_18008e870(longlong param_1,undefined4 *param_2,undefined8 **********param_3);
void FUN_18008e9d0(longlong param_1);
undefined8 *FUN_18008ea90(undefined8 *param_1,LPCSTR param_2,undefined8 *param_3,undefined8 **********param_4);
void FUN_18008f020(undefined8 param_1,undefined4 *param_2,basic_istream<> *param_3);
undefined8 * FUN_18008f0f0(undefined8 *param_1,ulonglong param_2);
undefined8 * FUN_18008f130(undefined8 *param_1,longlong *param_2,longlong *param_3);
void * FUN_18008f2a0(void *param_1,uint param_2);
undefined4 * FUN_18008f2e0(longlong param_1,undefined4 *param_2,longlong *param_3);
void FUN_18008f680(longlong param_1);
void FUN_18008f6b0(longlong *param_1);
undefined8 FUN_18008f6f0(longlong param_1,byte param_2);
int FUN_18008f740(longlong param_1,longlong *param_2,undefined8 param_3);
void FUN_18008f7e0(undefined8 *param_1,uint param_2);
undefined8 FUN_18008f940(longlong param_1,undefined1 *param_2,undefined4 param_3);
void FUN_18008f9d0(longlong *param_1,longlong *param_2,longlong param_3,char param_4);
undefined8 FUN_18008fc90(longlong param_1,uint param_2);
undefined8 FUN_18008fd20(longlong param_1,uint param_2);
void FUN_18008fdb0(longlong *param_1);
longlong FUN_18008fe50(ulonglong *param_1,longlong param_2);
void FUN_18008ff10(undefined4 *param_1,undefined8 *param_2,ulonglong *param_3);
void FUN_18008ffa0(longlong *param_1);
void FUN_180090040(longlong param_1,longlong *param_2,longlong param_3);
void FUN_180090870(longlong *param_1,longlong *param_2,longlong param_3);
void FUN_180090e40(longlong *param_1);
void FUN_180090e90(undefined8 param_1,longlong *param_2);
undefined4 * FUN_180090ee0(undefined4 *param_1,longlong *param_2);
void FUN_180091060(longlong *param_1);
undefined8 * FUN_1800910f0(undefined8 *param_1,ulonglong param_2);
undefined1 FUN_180091120(longlong *param_1,longlong *param_2);
undefined1 FUN_180091270(longlong *param_1,LPCSTR param_2);
undefined1 FUN_1800915a0(longlong *param_1,undefined8 param_2);
void * FUN_1800917a0(ulonglong *param_1,void *param_2,longlong param_3,longlong param_4);
uint * FUN_180091870(uint *param_1,undefined8 *param_2,undefined8 *param_3,ulonglong *param_4);
void FUN_180091940(longlong param_1);
undefined8 FUN_1800919e0(longlong param_1,undefined8 *param_2);
void FUN_180091a80(longlong param_1,undefined8 *param_2,undefined8 *param_3);
longlong * FUN_180091e00(float *param_1,longlong *param_2,undefined8 *param_3);
void FUN_1800920b0(undefined8 *param_1,longlong *param_2);
void FUN_180092cf0(undefined8 *param_1,longlong *param_2);
undefined8 * FUN_180094430(undefined8 *param_1,ulonglong param_2);
longlong FUN_180094470(longlong **param_1);
undefined8 * FUN_1800945c0(undefined8 *param_1,uint param_2);
undefined8 * FUN_180094610(longlong param_1);
ulonglong FUN_180094660(longlong param_1,longlong *param_2);
void FUN_180094740(longlong param_1,longlong *param_2);
longlong * FUN_180094760(longlong *param_1,undefined8 *param_2);
undefined8 * FUN_180094800(undefined8 *param_1,ulonglong param_2);
longlong FUN_180094830(longlong **param_1);
undefined8 * FUN_180094a20(undefined8 *param_1,uint param_2);
undefined8 * FUN_180094a90(longlong param_1);
ulonglong FUN_180094ae0(longlong param_1,longlong *param_2);
void FUN_180094be0(longlong param_1,longlong *param_2);
undefined8 * FUN_180094c30(undefined8 *param_1,longlong param_2);
void FUN_180094da0(longlong param_1);
void FUN_180094eb0(undefined8 *param_1);
double FUN_180094ee0(double *param_1);
void FUN_180094f40(longlong param_1,undefined8 *param_2);
void FUN_180095000(longlong *param_1,longlong *param_2);
longlong * FUN_180095140(longlong *param_1,longlong *param_2,longlong *param_3,longlong *param_4);
longlong * FUN_180095270(longlong *param_1,longlong *param_2,longlong param_3,undefined8 *param_4);
void FUN_180095430(longlong param_1);
void FUN_180095440(longlong param_1);
void FUN_180095450(longlong param_1);
void FUN_180095460(longlong param_1);
void FUN_180095480(longlong param_1);
void FUN_180095490(longlong param_1);
double * FUN_1800954a0(undefined8 *param_1,double *param_2,ulonglong *param_3);
void FUN_180095e40(longlong *param_1,uint *param_2);
void FUN_180096020(void);
void FUN_180096110(undefined4 param_1,undefined4 param_2,undefined8 param_3,undefined4 param_4,undefined8 param_5,undefined4 param_6);
void FUN_180096230(undefined4 param_1,undefined4 param_2,undefined8 param_3,undefined4 param_4,undefined8 param_5,undefined4 param_6);
void FUN_180096350(int param_1,int param_2,undefined8 param_3,undefined4 param_4,undefined8 param_5,undefined4 param_6);
void FUN_180096400(int param_1,int param_2,undefined8 param_3,undefined4 param_4,undefined8 param_5,undefined4 param_6);
void FUN_1800964ab(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_clEnqueueReadBufferRect(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_clEnqueueWriteBufferRect(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void cudaDeviceSynchronize(void);
void FUN_180096542(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_cudaDeviceSynchronize(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void cudaLaunchKernel(void);
void DelayLoad_cudaLaunchKernel(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void __cudaPushCallConfiguration(void);
void DelayLoad___cudaPushCallConfiguration(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void __cudaPopCallConfiguration(void);
void DelayLoad___cudaPopCallConfiguration(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void __cudaRegisterFatBinary(void);
void DelayLoad___cudaRegisterFatBinary(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void __cudaRegisterFatBinaryEnd(void);
void DelayLoad___cudaRegisterFatBinaryEnd(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad___cudaUnregisterFatBinary(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void __cudaRegisterFunction(void);
void DelayLoad___cudaRegisterFunction(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_cudaMemcpy2D(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_cudaGraphicsUnregisterResource(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_cudaGraphicsMapResources(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_cudaGraphicsGLRegisterImage(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_cudaMemcpyToArrayAsync(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_cudaMemcpyFromArrayAsync(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_cudaFree(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_cudaGraphicsSubResourceGetMappedArray(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_cudaMalloc(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_cudaGraphicsUnmapResources(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_cudaGetDeviceCount(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void FUN_1800966c9(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?GetCpuMemory@Gpu_ImageBuffer@UNIVERSE_5_1_U@@QEBAPEAEXZ(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?AllocateCpuMemory@Gpu_ImageBuffer@UNIVERSE_5_1_U@@QEAAX_K@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?makeGpuShaderContext@UNIVERSE_5_1_U@@YAPEAVGpu_ShaderContext_API@1@PEBD@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?usingDevice@Gpu_Engine@UNIVERSE_5_1_U@@QEAA_NPEBX@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?deleteRenderer@Gpu_Engine@UNIVERSE_5_1_U@@QEAAXXZ(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?lockRenderer@Gpu_Engine@UNIVERSE_5_1_U@@SA?AV?$lock_guard@Vrecursive_mutex@std@@@std@@XZ(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?provideLockedGpuShaderContext@UNIVERSE_5_1_U@@YAPEAVGpu_ShaderContext_API@1@PEAV21@_NPEAXPEBD@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?resetoreAndDestroyLockedGpuShaderContext@UNIVERSE_5_1_U@@YAHPEAVGpu_ShaderContext_API@1@@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?resetoreLockedGpuShaderContext@UNIVERSE_5_1_U@@YAHPEAVGpu_ShaderContext_API@1@@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?teardownGpuShaderContext@UNIVERSE_5_1_U@@YAXPEAVGpu_ShaderContext_API@1@@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?provideLockedGpuShaderContext2@UNIVERSE_5_1_U@@YAPEAVGpu_ShaderContext_API@1@PEAV21@_NPEAXPEBD22@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?SetAnisotropic@Properties@legacy@UNIVERSE_5_1_U@@SAXAEB_NAEA_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?SetMask@Properties@legacy@UNIVERSE_5_1_U@@SAXAEBHAEAH@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?GetMask@Properties@legacy@UNIVERSE_5_1_U@@SAHAEBH@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?SetFg@Properties@legacy@UNIVERSE_5_1_U@@SAXAEBHAEAH@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?GetFg@Properties@legacy@UNIVERSE_5_1_U@@SAHAEBH@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?SetBg@Properties@legacy@UNIVERSE_5_1_U@@SAXAEBHAEAH@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?GetBg@Properties@legacy@UNIVERSE_5_1_U@@SAHAEBH@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?GetDuration@Properties@legacy@UNIVERSE_5_1_U@@SAMAEBM@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?GetFPS@Properties@legacy@UNIVERSE_5_1_U@@SAMAEBM@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?GetTime@Properties@legacy@UNIVERSE_5_1_U@@SAMAEBM@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?SetFrameFloat@Properties@legacy@UNIVERSE_5_1_U@@SAXAEBMAEAM@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?GetFrameFloat@Properties@legacy@UNIVERSE_5_1_U@@SAMAEBM@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?SetFrame@Properties@legacy@UNIVERSE_5_1_U@@SAXAEBHAEAH@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?GetFrame@Properties@legacy@UNIVERSE_5_1_U@@SAHAEBH@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?GetAspect@Properties@legacy@UNIVERSE_5_1_U@@SAMAEBM@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?GetScaleY@Properties@legacy@UNIVERSE_5_1_U@@SAMAEBM@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?GetScaleX@Properties@legacy@UNIVERSE_5_1_U@@SAMAEBM@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?GetCameraScale@Properties@legacy@UNIVERSE_5_1_U@@SAMAEBM@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?GetHeight@Properties@legacy@UNIVERSE_5_1_U@@SAMAEBM@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?GetWidth@Properties@legacy@UNIVERSE_5_1_U@@SAMAEBM@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?GetFullHeight@Properties@legacy@UNIVERSE_5_1_U@@SAMAEBM@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?GetFullWidth@Properties@legacy@UNIVERSE_5_1_U@@SAMAEBM@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?SetForbidden@Properties@legacy@UNIVERSE_5_1_U@@SAXAEBMAEAM@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?SetOld_premult@Properties@legacy@UNIVERSE_5_1_U@@SAXAEBHAEAH@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?SetIgnore@Properties@legacy@UNIVERSE_5_1_U@@SAXAEB_NAEA_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?SetColorSpaceAware@Properties@legacy@UNIVERSE_5_1_U@@SAXAEB_NAEA_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?SetMaxResolution@Properties@legacy@UNIVERSE_5_1_U@@SAXAEBHAEAH@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?SetTemporal@Properties@legacy@UNIVERSE_5_1_U@@SAXAEB_NAEA_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?SetLayers@Properties@legacy@UNIVERSE_5_1_U@@SAXAEB_NAEA_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?SetBorder@Properties@legacy@UNIVERSE_5_1_U@@SAXAEB_NAEA_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?SetTransition@Properties@legacy@UNIVERSE_5_1_U@@SAXAEB_NAEA_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?SetAnimate@Properties@legacy@UNIVERSE_5_1_U@@SAXAEB_NAEA_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?SetPercent@Properties@legacy@UNIVERSE_5_1_U@@SAXAEB_NAEA_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?SetOpen@Properties@legacy@UNIVERSE_5_1_U@@SAXAEB_NAEA_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?SetVisible@Properties@legacy@UNIVERSE_5_1_U@@SAXAEB_NAEA_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?SetEnable@Properties@legacy@UNIVERSE_5_1_U@@SAXAEB_NAEA_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?SetUpdate@Properties@legacy@UNIVERSE_5_1_U@@SAXAEB_NAEA_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?SetMinimal@Properties@legacy@UNIVERSE_5_1_U@@SAXAEB_NAEA_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?SetNulls@Properties@legacy@UNIVERSE_5_1_U@@SAXAEB_NAEA_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?SetRXLights@Properties@legacy@UNIVERSE_5_1_U@@SAXAEB_NAEA_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?SetLights@Properties@legacy@UNIVERSE_5_1_U@@SAXAEB_NAEA_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?SetRequireParamIds@Properties@legacy@UNIVERSE_5_1_U@@SAXAEB_NAEA_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?SetRXCamera@Properties@legacy@UNIVERSE_5_1_U@@SAXAEB_NAEA_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?SetOldCamera@Properties@legacy@UNIVERSE_5_1_U@@SAXAEB_NAEA_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?SetVary@Properties@legacy@UNIVERSE_5_1_U@@SAXAEB_NAEA_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?SetGenerator@Properties@legacy@UNIVERSE_5_1_U@@SAXAEB_NAEA_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?SetCustom@Properties@legacy@UNIVERSE_5_1_U@@SAXAEB_NAEA_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?GetInt@Properties@legacy@UNIVERSE_5_1_U@@SAHAEBH@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?GetBool@Properties@legacy@UNIVERSE_5_1_U@@SA_NAEB_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?clearCache@Gpu_Engine@UNIVERSE_5_1_U@@QEAAXXZ(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?changeSpace@Gpu_Engine@UNIVERSE_5_1_U@@QEAAXW4MatrixMode@2@@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?ortho@Gpu_Engine@UNIVERSE_5_1_U@@QEAAXMMMMMM@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?perspective@Gpu_Engine@UNIVERSE_5_1_U@@QEAAXMMMM@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?scale@Gpu_Engine@UNIVERSE_5_1_U@@QEAAXAEBU?$vec@$02M$0A@@Math@Axion@@@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?identity@Gpu_Engine@UNIVERSE_5_1_U@@QEAAXXZ(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?getPerspective@Gpu_Engine@UNIVERSE_5_1_U@@SA?AU?$mat@$03$03M$0A@@Math@Axion@@MMMM@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?currentRenderer@Gpu_Engine@UNIVERSE_5_1_U@@SAAEAVRenderer@Pipeline@Axion@@XZ(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?isFeatureBlacklisted@UNIVERSE_5_1_U@@YA_NAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@000HHH@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?ClearCpuMemory@Gpu_ImageBuffer@UNIVERSE_5_1_U@@QEAAXXZ(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?SetCpuMemory@Gpu_ImageBuffer@UNIVERSE_5_1_U@@QEAAXPEAE@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?translate@GL@legacy@UNIVERSE_5_1_U@@SAXMMM@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?bind@GL@legacy@UNIVERSE_5_1_U@@SAXHH@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?clear@GL@legacy@UNIVERSE_5_1_U@@SAXAEBVVector3@23@M@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?end@GL@legacy@UNIVERSE_5_1_U@@SAXXZ(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?begin@GL@legacy@UNIVERSE_5_1_U@@SAX_N0H@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?perspectiveTransition@GL@legacy@UNIVERSE_5_1_U@@SAXMM@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?frontCard@GL@legacy@UNIVERSE_5_1_U@@SAXMMMMMM@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?saturation@LX@legacy@UNIVERSE_5_1_U@@SAXMMAEBVVector3@23@@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?color@BX@legacy@UNIVERSE_5_1_U@@SAXMMMM_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?softLight@BX@legacy@UNIVERSE_5_1_U@@SAXMMMM_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?infront@BX@legacy@UNIVERSE_5_1_U@@SAXMMMM_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?add@BX@legacy@UNIVERSE_5_1_U@@SAXMMMM_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?wiggle@FX@legacy@UNIVERSE_5_1_U@@SAMMMHMV?$optional@M@std@@@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?cornerPinCubic@FX@legacy@UNIVERSE_5_1_U@@SAXAEBVVector2@23@000_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?image@FX@legacy@UNIVERSE_5_1_U@@SAHAEBVString@System@Axion@@HI@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?store@FX@legacy@UNIVERSE_5_1_U@@SAH_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?blur@FX@legacy@UNIVERSE_5_1_U@@SAXM@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?invert@FX@legacy@UNIVERSE_5_1_U@@SAXXZ(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?setBlue@FX@legacy@UNIVERSE_5_1_U@@SAXM@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?setGreen@FX@legacy@UNIVERSE_5_1_U@@SAXM@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?setRed@FX@legacy@UNIVERSE_5_1_U@@SAXM@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?transformScale@FX@legacy@UNIVERSE_5_1_U@@SAXMM@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?transformMove@FX@legacy@UNIVERSE_5_1_U@@SAXMM@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?transform2@FX@legacy@UNIVERSE_5_1_U@@SAXMMMMMMMHM_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?sharpen@FX@legacy@UNIVERSE_5_1_U@@SAXM@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?blurVertical@FX@legacy@UNIVERSE_5_1_U@@SAXM@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?blurV@FX@legacy@UNIVERSE_5_1_U@@SAXM@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?blurH@FX@legacy@UNIVERSE_5_1_U@@SAXM@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?colorize@FX@legacy@UNIVERSE_5_1_U@@SAXMAEBVVector3@23@00@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?blocky@FX@legacy@UNIVERSE_5_1_U@@SAXM_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?blur3x3@FX@legacy@UNIVERSE_5_1_U@@SAXXZ(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?levels2@FX@legacy@UNIVERSE_5_1_U@@SAXMMMMM_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?blit@FX@legacy@UNIVERSE_5_1_U@@SAXH_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?swapChannel@FX@legacy@UNIVERSE_5_1_U@@SAXHHHH@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?crop@FX@legacy@UNIVERSE_5_1_U@@SAXAEBVVector2@23@0_N1@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?transitionLayerB@FX@legacy@UNIVERSE_5_1_U@@SAHM_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?transitionLayerA@FX@legacy@UNIVERSE_5_1_U@@SAHM_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?tileClamp@FX@legacy@UNIVERSE_5_1_U@@SAXMAEBVVector2@23@M@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?tileMirror@FX@legacy@UNIVERSE_5_1_U@@SAXMAEBVVector2@23@M@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?displacementMap@FX@legacy@UNIVERSE_5_1_U@@SAXAEBVVector2@23@AEBU?$vec@$03H$0A@@Math@Axion@@1M0H_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?rngRgb@FX@legacy@UNIVERSE_5_1_U@@SAXMMMHMM_N@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?tileWrap2@FX@legacy@UNIVERSE_5_1_U@@SAXMMAEBVVector2@23@M@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?line@FX@legacy@UNIVERSE_5_1_U@@SAXMAEBVVector2@23@0AEBVVector3@23@MH@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?solid@GX@legacy@UNIVERSE_5_1_U@@SAXAEBVVector3@23@M@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?turbulenceNoise@GX@legacy@UNIVERSE_5_1_U@@SAXMHHMMMMMAEBVVector2@23@MMMMMHH@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?powerMask@MX@legacy@UNIVERSE_5_1_U@@SAXHH_NAEBVVector2@23@1MMMM00@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?promptForPreset@UI@legacy@UNIVERSE_5_1_U@@SA_NPEADIAEBVString@System@Axion@@AEBUuiDialogPreset@23@@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?savePreset@UI@legacy@UNIVERSE_5_1_U@@SAXXZ(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?preset@UI@legacy@UNIVERSE_5_1_U@@SAXAEBVString@System@Axion@@@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?clicked@UI@legacy@UNIVERSE_5_1_U@@SA_NAEBVString@System@Axion@@@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?color@UI@legacy@UNIVERSE_5_1_U@@SA?AU?$vec@$02M$0A@@Math@Axion@@AEBVString@System@6@VVector3@23@H@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?groupEnd@UI@legacy@UNIVERSE_5_1_U@@SAXH@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?groupBegin@UI@legacy@UNIVERSE_5_1_U@@SAXAEBVString@System@Axion@@H@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?checkbox@UI@legacy@UNIVERSE_5_1_U@@SA_NAEBVString@System@Axion@@_NH@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?slider@UI@legacy@UNIVERSE_5_1_U@@SAMAEBVString@System@Axion@@MMMMMHH@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?islider@UI@legacy@UNIVERSE_5_1_U@@SAHAEBVString@System@Axion@@HHHHHHH@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?button@UI@legacy@UNIVERSE_5_1_U@@SA?AV?$StackBaseString@D@System@Axion@@AEBVString@56@0H@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?menu@UI@legacy@UNIVERSE_5_1_U@@SAHAEBVString@System@Axion@@HAEBV?$vector@PEBDV?$allocator@PEBD@std@@@std@@H@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?plugin@UI@legacy@UNIVERSE_5_1_U@@SAXAEBVString@System@Axion@@0000000000@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?transitionMixLayers@UI@legacy@UNIVERSE_5_1_U@@SAMXZ(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?analytics@UI@legacy@UNIVERSE_5_1_U@@SAXAEBVString@System@Axion@@0@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?license@UI@legacy@UNIVERSE_5_1_U@@SAXAEBVString@System@Axion@@@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?resourceFolder@UI@legacy@UNIVERSE_5_1_U@@SAXAEBVString@System@Axion@@@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?importModules@UI@legacy@UNIVERSE_5_1_U@@SAXAEBVString@System@Axion@@@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?fillLast2D@RX@legacy@UNIVERSE_5_1_U@@SAXXZ(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?rect2D@RX@legacy@UNIVERSE_5_1_U@@SAXMMMMW4DataType@123@@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?seed@M2@legacy@UNIVERSE_5_1_U@@SAXH@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?animation@M2@legacy@UNIVERSE_5_1_U@@SAMHMMMM@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
void DelayLoad_?linear@M2@legacy@UNIVERSE_5_1_U@@SAMAEBM0000@Z(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4);
ulonglong FUN_180096dd4(DWORD param_1,longlong *param_2);
HLOCAL __stdcall LocalFree(HLOCAL hMem);
ulonglong FUN_180096e74(void);
undefined8 __std_fs_convert_narrow_to_wide(UINT param_1,LPCSTR param_2,int param_3,LPWSTR param_4,int param_5);
undefined8 __std_fs_convert_wide_to_narrow(UINT param_1,LPCWSTR param_2,int param_3,LPSTR param_4,int param_5);
void __thiscall std::basic_streambuf<>::_Lock(basic_streambuf<> *this);
void __thiscall std::basic_streambuf<>::_Unlock(basic_streambuf<> *this);
__int64 __thiscall std::basic_streambuf<>::showmanyc(basic_streambuf<> *this);
int __thiscall std::basic_streambuf<>::uflow(basic_streambuf<> *this);
__int64 __thiscall std::basic_streambuf<>::xsgetn(basic_streambuf<> *this,char *param_1,__int64 param_2);
__int64 __thiscall std::basic_streambuf<>::xsputn(basic_streambuf<> *this,char *param_1,__int64 param_2);
basic_streambuf<> * __thiscall std::basic_streambuf<>::setbuf(basic_streambuf<> *this,char *param_1,__int64 param_2);
int __thiscall std::basic_streambuf<>::sync(basic_streambuf<> *this);
void __thiscall std::basic_streambuf<>::imbue(basic_streambuf<> *this,locale *param_1);
_Locimp * __cdecl std::locale::_Getgloballocale(void);
_Locimp * __cdecl std::locale::_Init(bool param_1);
void __cdecl std::_Facet_Register(_Facet_base *param_1);
_Facet_base * __thiscall std::locale::facet::_Decref(facet *this);
void __thiscall std::locale::facet::_Incref(facet *this);
undefined1 (*) [32]FUN_1800970a0(undefined1 (*param_1) [32],undefined1 (*param_2) [32],uint param_3);
ulonglong __scrt_acquire_startup_lock(void);
undefined8 __scrt_dllmain_after_initialize_c(void);
bool FUN_1800971b8(void);
undefined1 FUN_1800971d0(void);
undefined1 FUN_1800971f8(void);
void __scrt_dllmain_exception_filter(undefined8 param_1,int param_2,undefined8 param_3,undefined *param_4,undefined4 param_5,undefined8 param_6);
void __scrt_dllmain_uninitialize_c(void);
void FUN_1800972a0(void);
longlong FUN_1800972b4(int param_1);
undefined8 FUN_1800972f0(uint param_1);
ulonglong FUN_18009737c(longlong param_1);
void __scrt_release_startup_lock(char param_1);
undefined1 __scrt_uninitialize_crt(undefined8 param_1,char param_2);
_onexit_t __cdecl _onexit(_onexit_t _Func);
int __cdecl atexit(_func_5014 *param_1);
undefined8 __GSHandlerCheck(undefined8 param_1,undefined8 param_2,undefined8 param_3,longlong param_4);
ulonglong __GSHandlerCheckCommon(undefined8 param_1,longlong param_2);
void __cdecl __security_check_cookie(uintptr_t _StackCookie);
void FUN_180097570(size_t param_1);
void FUN_1800975b4(void *param_1);
undefined8 * FUN_1800975bc(undefined8 *param_1,ulonglong param_2);
void FUN_1800975e8(undefined4 *param_1);
void _Init_thread_footer(int *param_1);
void FUN_180097690(int *param_1);
void __cdecl`eh_vector_destructor_iterator'(void *param_1,__uint64 param_2,__uint64 param_3,_func_void_void_ptr *param_4);
void __cdecl __ArrayUnwind(void *param_1,__uint64 param_2,__uint64 param_3,_func_void_void_ptr *param_4);
void thunk_FUN_180097570(size_t param_1);
void __raise_securityfailure(_EXCEPTION_POINTERS *param_1);
void FUN_180097814(void);
void capture_previous_context(PCONTEXT param_1);
ulonglong FUN_18009795c(undefined8 param_1,int param_2,longlong param_3);
undefined8 FUN_1800979ac(undefined8 param_1,undefined8 param_2);
undefined1 FUN_180097ac4(undefined8 param_1);
int __cdecl dllmain_dispatch(HINSTANCE__ *param_1,ulong param_2,void *param_3);
void entry(HINSTANCE__ *param_1,ulong param_2,void *param_3);
undefined8 FUN_180097cb8(void);
bool __scrt_is_ucrt_dll_in_use(void);
void FUN_180097e70(void);
void FUN_180097e78(undefined4 param_1);
undefined8 * FUN_180097fc0(undefined8 *param_1);
void FUN_180097fe0(void);
void __cdecl __security_init_cookie(void);
void FUN_1800980ac(void);
void FUN_1800980bc(void);
void FUN_1800980c8(void);
undefined * FUN_1800980e4(void);
void FUN_1800980ec(void);
void FUN_180098128(void);
NTSTATUS __stdcall BCryptOpenAlgorithmProvider(BCRYPT_ALG_HANDLE *phAlgorithm,LPCWSTR pszAlgId,LPCWSTR pszImplementation,ULONG dwFlags);
NTSTATUS __stdcall BCryptCloseAlgorithmProvider(BCRYPT_ALG_HANDLE hAlgorithm,ULONG dwFlags);
NTSTATUS __stdcall BCryptGenRandom(BCRYPT_ALG_HANDLE hAlgorithm,PUCHAR pbBuffer,ULONG cbBuffer,ULONG dwFlags);
undefined8 * FUN_180098190(undefined8 *param_1,longlong param_2);
void FUN_1800981f0(longlong param_1);
void FUN_180098230(undefined8 *param_1);
void FUN_180098250(void *param_1);
void * __thiscall std::exception::`scalar_deleting_destructor'(exception *this,uint param_1);
DWORD * FUN_1800982b0(DWORD *param_1,undefined8 *param_2,LPCWSTR param_3,uint param_4,longlong *param_5,longlong *param_6,uint *param_7,uint *param_8);
int * FUN_180098d90(int *param_1,longlong param_2,longlong *param_3,uint *param_4,uint *param_5);
void FUN_180099100(undefined8 *param_1,LPCWSTR param_2,uint param_3,longlong *param_4,ulonglong *param_5);
void FUN_1800997b0(longlong *param_1,DWORD *param_2);
void FUN_180099d10(void);
int * FUN_180099d30(int *param_1,int param_2);
uint FUN_180099db0(short *param_1,ulonglong param_2);
void FUN_180099fa0(uint param_1,int *param_2,short *param_3,uint *param_4,uint *param_5);
undefined8 FUN_18009a060(uint param_1);
undefined8 * FUN_18009a240(undefined8 *param_1,undefined8 *param_2,ulonglong *param_3);
void FUN_18009a630(void);
ulonglong FUN_18009a750(LPCWSTR param_1,LPCWSTR param_2,byte param_3,ulonglong *param_4);
ulonglong FUN_18009aad0(LPCWSTR param_1,LPCWSTR param_2,ulonglong *param_3);
undefined8 FUN_18009abc0(LPCWSTR param_1,ulonglong *param_2);
undefined4 FUN_18009ac90(HANDLE param_1);
undefined8 FUN_18009ad10(undefined8 param_1,undefined8 param_2,longlong param_3,longlong param_4);
undefined8 * FUN_18009ad60(undefined8 *param_1,uint param_2,undefined8 *param_3,ulonglong *param_4);
void FUN_18009aed0(LPCWSTR param_1,ulonglong *param_2);
ulonglong FUN_18009af00(LPCWSTR param_1,ulonglong *param_2);
ulonglong FUN_18009b090(LPCWSTR param_1,uint param_2,ulonglong *param_3);
DWORD FUN_18009b1f0(undefined8 param_1,int param_2);
uint * FUN_18009b3f0(uint *param_1,LPCWSTR param_2,ulonglong *param_3);
uint * FUN_18009b430(uint *param_1,HANDLE param_2,short *param_3,ulonglong *param_4);
uint * FUN_18009b550(uint *param_1,LPCWSTR param_2,ulonglong *param_3);
uint * FUN_18009b6d0(uint *param_1,LPCWSTR param_2,ulonglong *param_3);
undefined8 * FUN_18009b870(undefined8 *param_1,ulonglong *param_2);
undefined8 * FUN_18009bce0(undefined8 *param_1,int param_2,longlong *param_3,void *param_4);
void FUN_18009bfd0(undefined8 *param_1);
void FUN_18009bff0(undefined8 *param_1,uint param_2);
void FUN_18009c0b0(char *param_1,char *param_2,undefined8 *param_3,_Facet_base *param_4);
void FUN_18009c1a0(wchar_t *param_1,wchar_t *param_2,longlong *param_3,_Facet_base *param_4);
void FUN_18009c280(char *param_1,char *param_2,wchar_t *param_3,wchar_t *param_4,undefined8 *param_5,codecvt<> *param_6);
void FUN_18009c330(wchar_t *param_1,wchar_t *param_2,char *param_3,char *param_4,longlong *param_5,codecvt<> *param_6);
longlong * FUN_18009c3e0(undefined8 *param_1,longlong *param_2);
longlong * FUN_18009c710(DWORD *param_1,longlong *param_2);
codecvt<> * FUN_18009cb20(codecvt<> *param_1,uint param_2);
longlong FUN_18009cb60(longlong *param_1);
void FUN_18009cbc0(short *param_1,short *param_2,short *param_3);
void FUN_18009cd40(ulonglong *param_1,short *param_2,short *param_3);
_Facet_base * FUN_18009cf80(void);
longlong FUN_18009d0d0(longlong param_1);
void FUN_18009d1b0(void);
longlong * FUN_18009d260(undefined8 *param_1,longlong *param_2,longlong param_3,longlong param_4);
longlong FUN_18009d310(short *param_1);
longlong FUN_18009d420(short *param_1);
ulonglong FUN_18009d4c0(short *param_1);
ulonglong FUN_18009d5b0(short *param_1,ulonglong param_2,ulonglong *param_3);
longlong * FUN_18009d790(longlong *param_1);
undefined8 * FUN_18009d830(undefined8 *param_1,longlong param_2);
undefined8 *FUN_18009d8b0(undefined8 *param_1,void *param_2,undefined8 *param_3,undefined8 *param_4,DWORD *param_5);
undefined8 * FUN_18009da90(undefined8 *param_1,void *param_2,undefined8 *param_3,DWORD *param_4);
undefined8 * FUN_18009dc70(undefined8 *param_1,void *param_2,DWORD *param_3);
undefined8 * FUN_18009de50(undefined8 *param_1,DWORD *param_2,void *param_3);
void FUN_18009e0f0(undefined8 *param_1);
void FUN_18009e140(undefined8 *param_1);
void FUN_18009e1c0(longlong param_1);
undefined8 * FUN_18009e1f0(undefined8 *param_1,uint param_2);
void FUN_18009e290(int param_1,undefined8 *param_2,undefined8 *param_3,ulonglong *param_4,void *param_5);
void FUN_18009e390(int param_1,undefined8 *param_2,ulonglong *param_3,void *param_4);
void FUN_18009e490(int param_1,ulonglong *param_2,void *param_3);
char * FUN_18009e580(longlong param_1);
undefined ** FUN_18009e990(void);
undefined8 * FUN_18009e9a0(undefined8 param_1,undefined8 *param_2,int param_3);
char * FUN_18009eab0(longlong *param_1,undefined4 param_2,char *param_3,longlong param_4);
char * FUN_18009eb80(void);
undefined8 FUN_18009eb90(undefined8 param_1,undefined8 param_2,LPCSTR param_3,undefined8 param_4,undefined8 *param_5,LPWSTR param_6,longlong param_7,undefined8 *param_8);
undefined8 FUN_18009ec20(undefined8 param_1,undefined8 param_2,LPCWSTR param_3,longlong param_4,longlong *param_5,LPSTR param_6,int param_7,undefined8 *param_8);
CAtlBaseModule * __thiscall ATL::CAtlBaseModule::CAtlBaseModule(CAtlBaseModule *this);
void __thiscall ATL::CAtlBaseModule::~CAtlBaseModule(CAtlBaseModule *this);
void __CxxFrameHandler4(void);
void __std_exception_destroy(void);
void __std_exception_copy(void);
void _purecall(void);
void __current_exception(void);
void __current_exception_context(void);
void * __cdecl memset(void *_Dst,int _Val,size_t _Size);
void __stdcall _CxxThrowException(void *pExceptionObject,ThrowInfo *pThrowInfo);
void __std_type_info_destroy_list(void);
void __RTDynamicCast(void);
void * __cdecl memcpy(void *_Dst,void *_Src,size_t _Size);
void * __cdecl memmove(void *_Dst,void *_Src,size_t _Size);
int __cdecl memcmp(void *_Buf1,void *_Buf2,size_t _Size);
float __cdecl ceilf(float _X);
float __cdecl sinf(float _X);
void terminate(void);
void __cdecl free(void *_Memory);
void * __cdecl malloc(size_t _Size);
void _seh_filter_dll(void);
void _configure_narrow_argv(void);
void _initialize_narrow_environment(void);
void _initialize_onexit_table(void);
void _register_onexit_function(void);
void _execute_onexit_table(void);
void _crt_atexit(void);
void __cdecl _cexit(void);
int __cdecl _callnewh(size_t _Size);
void _initterm(void);
void _initterm_e(void);
void FID_conflict:__GSHandlerCheck_EH(longlong param_1,undefined8 param_2,undefined8 param_3,longlong param_4);
void __chkstk(void);
void * __cdecl memchr(void *_Buf,int _Val,size_t _MaxCount);
double __cdecl atan2(double _Y,double _X);
double __cdecl ceil(double _X);
double __cdecl cos(double _X);
double __cdecl floor(double _X);
double __cdecl sin(double _X);
int __cdecl strcmp(char *_Str1,char *_Str2);
void _guard_dispatch_icall(void);
void _guard_dispatch_icall(void);
void FUN_18009efc0(undefined8 param_1,longlong param_2);
void FUN_18009f000(undefined8 param_1,longlong param_2);
void FUN_18009f030(undefined8 param_1,longlong param_2);
void FUN_18009f150(undefined8 param_1,longlong param_2);
void FUN_18009f180(undefined8 param_1,longlong param_2);
void FUN_18009f1b0(undefined8 param_1,longlong param_2);
void FUN_18009f1e0(undefined8 param_1,longlong param_2);
void FUN_18009f210(undefined8 param_1,longlong param_2);
void FUN_18009f240(undefined8 param_1,longlong param_2);
void FUN_18009f290(undefined8 param_1,longlong param_2);
void FUN_18009f320(undefined8 param_1,longlong param_2);
void FUN_18009f340(undefined8 param_1,longlong param_2);
void FUN_18009f370(undefined8 param_1,longlong param_2);
void FUN_18009f390(undefined8 param_1,longlong param_2);
void FUN_18009f3c0(undefined8 param_1,longlong param_2);
undefined8 FUN_18009f400(void);
void FUN_18009f460(undefined8 param_1,longlong param_2);
void FUN_18009f4a0(undefined8 param_1,longlong param_2);
void FUN_18009f4d0(undefined8 param_1,longlong param_2);
void FUN_18009f510(undefined8 param_1,longlong param_2);
void FUN_18009f580(undefined8 param_1,longlong param_2);
void FUN_18009f5f0(undefined8 param_1,longlong param_2);
void FUN_18009f610(undefined8 param_1,longlong param_2);
void FUN_18009f640(undefined8 param_1,longlong param_2);
void FUN_18009f6a0(undefined8 param_1,longlong param_2);
void FUN_18009f6f0(undefined8 param_1,longlong param_2);
void FUN_18009f860(undefined8 param_1,longlong param_2);
void FUN_18009f8d0(undefined8 param_1,longlong param_2);
void FUN_18009f990(undefined8 param_1,longlong param_2);
undefined8 FUN_18009fbe0(undefined8 param_1,longlong param_2);
undefined8 FUN_18009fcc0(undefined8 param_1,longlong param_2);
undefined8 FUN_18009fe20(void);
void FUN_18009ff60(undefined8 param_1,longlong param_2);
undefined8 FUN_18009ff80(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a0060(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a01c0(void);
undefined8 FUN_1800a02c0(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a03a0(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a0500(void);
void FUN_1800a0610(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a0630(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a0710(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a0870(void);
void FUN_1800a08f0(undefined8 param_1,longlong param_2);
void FUN_1800a0930(undefined8 param_1,longlong param_2);
void FUN_1800a0960(undefined8 param_1,longlong param_2);
void FUN_1800a0a60(undefined8 param_1,longlong param_2);
void FUN_1800a0b00(undefined8 param_1,longlong param_2);
void FUN_1800a0b80(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a0ba0(undefined8 param_1,longlong param_2);
void FUN_1800a0d10(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a0d70(undefined8 param_1,longlong param_2);
void FUN_1800a0db0(undefined8 param_1,longlong param_2);
void FUN_1800a0e30(undefined8 param_1,longlong param_2);
void FUN_1800a0ec0(undefined8 param_1,longlong param_2);
void FUN_1800a0f20(undefined8 param_1,longlong param_2);
void FUN_1800a0f70(undefined8 param_1,longlong param_2);
void FUN_1800a0fc0(undefined8 param_1,longlong param_2);
void FUN_1800a0ff0(undefined8 param_1,longlong param_2);
void FUN_1800a1030(undefined8 param_1,longlong param_2);
void FUN_1800a1070(undefined8 param_1,longlong param_2);
void FUN_1800a1660(undefined8 param_1,longlong param_2);
void FUN_1800a16e0(undefined8 param_1,longlong param_2);
void FUN_1800a1720(undefined8 param_1,longlong param_2);
void FUN_1800a1770(undefined8 param_1,longlong param_2);
void FUN_1800a17a0(undefined8 param_1,longlong param_2);
void FUN_1800a17e0(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a1840(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a1890(undefined8 param_1,longlong param_2);
void FUN_1800a18d0(undefined8 param_1,longlong param_2);
void FUN_1800a1900(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a1960(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a1990(undefined8 param_1,longlong param_2);
void FUN_1800a1af0(undefined8 param_1,longlong param_2);
void FUN_1800a1bf0(undefined8 param_1,longlong param_2);
void FUN_1800a1c20(undefined8 param_1,longlong param_2);
void FUN_1800a1c50(undefined8 param_1,longlong param_2);
void FUN_1800a1ca0(undefined8 param_1,longlong param_2);
void FUN_1800a1d00(undefined8 param_1,longlong param_2);
void FUN_1800a1d60(undefined8 param_1,longlong param_2);
void FUN_1800a1db0(undefined8 param_1,longlong param_2);
void FUN_1800a1de0(undefined8 param_1,longlong param_2);
void FUN_1800a1e30(undefined8 param_1,longlong param_2);
void FUN_1800a1e60(undefined8 param_1,longlong param_2);
void FUN_1800a1e90(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a1ec0(undefined8 param_1,longlong param_2);
void FUN_1800a1f40(undefined8 param_1,longlong param_2);
void FUN_1800a1fb0(undefined8 param_1,longlong param_2);
void FUN_1800a2010(undefined8 param_1,longlong param_2);
void FUN_1800a2100(undefined8 param_1,longlong param_2);
void FUN_1800a2130(undefined8 param_1,longlong param_2);
void FUN_1800a2160(undefined8 param_1,longlong param_2);
void FUN_1800a23e0(undefined8 param_1,longlong param_2);
void FUN_1800a26c0(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a26f0(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a2740(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a2830(void);
undefined8 FUN_1800a28a0(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a2940(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a2a70(void);
undefined8 FUN_1800a2b30(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a2b80(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a2c70(void);
void FUN_1800a2d40(undefined8 param_1,longlong param_2);
void FUN_1800a2e00(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a2e50(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a2e90(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a2f70(void);
undefined8 FUN_1800a3100(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a3150(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a3240(void);
void FUN_1800a32c0(undefined8 param_1,longlong param_2);
void FUN_1800a3350(undefined8 param_1,longlong param_2);
void FUN_1800a33a0(undefined8 param_1,longlong param_2);
void FUN_1800a3430(undefined8 param_1,longlong param_2);
void FUN_1800a3460(undefined8 param_1,longlong param_2);
void FUN_1800a3490(undefined8 param_1,longlong param_2);
void FUN_1800a34c0(undefined8 param_1,longlong param_2);
void FUN_1800a3500(undefined8 param_1,longlong param_2);
void FUN_1800a3540(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a35c0(undefined8 param_1,longlong param_2);
void FUN_1800a3600(undefined8 param_1,longlong param_2);
void FUN_1800a3640(undefined8 param_1,longlong param_2);
void FUN_1800a3670(undefined8 param_1,longlong param_2);
void FUN_1800a3780(undefined8 param_1,longlong param_2);
void FUN_1800a37d0(undefined8 param_1,longlong param_2);
void FUN_1800a37f0(undefined8 param_1,longlong param_2);
void FUN_1800a3850(undefined8 param_1,longlong param_2);
void FUN_1800a3880(undefined8 param_1,longlong param_2);
void FUN_1800a38d0(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a39c0(void);
undefined8 FUN_1800a39f0(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a3a40(undefined8 param_1,longlong param_2);
void FUN_1800a3a70(undefined8 param_1,longlong param_2);
void FUN_1800a3aa0(undefined8 param_1,longlong param_2);
void FUN_1800a3ae0(undefined8 param_1,longlong param_2);
void FUN_1800a3b20(undefined8 param_1,longlong param_2);
void FUN_1800a3bb0(undefined8 param_1,longlong param_2);
void FUN_1800a3bd0(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a3c00(void);
void FUN_1800a3c20(undefined8 param_1,longlong param_2);
void FUN_1800a3c50(undefined8 param_1,longlong param_2);
void FUN_1800a3c90(undefined8 param_1,longlong param_2);
void FUN_1800a3d20(undefined8 param_1,longlong param_2);
void FUN_1800a3d40(undefined8 param_1,longlong param_2);
void FUN_1800a3d90(undefined8 param_1,longlong param_2);
void FUN_1800a3dc0(undefined8 param_1,longlong param_2);
void FUN_1800a3df0(undefined8 param_1,longlong param_2);
void FUN_1800a3e60(undefined8 param_1,longlong param_2);
void FUN_1800a3ea0(undefined8 param_1,longlong param_2);
void FUN_1800a3ef0(undefined8 param_1,longlong param_2);
void FUN_1800a3f20(undefined8 param_1,longlong param_2);
void FUN_1800a3f60(undefined8 param_1,longlong param_2);
void FUN_1800a3f90(undefined8 param_1,longlong param_2);
void FUN_1800a3fc0(undefined8 param_1,longlong param_2);
void FUN_1800a3ff0(undefined8 param_1,longlong param_2);
void FUN_1800a4020(undefined8 param_1,longlong param_2);
void FUN_1800a4050(undefined8 param_1,longlong param_2);
void FUN_1800a4080(undefined8 param_1,longlong param_2);
void FUN_1800a40b0(undefined8 param_1,longlong param_2);
void FUN_1800a40e0(undefined8 param_1,longlong param_2);
void FUN_1800a4110(undefined8 param_1,longlong param_2);
void FUN_1800a4140(undefined8 param_1,longlong param_2);
void FUN_1800a4170(undefined8 param_1,longlong param_2);
void FUN_1800a41a0(undefined8 param_1,longlong param_2);
void FUN_1800a41d0(undefined8 param_1,longlong param_2);
void FUN_1800a4200(undefined8 param_1,longlong param_2);
void FUN_1800a4230(undefined8 param_1,longlong param_2);
void FUN_1800a4280(undefined8 param_1,longlong param_2);
void FUN_1800a42b0(undefined8 param_1,longlong param_2);
void FUN_1800a42e0(undefined8 param_1,longlong param_2);
void FUN_1800a4310(undefined8 param_1,longlong param_2);
void FUN_1800a4340(undefined8 param_1,longlong param_2);
void FUN_1800a4370(undefined8 param_1,longlong param_2);
void FUN_1800a43a0(undefined8 param_1,longlong param_2);
void FUN_1800a43d0(undefined8 param_1,longlong param_2);
void FUN_1800a4400(undefined8 param_1,longlong param_2);
void FUN_1800a4430(undefined8 param_1,longlong param_2);
void FUN_1800a4460(undefined8 param_1,longlong param_2);
void FUN_1800a4490(undefined8 param_1,longlong param_2);
void FUN_1800a44e0(undefined8 param_1,longlong param_2);
void FUN_1800a4510(undefined8 param_1,longlong param_2);
void FUN_1800a4550(undefined8 param_1,longlong param_2);
void FUN_1800a4570(undefined8 param_1,longlong param_2);
void FUN_1800a45a0(undefined8 param_1,longlong param_2);
void FUN_1800a45d0(undefined8 param_1,longlong param_2);
void FUN_1800a4600(undefined8 param_1,longlong param_2);
void FUN_1800a4640(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a4670(undefined8 param_1,longlong param_2);
void FUN_1800a46f0(undefined8 param_1,longlong param_2);
void FUN_1800a48b0(undefined8 param_1,longlong param_2);
void FUN_1800a4940(undefined8 param_1,longlong param_2);
void FUN_1800a4ad0(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a4b00(undefined8 param_1,longlong param_2);
void FUN_1800a4ba0(undefined8 param_1,longlong param_2);
void FUN_1800a4bd0(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a4c30(void);
undefined8 FUN_1800a4cb0(void);
void FUN_1800a4ce0(undefined8 param_1,longlong param_2);
void FUN_1800a4e20(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a4fb0(undefined8 param_1,longlong param_2);
void FUN_1800a5040(undefined8 param_1,longlong param_2);
void FUN_1800a50a0(undefined8 param_1,longlong param_2);
void FUN_1800a50d0(undefined8 param_1,longlong param_2);
void FUN_1800a5100(undefined8 param_1,longlong param_2);
void FUN_1800a5130(undefined8 param_1,longlong param_2);
void FUN_1800a5160(undefined8 param_1,longlong param_2);
void FUN_1800a5190(undefined8 param_1,longlong param_2);
void FUN_1800a51c0(undefined8 param_1,longlong param_2);
void FUN_1800a51f0(undefined8 param_1,longlong param_2);
void FUN_1800a5220(undefined8 param_1,longlong param_2);
void FUN_1800a5250(undefined8 param_1,longlong param_2);
void FUN_1800a5280(undefined8 param_1,longlong param_2);
void FUN_1800a52b0(undefined8 param_1,longlong param_2);
void FUN_1800a52e0(undefined8 param_1,longlong param_2);
void FUN_1800a5310(undefined8 param_1,longlong param_2);
void FUN_1800a5340(undefined8 param_1,longlong param_2);
void FUN_1800a5370(undefined8 param_1,longlong param_2);
void FUN_1800a53a0(undefined8 param_1,longlong param_2);
void FUN_1800a53d0(undefined8 param_1,longlong param_2);
void FUN_1800a5400(undefined8 param_1,longlong param_2);
void FUN_1800a5430(undefined8 param_1,longlong param_2);
void FUN_1800a5460(undefined8 param_1,longlong param_2);
void FUN_1800a5490(undefined8 param_1,longlong param_2);
void FUN_1800a54c0(undefined8 param_1,longlong param_2);
void FUN_1800a54f0(undefined8 param_1,longlong param_2);
void FUN_1800a5520(undefined8 param_1,longlong param_2);
void FUN_1800a5550(undefined8 param_1,longlong param_2);
void FUN_1800a5580(undefined8 param_1,longlong param_2);
void FUN_1800a55b0(undefined8 param_1,longlong param_2);
void FUN_1800a55e0(undefined8 param_1,longlong param_2);
void FUN_1800a5610(undefined8 param_1,longlong param_2);
void FUN_1800a5940(undefined8 param_1,longlong param_2);
void FUN_1800a5970(undefined8 param_1,longlong param_2);
void FUN_1800a59a0(undefined8 param_1,longlong param_2);
void FUN_1800a5a20(undefined8 param_1,longlong param_2);
void FUN_1800a5a50(undefined8 param_1,longlong param_2);
void FUN_1800a5a80(undefined8 param_1,longlong param_2);
void FUN_1800a5ad0(undefined8 param_1,longlong param_2);
void FUN_1800a5b20(undefined8 param_1,longlong param_2);
void FUN_1800a5b50(undefined8 param_1,longlong param_2);
bool FUN_1800a5bfc(undefined8 *param_1);
void FUN_1800a5c14(undefined8 param_1,longlong param_2);
undefined4 FUN_1800a5c40(undefined8 param_1,longlong param_2);
void FUN_1800a5c9f(undefined8 param_1,longlong param_2);
void FUN_1800a5cb6(undefined8 param_1,longlong param_2);
void FUN_1800a5ccf(void);
void FUN_1800a5ce3(undefined8 *param_1,longlong param_2);
void FUN_1800a5d70(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a5da0(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a5ec0(undefined8 param_1,longlong param_2);
void FUN_1800a6010(undefined8 param_1,longlong param_2);
void FUN_1800a6040(undefined8 param_1,longlong param_2);
void FUN_1800a6070(undefined8 param_1,longlong param_2);
void FUN_1800a60b0(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a60e0(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a6160(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a61e0(undefined8 param_1,longlong param_2);
void FUN_1800a6240(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a6270(undefined8 param_1,longlong param_2);
void FUN_1800a62c0(undefined8 param_1,longlong param_2);
undefined8 FUN_1800a62f0(undefined8 param_1,longlong param_2);
void FUN_1800a6330(void);
void FUN_1800a63b0(void);
void FUN_1800a63e0(void);
void FUN_1800a6470(void);
void FUN_1800a64f0(void);
void FUN_1800a6590(void);
void FUN_1800a6620(void);
void FUN_1800a6690(void);
void FUN_1800a6720(void);
void FUN_1800a6780(void);
void FUN_1800a67e0(void);
void FUN_1800a6840(void);
void FUN_1800a68a0(void);
void FUN_1800a6900(void);
void FUN_1800a6960(void);
void FUN_1800a69c0(void);
void FUN_1800a6a20(void);
void FUN_1800a6a80(void);
void FUN_1800a6ae0(void);
void FUN_1800a6b40(void);
void FUN_1800a6ba0(void);
void FUN_1800a6c00(void);
void FUN_1800a6c60(void);
void FUN_1800a6cc0(void);
void FUN_1800a6d20(void);
void FUN_1800a6d80(void);
void FUN_1800a6de0(void);
void FUN_1800a6e40(void);
void FUN_1800a6ea0(void);
void FUN_1800a6f00(void);
void FUN_1800a6f60(void);
void FUN_1800a6fc0(void);
void FUN_1800a7020(void);
void FUN_1800a7080(void);
void FUN_1800a70e0(void);
void FUN_1800a7140(void);
void FUN_1800a71a0(void);
void FUN_1800a7200(void);
void FUN_1800a7260(void);
void FUN_1800a72c0(void);
void FUN_1800a7320(void);
void FUN_1800a7380(void);
void FUN_1800a73e0(void);
void FUN_1800a7440(void);
void FUN_1800a74a0(void);
void FUN_1800a7500(void);
void FUN_1800a7560(void);
void FUN_1800a75c0(void);
void FUN_1800a7620(void);
void FUN_1800a7680(void);
void FUN_1800a76f0(void);
void FUN_1800a7760(void);
void FUN_1800a77d0(void);
void FUN_1800a7890(void);
void FUN_1800a7930(void);
void FUN_1800a79a0(void);
void FUN_1800a7a10(void);
void FUN_1800a7a80(void);
void FUN_1800a7af0(void);
void FUN_1800a7b60(void);
void FUN_1800a7bd0(void);
void FUN_1800a7c40(void);
void FUN_1800a7cb0(void);
void FUN_1800a7d20(void);
void FUN_1800a7d90(void);
void FUN_1800a7e00(void);
void FUN_1800a7e70(void);
void FUN_1800a7ee0(void);
void FUN_1800a7f50(void);
void FUN_1800a7fc0(void);
void FUN_1800a8030(void);
void FUN_1800a80a0(void);
void FUN_1800a8110(void);
void FUN_1800a8180(void);
void FUN_1800a81f0(void);
void FUN_1800a8260(void);
void FUN_1800a82d0(void);
void FUN_1800a8340(void);
void FUN_1800a83b0(void);
void FUN_1800a8420(void);
void FUN_1800a8490(void);
void FUN_1800a8500(void);
void FUN_1800a8570(void);
void FUN_1800a85e0(void);
void FUN_1800a8650(void);
void FUN_1800a86c0(void);
void FUN_1800a8730(void);
void FUN_1800a87a0(void);
void FUN_1800a8810(void);
void FUN_1800a8880(void);
void FUN_1800a88f0(void);
void FUN_1800a8960(void);
void FUN_1800a89d0(void);
void FUN_1800a8a40(void);
void FUN_1800a8ab0(void);
void FUN_1800a8b20(void);
void FUN_1800a8b90(void);
void FUN_1800a8c00(void);
void FUN_1800a8c70(void);
void FUN_1800a8ce0(void);
void FUN_1800a8d50(void);
void FUN_1800a8dc0(void);
void FUN_1800a8e30(void);
void FUN_1800a8ea0(void);
void FUN_1800a8f10(void);
void FUN_1800a8f80(void);
void FUN_1800a8ff0(void);
void FUN_1800a9060(void);
void FUN_1800a90d0(void);
void FUN_1800a9140(void);
void FUN_1800a91b0(void);
void FUN_1800a9220(void);
void FUN_1800a9290(void);
void FUN_1800a9300(void);
void FUN_1800a9370(void);
void FUN_1800a93e0(void);
void FUN_1800a9450(void);
void FUN_1800a94c0(void);
void FUN_1800a9530(void);
void FUN_1800a9540(void);
void FUN_1800a9550(void);
void FUN_1800a95c0(void);
void FUN_1800a9640(void);
void FUN_1800a96b0(void);
void FUN_1800a9720(void);
void FUN_1800a97a0(void);
void FUN_1800a9810(void);
void FUN_1800a9890(void);
void FUN_1800a98a0(void);
void __thiscall std::_Fac_tidy_reg_t::~_Fac_tidy_reg_t(_Fac_tidy_reg_t *this);

