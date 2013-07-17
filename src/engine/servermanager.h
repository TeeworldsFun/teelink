/*
    unsigned char*
*/
#ifndef ENGINE_SERVERMANAGER_H
#define ENGINE_SERVERMANAGER_H

#include "kernel.h"
#include <string>

#define MAX_LOCAL_SERVERS 12

class IServerManager : public IInterface
{
	MACRO_INTERFACE("servermanager", 0)
public:
	class CMServerEntry
	{
	public:
        bool m_Used;

        char m_Port[5];
        char m_Type[35];
        char m_FileConfg[125];
        char m_Exec[125];
	};

    virtual void Init() = 0;
    virtual void ShutdownAll() = 0;

    virtual void SearchServersBin() = 0;

    virtual CMServerEntry GetInfoServer(int index) const = 0;
	virtual bool CreateServer(char *fileconfg, char *exec) = 0;
	virtual bool DestroyServer(int instance) = 0;


	virtual std::string GetExec(int index) = 0;
	virtual int NumBinExecs() const = 0;

	virtual bool SaveConfigFile(const char *filename) = 0;
	virtual void GetConfigParam(const char *param, const char *filename, char *aDest, unsigned int sizeDest, unsigned int offset = 0) = 0;
};

#endif
