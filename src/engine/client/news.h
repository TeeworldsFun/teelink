/*
    unsigned char*
*/
#ifndef ENGINE_CLIENT_NEWS_H
#define ENGINE_CLIENT_NEWS_H

#include <base/system.h>
#include <engine/news.h>
#include <vector>
#include <string>

class CNews : public INews
{
public:
    CNews();

    void Init();

    void RefreshForumFails(unsigned int page = 0);
    void RefreshTeeworlds(unsigned int page = 0); //TODO: implemented it!
    size_t TotalNews() { return m_vNews.size(); }
    SNewsPost GetNew(size_t npos) { return m_vNews[npos]; }

    int GetState() { return m_State; }
    void LoadTextures();

    void SetPage(unsigned int page) { m_PageIndex = page; };

    std::vector<SNewsPost> m_vNews;
    int m_State;
    unsigned int m_PageIndex;

protected:
    class IStorageTW *m_pStorage;
    class IGraphics *m_pGraphics;

    bool GetHTTPFile(std::string aUrl, const char *dst);
    void FreePost();
};
#endif
