#include <base/system.h>
#include <engine/shared/network.h>
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip> // std::setprecision
#include <map>

class CLogNetworkReader
{
	class CDumpRecord
	{
	public:
		CDumpRecord()
		{
			m_Type = -1;
			m_Size = 0;
			m_pData = 0x0;
		}
		~CDumpRecord()
		{
			Reset();
		}

		void Reset()
		{
			if (m_pData)
			{
				delete [] m_pData;
				m_pData = 0x0;
			}
		}

		int m_Type;
		int m_Size;
		char unsigned *m_pData;
	};

	std::ifstream m_Stream;

	unsigned int m_TotalDumpRecords;
	unsigned int m_NumPacketsRead;
	unsigned int m_NumChunksRead;

	unsigned int m_TotalBytesPreProcessed;
	unsigned int m_TotalBytesPostProcessed;
	unsigned int m_FreqTable[256]; // Pre-Processed Packets

	std::map<int, int> m_ControlIds;
	unsigned int m_NumConnlessPackets;
	unsigned int m_NumControlPackets;
	unsigned int m_NumResendPackets;
	unsigned int m_NumCompressionPackets;
	unsigned int m_NumNoFlagPackets;
	unsigned int m_NumInvalidPackets;

	unsigned int m_NumVitalChunks;
	unsigned int m_NumResendChunks;
	unsigned int m_NumNoFlagChunks;


	void ReadDumpRegistry(CDumpRecord *pDumpLine)
	{
		m_Stream.read(reinterpret_cast<char*>(&pDumpLine->m_Type), sizeof(pDumpLine->m_Type));
		m_Stream.read(reinterpret_cast<char*>(&pDumpLine->m_Size), sizeof(pDumpLine->m_Size));
		pDumpLine->m_pData = new unsigned char[pDumpLine->m_Size];
		m_Stream.read(reinterpret_cast<char*>(pDumpLine->m_pData), pDumpLine->m_Size);

		if (pDumpLine->m_Type == 1)
			for (int i=0; i<pDumpLine->m_Size; ++m_FreqTable[pDumpLine->m_pData[i++]]);
	}

	void PrintInfo(const std::string &File) const
	{
		std::cout << "Log Network File: " << File << std::endl << std::endl;
		std::cout << "Total Dump Records: " << m_TotalDumpRecords << " (" << m_TotalDumpRecords/2 << " unique)" << std::endl << std::endl;
		std::cout << "Num. Packets Read: " << m_NumPacketsRead << std::endl;
		std::cout << "  - Connless: " << m_NumConnlessPackets << std::endl;
		std::cout << "  - Control: " << m_NumControlPackets << std::endl;
		std::map<int, int>::const_iterator cit = m_ControlIds.begin();
		while (cit != m_ControlIds.end())
		{
			std::cout << "       " << (*cit).first << " \t" << (*cit).second << " times" << std::endl;
			++cit;
		}
		std::cout << "  - Resend: " << m_NumResendPackets << std::endl;
		std::cout << "  - Compression: " << m_NumCompressionPackets << std::endl;
		std::cout << "  - No Flagged: " << m_NumNoFlagPackets << std::endl;
		std::cout << "Num. Invalid Packets: " << m_NumInvalidPackets << std::endl;
		std::cout << std::endl;
		std::cout << "Num. Chunks Read: " << m_NumChunksRead << std::endl;
		std::cout << "  - Vital: " << m_NumVitalChunks << std::endl;
		std::cout << "  - Resend: " << m_NumResendChunks << std::endl;
		std::cout << "  - No Flagged: " << m_NumNoFlagChunks << std::endl;
		std::cout << std::endl;
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
		m_TotalBytesPostProcessed = 0u;
		m_TotalBytesPreProcessed = 0u;
		m_NumPacketsRead = 0u;
		m_NumChunksRead = 0u;
		m_NumConnlessPackets = 0u;
		m_NumControlPackets = 0u;
		m_NumResendPackets = 0u;
		m_NumCompressionPackets = 0u;
		m_NumNoFlagPackets = 0u;
		m_NumInvalidPackets = 0u;
		m_TotalDumpRecords = 0u;
		m_NumVitalChunks = 0u;
		m_NumResendChunks = 0u;
		m_NumNoFlagChunks = 0u;
		m_ControlIds.clear();
		for (int i=0; i<256; m_FreqTable[i++]=0u);
	}

	void Read(std::string File)
	{
		CDumpRecord DumpLine;
		CNetChunkHeader Header;
		m_Stream = std::ifstream(File, std::ios_base::in | std::ios_base::binary);
		if (m_Stream.is_open())
		{
			Reset();
			do
			{
				ReadDumpRegistry(&DumpLine);
				{
					++m_TotalDumpRecords;

					if (DumpLine.m_Type == 0)
					{
						m_TotalBytesPostProcessed += DumpLine.m_Size;

						unsigned int CurrentChunk = 0;
						CNetPacketConstruct PacketConstruct;
						if (CNetBase::UnpackPacket(DumpLine.m_pData, DumpLine.m_Size, &PacketConstruct) == 0)
						{
							// ** ANALIZE PACKET **
							++m_NumPacketsRead;

							if (PacketConstruct.m_Flags&NET_PACKETFLAG_CONTROL)
							{
								++m_NumControlPackets;

								const int MsgId = PacketConstruct.m_aChunkData[0];
								std::map<int, int>::iterator it = m_ControlIds.find(MsgId);
								if (it == m_ControlIds.end())
									m_ControlIds.insert(std::make_pair(MsgId, 1));
								else
									++(*it).second;
							}
							if (PacketConstruct.m_Flags&NET_PACKETFLAG_CONNLESS)
								++m_NumConnlessPackets;
							if (PacketConstruct.m_Flags&NET_PACKETFLAG_RESEND)
								++m_NumResendPackets;
							if (PacketConstruct.m_Flags&NET_PACKETFLAG_COMPRESSION)
								++m_NumCompressionPackets;
							if (!PacketConstruct.m_Flags)
								++m_NumNoFlagPackets;


							// ** ANALIZE PACKET CHUNKS **
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

								if (Header.m_Flags&NET_CHUNKFLAG_VITAL)
									++m_NumVitalChunks;
								if (Header.m_Flags&NET_CHUNKFLAG_RESEND)
									++m_NumResendChunks;
								if (!Header.m_Flags)
									++m_NumNoFlagChunks;

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

	CNetBase::Init();
	CLogNetworkReader LogNetReader;
	for (int i=1; i<argc; LogNetReader.Read(argv[i++]));
	return 0;
}
