//////////////////////////////////////////////////////////////////////////////////////
// 																				    //
//         KW - HOMEBREW BLOCKER SUPPORT CODE TO USE IN open_path_hook()            //
//            																        //
//   Functions, global vars and directives are here to improve code readability     //
//																					//
//////////////////////////////////////////////////////////////////////////////////////

#include <lv2/thread.h>
#include <lv2/io.h>
#include <lv2/error.h>
#include "homebrew_blocker.h"
#include "mappath.h"
#include "modulespatch.h"
#include "ps3mapi_core.h"

#define BLACKLIST_FILENAME	"/dev_hdd0/tmp/blacklist.cfg"
#define WHITELIST_FILENAME	"/dev_hdd0/tmp/whitelist.cfg"

#define MAX_LIST_ENTRIES	32 // Maximum elements for noth the custom blacklist and whitelist.

int CFW2OFW_game = 0;
//uint8_t allow_restore_sc = 1;
uint8_t syscalls_mode = 0; 

static int __initialized_lists = 0; // Are the lists initialized ?
static int __blacklist_entries = 0; // Module global var to hold the current blacklist entries.
static char __blacklist[9 * MAX_LIST_ENTRIES];
static int __whitelist_entries = 0; // Module global var to hold the current whitelist entries.
static char __whitelist[9 * MAX_LIST_ENTRIES];

// inits a list.
// returns the number of elements read from file
static int init_list(char *list, char *path, int maxentries)
{
	int loaded, f;

	if (cellFsOpen(path, CELL_FS_O_RDONLY, &f, 0, NULL, 0) != 0)
		return CELL_FS_SUCCEEDED; // failed to open

	loaded = 0;

	while (loaded < maxentries)
	{
		char line[128];
		int eof;
		
		if (read_text_line(f, line, sizeof(line), &eof) > 0)
		if (strlen(line) >= 9) // avoid copying empty lines
		{
			strncpy(list + (9 * loaded), line, 9); // copy only the first 9 chars - if it has lees than 9, it will fail future checks. should correct in file.
			loaded++;
		}

		if (eof)
			break;
	}

	cellFsClose(f);
	return loaded;
}

// tests if a char gameid[9] is in the blacklist or whitelist
// initialize the both lists, if not yet initialized;
// receives the list to test blacklist (1) or whitelist (0), and the gameid
// to initialize the lists, tries to read them from file BLACKLIST_FILENAME and WHITELIST_FILENAME
static int listed(int blacklist, char *gameid)
{
	if (!__initialized_lists)
	{
		// initialize the lists if not yet done
		__blacklist_entries = init_list(__blacklist, BLACKLIST_FILENAME, MAX_LIST_ENTRIES);
		__whitelist_entries = init_list(__whitelist, WHITELIST_FILENAME, MAX_LIST_ENTRIES);
		__initialized_lists = 1;
	}

	char *list = blacklist ? __blacklist : __whitelist;
    int elements = blacklist ? __blacklist_entries : __whitelist_entries;

	for (int i = 0; i < elements; i++)
	{
		if (!strncmp(list + (9 * i), gameid, 9))
			return 1; // gameid is in the lists
	}

	return SUCCEEDED; // if it got here, it is not in the list. return 0
}

static int check_syscalls()
{
	return ((*(uint64_t *)MKA(syscall_table_symbol + 8 * 6)) == (*(uint64_t *)MKA(syscall_table_symbol)));
}

// BEGIN KW & AV block access to homebrews when syscalls are disabled
// After the core tests it will test first if the gameid is in whitelist.cfg (superseeds previous tests)
// In the it will test if the gameid is in blacklist.cfg (superseeds all previous tests)
// ** WARNING ** This syscall disablement test assumes that the syscall table entry 6 (peek) was replaced by the original value (equals syscall 0 entry) as done by PSNPatch
// ** WARNING ** If only a parcial disablement was made, this assumption WILL FAIL !!!

extern char umd_file;
static uint8_t block_psp_launcher = 0;

int block_homebrew(const char *path)
{
	int allow = 1;
	//uint8_t is_hdd0 = !strncmp(path, "/dev_hdd0/", 10);
	//uint8_t is_game_dir = (is_hdd0 && !strncmp(path + 10, "game/", 5));

	uint8_t is_hdd0 = !strncmp(path, "/dev_hdd0/game/", 15);

	if(is_hdd0)
	{
		char *gameid = (char *)path + 15;

		// Block PSP Launchers if PSP UMD was not set (By @aldostools)
		if (!umd_file && 
			(!strncmp(gameid, "PSPC66820/USRDIR", 16) || 
			 !strncmp(gameid, "PSPM66820/USRDIR", 16)))
		{
			block_psp_launcher = 1;
			return allow;
		}

		// CFW2OFW fix by Evilnat
		if(strstr(path, "/LIC.EDAT"))
			CFW2OFW_game = 1;

		if (check_syscalls() && strstr(gameid, "/EBOOT.BIN"))
		{
			// syscalls are disabled and an EBOOT.BIN is being called from hdd. Let's test it.

			// flag "whitelist" id's
			allow = !strncmp(gameid, "NP", 2) ||
					!strncmp(gameid, "BL", 2) ||
					!strncmp(gameid, "BC", 2) ||
					!strncmp(gameid, "KOEI3", 5) ||
					!strncmp(gameid, "KTGS3", 5) ||
					!strncmp(gameid, "MRTC0", 5) ||
					!strncmp(gameid, "ASIA0", 5) ||
					!strncmp(gameid, "_DEL_", 5) || // Fix data corruption if you uninstall game/game update/homebrew with syscall disabled # Alexander's
					!strncmp(gameid, "_INST_", 6) || // 80010006 error fix when trying to install a game update with syscall disabled. # Joonie's, Alexander's, Aldo's
					!strncmp(gameid, "GUST0", 5) ;

			// flag some "blacklist" id's
			if (!strncmp(gameid, "BLES806", 7)   || // Multiman and assorted tools are in the format BLES806**
				!strncmp(gameid, "BLJS10018", 9) || // PSNPatch Stealth (older versions were already detected as non-NP/BC/BL)
				!strncmp(gameid, "BLES08890", 9) || // PSNope by user
				!strncmp(gameid, "BLES13408", 9) || // FCEU NES Emulator
				!strncmp(gameid, "BLES01337", 9) || // Awesome File Manager
				!strncmp(gameid, "BLND00001", 9) || // dev_blind
				!strncmp(gameid, "NPEA90124", 9) || // SEN Enabler
				!strncmp(gameid, "NP0", 3)          // Apollo Save Tool (NP0APOLLO) / pkgi-ps3 (NP00PKGI3)
				//!strcmp (path, "/dev_bdvd/PS3_UPDATE/PS3UPDAT.PUP") //bluray disk updates
				) allow = 0;

			// test whitelist.cfg and blacklist.cfg
			if (listed(0, gameid)) // whitelist.cfg test
				allow = 1;
			if (listed(1, gameid)) // blacklist.cfg test
				allow = 0;
		}
	}
	else if (block_psp_launcher && !umd_file && !strncmp(path, "/dev_flash/pspemu", 17))
		block_psp_launcher = allow = 0;

	return allow;
}

void restore_syscalls(const char *path)
{
	// Restore disabled CFW Syscalls without reboot just entering to Settings > System Update on XMB - aldostools
	/*if(allow_restore_sc)
	{*/
		if(!strcmp(path, "/dev_flash/vsh/module/software_update_plugin.sprx"))
			create_syscalls();
	//}
}

void check_signin(const char *path)
{
	if(!strcmp(path, "/dev_flash/vsh/module/npsignin_plugin.sprx"))
	{
		// Lock/Unlock Sign In to PSN if DeViL303's RCO exists
		if(check_syscalls())
			map_path(NPSIGNIN_UNLOCK, NULL, 0);
		else
		{
			CellFsStat stat;
			if(cellFsStat(NPSIGNIN_LOCK, &stat) == CELL_FS_SUCCEEDED)
				map_path(NPSIGNIN_UNLOCK, NPSIGNIN_LOCK, 0);
			else
				map_path(NPSIGNIN_UNLOCK, NULL, 0);
		}
	}
}

// CFW syscalls mode on boot (By Evilnat)
void disable_cfw_syscalls(int mode)
{
	static uint16_t syscalls[17] = 
	{ 
		1022, 204, 203, 202, 201, 200, 9, 10, 11, 15, 20, 35, 36, 38, 6, 8, 7
	};

	if(!safe_mode)
	{
		if(mode == 1)
		{
			for(uint8_t i = 0; i < 17; i++) // Disabling all CFW syscalls
				ps3mapi_disable_syscall(syscalls[i]);
		}
		else if(mode == 2)
			ps3mapi_partial_disable_syscall8 = 2; // Keep PS3M_API Features only.
	}
}
