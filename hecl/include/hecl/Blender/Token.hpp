#ifndef HECL_BLENDERTOKEN_HPP
#define HECL_BLENDERTOKEN_HPP

#include <memory>

namespace hecl::blender
{
class Connection;

class Token
{
    std::unique_ptr<Connection> m_conn;
public:
    Connection& getBlenderConnection();
    void shutdown();

    Token() = default;
    ~Token();
    Token(const Token&)=delete;
    Token& operator=(const Token&)=delete;
    Token(Token&&)=default;
    Token& operator=(Token&&)=default;
};

}

#endif // HECL_BLENDERTOKEN_HPP
