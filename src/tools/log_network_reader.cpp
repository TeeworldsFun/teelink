#include <base/system.h>
#include <engine/shared/network.h>
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip> // std::setprecision
#include <map>

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

	// ** Network Info
	unsigned int m_TotalBytesPreProcessed;
	unsigned int m_TotalBytesPostProcessed;
	unsigned int m_NumInvalidPackets;
	unsigned int m_FreqTable[256]; // Pre-Processed Packets

	std::map<int, int> m_ConlessControlIds;
	std::map<int, int> m_ControlIds;
	unsigned int m_NumPacketsRead;
	unsigned int m_NumChunksRead;
	unsigned int m_NumConnlessChunks;
	unsigned int m_NumConnChunks;
	unsigned int m_NumControlChunks;
	unsigned int m_NumResendChunks;
	unsigned int m_NumCompressionChunks;
	unsigned int m_NumNoFlagChunks;


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

	void PrintInfo(const std::string &File) const
	{
		std::cout << "Log Network File: " << File << std::endl << std::endl;
		std::cout << "Num. Chunks Read: " << m_NumChunksRead << " (" << (m_NumPacketsRead - m_NumInvalidPackets)  << " packets)" << std::endl;
		std::cout << " + Connless: " << m_NumConnlessChunks << std::endl;
		std::map<int, int>::const_iterator cit = m_ConlessControlIds.cbegin();
		while (cit != m_ConlessControlIds.end())
		{
			std::cout << "       " << (*cit).first << " \t" << (*cit).second << " times" << std::endl;
			++cit;
		}
		std::cout << " + Conn: " << m_NumConnChunks << std::endl;
		std::cout << "  - Control: " << m_NumControlChunks << std::endl;
		cit = m_ControlIds.cbegin();
		while (cit != m_ControlIds.end())
		{
			std::cout << "       " << (*cit).first << " \t" << (*cit).second << " times" << std::endl;
			++cit;
		}
		std::cout << "  - Resend: " << m_NumResendChunks << std::endl;
		std::cout << "  - Compression: " << m_NumCompressionChunks << std::endl;
		std::cout << "  - No Flagged: " << m_NumNoFlagChunks << std::endl;
		std::cout << "Num. Invalid Packets: " << m_NumInvalidPackets << std::endl << std::endl;
		const int TotalBytes = m_TotalBytesPreProcessed - m_TotalBytesPostProcessed;
		const float Perc = 100.0f - (m_TotalBytesPostProcessed * 100.0f / m_TotalBytesPreProcessed);
		std::cout << "Total Bytes Pre-Processed: " << m_TotalBytesPreProcessed << std::endl;
		std::cout << "Total Bytes Post-Processed: " << m_TotalBytesPostProcessed << std::endl;
		std::cout << "Total Difference: " << std::fixed << std::setprecision(2) << Perc << "% (" << TotalBytes << " bytes)" << std::endl;
		std::cout << std::endl << "Frequency Table (Pre-Processed Packets Data):" << std::endl;
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
		m_TotalBytesPostProcessed = 0;
		m_TotalBytesPreProcessed = 0;
		m_NumPacketsRead = 0;
		m_NumChunksRead = 0;
		m_NumConnlessChunks = 0;
		m_NumConnChunks = 0;
		m_NumControlChunks = 0;
		m_NumResendChunks = 0;
		m_NumCompressionChunks = 0;
		m_NumNoFlagChunks = 0;
		m_NumInvalidPackets = 0;
		m_ConlessControlIds.clear();
		m_ControlIds.clear();
		for (int i=0; i<256; m_FreqTable[i++]=0);
	}

	void Read(std::string File)
	{
		CLine DumpLine;
		CNetChunkHeader Header;
		m_Stream = std::ifstream(File, std::ios_base::in | std::ios_base::binary);
		if (m_Stream.is_open())
		{
			Reset();
			do
			{
				if (ReadPacketInfo(&DumpLine))
				{
					if (DumpLine.m_Type == 0)
					{
						m_TotalBytesPostProcessed += DumpLine.m_Size;

						unsigned int CurrentChunk = 0;
						CNetPacketConstruct PacketConstruct;
						if (CNetBase::UnpackPacket(DumpLine.m_pData, DumpLine.m_Size, &PacketConstruct) == 0)
						{
							++m_NumPacketsRead;

							const unsigned char *pEnd = PacketConstruct.m_aChunkData + PacketConstruct.m_DataSize;

							while (1)
							{
								if (CurrentChunk >= PacketConstruct.m_NumChunks)
									break;

								unsigned char *pData = PacketConstruct.m_aChunkData;
								for(int i = 0; i < CurrentChunk; i++)
								{
									pData = Header.Unpack(pData);
									pData += Header.m_Size;
								}

								// unpack the header
								pData = Header.Unpack(pData);
								++CurrentChunk;

								if(pData+Header.m_Size > pEnd)
									break;

								if (Header.m_Flags&NET_PACKETFLAG_CONNLESS)
								{
									++m_NumConnlessChunks;
									std::map<int, int>::iterator it = m_ConlessControlIds.find(pData[0]);
									if (it == m_ConlessControlIds.end())
										m_ConlessControlIds.insert(std::make_pair(pData[0], 1));
									else
										++(*it).second;
								} else
								{
									++m_NumConnChunks;

									if (Header.m_Flags&NET_PACKETFLAG_CONTROL)
									{
										++m_NumControlChunks;
										std::map<int, int>::iterator it = m_ControlIds.find(pData[0]);
										if (it == m_ControlIds.end())
											m_ControlIds.insert(std::make_pair(pData[0], 1));
										else
											++(*it).second;
									}
									if (Header.m_Flags&NET_PACKETFLAG_RESEND)
										++m_NumResendChunks;
									if (Header.m_Flags&NET_PACKETFLAG_COMPRESSION)
										++m_NumCompressionChunks;
									if (!Header.m_Flags)
										++m_NumNoFlagChunks;
								}

								++m_NumChunksRead;
							}
						} else
							++m_NumInvalidPackets;
					}
					else
						m_TotalBytesPreProcessed += DumpLine.m_Size;

					DumpLine.Reset();
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
