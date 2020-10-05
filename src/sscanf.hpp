#if defined _WIN32 || defined __CYGWIN__
	#define PAWN_NATIVE_API __cdecl
#elif defined __linux__ || defined __APPLE__
	#define PAWN_NATIVE_API __attribute__((cdecl))
#endif

typedef cell (PAWN_NATIVE_API *sscanf_t)(AMX * amx, char * string, char * format, cell * params, int paramCount, char * file, int line);
extern sscanf_t PawnSScanf;

bool FindSscanf();

