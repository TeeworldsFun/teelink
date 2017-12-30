/* Alexandre Díaz - 2017 - TW Log Network Reader */
#include <base/system.h>
#include <engine/shared/network.h>
#include <map>

class CLogNetworkReader
{
	class CDumpRecord
	{
	public:
		CDumpRecord()
		{
			m_pData = 0x0;
			Reset();
		}
		~CDumpRecord()
		{
			Reset();
		}

		void Reset()
		{
			mem_free(m_pData);
			m_pData = 0x0;

			m_Type = -1;
			m_Size = -1;
		}

		int m_Type;
		int m_Size;
		unsigned char *m_pData;
	};

	IOHANDLE m_FileHandle;

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


	void ReadNextDumpRecord(CDumpRecord *pDumpRecord)
	{
		io_read(m_FileHandle, &pDumpRecord->m_Type, sizeof(pDumpRecord->m_Type));
		io_read(m_FileHandle, &pDumpRecord->m_Size, sizeof(pDumpRecord->m_Size));
		pDumpRecord->m_pData = static_cast<unsigned char*>(mem_alloc(pDumpRecord->m_Size, 1));
		dbg_assert(pDumpRecord->m_pData != 0x0, "Can't allocate memory!");
		io_read(m_FileHandle, pDumpRecord->m_pData, pDumpRecord->m_Size);
	}

	void PrintInfo() const
	{
		dbg_msg("NetDump", "Total Dump Records: %u (%u unique)\n", m_TotalDumpRecords, m_TotalDumpRecords/2);

		dbg_msg("NetDump", "Num. Packets Read: %u", m_NumPacketsRead);
		dbg_msg("NetDump", "  - Connless: %u", m_NumConnlessPackets);
		dbg_msg("NetDump", "  - Control: %u", m_NumControlPackets);
		std::map<int, int>::const_iterator cit = m_ControlIds.begin();
		while (cit != m_ControlIds.end())
		{
			dbg_msg("NetDump", "       %d \t%d times", (*cit).first, (*cit).second);
			++cit;
		}
		dbg_msg("NetDump", "  - Resend: %u", m_NumResendPackets);
		dbg_msg("NetDump", "  - Compression: %u", m_NumCompressionPackets);
		dbg_msg("NetDump", "  - No Flagged: %u", m_NumNoFlagPackets);
		dbg_msg("NetDump", "Num. Invalid Packets: %u\n", m_NumInvalidPackets);

		dbg_msg("NetDump", "Num. Chunks Read: %u",m_NumChunksRead);
		dbg_msg("NetDump", "  - Vital: %u", m_NumVitalChunks);
		dbg_msg("NetDump", "  - Resend: %u", m_NumResendChunks);
		dbg_msg("NetDump", "  - No Flagged: %u\n", m_NumNoFlagChunks);

		const int DiffBytes = m_TotalBytesPreProcessed - m_TotalBytesPostProcessed;
		const float Perc = 100.0f - (m_TotalBytesPostProcessed * 100.0f / m_TotalBytesPreProcessed);
		dbg_msg("NetDump", "Total Bytes Pre-Processed: %u", m_TotalBytesPreProcessed);
		dbg_msg("NetDump", "Total Bytes Post-Processed: %u", m_TotalBytesPostProcessed);
		dbg_msg("NetDump", "Total Difference: %.2f (%d bytes)\n", Perc, DiffBytes);

		dbg_msg("NetDump", "Frequency Table (Pre-Processed Packets Data):");
		// FIXME
		char aBuff[3072] = {0};
		for (int i=0; i<255; i++)
		{
			char aHexVal[32] = {0};
			str_format(aHexVal, sizeof(aHexVal), "%#0x%c ", m_FreqTable[i], (i < 254)?',':'\0');
			str_append(aBuff, aHexVal, sizeof(aBuff));
		}
		dbg_msg("NetDump", "%s", aBuff);
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

	void Read(const char *pFile)
	{
		m_FileHandle = io_open(pFile, IOFLAG_READ);
		if (m_FileHandle)
		{
			io_seek(m_FileHandle, 0, IOSEEK_END);
			const long int FileSize = io_tell(m_FileHandle);
			io_seek(m_FileHandle, 0, IOSEEK_START);

			if (FileSize <= 0)
				return;
			Reset();

			CDumpRecord DumpRecord;
			do
			{
				ReadNextDumpRecord(&DumpRecord);
				if (DumpRecord.m_Type == -1)
					continue;

				++m_TotalDumpRecords;

				if (DumpRecord.m_Type == 0)
				{ // Is a Post-Processed Record (the information as it is sent/received through the network)
					m_TotalBytesPostProcessed += DumpRecord.m_Size;

					unsigned int CurrentChunk = 0u;
					CNetPacketConstruct PacketConstruct;
					if (CNetBase::UnpackPacket(DumpRecord.m_pData, DumpRecord.m_Size, &PacketConstruct) == 0)
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
						CNetChunkHeader Header;
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
				else if (DumpRecord.m_Type == 1)
				{
					m_TotalBytesPreProcessed += DumpRecord.m_Size;

					for (int i=0; i<DumpRecord.m_Size; ++m_FreqTable[DumpRecord.m_pData[i++]]);
				}

				DumpRecord.Reset();
			} while (io_tell(m_FileHandle) < FileSize);

			io_close(m_FileHandle);
			m_FileHandle = 0;

			dbg_msg("NetDump", "Log Network File: %s\n", pFile);
			PrintInfo();
		}
		else
			dbg_msg("NetDump", "Can't read '%s'!", pFile);
	}
};

int main(int argc, const char **argv)
{
	dbg_logger_stdout();

	if (argc == 1)
	{
		dbg_msg("NetDump", "Invalid parameters!\n\tUsage: %s FILE1 [ FILE2... ]", argv[0]);
		return -1;
	}

	CNetBase::Init();
	CLogNetworkReader LogNetReader;
	for (int i=1; i<argc; LogNetReader.Read(argv[i++]));
	return 0;
}
