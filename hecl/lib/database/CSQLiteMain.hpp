#ifndef CSQLITEMAIN_HPP
#define CSQLITEMAIN_HPP

#include <sqlite3.h>
#include <stdexcept>
#include <functional>

#include "HECLDatabase.hpp"
#include "sqlite_hecl_vfs.h"

namespace HECLDatabase
{

static const char* skMAINDBINIT =
"PRAGMA foreign_keys = ON;\n"
"CREATE TABLE IF NOT EXISTS grps("
    "grpid INTEGER PRIMARY KEY," /* Unique group identifier (used as in-game ref) */
    "path);\n" /* Directory path collecting working files for group */
"CREATE TABLE IF NOT EXISTS objs("
    "objid INTEGER PRIMARY KEY," /* Unique object identifier (used as in-game ref) */
    "path," /* Path of working file */
    "subpath DEFAULT NULL," /* String name of sub-object within working file (i.e. blender object) */
    "cookedHash64 INTEGER DEFAULT NULL," /* Hash of last cooking pass */
    "cookedTime64 INTEGER DEFAULT NULL);\n"; /* UTC unix-time of last cooking pass */

#define PREPSTMT(stmtSrc, outVar)\
if (sqlite3_prepare_v2(m_db, stmtSrc, 0, &outVar, NULL) != SQLITE_OK)\
{\
    throw std::runtime_error(sqlite3_errmsg(m_db));\
    sqlite3_close(m_db);\
    return;\
}

class CSQLiteMain
{
    sqlite3* m_db;

    struct SCloseBuf
    {
        void* buf = NULL;
        size_t sz = 0;
    };
    static void _vfsClose(void* buf, size_t bufSz, SCloseBuf* ctx)
    {
        ctx->buf = buf;
        ctx->sz = bufSz;
    }

public:
    CSQLiteMain(const std::string& path)
    {
        /* Open database connection */
        int errCode = 0;
        if ((errCode = sqlite3_open(path.c_str(), &m_db) != SQLITE_OK))
        {
            throw std::runtime_error(sqlite3_errstr(errCode));
            sqlite3_close(m_db);
            return;
        }

        /* Execute bootstrap statements */
        char* errMsg = NULL;
        sqlite3_exec(m_db, skMAINDBINIT, NULL, NULL, &errMsg);
        if (errMsg)
        {
            throw std::runtime_error(errMsg);
            sqlite3_free(errMsg);
            sqlite3_close(m_db);
            return;
        }

        /* Precompile statements */

    }

    ~CSQLiteMain()
    {
        sqlite3_close(m_db);
    }


    void* fillDBBuffer(size_t& bufSzOut) const
    {
        /* Instructs vfs that a close operation is premature and buffer should be freed */
        sqlite_hecl_mem_vfs_register(NULL, NULL);

        /* Open pure-memory DB */
        sqlite3* memDb;
        int errCode;
        if ((errCode = sqlite3_open_v2("", &memDb, SQLITE_OPEN_READWRITE, "hecl_mem")) != SQLITE_OK)
        {
            throw std::runtime_error(sqlite3_errstr(errCode));
            sqlite3_close(memDb);
            return NULL;
        }

        /* Perform backup (row copy) */
        sqlite3_backup* backup = sqlite3_backup_init(memDb, "main", m_db, "main");
        if (!backup)
        {
            throw std::runtime_error(sqlite3_errmsg(memDb));
            sqlite3_close(memDb);
            return NULL;
        }
        sqlite3_backup_step(backup, -1);
        sqlite3_backup_finish(backup);

        /* Now a close operation is useful; register close callback */
        SCloseBuf closeBuf;
        sqlite_hecl_mem_vfs_register((TCloseCallback)_vfsClose, &closeBuf);
        sqlite3_close(memDb);

        /* This should be set by close callback */
        if (!closeBuf.buf)
        {
            throw std::runtime_error("close operation did not write buffer");
            return NULL;
        }

        /* All good! */
        bufSzOut = closeBuf.sz;
        return closeBuf.buf;
    }

    static void freeDBBuffer(void* buf)
    {
        sqlite3_free(buf);
    }

};

}

#endif // CSQLITEMAIN_HPP
