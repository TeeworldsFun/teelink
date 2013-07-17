#include <base/math.h>
#include <base/system.h>
#include <engine/storage.h>
#include <engine/graphics.h>
#include <engine/shared/config.h>
#include <game/version.h>
#include <game/client/components/menus.h>

#if defined(CONF_FAMILY_UNIX)
	#include <sys/time.h>
	#include <unistd.h>

	/* unix net includes */
	#include <sys/stat.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <sys/ioctl.h>
	#include <errno.h>
	#include <netdb.h>
	#include <netinet/in.h>
	#include <fcntl.h>
	#include <pthread.h>
	#include <arpa/inet.h>

	#include <dirent.h>

	#if defined(CONF_PLATFORM_MACOSX)
		#include <Carbon/Carbon.h>
	#endif

#elif defined(CONF_FAMILY_WINDOWS)
	#define WIN32_LEAN_AND_MEAN
	#define _WIN32_WINNT 0x0501 /* required for mingw to get getaddrinfo to work */
	#include <windows.h>
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include <Shellapi.h>
	#include <shlobj.h>
	#include <fcntl.h>
	#include <direct.h>
	#include <errno.h>
#else
	#error NOT IMPLEMENTED
#endif

#include <string.h>
#include <cstdio> //remove

#include "news.h"

static NETSOCKET invalid_socket = {NETTYPE_INVALID, -1, -1};

CNews::CNews()
{
    m_vNews.clear();
    m_pStorage = 0x0;
    m_pGraphics = 0x0;
    m_State = STATE_EMPTY;

    m_PageIndex = 1;
}

void CNews::Init()
{
    m_pStorage = Kernel()->RequestInterface<IStorageTW>();
    m_pGraphics = Kernel()->RequestInterface<IGraphics>();
}

void CNews::FreePost()
{
    for (size_t i=0; i<m_vNews.size(); i++)
    {
        if (m_vNews[i].m_ImageID != -1)
        {
            m_pGraphics->UnloadTexture(m_vNews[i].m_ImageID);
        }
    }

    m_vNews.clear();
}

void CNews::RefreshForumFails(unsigned int page)
{
    FreePost();

    char aBuff[1024];
    char aUrl[512];
    SNewsPost Post;

    m_State = STATE_DOWNLOADING;

    dbg_msg("News", "Getting News...");
    page = m_PageIndex;
    str_format(aUrl, sizeof(aUrl), "teeworldsforumfails.posterous.com/?page=%d", page);
    if (!GetHTTPFile(aUrl,"newscache/forum_fails.html"))
    {
        dbg_msg("News", "ERROR: Can't download news :S");
        m_State = STATE_EMPTY;
        return;
    }

    dbg_msg("News", "Ok, processing data... ");
    IOHANDLE htmlFile = m_pStorage->OpenFile("newscache/forum_fails.html", IOFLAG_READ, IStorageTW::TYPE_SAVE);
    if (!htmlFile)
        return;

    //read data
    std::string ReadData;
    bool inScript = false;
    while (io_read(htmlFile, aBuff, sizeof(aBuff)) > 0)
    {
        for (size_t i=0; i<sizeof(aBuff); i++)
        {
            if (aBuff[i]=='\n')
            {
                if (i>0 && aBuff[i-1] == '\r')
                    ReadData = ReadData.substr(0, ReadData.length()-2);

                if (!inScript)
                {
                    if (ReadData.find("<script>") != std::string::npos)
                    {
                        memset(Post.m_Date, 0, sizeof(Post.m_Date));
                        memset(Post.m_Title, 0, sizeof(Post.m_Title));
                        memset(Post.m_Text, 0, sizeof(Post.m_Text));
                        memset(Post.m_Image, 0, sizeof(Post.m_Image));
                        Post.m_ImageID = 0;
                        inScript = true;
                    }
                }
                else
                {
                    //Get Date
                    if (ReadData.find("\"description\":\"") != std::string::npos)
                    {
                        int posS = ReadData.find("\"description\":\"");
                        int posE = posS+15;
                        do {
                            posE = ReadData.find_first_of("\"", posE+1);
                        } while (ReadData[posE-1] == '\\');
                        str_copy(Post.m_Text, ReadData.substr(posS+15, posE-(posS+15)).c_str(), sizeof(Post.m_Text));
                    }
                    //Get Title
                    if (ReadData.find("\"name\":\"") != std::string::npos)
                    {
                        int posS = ReadData.find("\"name\":\"");
                        int posE = ReadData.find_first_of("\"", posS+9);
                        str_copy(Post.m_Title, ReadData.substr(posS+8, posE-(posS+8)).c_str(), sizeof(Post.m_Title));
                    }
                    //Get Image
                    if (ReadData.find("\"src\":\"") != std::string::npos)
                    {
                        int posS = ReadData.find("\"src\":\"");
                        int posE = ReadData.find_first_of("\"", posS+8);

                        std::string fullpath(ReadData.substr(posS+7, posE-(posS+7)).c_str());
                        std::string path, filename;
                        posS = fullpath.find_last_of("\\/");
                        path = fullpath.substr(0, posS+1);
                        filename = fullpath.substr(posS+1);
                        posS = filename.find_first_of(".");
                        filename = filename.substr(0, posS+1);
                        filename.append("PNG");
                        fullpath.clear();
                        fullpath.append(path);
                        fullpath.append(filename);

                        char aBuf[128];
                        if (!m_pStorage->FindFile(filename.c_str(), "newscache", IStorageTW::TYPE_ALL, aBuf, sizeof(aBuf)))
                        {
                            filename.insert(0, "newscache/");
                            if (GetHTTPFile(fullpath.c_str(), filename.c_str()))
                                str_copy(Post.m_Image, filename.c_str(), sizeof(Post.m_Image));
                        }
                        else
                        {
                            filename.insert(0, "newscache/");
                            str_copy(Post.m_Image, filename.c_str(), sizeof(Post.m_Image));
                        }
                    }

                    if (ReadData.find("</script>") != std::string::npos)
                    {
                        inScript = false;
                        if (Post.m_Title[0] != 0)
                            m_vNews.push_back(Post);
                    }
                }

                ReadData.clear();
                continue;
            }

            ReadData+=aBuff[i];
        }
    }

    io_close(htmlFile);
    m_pStorage->RemoveFile("newscache/forum_fails.html", IStorageTW::TYPE_SAVE);

    m_State = STATE_READY;
}

void CNews::RefreshTeeworlds(unsigned int page)
{
    m_vNews.clear();
}

void CNews::LoadTextures()
{
    for (size_t i=0; i<m_vNews.size(); i++)
    {
        if (m_vNews[i].m_Image[0] != 0)
        {
            m_vNews[i].m_ImageID = m_pGraphics->LoadTexture(m_vNews[i].m_Image, IStorageTW::TYPE_ALL, CImageInfo::FORMAT_AUTO, IGraphics::TEXLOAD_NORESAMPLE);
        }
        else
            m_vNews[i].m_ImageID = -1;
    }
}

bool CNews::GetHTTPFile(std::string aUrl, const char *dst)
{
    NETSOCKET Socket = invalid_socket;
    NETADDR HostAddress;
    char aNetBuff[2048];

    char Host[255], Params[4000];
    int posDel = 0;
    int posEnd = 0;

    if ((posDel = aUrl.find("www.")) != std::string::npos)
    {
        posEnd = aUrl.find_first_of("/",posDel+5);
        str_copy(Host,aUrl.substr(posDel+5, posEnd-(posDel+4)).c_str(), sizeof(Host));
    }
    else if ((posDel = aUrl.find("http://")) != std::string::npos)
    {
        posEnd = aUrl.find_first_of("/",posDel+8);
        str_copy(Host,aUrl.substr(posDel+7, posEnd-(posDel+7)).c_str(), sizeof(Host));
    }
    else
    {
        posDel = 0;
        posEnd = aUrl.find_first_of("/", 0);
        str_copy(Host,aUrl.substr(0, posEnd).c_str(), sizeof(Host));
    }


    str_copy(Params,aUrl.substr(posEnd).c_str(), sizeof(Params));

    //Lookup
	if(net_host_lookup(Host, &HostAddress, NETTYPE_IPV4) != 0)
	{
        dbg_msg("NEWS","ERROR: Can't lookup: %s", Host);
        return false;
	}

    //Connect
    int socketID = socket(AF_INET, SOCK_STREAM, 0);
	if(socketID < 0)
	{
        dbg_msg("NEWS","ERROR: Can't create socket.");
		return false;
	}

    Socket.type = NETTYPE_IPV4;
    Socket.ipv4sock = socketID;
    HostAddress.port = 80;

	if(net_tcp_connect(Socket, &HostAddress) != 0)
	{
	    net_tcp_close(Socket);
	    dbg_msg("NEWS","ERROR: Can't connect with '%s'...", Host);
		return false;
	}

    //Send request
    str_format(aNetBuff, sizeof(aNetBuff), "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", Params, Host);
    net_tcp_send(Socket, aNetBuff, strlen(aNetBuff));



    //read data
    IOHANDLE dstFile = m_pStorage->OpenFile(dst, IOFLAG_WRITE, IStorageTW::TYPE_SAVE);
    if (!dstFile)
    {
        net_tcp_close(Socket);
        dbg_msg("NEWS","ERROR: Can't write file in disk.");
        return false;
    }

    std::string NetData;
    int TotalRecv = 0;
    int TotalBytes = 0;
    int CurrentRecv = 0;
    bool isHead = true;
    int enterCtrl = 0;
    while ((CurrentRecv = net_tcp_recv(Socket, aNetBuff, sizeof(aNetBuff))) > 0)
    {
        for (int i=0; i<CurrentRecv ; i++)
        {
            if (isHead)
            {
                if (aNetBuff[i]=='\n')
                {
                    enterCtrl++;
                    if (isHead && enterCtrl == 2)
                    {
                        isHead = false;
                        NetData.clear();
                        continue;
                    }

                    int posDel = NetData.find_first_of(":");
                    if (posDel > 0 && str_comp_nocase(NetData.substr(0, posDel).c_str(),"content-length") == 0)
                        TotalBytes = atoi(NetData.substr(posDel+2).c_str());
                    else if (posDel > 0 && str_comp_nocase(NetData.substr(0, posDel).c_str(),"Location") == 0)
                    {
                        dbg_msg("NEWS", "Redirecting...");
                        io_close(dstFile);
                        net_tcp_close(Socket);
                        return GetHTTPFile(NetData.substr(posDel+2,(NetData.length()-(posDel+2))-1).c_str(), dst);
                    }

                    NetData.clear();
                    continue;
                }
                else if (aNetBuff[i]!='\r')
                    enterCtrl=0;

                NetData+=aNetBuff[i];
            }
            else
            {
                if (TotalBytes == 0)
                {
                    io_close(dstFile);
                    net_tcp_close(Socket);
                    dbg_msg("NEWS","ERROR: Error with file size.");
                    return false;
                }

                io_write(dstFile, &aNetBuff[i], 1);

                TotalRecv++;
                if (TotalRecv == TotalBytes)
                    break;
            }
        }
    }

    //Finish
    io_close(dstFile);
    net_tcp_close(Socket);

    return true;
}

void ThreadRefreshForumFails(void *params)
{
    CNews *pNews = static_cast<CNews*>(params);

    pNews->RefreshForumFails();
}
