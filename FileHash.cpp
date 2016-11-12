#include "FileHash.h"

void FileHash::AllocateBuffer()
{
	buffer_ = (PBYTE)VirtualAlloc(NULL, buffersize_, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

void FileHash::FreeBuffer()
{
	if (buffer_ != NULL)
	{
		VirtualFree(buffer_, 0, MEM_RELEASE);
	}
}

void FileHash::Compute(BOOL& cancellationflag)
{
	Initialize();
	DWORD bytesread = 0;
	FileHashBytesProcessedEventArgs fhbpea;
	fhbpea.bytesprocessed.QuadPart = 0;
	DWORD runningnotificationblocksize = 0;
	do
	{
		if (cancellationflag)
		{
			return;
		}

		bytesread = filestream_.Read(buffer_, buffersize_);
		if (bytesread > 0)
		{
			Update(bytesread);
		}

		if (bytesprocessedevent_ != nullptr)
		{
			fhbpea.bytesprocessed.QuadPart += bytesread;
			runningnotificationblocksize += bytesread;
			if (runningnotificationblocksize >= bytesprocessednotificationblocksize_ || bytesread == 0)
			{
				if (bytesread > 0)
				{
					runningnotificationblocksize -= bytesprocessednotificationblocksize_;
				}

				bytesprocessedevent_(fhbpea);
			}
		}
	}
	while (bytesread > 0);

	Finalize();
	ConvertHashToDigestString();
}

std::wstring FileHash::ConvertByteArrayToHexString(BYTE arr[], UINT length)
{
	std::wstringstream wss;
	for (UINT i = 0; i < length; i++)
	{
		wss << std::setw(2) << std::setfill(L'0') << std::uppercase << std::hex << arr[i];
	}

	return wss.str();
}

std::wstring FileHash::ConvertUInt32ToHexString(UINT32 value)
{
	std::wstringstream wss;
	wss << std::setw(8) << std::setfill(L'0') << std::uppercase << std::hex << value;
	return wss.str();
}
