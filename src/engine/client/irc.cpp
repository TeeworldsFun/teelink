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
#include <ctime>
#include <cstdio> //remove

#include "irc.h"

static NETSOCKET invalid_socket = {NETTYPE_INVALID, -1, -1};

CIrc::CIrc()
{
    m_IrcComs.clear();
    m_pStorage = 0x0;
    m_pGraphics = 0x0;
    m_State = STATE_DISCONNECTED;
    m_Socket = invalid_socket;
    char tmpNick[25]={0};
    str_format(tmpNick, sizeof(tmpNick), "HC-%d", time(NULL));
    m_Nick = tmpNick;
    SetActiveCom(-1);
}

void CIrc::Init()
{
    m_pStorage = Kernel()->RequestInterface<IStorageTW>();
    m_pGraphics = Kernel()->RequestInterface<IGraphics>();
    m_pGameClient = Kernel()->RequestInterface<IGameClient>();
}

void CIrc::SetActiveCom(size_t index)
{
    if (index < 0 || index >= m_IrcComs.size())
        index = 0;

    m_ActiveCom = index;
    CIrcCom *pCom = GetCom(index);
    if (pCom)
    {
        pCom->m_UnreadMsg = false;
        pCom->m_NumUnreadMsg = 0;
    }
}

CIrcCom* CIrc::GetActiveCom()
{
    if (m_ActiveCom < 0 ||m_ActiveCom >= m_IrcComs.size())
        return 0x0;

    std::list<CIrcCom*>::iterator it = m_IrcComs.begin();
    std::advance(it, m_ActiveCom);
    return (*it);
}

CIrcCom* CIrc::GetCom(size_t index)
{
    if (index < 0 || index >= m_IrcComs.size())
        return 0x0;

    std::list<CIrcCom*>::iterator it = m_IrcComs.begin();
    std::advance(it, index);
    return (*it);
}
CIrcCom* CIrc::GetCom(std::string name)
{
    std::list<CIrcCom*>::iterator it = m_IrcComs.begin();

    while (it != m_IrcComs.end())
    {
        if ((*it)->GetType() == CIrcCom::TYPE_CHANNEL)
        {
            CComChan *pChan = static_cast<CComChan*>((*it));
            if (str_comp_nocase(name.c_str(), pChan->m_Channel) == 0)
                return (*it);
        }
        else if ((*it)->GetType() == CIrcCom::TYPE_QUERY)
        {
            CComQuery *pQuery = static_cast<CComQuery*>((*it));
            if (str_comp_nocase(name.c_str(), pQuery->m_User) == 0)
                return (*it);
        }

        ++it;
    }

    return 0x0;
}

void CIrc::StartConnection()
{
    char aNetBuff[2048];

    m_State = STATE_CONNECTING;
    //Lookup
	if(net_host_lookup(g_Config.m_IrcServer, &m_HostAddress, NETTYPE_IPV4) != 0)
	{
        dbg_msg("IRC","ERROR: Can't lookup Quakenet");
        m_State = STATE_DISCONNECTED;
        return;
	}
    //Connect
    int socketID = socket(AF_INET, SOCK_STREAM, 0);
	if(socketID < 0)
	{
        dbg_msg("IRC","ERROR: Can't create socket.");
        m_State = STATE_DISCONNECTED;
		return;
	}

    m_Socket.type = NETTYPE_IPV4;
    m_Socket.ipv4sock = socketID;
    m_HostAddress.port = g_Config.m_IrcPort;

	if(net_tcp_connect(m_Socket, &m_HostAddress) != 0)
	{
	    net_tcp_close(m_Socket);
	    dbg_msg("IRC","ERROR: Can't connect with 'quakenet'...");
	    m_State = STATE_DISCONNECTED;
		return;
	}

    //Send request
    str_format(aNetBuff, sizeof(aNetBuff), "CAP LS\r\n");
    net_tcp_send(m_Socket, aNetBuff, strlen(aNetBuff));
    str_format(aNetBuff, sizeof(aNetBuff), "NICK %s\r\n", m_Nick.c_str());
    net_tcp_send(m_Socket, aNetBuff, strlen(aNetBuff));
    str_format(aNetBuff, sizeof(aNetBuff), "USER HClient 0 * :HClient\r\n", m_Nick.c_str());
    net_tcp_send(m_Socket, aNetBuff, strlen(aNetBuff));

    //Status Tab
    CComQuery *pStatus = new CComQuery();
    str_copy(pStatus->m_User, "@Status", sizeof(pStatus->m_User));
    m_IrcComs.push_back(pStatus);
    SetActiveCom(0);

    m_State = STATE_CONNECTED;

    std::string NetData;
    int TotalRecv = 0;
    int TotalBytes = 0;
    int CurrentRecv = 0;
    char LastPong[255]={0};
    while ((CurrentRecv = net_tcp_recv(m_Socket, aNetBuff, sizeof(aNetBuff))) > 0 && m_State == STATE_CONNECTED)
    {
        for (int i=0; i<CurrentRecv; i++)
        {
            if (aNetBuff[i]=='\r' || aNetBuff[i]=='\t')
                 continue;

            if (aNetBuff[i]=='\n')
            {
                int del = NetData.find_first_of(":");
                int ldel = 0;
                if (del > 0)
                { //NeT Message
                    std::string aMsgID = NetData.substr(0, del-1);
                    std::string aMsgText = NetData.substr(del+1, NetData.length()-del-1);
                    if (aMsgID.compare("PING") == 0)
                    {
                        char aBuff[255];
                        str_format(aNetBuff, sizeof(aNetBuff), "PONG %s :%s\r\n", LastPong, aMsgText.c_str());
                        net_tcp_send(m_Socket, aNetBuff, strlen(aNetBuff));
                        /*CIrcCom *pCom = GetCom("@Status");
                        str_format(aBuff, sizeof(aBuff), "PING [%s]", aMsgText.c_str());
                        pCom->m_Buffer.push_back(aBuff);*/
                        LastPong[0]=0;
                    }
                    else if (aMsgID.compare("PONG") == 0)
                        str_copy(LastPong, aMsgText.c_str(), sizeof(LastPong));
                    else
                    {
                        CIrcCom *pCom = GetCom("@Status");
                        pCom->m_Buffer.push_back(aMsgText);
                    }
                } else
                { //Raw Message
                    del = NetData.find_first_of(" ");
                    std::string aMsgFServer = NetData.substr(1, del);
                    ldel = del;
                    del = NetData.find_first_of(" ",del+1);
                    std::string aMsgID = NetData.substr(ldel+1, del-ldel-1);

                    //dbg_msg("IRC", "Raw MSG [%s]: %s",aMsgID.c_str(), aMsgFServer.c_str());
                    //dbg_msg("IRC-RAW", NetData.c_str());

                    if (aMsgID.compare("001") == 0)
                    {
                        //Auto-Join
                        JoinTo("#H-Client");
                    }
                    else if (aMsgID.compare("332") == 0)
                    {
                        del = NetData.find_first_of(" ",del+1);
                        ldel = NetData.find_first_of(" ",del+1);
                        std::string aMsgChan = NetData.substr(del+1,ldel-del-1);

                        del = NetData.find_first_of(":", 1);
                        std::string aMsgTopic = NetData.substr(del+1, NetData.length()-del-1);

                        CComChan *pChan = static_cast<CComChan*>(GetCom(aMsgChan));
                        if (pChan)
                            pChan->m_Topic = aMsgTopic;
                    }
                    else if (aMsgID.compare("353") == 0)
                    {
                        char aBuff[255];
                        del = NetData.find_first_of("=");
                        ldel = NetData.find_first_of(" ",del+2);

                        std::string aMsgChan = NetData.substr(del+2, ldel-del-2);
                        del = NetData.find_first_of(":",1);
                        std::string aMsgUsers = NetData.substr(del+1, NetData.length()-del-1);

                        CComChan *pChan = static_cast<CComChan*>(GetCom(aMsgChan));
                        if (pChan)
                        {
                            int del=0, ldel=0;
                            do{
                                del = aMsgUsers.find_first_of(" ",del+1);
                                pChan->m_Users.push_back(aMsgUsers.substr(ldel, del-ldel));
                                ldel=del+1;
                            } while (del != std::string::npos);
                        }
                    }
                    else if (aMsgID.compare("366") == 0)
                    {
                        del = NetData.find_first_of(" ",del+1);
                        ldel = NetData.find_first_of(" ",del+1);
                        std::string aMsgChan = NetData.substr(del+1,ldel-del-1);

                        CComChan *pChan = static_cast<CComChan*>(GetCom(aMsgChan));
                        if (pChan)
                            pChan->m_Users.sort();
                    }
                    else if (aMsgID.compare("401") == 0)
                    {
                        char aBuff[255];
                        del = NetData.find_first_of(" ",del+1);
                        ldel = NetData.find_first_of(" ",del+1);
                        std::string aMsgFrom = NetData.substr(del+1,ldel-del-1);
                        del = NetData.find_first_of(":",1);
                        std::string aMsgText = NetData.substr(del+1, NetData.length()-del-1);

                        CIrcCom *pCom = GetCom(aMsgFrom);
                        if (!pCom)
                            pCom = GetCom("@Status");

                        str_format(aBuff, sizeof(aBuff), "*** '%s' %s", aMsgFrom.c_str(), aMsgText.c_str());
                        pCom->m_Buffer.push_back(aBuff);

                    }
                    else if (aMsgID.compare("421") == 0)
                    {
                        char aBuff[255];
                        del = NetData.find_first_of(" ",del+1);
                        ldel = NetData.find_first_of(" ",del+1);
                        std::string aMsgCmd = NetData.substr(del+1,ldel-del-1);
                        del = NetData.find_first_of(":",1);
                        std::string aMsgText = NetData.substr(del+1, NetData.length()-del-1);

                        CIrcCom *pCom = GetCom("@Status");
                        str_format(aBuff, sizeof(aBuff), "'%s' %s", aMsgCmd.c_str(), aMsgText.c_str());
                        pCom->m_Buffer.push_back(aBuff);

                    }
                    else if (aMsgID.compare("JOIN") == 0)
                    {
                        std::string aMsgChannel = NetData.substr(del+1, NetData.length()-del-1);
                        del = aMsgFServer.find_first_of("!");
                        std::string aMsgFrom = aMsgFServer.substr(0,del);

                        if (aMsgFrom == m_Nick)
                        {
                            CComChan *pNewChan = new CComChan();
                            pNewChan->m_UnreadMsg = false;
                            pNewChan->m_NumUnreadMsg = 0;
                            str_copy(pNewChan->m_Channel, aMsgChannel.c_str(), sizeof(pNewChan->m_Channel));
                            m_IrcComs.push_back(pNewChan);
                            SetActiveCom(m_IrcComs.size()-1);
                        }
                        else
                        {
                            CComChan *pChan = static_cast<CComChan*>(GetCom(aMsgChannel));
                            if (pChan)
                            {
                                pChan->m_Users.push_back(aMsgFrom);
                                pChan->m_Users.sort();
                                char aBuff[255];
                                str_format(aBuff, sizeof(aBuff), "*** '%s' has joined %s", aMsgFrom.c_str(), aMsgChannel.c_str());
                                pChan->m_Buffer.push_back(aBuff);
                            }
                        }
                    }
                    else if (aMsgID.compare("PART") == 0)
                    {
                        std::string aMsgChannel = NetData.substr(del+1, NetData.length()-del-1);
                        del = aMsgFServer.find_first_of("!");
                        std::string aMsgFrom = aMsgFServer.substr(0,del);
                        char aBuff[255];

                        if (aMsgFrom == m_Nick)
                        {
                            CIrcCom *pCom = GetCom(aMsgChannel);
                            if (pCom)
                            {
                                m_IrcComs.remove(pCom);
                                delete pCom;
                                pCom=0x0;
                                SetActiveCom(m_IrcComs.size()-1);
                            }
                        }
                        else
                        {
                            CComChan *pChan = static_cast<CComChan*>(GetCom(aMsgChannel));
                            if (pChan)
                            {
                                pChan->m_Users.remove(aMsgFrom);
                                str_format(aBuff, sizeof(aBuff), "@%s", aMsgFrom.c_str());
                                pChan->m_Users.remove(std::string(aBuff));
                                str_format(aBuff, sizeof(aBuff), "+%s", aMsgFrom.c_str());
                                pChan->m_Users.remove(std::string(aBuff));

                                str_format(aBuff, sizeof(aBuff), "*** '%s' part %s", aMsgFrom.c_str(), aMsgChannel.c_str());
                                pChan->m_Buffer.push_back(aBuff);
                            }
                        }
                    }
                    else if (aMsgID.compare("QUIT") == 0)
                    {
                        del = NetData.find_first_of(":",1);
                        std::string aMsgText = NetData.substr(del+1, NetData.length()-del-1);
                        del = aMsgFServer.find_first_of("!");
                        std::string aMsgFrom = aMsgFServer.substr(0,del);
                        char aBuff[255];

                        if (aMsgFrom != m_Nick)
                        {
                            std::list<CIrcCom*>::iterator it = m_IrcComs.begin();
                            while (it != m_IrcComs.end())
                            {
                                if (!(*it) || (*it)->GetType() != CIrcCom::TYPE_CHANNEL)
                                {
                                    ++it;
                                    continue;
                                }

                                CComChan *pChan = static_cast<CComChan*>((*it));
                                if (!pChan)
                                {
                                    ++it;
                                    continue;
                                }

                                pChan->m_Users.remove(aMsgFrom);
                                str_format(aBuff, sizeof(aBuff), "@%s", aMsgFrom.c_str());
                                pChan->m_Users.remove(std::string(aBuff));
                                str_format(aBuff, sizeof(aBuff), "+%s", aMsgFrom.c_str());
                                pChan->m_Users.remove(std::string(aBuff));

                                str_format(aBuff, sizeof(aBuff), "*** '%s' quit (%s)", aMsgFrom.c_str(), aMsgText.c_str());
                                pChan->m_Buffer.push_back(aBuff);

                                ++it;
                            }
                        }
                    }
                    else if (aMsgID.compare("TOPIC") == 0)
                    {
                        ldel = NetData.find_first_of(" ",del+1);
                        std::string aMsgChan = NetData.substr(del+1, ldel-del-1);
                        del = NetData.find_first_of(":",1);
                        std::string aMsgText = NetData.substr(del+1, NetData.length()-del-1);
                        del = aMsgFServer.find_first_of("!");
                        std::string aMsgFrom = aMsgFServer.substr(0,del);
                        char aBuff[1024];

                        CComChan *pChan = static_cast<CComChan*>(GetCom(aMsgChan));
                        if (pChan)
                        {
                            pChan->m_Topic = aMsgText;
                            str_format(aBuff, sizeof(aBuff), "*** '%s' has changed topic to '%s'", aMsgFrom.c_str(), aMsgText.c_str());
                            pChan->m_Buffer.push_back(aBuff);
                        }
                    }
                    else if (aMsgID.compare("PRIVMSG") == 0)
                    {
                        char aBuff[1024];
                        ldel = NetData.find_first_of(" ", del+1);
                        std::string aMsgChan = NetData.substr(del+1, ldel-del-1);

                        del = NetData.find_first_of(":", 1);
                        std::string aMsgText = NetData.substr(del+1, NetData.length()-del-1);

                        del = aMsgFServer.find_first_of("!");
                        std::string aMsgFrom = aMsgFServer.substr(0,del);

                        if (aMsgChan == m_Nick)
                        {
                            CIrcCom *pCom = GetCom(aMsgFrom);
                            if (!pCom)
                            {
                                CComQuery *pNewQuery = new CComQuery();
                                pNewQuery->m_UnreadMsg = true;
                                pNewQuery->m_NumUnreadMsg = 1;
                                str_copy(pNewQuery->m_User, aMsgFrom.c_str(), sizeof(pNewQuery->m_User));
                                m_IrcComs.push_back(pNewQuery);

                                str_format(aBuff, sizeof(aBuff), "<%s> %s", aMsgFrom.c_str(), aMsgText.c_str());
                                pNewQuery->m_Buffer.push_back(aBuff);
                            }
                            else
                            {
                                if (pCom != GetActiveCom())
                                {
                                    pCom->m_UnreadMsg = true;
                                    pCom->m_NumUnreadMsg++;
                                }
                                str_format(aBuff, sizeof(aBuff), "<%s> %s", aMsgFrom.c_str(), aMsgText.c_str());
                                pCom->m_Buffer.push_back(aBuff);
                            }

                            if (pCom == GetActiveCom())
                            {
                                aMsgFrom.insert(0,"<"); aMsgFrom.append("> ");
                                m_pGameClient->OnMessageIrc("", aMsgFrom.c_str(), aMsgText.c_str());
                            }
                        }
                        else
                        {
                            CIrcCom *pCom = GetCom(aMsgChan);
                            if (pCom)
                            {
                                if (pCom != GetActiveCom())
                                {
                                    pCom->m_UnreadMsg = true;
                                    pCom->m_NumUnreadMsg++;
                                }
                                str_format(aBuff, sizeof(aBuff), "<%s> %s", aMsgFrom.c_str(), aMsgText.c_str());
                                pCom->m_Buffer.push_back(aBuff);
                            }

                            if (pCom == GetActiveCom())
                            {
                                aMsgChan.insert(0,"["); aMsgChan.append("] ");
                                aMsgFrom.insert(0,"<"); aMsgFrom.append("> ");
                                m_pGameClient->OnMessageIrc(aMsgChan.c_str(), aMsgFrom.c_str(), aMsgText.c_str());
                            }
                        }
                    }
                    else if (aMsgID.compare("NICK") == 0)
                    {
                        char aBuff[255];
                        del = aMsgFServer.find_first_of("!");
                        std::string aMsgOldNick = aMsgFServer.substr(0,del);

                        del = NetData.find_first_of(":", 1);
                        std::string aMsgNewNick = NetData.substr(del+1, NetData.length()-del-1);

                        if (aMsgOldNick == m_Nick)
                            m_Nick = aMsgNewNick;

                        std::list<CIrcCom*>::iterator it = m_IrcComs.begin();
                        while (it != m_IrcComs.end())
                        {
                            if ((*it)->GetType() == CIrcCom::TYPE_QUERY)
                            {
                                CComQuery *pQuery = static_cast<CComQuery*>((*it));
                                if (str_comp_nocase(pQuery->m_User, aMsgOldNick.c_str()) == 0)
                                {
                                    str_format(aBuff, sizeof(aBuff), "*** '%s' has changed nick to '%s'", aMsgOldNick.c_str(), aMsgNewNick.c_str());
                                    pQuery->m_Buffer.push_back(aBuff);
                                }
                            }
                            else if ((*it)->GetType() == CIrcCom::TYPE_CHANNEL)
                            { //TODO: Rewrite this!! duplicate code :P
                                CComChan *pChan = static_cast<CComChan*>((*it));
                                std::list<std::string>::iterator itU = pChan->m_Users.begin();
                                while (itU != pChan->m_Users.end())
                                {
                                    std::string NickOper = aMsgOldNick; NickOper.insert(0, "@");
                                    std::string NickVoice = aMsgOldNick; NickVoice.insert(0, "+");

                                    if (str_comp_nocase((*itU).c_str(), aMsgOldNick.c_str()) == 0)
                                    {
                                        (*itU) = aMsgNewNick;
                                        str_format(aBuff, sizeof(aBuff), "*** '%s' has changed nick to '%s'", aMsgOldNick.c_str(), aMsgNewNick.c_str());
                                        pChan->m_Buffer.push_back(aBuff);
                                        pChan->m_Users.sort();
                                        break;
                                    }
                                    else if (str_comp_nocase((*itU).c_str(), NickOper.c_str()) == 0)
                                    {
                                        (*itU) = aMsgNewNick;
                                        (*itU).insert(0, "@");
                                        str_format(aBuff, sizeof(aBuff), "*** '%s' has changed nick to '%s'", aMsgOldNick.c_str(), aMsgNewNick.c_str());
                                        pChan->m_Buffer.push_back(aBuff);
                                        pChan->m_Users.sort();
                                        break;
                                    }
                                    else if (str_comp_nocase((*itU).c_str(), NickVoice.c_str()) == 0)
                                    {
                                        (*itU) = aMsgNewNick;
                                        (*itU).insert(0, "+");
                                        str_format(aBuff, sizeof(aBuff), "*** '%s' has changed nick to '%s'", aMsgOldNick.c_str(), aMsgNewNick.c_str());
                                        pChan->m_Buffer.push_back(aBuff);
                                        pChan->m_Users.sort();
                                        break;
                                    }

                                    ++itU;
                                }
                            }

                            ++it;
                        }
                    }
                    else if (aMsgID.compare("MODE") == 0)
                    {
                        char aBuff[255];
                        del = aMsgFServer.find_first_of("!");
                        std::string aNickFrom = aMsgFServer.substr(0,del);

                        del = NetData.find_first_of(" ");
                        ldel = NetData.find_first_of(" ", del+1);
                        del = NetData.find_first_of(" ", ldel+1);
                        std::string aChannel = NetData.substr(ldel+1, (del)-(ldel+1));

                        ldel = NetData.find_first_of(" ", del+1);
                        std::string aMode = NetData.substr(del+1, ldel-(del+1));

                        ldel = NetData.find_first_of(" ", del+1);
                        del = NetData.find_first_of(" ", ldel+1);
                        std::string aNickTo = NetData.substr(ldel+1, del-(ldel+1));


                        CIrcCom *pCom = GetCom(aChannel);
                        if (pCom && pCom->GetType() == CIrcCom::TYPE_CHANNEL)
                        {
                            CComChan *pChan = static_cast<CComChan*>(pCom);
                            if (pChan)
                            {
                                str_format(aBuff, sizeof(aBuff), "*** '%s' change mode '%s' to %s", aNickFrom.c_str(), aNickTo.c_str(), aMode.c_str());
                                pChan->m_Buffer.push_back(aBuff);

                                std::string aNewNick = aNickTo;
                                std::string aNickToVoz = aNickTo; aNickToVoz.insert(0, "+");

                                if (aMode[0] == '+')
                                {
                                    if (aMode[1] == 'o')
                                        aNewNick.insert(0, "@");
                                    else if (aMode[1] == 'v')
                                        aNewNick.insert(0, "+");
                                }
                                else if (aMode[0] == '-')
                                {
                                    if (aMode[1] == 'o')
                                        aNickTo.insert(0, "@");
                                }

                                std::list<std::string>::iterator it = pChan->m_Users.begin();
                                while (it != pChan->m_Users.end())
                                {
                                    if ((*it).compare(aNickTo) == 0 || (*it).compare(aNickToVoz) == 0)
                                    {
                                        (*it) = aNewNick;
                                        break;
                                    }
                                    ++it;
                                }

                                pChan->m_Users.sort();
                            }
                        }
                    }
                    else if (aMsgID.compare("KICK") == 0)
                    {
                        char aBuff[255];
                        del = aMsgFServer.find_first_of("!");
                        std::string aNickFrom = aMsgFServer.substr(0,del);

                        del = NetData.find_first_of(" ");
                        ldel = NetData.find_first_of(" ", del+1);
                        del = NetData.find_first_of(" ", ldel+1);
                        std::string aChannel = NetData.substr(ldel+1, (del)-(ldel+1));

                        ldel = NetData.find_first_of(" ", del+1);
                        std::string aNickTo = NetData.substr(del+1, ldel-(del+1));

                        ldel = NetData.find_first_of(":", 1);
                        std::string aKickReason = NetData.substr(ldel+1);


                        CIrcCom *pCom = GetCom(aChannel);
                        if (pCom && pCom->GetType() == CIrcCom::TYPE_CHANNEL)
                        {
                            if (aNickTo == m_Nick)
                            {
                                m_IrcComs.remove(pCom);
                                delete pCom;
                                pCom=0x0;
                                SetActiveCom(0);
                            }
                            else
                            {
                                CComChan *pChan = static_cast<CComChan*>(pCom);

                                str_format(aBuff, sizeof(aBuff), "*** '%s' kick '%s' (%s)", aNickFrom.c_str(), aNickTo.c_str(), aKickReason.c_str());
                                pChan->m_Buffer.push_back(aBuff);

                                pChan->m_Users.remove(aNickTo);
                                aNickTo.insert(0, "@");
                                pChan->m_Users.remove(aNickTo);
                                aNickTo[0]='+';
                                pChan->m_Users.remove(aNickTo);
                            }
                        }
                    }
                    else
                    {
                        char aBuff[1024];
                        ldel = NetData.find_first_of(" ", del+1);
                        del = ldel;
                        ldel = NetData.find_first_of(" ", del+1);
                        std::string aMsgData = NetData.substr(del+1, ldel-del-1);
                        del = NetData.find_first_of(":", 1);
                        std::string aMsgText = NetData.substr(del+1, NetData.length()-del-1);

                        if (ldel < del && ldel != std::string::npos)
                            str_format(aBuff, sizeof(aBuff), "%s %s", aMsgData.c_str(), aMsgText.c_str());
                        else
                            str_format(aBuff, sizeof(aBuff), "%s", aMsgText.c_str());

                        CIrcCom *pCom = GetCom("@Status");
                        pCom->m_Buffer.push_back(aBuff);
                    }
                }

                NetData.clear();
                continue;
            }

            NetData+=aNetBuff[i];
        }
    }

    //Finish
    net_tcp_close(m_Socket);
    m_State = STATE_DISCONNECTED;

    EndConnection();
}

void CIrc::NextRoom()
{
    if (m_ActiveCom >= m_IrcComs.size()-1)
        SetActiveCom((m_IrcComs.size()>1)?1:0);
    else if (m_ActiveCom <= 0)
        SetActiveCom(m_IrcComs.size()-1);
    else
        SetActiveCom(m_ActiveCom+1);
}

void CIrc::OpenQuery(const char *to)
{
    CComQuery *pQuery = new CComQuery();
    if (to[0] == '@' || to[0] == '+')
        str_copy(pQuery->m_User, to+1, sizeof(pQuery->m_User));
    else
        str_copy(pQuery->m_User, to, sizeof(pQuery->m_User));
    m_IrcComs.push_back(pQuery);
    SetActiveCom(m_IrcComs.size()-1);
}

void CIrc::JoinTo(const char *to)
{
    char aBuff[255];
    str_format(aBuff, sizeof(aBuff), "JOIN %s\r\n", to);
    net_tcp_send(m_Socket, aBuff, strlen(aBuff));
}

void CIrc::SetMode(const char *mode, const char *to)
{
    char aBuff[255];

    CIrcCom *pCom = GetActiveCom();
    if (!pCom || pCom->GetType() == CIrcCom::TYPE_QUERY)
        return;

    CComChan *pChan = static_cast<CComChan*>(pCom);
    if (!pChan)
        return;

    if (!to || to[0] == 0)
        str_format(aBuff, sizeof(aBuff), "MODE %s %s %s\r\n", pChan->m_Channel, mode, m_Nick.c_str());
    else
       str_format(aBuff, sizeof(aBuff), "MODE %s %s %s\r\n", pChan->m_Channel, mode, to);

    net_tcp_send(m_Socket, aBuff, strlen(aBuff));
}

void CIrc::SetTopic(const char *topic)
{
    CIrcCom *pCom = GetActiveCom();
    if (!pCom || pCom->GetType() != CIrcCom::TYPE_CHANNEL)
        return;

    CComChan *pChan = static_cast<CComChan*>(pCom);
    char aBuff[1024];
    str_format(aBuff, sizeof(aBuff), "TOPIC %s :%s\r\n", pChan->m_Channel, topic);
    net_tcp_send(m_Socket, aBuff, strlen(aBuff));
}

void CIrc::Part()
{
    CIrcCom *pCom = GetCom(m_ActiveCom);
    if (!pCom)
        return;

    if (pCom->GetType() == CIrcCom::TYPE_CHANNEL)
    {
        CComChan *pChan = static_cast<CComChan*>(pCom);
        char aBuff[512];
        str_format(aBuff, sizeof(aBuff), "PART %s\r\n", pChan->m_Channel);
        net_tcp_send(m_Socket, aBuff, strlen(aBuff));

        m_IrcComs.remove(pCom);
        delete pCom;
        pCom=0x0;
        SetActiveCom(m_IrcComs.size()-1);
    }
    else if (pCom->GetType() == CIrcCom::TYPE_QUERY)
    {
        CComQuery *pQuery = static_cast<CComQuery*>(pCom);
        if (str_comp_nocase(pQuery->m_User, "@Status") == 0)
            return;

        m_IrcComs.remove(pCom);
        delete pCom;
        pCom=0x0;
        SetActiveCom(m_IrcComs.size()-1);
    }
}

void CIrc::EndConnection()
{
    if (m_State != STATE_DISCONNECTED)
    {
        char aBuff[20];
        str_format(aBuff, sizeof(aBuff), "QUIT :H-Client\r\n");
        net_tcp_send(m_Socket, aBuff, strlen(aBuff));
        m_State = STATE_DISCONNECTED;
    }

    std::list<CIrcCom*>::iterator it = m_IrcComs.begin();
    while (it != m_IrcComs.end())
    {
        delete (*it);
        it = m_IrcComs.erase(it);
    }
}

void CIrc::SendMsg(const char *to, const char *msg)
{ //TODO: Rework this! duplicate code :P
    if (GetState() == STATE_DISCONNECTED || !msg || msg[0] == 0)
        return;

    char aBuff[1024];

    if (!to || to[0] == 0)
    {
        if (m_ActiveCom == -1)
            return;

        std::list<CIrcCom*>::iterator it = m_IrcComs.begin();
        std::advance(it, m_ActiveCom);
        if ((*it)->GetType() == CIrcCom::TYPE_CHANNEL)
        {
            CComChan *pChan = static_cast<CComChan*>((*it));
            str_format(aBuff, sizeof(aBuff), "PRIVMSG %s :%s\r\n", pChan->m_Channel, msg);
            net_tcp_send(m_Socket, aBuff, strlen(aBuff));

            str_format(aBuff, sizeof(aBuff),"<%s> %s", GetNick(), msg);
            pChan->m_Buffer.push_back(aBuff);
        }
        else if ((*it)->GetType() == CIrcCom::TYPE_QUERY)
        {
            CComQuery *pQuery = static_cast<CComQuery*>((*it));
            if (str_comp_nocase(pQuery->m_User, "@Status") == 0)
            {
                str_format(aBuff, sizeof(aBuff),"** You can't send messages to '@Status'!", GetNick(), msg);
                pQuery->m_Buffer.push_back(aBuff);
                return;
            }

            str_format(aBuff, sizeof(aBuff), "PRIVMSG %s :%s\r\n", pQuery->m_User, msg);
            net_tcp_send(m_Socket, aBuff, strlen(aBuff));

            str_format(aBuff, sizeof(aBuff),"<%s> %s", GetNick(), msg);
            pQuery->m_Buffer.push_back(aBuff);
        }
        else
            return;
    }
    else
    {
        str_format(aBuff, sizeof(aBuff), "PRIVMSG %s :%s\r\n", to, msg);
        net_tcp_send(m_Socket, aBuff, strlen(aBuff));

        CIrcCom *pCom = GetCom(to);
        if (pCom)
        {
            str_format(aBuff, sizeof(aBuff),"<%s> %s", GetNick(), msg);
            pCom->m_Buffer.push_back(aBuff);
        }
    }
}

void CIrc::SendRaw(const char *rawmsg)
{
    if (!rawmsg || rawmsg[0] == 0)
        return;

    char aBuff[1024];
    str_format(aBuff, sizeof(aBuff), "%s\r\n", rawmsg+1);
    net_tcp_send(m_Socket, aBuff, strlen(aBuff));
}

void CIrc::SetNick(const char *nick)
{
    if (m_State == STATE_CONNECTED)
    {
        char aBuff[50];
        str_format(aBuff, sizeof(aBuff), "NICK %s\r\n", nick);
        net_tcp_send(m_Socket, aBuff, strlen(aBuff));
    }

    m_Nick = nick;
}

void ThreadIrcConnection(void *params)
{
    CIrc *pIrc = static_cast<CIrc*>(params);

    pIrc->StartConnection();
}
