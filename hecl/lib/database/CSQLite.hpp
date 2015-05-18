#ifndef CSQLITE_HPP
#define CSQLITE_HPP

#include <sqlite3.h>
#include <stdexcept>
#include <functional>

#include "HECLDatabase.hpp"
#include "sqlite_hecl_vfs.h"

namespace HECLDatabase
{

/* Private sqlite3 backend to be used by database subclasses */

static const char* skDBINIT =
"PRAGMA foreign_keys = ON;\n"
"CREATE TABLE IF NOT EXISTS objects(rowid INTEGER PRIMARY KEY,"
                                   "name UNIQUE,"
                                   "type4cc UNSIGNED INTEGER,"
                                   "hash64 UNSIGNED INTEGER,"
                                   "compLen UNSIGNED INTEGER,"
                                   "decompLen UNSIGNED INTEGER);\n"
"CREATE INDEX IF NOT EXISTS nameidx ON objects(name);\n"
"CREATE TABLE IF NOT EXISTS deplinks(groupId, "
                                    "objId REFERENCES objects(rowid) ON DELETE CASCADE, "
                                    "UNIQUE (groupId, objId) ON CONFLICT IGNORE);\n"
"CREATE INDEX IF NOT EXISTS grpidx ON deplinks(groupId);\n"
"CREATE INDEX IF NOT EXISTS depidx ON deplinks(objId);\n";

#define PREPSTMT(stmtSrc, outVar)\
if (sqlite3_prepare_v2(m_db, stmtSrc, 0, &outVar, NULL) != SQLITE_OK)\
{\
    throw std::runtime_error(sqlite3_errmsg(m_db));\
    sqlite3_close(m_db);\
    return;\
}

class CSQLite
{
    sqlite3* m_db;

    sqlite3_stmt* m_selObjects;
    sqlite3_stmt* m_selObjectByName;
    sqlite3_stmt* m_selDistictDepGroups;
    sqlite3_stmt* m_selDepGroupObjects;
    sqlite3_stmt* m_insObject;

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
    CSQLite(const char* path, bool readonly)
    {
        /* Open database connection */
        int errCode = 0;
        if ((errCode = sqlite3_open_v2(path, &m_db, readonly ?
                                       SQLITE_OPEN_READONLY :
                                       SQLITE_OPEN_READWRITE |
                                       SQLITE_OPEN_CREATE, NULL)) != SQLITE_OK)
        {
            throw std::runtime_error(sqlite3_errstr(errCode));
            sqlite3_close(m_db);
            return;
        }

        /* Execute bootstrap statements */
        char* errMsg = NULL;
        sqlite3_exec(m_db, skDBINIT, NULL, NULL, &errMsg);
        if (errMsg)
        {
            throw std::runtime_error(errMsg);
            sqlite3_free(errMsg);
            sqlite3_close(m_db);
            return;
        }

        /* Precompile statements */
        PREPSTMT("SELECT rowid,name,type4cc,hash64,compLen,decompLen FROM objects", m_selObjects);
        PREPSTMT("SELECT rowid FROM objects WHERE name=?1", m_selObjectByName);
        PREPSTMT("SELECT DISTINCT groupId FROM deplinks", m_selDistictDepGroups);
        PREPSTMT("SELECT DISTINCT objId FROM deplinks WHERE groupId=?1", m_selDepGroupObjects);
        PREPSTMT("INSERT INTO objects(name,type4cc,hash64,compLen,decompLen) VALUES (?1,?2,?3,?4,?5)", m_insObject);
    }

    ~CSQLite()
    {
        sqlite3_finalize(m_selObjects);
        sqlite3_finalize(m_selObjectByName);
        sqlite3_finalize(m_selDistictDepGroups);
        sqlite3_finalize(m_selDepGroupObjects);
        sqlite3_finalize(m_insObject);
        sqlite3_close(m_db);
    }

    void buildMemoryIndex(const std::function<void(size_t&&,       // id
                                                   const std::string&&, // name
                                                   uint32_t&&,          // type4cc
                                                   uint64_t&&,          // hash64
                                                   size_t&&,       // compLen
                                                   size_t&&)>&     // decompLen
                                                   objectAdder)
    {
        while (sqlite3_step(m_selObjects) == SQLITE_ROW)
        {
            /* <3 Move Lambdas!! */
            objectAdder(sqlite3_column_int64(m_selObjects, 0),
                        (const char*)sqlite3_column_text(m_selObjects, 1),
                        sqlite3_column_int(m_selObjects, 2),
                        sqlite3_column_int64(m_selObjects, 3),
                        sqlite3_column_int64(m_selObjects, 4),
                        sqlite3_column_int64(m_selObjects, 5));
        }
        sqlite3_reset(m_selObjects);
    }

    size_t objectIdFromName(const std::string& name)
    {
        sqlite3_bind_text(m_selObjectByName, 1, name.c_str(), name.length(), NULL);
        size_t retval = 0;
        if (sqlite3_step(m_selObjectByName) == SQLITE_ROW)
            retval = sqlite3_column_int64(m_selObjectByName, 0);
        sqlite3_reset(m_selObjectByName);
        return retval;
    }

    bool insertObject(const std::string& name, const FourCC& type, const ObjectHash& hash, size_t compLen, size_t decompLen)
    {
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

#endif // CSQLITE_HPP
