/**********************************************************************
Copyright 2020 Advanced Micro Devices, Inc
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
********************************************************************/
#include "athenaWrap.h"
#include "../Utils/Utils.h"

#include <string>
#include <fstream>
#include <algorithm>
#ifdef WIN32
	#include <Windows.h>
	#include <experimental/filesystem>
#else
	#include <sys/time.h>
	#include <sys/stat.h>
	#include <sys/types.h>
	#include <dirent.h>
    #include <sys/stat.h>
#endif
#include <chrono>
#include <time.h>
#include <thread>

AthenaWrapper* AthenaWrapper::GetAthenaWrapper(void)
{
	static AthenaWrapper instance;
	return &instance;
}

AthenaWrapper::AthenaWrapper()
	: m_athenaFile(nullptr)
	, m_isEnabled(true)
	, m_folderPath()
	, sendFileAsync()
{
	StartNewFile();

#ifdef WIN32
	std::string temp(getenv("TEMP"));
#else
    std::string temp(getenv("TMPDIR"));
#endif
	m_folderPath = SharedComponentsUtils::s2ws(temp);
#ifdef WIN32
	m_folderPath += L"\\rprathena";
#else
    m_folderPath += L"rprathena";
#endif
}

AthenaWrapper::~AthenaWrapper()
{}

void AthenaWrapper::Finalize()
{
	sendFileAsync.clear();
}

void AthenaWrapper::StartNewFile(void)
{
	if (m_athenaFile.get() != nullptr)
		return; // TODO : make this check more pretty

	// Begin new file
	m_athenaFile = std::make_unique<AthenaFile>();
	m_athenaFile->pImpl = std::make_unique<AthenaFileImpl>();
}

void AthenaWrapper::SetEnabled(bool enable /*= true*/)
{
	m_isEnabled = enable;
}

void AthenaWrapper::SetTempFolder(const std::wstring& folderPath)
{
	m_folderPath = folderPath;
}

std::wstring athenaUniqueFilename(const char* guidstr)
{
	if (!guidstr)
	{
		return NULL;
	}

	std::wstring uniquename;
	uniquename = SharedComponentsUtils::s2ws(guidstr);
	uniquename += L"_";

	std::wostringstream stream;
	//1111111_2019 02 22 21 27 18 0342

#ifdef WIN32
	SYSTEMTIME systme;
	GetSystemTime(&systme);
	stream << std::setfill(L'0') << std::setw(4) << systme.wYear;
	stream << std::setfill(L'0') << std::setw(2) << systme.wMonth;
	stream << std::setfill(L'0') << std::setw(2) << systme.wDay;
	stream << std::setfill(L'0') << std::setw(2) << systme.wHour;
	stream << std::setfill(L'0') << std::setw(2) << systme.wMinute;
	stream << std::setfill(L'0') << std::setw(2) << systme.wSecond;
	stream << std::setfill(L'0') << std::setw(4) << systme.wMilliseconds;
#else
	timeval timeval;
	gettimeofday(&timeval, NULL);
	long millis = (timeval.tv_sec * 1000) + (timeval.tv_usec / 1000);
	time_t t = time(NULL);
	tm* timePtr = localtime(&t);
	stream << std::setfill(L'0') << std::setw(4) << (timePtr->tm_year) + 1900;
	stream << std::setfill(L'0') << std::setw(2) << (timePtr->tm_mon) + 1;
	stream << std::setfill(L'0') << std::setw(2) << (timePtr->tm_mday);
	stream << std::setfill(L'0') << std::setw(2) << (timePtr->tm_hour);
	stream << std::setfill(L'0') << std::setw(2) << (timePtr->tm_min);
	stream << std::setfill(L'0') << std::setw(2) << (timePtr->tm_sec);
	stream << std::setfill(L'0') << std::setw(4) << millis;
#endif

	uniquename += stream.str();
	uniquename += L".json";

	return uniquename;
}

AthenaStatus athenaFileWrite(AthenaFilePtr& pJson, const wchar_t* filePath)
{
	if (!pJson.get() || !filePath)
	{
		return kInvalidParam;
	}

#ifdef WIN32
	// MSVS added an overload to accommodate using open with wide strings where xcode did not.
	std::ofstream o(filePath);
#else
	// thus different path for xcode is needed
    std::string s_filePath = SharedComponentsUtils::ws2s(filePath);
	std::ofstream o(s_filePath);
#endif

	o << std::setw(4) << pJson->pImpl->mJson << std::endl;
	o.close();
	return kSuccess;
}

void strPrepareForPython(std::string& source)
{
	size_t start_pos = 0;
	while ((start_pos = source.find("\\", start_pos)) != std::string::npos)
	{
		source.replace(start_pos, 1, "//");
	}
}

AthenaStatus athenaUpload(std::wstring& sendFile, wchar_t* fileExtension, std::wstring& filename, std::function<int(std::string)>& actionFunc)
{
	if ((sendFile.length() == 0) || !fileExtension)
	{
		return kInvalidParam;
	}

	std::wstring ext(L"json");
	if (ext != fileExtension)
	{
		return kInvalidParam;
	}

	AthenaStatus successFlag = kSuccess;

	std::string str_sendFile = SharedComponentsUtils::ws2s(sendFile);
	std::string str_filename = SharedComponentsUtils::ws2s(filename);

	strPrepareForPython(str_sendFile);
	strPrepareForPython(str_filename);

	std::string pyCommand = 
		"import boto3 \n"
		"from botocore.exceptions import ClientError \n"
		"from boto3.exceptions import S3UploadFailedError \n"
		"ACCESS_KEY = '##removed##' \n"
		"SECRET_KEY = '##removed##' \n"
		"BUCKET_NAME = 'amd-athena-prorender' \n"
		"try: \n"
		"	client = boto3.client('s3', aws_access_key_id=ACCESS_KEY, aws_secret_access_key=SECRET_KEY) \n"
		"	client.upload_file('" + str_sendFile + "', BUCKET_NAME, '" + str_filename + "') \n"
		"	print(\"successfully uploaded data to AWS!\")\n"
		"except S3UploadFailedError as e: \n"
		"	print(str(e)) \n"
		"except ClientError as e: \n"
		"	print (e.response['Error']['Code']) \n"
		"	print('ClientError') \n"
		"except BaseException as e: \n"
		"	print('An error occurred.') \n"
		"	print(type(e)) \n"
		"	print(str(e)) \n"
		"finally: \n"
		"	os.remove('" + str_sendFile + "') \n";

	int res = actionFunc(pyCommand);
	successFlag = (AthenaStatus) res;

	return successFlag;
}

bool AthenaWrapper::AthenaSendFile(std::function<int(std::string)>& actionFunc)
{
	// athena disabled by ui => return
	if (!m_isEnabled)
		return true;

	// back-off if not inited
	if (!m_athenaFile.get())
		return false;

	// generate file uid
	srand(static_cast<unsigned int>(time(NULL)));
	std::string athenaUID = std::to_string(rand());

	const std::wstring uniqueFileName = athenaUniqueFilename(athenaUID.c_str());

	// create folder
#ifdef WIN32
	bool folderCreated = std::experimental::filesystem::create_directory(m_folderPath);
#else
    std::string s_folderPath = SharedComponentsUtils::ws2s(m_folderPath);
	bool folderCreated = (mkdir(s_folderPath.c_str(), 0777) != -1);
#endif

	// create and write file
	AthenaStatus aStatus;
#ifdef WIN32
	std::wstring fullPath = m_folderPath + L"\\" + uniqueFileName;
#else
    std::wstring fullPath = m_folderPath + L"/" + uniqueFileName;
#endif
	aStatus = athenaFileWrite(m_athenaFile, fullPath.c_str());
	if (aStatus != AthenaStatus::kSuccess)
	{
		return false;
	}

	// upload file
	// - need to copy filepath when passing it to keep string data in other thread
	auto handle = std::async(std::launch::async, [&] (std::wstring fullpath, std::wstring filename)->bool
	{
		AthenaStatus aStatus = athenaUpload(fullpath, L"json", filename, actionFunc);

		// report command execution result
		if (aStatus != AthenaStatus::kSuccess)
		{
			return false;
		}
		return true;
	}, 
	fullPath, uniqueFileName);

	// move future so that routine could be executed in background
	sendFileAsync.push_back(std::move(handle));

	return true;
}
