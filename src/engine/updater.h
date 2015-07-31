/* (c) unsigned char*. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at https://github.com/CytraL/HClient */
#ifndef ENGINE_UPDATER_H
#define ENGINE_UPDATER_H

#include "kernel.h"
#include <game/client/components/menus.h>
#include <base/tl/array.h>
#include <string>

class IUpdater : public IInterface
{
	MACRO_INTERFACE("updater", 0)
public:
	virtual void CheckUpdates(CMenus *pMenus) = 0;
	virtual void DoUpdates(CMenus *pMenus) = 0;
	virtual bool Updated() = 0;
	virtual bool NeedResetClient() const = 0;
    virtual bool NeedUpdateClient() const = 0;
	virtual bool NeedUpdateServer() const = 0;
	virtual void ExecuteExit() = 0;
	virtual const char* GetNewVersion() const = 0;
    virtual array<std::string>& GetFilesToRemove() = 0;
	virtual array<std::string>& GetFilesToDownload() = 0;

    virtual const char* GetCurrentDownloadFileName() const = 0;
	virtual float GetCurrentDownloadProgress() = 0;
};

struct InfoUpdatesThread
{
    IUpdater *m_pUpdater;
    CMenus *m_pMenus;
};

void ThreadUpdates(void *);

#endif

