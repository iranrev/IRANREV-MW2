#define CALL_NEAR32 0xE8U
#define JMP_NEAR32 0xE9U
#define NOP 0x90U

struct CallHook {
	BYTE bOriginalCode[5];
	PBYTE pPlace;
	PVOID pOriginal;

	void initialize(const char* pOriginalCode, PBYTE place);
	int installHook(void (*hookToInstall)(), bool unprotect);
	int releaseHook(bool unprotect);
};

struct CallHook4D1 {
	BYTE bOriginalCode[5];
	PBYTE pPlace;
	void* pOriginal;
	void* hook;

	void initialize(DWORD place, void *hookToInstall = NULL);
	void installHook(void* hookToInstall = NULL);
	void releaseHook();
};

struct PointerHook {
	PVOID* pPlace;
	PVOID pOriginal;

	void initialize(PVOID* place);
	int installHook(void (*hookToInstall)(), bool unprotect);
	int releaseHook(bool unprotect);
};

struct StompHook {
	BYTE bOriginalCode[15];
	BYTE bCountBytes;
	PBYTE pPlace;

	void initialize(const char* pOriginalCode, BYTE countBytes, PBYTE place);
	int installHook(void (*hookToInstall)(), bool useJump, bool unprotect);
	int releaseHook(bool unprotect);
};
struct StompHook4D1 {
	BYTE bOriginalCode[15];
	BYTE bCountBytes;
	PBYTE pPlace;
	void* hook;
	bool jump;

	void initialize(DWORD place, void *hookToInstall = NULL, BYTE countBytes = 5, bool useJump = true);
	void installHook(void* hookToInstall = NULL);
	void releaseHook();
};

void _nop(void* pAddress, DWORD size);

#define nop(a, v) _nop((void*)(a), (v))
