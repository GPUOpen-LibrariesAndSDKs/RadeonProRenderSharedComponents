#pragma once
#include "Athena/RprAthena.h"
#include <string>
#include <future>

class AthenaWrapper
{
private:
	AthenaWrapper(void);
	~AthenaWrapper(void);

private:
	AthenaOptionsPtr m_athenaOptions;
	AthenaFilePtr m_athenaFile;
	bool m_isEnabled;
	std::wstring m_folderPath;
	std::vector<std::future<bool> > sendFileAsync;

public:
	static AthenaWrapper* GetAthenaWrapper(void);

	void StartNewFile(void);
	void Finalize(void);

	AthenaWrapper(const AthenaWrapper&) = delete;
	AthenaWrapper& operator=(const AthenaWrapper&) = delete;

	bool WriteField(const std::string& fieldName, const std::string& value);

	AthenaOptionsPtr GetAthenaOptions(void);
	AthenaFilePtr GetAthenaFile(void);

	void SetEnabled(bool enable = true);

	bool AthenaSendFile(void);

	void SetTempFolder(std::wstring folderPath);
};

#define CHECK_ASTATUS(_status)							\
{														\
	AthenaStatus _athena_status = (_status);			\
	if ( AthenaStatus::kSuccess != _athena_status )		\
	{													\
		throw;											\
	}													\
}

