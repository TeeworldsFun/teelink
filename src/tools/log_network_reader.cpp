#include <iostream>
#include <fstream>
#include <string>
#include <iomanip> // std::setprecision

class CLogNetworkReader
{
	class CLine
	{
	public:
		CLine()
		{
			m_Type = -1;
			m_Size = 0;
			m_pData = nullptr;
		}
		~CLine()
		{
			Reset();
		}

		void Reset()
		{
			if (m_pData)
			{
				delete [] m_pData;
				m_pData = nullptr;
			}
		}

		int m_Type;
		int m_Size;
		char unsigned *m_pData;
	};

	std::ifstream m_Stream;
	unsigned int m_TotalSentCompress;
	unsigned int m_TotalSent;
	unsigned int m_NumPacketsRead;
	unsigned int m_TotalBlankLines;
	unsigned int m_TotalBlankBytes;
	unsigned int m_FreqTable[256];

	bool ReadPacketInfo(CLine *pDumpLine)
	{
		m_Stream.read(reinterpret_cast<char*>(&pDumpLine->m_Type), sizeof(pDumpLine->m_Type));
		m_Stream.read(reinterpret_cast<char*>(&pDumpLine->m_Size), sizeof(pDumpLine->m_Size));
		if (pDumpLine->m_Size == 0)
			return false;
		pDumpLine->m_pData = new unsigned char[pDumpLine->m_Size];
		m_Stream.read(reinterpret_cast<char*>(pDumpLine->m_pData), pDumpLine->m_Size);

		if (pDumpLine->m_Type == 1)
			for (int i=0; i<pDumpLine->m_Size; ++m_FreqTable[pDumpLine->m_pData[i++]]);

		return true;
	}

	bool IsBlankLine(const unsigned char *pData, int Size) const
	{
		for (int i=0; i<Size-1; i++)
		{
			if (pData[i] != 0x20)
				return false;
		}

		return true;
	}

	void PrintInfo(const std::string &File) const
	{
		std::cout << "Log Network File: " << File << std::endl;
		std::cout << "Num. Packets Read: " << m_NumPacketsRead << std::endl;
		const int TotalBytes = m_TotalSent - m_TotalSentCompress;
		const float Perc = 100.0f - (m_TotalSentCompress * 100.0f / m_TotalSent);
		std::cout << "Total Bytes: " << m_TotalSent << std::endl;
		std::cout << "Total Bytes Compressed: " << m_TotalSentCompress << std::endl;
		std::cout << "Total Compression: " << std::fixed << std::setprecision(2) << Perc << "% (" << TotalBytes << " bytes)" << std::endl;
		std::cout << "Total Blank Lines: " << m_TotalBlankLines << " (" << m_TotalBlankBytes << " bytes)" << std::endl << std::endl;
		std::cout << "Frequency Table:" << std::endl;
		for (int i=0; i<255; i++)
		{
			std::cout << "0x" << std::hex << m_FreqTable[i];
			if (i < 254)
				std::cout << ", ";
		}
		std::cout << std::endl << std::endl;
	}

public:
	void Reset()
	{
		m_TotalSentCompress = 0;
		m_TotalSent = 0;
		m_NumPacketsRead = 0;
		m_TotalBlankLines = 0;
		m_TotalBlankBytes = 0;
		for (int i=0; i<256; m_FreqTable[i++]=0);
	}

	void Read(std::string File)
	{
		CLine DumpLine;
		m_Stream = std::ifstream(File, std::ios_base::in | std::ios_base::binary);
		if (m_Stream.is_open())
		{
			Reset();
			do
			{
				if (ReadPacketInfo(&DumpLine))
				{
					if (DumpLine.m_Type == 1)
						m_TotalSent += DumpLine.m_Size;
					else
						m_TotalSentCompress += DumpLine.m_Size;

					if (IsBlankLine(DumpLine.m_pData, DumpLine.m_Size))
					{
						++m_TotalBlankLines;
						m_TotalBlankBytes += DumpLine.m_Size;
					}

					DumpLine.Reset();
					++m_NumPacketsRead;
				}
			} while (!m_Stream.eof());
			m_Stream.close();

			PrintInfo(File);
		}
		else
			std::cout << "Can't read '" << File << "'!" << std::endl;
	}
};

int main(int argc, const char **argv)
{
	if (argc < 2)
	{
		std::cout << "Invalid parameters!" << std::endl << "\tUsage: " << argv[0] << " FILE1 [ FILE2... ]" << std::endl;
		return -1;
	}

	CLogNetworkReader LogNetReader;
	for (int i=1; i<argc; LogNetReader.Read(argv[i++]));
	return 0;
}
