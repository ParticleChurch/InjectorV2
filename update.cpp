#include "update.hpp"
#include <iostream>


char getRandomChar()
{
	// a-zA-Z0-9
	constexpr int n = 26 + 26 + 10;
	int pick = rand() % n;
	if (pick < 26)
		return 'a' + pick;
	else if (pick < 26 + 26)
		return 'A' + pick - 26;
	else
		return '0' + pick - 26 - 26;
}

std::string Update::GenerateTempFileName(std::string dir)
{
	std::string out = "";

	do {
		for (int i = 0; i < 5; ++i)
			out += getRandomChar();
	} while (PathFileExistsA((dir + "\\.__injectortempupdate_" + out + ".exe").c_str()));

	return ".__injectortempupdate_" + out + ".exe";
}

namespace Update
{
	Error::Context ErrorContext = Error::Context::none;
	int ErrorCode = 0;
	std::string Directory = "";
	std::string FileName = "";
	std::string TempFileName = "";
	std::string LastTempFileFullPath = "";
}

void Update::deletePreviousVersion()
{
	if (LastTempFileFullPath.length() <= 3) return;
	size_t attempts = 0;
	while (!DeleteFileA(LastTempFileFullPath.c_str()) && ++attempts < 5)
	{
		Sleep(1000);
	}
}

Update::VersionCheckResult Update::versionCheck()
{
	size_t bytesRead = 0;
	char* result = HTTP::GET("https://www.a4g4.com/API/injector/version.php", &bytesRead);
	if (!result) return VersionCheckResult::Error;

	// make sure that the server responded with a real version instead of, say, an HTTP 404 page
	try {
		float x = std::stof(std::string(result, bytesRead));
		if (x <= 0.f)
			return VersionCheckResult::Error;
	}
	catch (std::exception&)
	{
		return VersionCheckResult::Error;
	}

	if (INJECTOR_CURRENT_VERSION_STRLEN != bytesRead || strncmp(INJECTOR_CURRENT_VERSION, result, INJECTOR_CURRENT_VERSION_STRLEN))
		return VersionCheckResult::NeedsUpdate;
	else
		return VersionCheckResult::UpToDate;
}

char* Update::downloadLatestVersion(size_t* bytesRead)
{
	return HTTP::GET("https://www.a4g4.com/API/injector/download.php", bytesRead);
	/*
	auto file = std::ifstream(
		Directory + "\\" + FileName,
		std::ios::in | std::ios::binary | std::ifstream::ate
	);
	size_t fileSize = (size_t)file.tellg();
	*bytesRead = fileSize;

	char* out = (char*)malloc(fileSize);
	file.seekg(0);
	file.read(out, fileSize);

	return out;
	*/
}

bool Update::renameMyself(std::string newName)
{
	if (0 != rename((Directory + "\\" + FileName).c_str(), (Directory + "\\" + newName).c_str()))
	{
		OutputDebugStringA((Directory + "\\" + FileName + "\n").c_str());
		OutputDebugStringA((Directory + "\\" + newName + "\n").c_str());
		ErrorContext = Error::Context::rename_win;
		ErrorCode = GetLastError();
		return false;
	}
	return true;
}

bool Update::writeNewVersion(char* file, size_t nBytes)
{
	auto outputFile = std::fstream(Directory + "\\" + FileName, std::ios::out | std::ios::binary);
	if (!outputFile.is_open())
	{
		ErrorContext = Error::Context::writeoutput_win;
		ErrorCode = GetLastError();
		return false;
	}
	outputFile.write(file, nBytes);
	outputFile.flush();
	outputFile.close();

	return true;
}

bool Update::executeNewVersion()
{
	STARTUPINFOA si = { sizeof(si) };
	PROCESS_INFORMATION pi;

	std::string exe = Directory + "\\" + FileName;
	std::string arg = Directory + "\\" + TempFileName;

	if (CreateProcessA((LPSTR)exe.c_str(), (LPSTR)("\"" + exe + "\" \"" + arg + "\"").c_str(), 0, 0, true, 0, 0, 0, &si, &pi))
	{
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return true;
	}
	else
	{
		ErrorContext = Error::Context::executenew_win;
		ErrorCode = GetLastError();
		return false;
	}
}

bool Update::init(std::string argv)
{
	srand((unsigned int)time(0));

	char exePath[MAX_PATH];
	GetModuleFileNameA(NULL, exePath, MAX_PATH);
	std::string fullPath(exePath);
	size_t slash = fullPath.rfind("\\");
	if (slash >= fullPath.length())
	{
		ErrorContext = Error::Context::init;
		ErrorCode = 1;
		return false;
	}
	Directory = fullPath.substr(0, slash);
	FileName = fullPath.substr(slash + 1);
	LastTempFileFullPath = argv;
	deletePreviousVersion();

	return true;
}

bool Update::run()
{
	// check if we need to update
	switch (versionCheck())
	{
	case VersionCheckResult::Error:
		return false;
	case VersionCheckResult::UpToDate:
		return true;
	default:
		// we need to update
		break;
	}

	// download new version
	size_t newVersionSize = 0;
	char* newVersionFile = downloadLatestVersion(&newVersionSize);
	if (!newVersionFile || newVersionSize == 0)
		return false;

	// rename myself to a temp filename
	TempFileName = GenerateTempFileName(Directory);
	if (!renameMyself(TempFileName))
		return false;

	// write new version to file
	writeNewVersion(newVersionFile, newVersionSize);

	if (!executeNewVersion())
		return false;

	std::exit(1);

	return true;
}
