#pragma once
#include <string>
#include <vector>
#include <future>
#include <sstream>

typedef enum
{
	kInvalidParam = -2,
	kInvalidPath = -1,
	kFailure = 1,
	kSuccess = 0,
} AthenaStatus;

struct AthenaFile;
typedef std::unique_ptr<AthenaFile> AthenaFilePtr;

class AthenaWrapper
{
private:
	AthenaWrapper(void);
	~AthenaWrapper(void);

private:
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

	void SetEnabled(bool enable = true);

	bool AthenaSendFile(std::function<int(std::string)>& actionFunc);

	void SetTempFolder(const std::wstring& folderPath);
};

#ifdef _DEBUG
#define CHECK_ASTATUS(_status)							\
{														\
	AthenaStatus _athena_status = (_status);			\
	if ( AthenaStatus::kSuccess != _athena_status )		\
	{													\
		std::stringstream cerr;							\
		cerr << "AWS error detected in " << __FILE__	\
			 <<	" at line "	<< __LINE__ ;				\
		throw std::runtime_error(cerr.str().c_str());	\
	}													\
}
#else
#define CHECK_ASTATUS(_status)   
#endif


