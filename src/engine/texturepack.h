/*
    unsigned char*
*/
#ifndef ENGINE_TEXTUREPACK_H
#define ENGINE_TEXTUREPACK_H

#include "kernel.h"
#include <list>

struct CTheme
{
    char m_Name[50];
    char m_FolderName[50];

    char m_Author[25];
    char m_Version[8];
    char m_Mail[128];
    char m_Web[128];
};

class ITexturePack : public IInterface
{
	MACRO_INTERFACE("texturepack", 0)

public:
    virtual void Init() = 0;
    virtual bool Load(const char *rawpackname) = 0;
    virtual std::list<CTheme> GetThemes() = 0;
};
#endif
