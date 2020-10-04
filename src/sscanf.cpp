#ifdef _WIN32
        #define WIN32_LEAN_AND_MEAN
        #define VC_EXTRALEAN
        #include <Windows.h>
        #include <psapi.h>
#else
        #include <dlfcn.h>
        #include <cstring>
#endif

#include "sscanf.hpp"
#include "sdk.hpp"

sscanf_t PawnSScanf;
extern logprintf_t logprintf;

#ifdef _WIN32
	bool FindSscanf()
	{
		// Find if sscanf is already loaded:
		logprintf(" >> plugin.mysql: finding sscanf...");
		HANDLE server = GetCurrentProcess();
		HMODULE hMods[1024];
		DWORD cbNeeded;
		HMODULE dll = 0;
		if (EnumProcessModules(server, hMods, sizeof(hMods), &cbNeeded))
		{
			for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
			{
				TCHAR szModName[MAX_PATH];
				if (GetModuleFileNameEx(server, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
				{
					int len = strlen(szModName);
					if (len >= 11 && strcmp("\\sscanf.DLL", szModName + len - 11) == 0)
					{
						dll = hMods[i];
						break;
					}
				}
			}
		}

		if (dll)
		{
			PawnSScanf = (sscanf_t)GetProcAddress(dll, "PawnSScanf");
			if (PawnSScanf)
			{
				logprintf(" >> plugin.mysql: succeeded: %p.\n", PawnSScanf);
				return true;
			}
			else
			{
				logprintf(" >> plugin.mysql: failed, ensure the plugin version is at least 2.10.3.\n");
				logprintf(" >> plugin.mysql: function 'cache_get_sscanf' disabled.\n");
				return false;
			}
		}
		else
		{
			logprintf(" >> plugin.mysql: failed, ensure the sscanf plugin is loaded first.\n");
			logprintf(" >> plugin.mysql: function 'cache_get_sscanf' disabled.\n");
			return false;
		}
	}
#else
	struct lmap
	{
		void
			* base_address;
		char
			* path;
		void
			* unk_1;
		struct lmap
			* next,
			* prev;
	};

	bool FindSscanf()
	{
		// Find if sscanf is already loaded:
		logprintf(" >> plugin.mysql: finding sscanf...");
		struct lmap
			* pl = (struct lmap *)dlopen(NULL, RTLD_NOW),
			* dll = 0;

		while (pl)
		{
			if (strcmp("plugins/sscanf.so", pl->path) == 0)
			{
				dll = pl;
				break;
			}
			pl = pl->next;
		}

		if (dll)
		{
			*(void **)(&PawnSScanf) = dlsym(dll, "PawnSScanf");
			if (PawnSScanf)
			{
				logprintf(" >> plugin.mysql: succeeded: %p.\n", PawnSScanf);
				return true;
			}
			else
			{
				logprintf(" >> plugin.mysql: failed, ensure the plugin version is at least 2.10.3.\n");
				logprintf(" >> plugin.mysql: function 'cache_get_sscanf' disabled.\n");
				return false;
			}
		}
		else
		{
			logprintf(" >> plugin.mysql: failed, ensure the sscanf plugin is loaded first.\n");
			logprintf(" >> plugin.mysql: function 'cache_get_sscanf' disabled.\n");
			return false;
		}
	}
#endif

