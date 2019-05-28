#include "athenaWrap.h"
#include <codecvt>
#include <string>

std::wstring s2ws(const std::string& str);
std::string ws2s(const std::wstring& wstr);

AthenaWrapper* AthenaWrapper::GetAthenaWrapper(void)
{
	static AthenaWrapper instance;
	return &instance;
}

AthenaWrapper::AthenaWrapper()
	: m_athenaOptions(nullptr)
	, m_athenaFile(nullptr)
	, m_isEnabled(true)
{
	m_athenaOptions = athenaCreateOptions();

	AthenaStatus aStatus = athenaInit(m_athenaOptions);
	CHECK_ASTATUS(aStatus);

	m_athenaFile = athenaFileOpen();
}

AthenaWrapper::~AthenaWrapper()
{
	AthenaStatus aStatus = athenaShutdown(m_athenaOptions);

	athenaDestroyOptions(m_athenaOptions);
}

void AthenaWrapper::SetEnabled(bool enable /*= true*/)
{
	m_isEnabled = enable;
}

bool AthenaWrapper::AthenaSendFile(void)
{
	if (!m_isEnabled)
		return true;

	char* pAthenaUID = athenaGuidStr(m_athenaOptions);
	std::string athenaUID (pAthenaUID);
	free (pAthenaUID);
	const PathStringType* pUniqueFileName = athenaUniqueFilename(athenaUID.c_str());
	const std::wstring uniqueFileName (pUniqueFileName);
	free ((PathStringType*)pUniqueFileName);

	// create and write file
	AthenaStatus aStatus;
	std::string appdata (getenv("APPDATA"));
	std::wstring folderPath = s2ws(appdata);
	std::wstring fullPath = folderPath + L"\\" + uniqueFileName;
	aStatus = athenaFileWrite(m_athenaFile, fullPath.c_str());
	if (aStatus != AthenaStatus::kSuccess)
	{
		return false;
	}

	aStatus = athenaFileClose(m_athenaFile);
	if (aStatus != AthenaStatus::kSuccess)
	{
		return false;
	}

	// upload file
	aStatus = athenaUpload(m_athenaOptions, folderPath.c_str(), L"json");
	if (aStatus != AthenaStatus::kSuccess)
	{
		return false;
	}

	// clear temp file
	int res = std::remove(ws2s(fullPath).c_str());
	if (res != 0)
		return false;

	return true;
}

bool AthenaWrapper::WriteField(const std::string& fieldName, const std::string& value)
{
	// ensure valid input
	if ((fieldName.length() == 0) || (value.length() == 0))
	{
		return false;
	}

	// proceed writing
	AthenaStatus aStatus;
	aStatus = athenaFileSetField(m_athenaFile, fieldName.c_str(), value.c_str());
	CHECK_ASTATUS(aStatus);
	if (aStatus != AthenaStatus::kSuccess)
	{
		return false;
	}

	// success!
	return true;
}

AthenaOptionsPtr AthenaWrapper::GetAthenaOptions(void)
{
	return m_athenaOptions;
}

AthenaFilePtr AthenaWrapper::GetAthenaFile(void)
{
	return m_athenaFile;
}

std::wstring s2ws(const std::string& str)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;

	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.from_bytes(str);
}

std::string ws2s(const std::wstring& wstr)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;

	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}

