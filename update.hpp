#pragma once

#include "HTTP.hpp"
#include <shlwapi.h>
#include <fstream>

#define INJECTOR_CURRENT_VERSION "1.7"
#define INJECTOR_CURRENT_VERSION_STRLEN 3

namespace Update
{
	namespace Error
	{
		enum class Context : int
		{
			none = 0,
			init,
			init_win,
			rename_win,
			writeoutput_win,
			executenew_win,
		};
	}

	extern Error::Context ErrorContext;
	extern int ErrorCode;
	extern std::string Directory;
	extern std::string FileName;
	extern std::string TempFileName;
	extern std::string LastTempFileFullPath;

	enum class VersionCheckResult : int
	{
		UpToDate = 0,
		NeedsUpdate,
		Error,
	};

	extern std::string GenerateTempFileName(std::string dir);
	extern void deletePreviousVersion();
	extern VersionCheckResult versionCheck();
	extern char* downloadLatestVersion(size_t* bytesRead);
	extern bool renameMyself(std::string newName);
	extern bool writeNewVersion(char* file, size_t nBytes);
	extern bool executeNewVersion();

	extern bool init(std::string argv);
	extern bool run();
}