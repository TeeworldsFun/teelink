/*
    unsigned char*
*/
#ifndef ENGINE_IRC_H
#define ENGINE_IRC_H

#include "kernel.h"
#include <vector>
#include <string>
#include <list>

class CIrcCom {
public:
    enum {
        TYPE_CHANNEL=0,
        TYPE_QUERY
    };

    CIrcCom(unsigned int type) { m_Type = type; }
    std::vector<std::string> m_Buffer;
    bool m_UnreadMsg;
    int m_NumUnreadMsg;

    unsigned int GetType() const { return m_Type; }
protected:
    unsigned int m_Type;
};
class CComChan : public CIrcCom
{
public:
    CComChan(): CIrcCom(CIrcCom::TYPE_CHANNEL) {}

    std::list<std::string> m_Users;
    std::string m_Topic;
    char m_Channel[25];
};
class CComQuery : public CIrcCom
{
public:
    CComQuery(): CIrcCom(CIrcCom::TYPE_QUERY) {}

    char m_User[25];
};

class IIrc : public IInterface
{
	MACRO_INTERFACE("irc", 0)
public:
    enum
    {
        STATE_DISCONNECTED=0,
        STATE_CONNECTED,
        STATE_CONNECTING,
        STATE_AWAY
    };


    virtual void Init() = 0;

    virtual int GetState() = 0;
    virtual void NextRoom() = 0;

    virtual void SetActiveCom(size_t index) = 0;
    virtual CIrcCom* GetActiveCom() = 0;
    virtual CIrcCom* GetCom(size_t index) = 0;
    virtual CIrcCom* GetCom(std::string name) = 0;
    virtual int GetNumComs() = 0;

    virtual void OpenQuery(const char *to) = 0;
    virtual void JoinTo(const char *to) = 0;
    virtual void SetTopic(const char *topic) = 0;
    virtual void Part() = 0;

    virtual void SetMode(const char *mode, const char *to) = 0;
    virtual void SetNick(const char *nick) = 0;
    virtual const char* GetNick() = 0;

    virtual void SendMsg(const char *to, const char *msg) = 0;
    virtual void SendRaw(const char *rawmsg) = 0;

    virtual void StartConnection() = 0;
    virtual void EndConnection() = 0;

};

void ThreadIrcConnection(void *params);
#endif
