#pragma once
#include "Athena/RprAthena.h"
#include <string>

class AthenaWrapper
{
private:
	AthenaWrapper(void);
	~AthenaWrapper(void);

private:
	AthenaOptionsPtr m_athenaOptions;
	AthenaFilePtr m_athenaFile;
	bool m_isEnabled;

public:
	static AthenaWrapper* GetAthenaWrapper(void);

	AthenaWrapper(const AthenaWrapper&) = delete;
	AthenaWrapper& operator=(const AthenaWrapper&) = delete;

	bool WriteField(std::string fieldName, std::string value);

	AthenaOptionsPtr GetAthenaOptions(void);
	AthenaFilePtr GetAthenaFile(void);

	void SetEnabled(bool enable = true);

	bool AthenaSendFile(void);
};

#define CHECK_ASTATUS(_status)							\
{														\
	AthenaStatus _athena_status = (_status);			\
	if ( AthenaStatus::kSuccess != _athena_status )		\
	{													\
		throw;											\
	}													\
}

