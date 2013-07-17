/*
    unsigned char*
*/
#ifndef ENGINE_NEWS_H
#define ENGINE_NEWS_H

#include "kernel.h"
#include <vector>

struct SNewsPost {
    char m_Date[128];
    char m_Title[128];
    char m_Text[2048];
    char m_Image[512];
    int m_ImageID;
};

class INews : public IInterface
{
	MACRO_INTERFACE("news", 0)
public:
    enum
    {
        MAX_NEWS_PER_PAGE=4,

        STATE_EMPTY=0,
        STATE_DOWNLOADING,
        STATE_READY,
    };


    virtual void Init() = 0;

    virtual void RefreshForumFails(unsigned int page = 0) = 0;
    virtual void RefreshTeeworlds(unsigned int page = 0) = 0;

    virtual size_t TotalNews() = 0;
    virtual SNewsPost GetNew(size_t npos) = 0;

    virtual int GetState() = 0;
    virtual void LoadTextures() = 0;

    virtual void SetPage(unsigned int page) = 0;

};

void ThreadRefreshForumFails(void *params);
#endif
