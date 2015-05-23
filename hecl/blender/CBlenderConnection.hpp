#ifndef CBLENDERCONNECTION_HPP
#define CBLENDERCONNECTION_HPP

#include <unistd.h>

class CBlenderConnection
{
    pid_t m_blenderProc;
public:
    CBlenderConnection();
    ~CBlenderConnection();
};

#endif // CBLENDERCONNECTION_HPP
