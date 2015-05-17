#ifndef CSQLITE_HPP
#define CSQLITE_HPP

#include <sqlite3.h>
#include <stdexcept>
#include <functional>

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

public:
    CSQLite(const char* path, bool readonly)
    {
        /* Open database connection */
        if (sqlite3_open_v2(path, &m_db, readonly ?
                            SQLITE_OPEN_READONLY :
                            SQLITE_OPEN_READWRITE |
                            SQLITE_OPEN_CREATE, NULL) != SQLITE_OK)
        {
            throw std::runtime_error(sqlite3_errmsg(m_db));
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
    }

    ~CSQLite()
    {
        sqlite3_finalize(m_selObjects);
        sqlite3_finalize(m_selObjectByName);
        sqlite3_finalize(m_selDistictDepGroups);
        sqlite3_finalize(m_selDepGroupObjects);
        sqlite3_close(m_db);
    }

    void buildMemoryIndex(const std::function<void(std::size_t&&,       // id
                                                   const std::string&&, // name
                                                   uint32_t&&,          // type4cc
                                                   uint64_t&&,          // hash64
                                                   std::size_t&&,       // compLen
                                                   std::size_t&&)>&     // decompLen
                                                   objectAdder)
    {
        while (sqlite3_step(m_selObjects) == SQLITE_ROW)
        {
            objectAdder(sqlite3_column_int64(m_selObjects, 0),
                        (const char*)sqlite3_column_text(m_selObjects, 1),
                        sqlite3_column_int(m_selObjects, 2),
                        sqlite3_column_int64(m_selObjects, 3),
                        sqlite3_column_int64(m_selObjects, 4),
                        sqlite3_column_int64(m_selObjects, 5));
        }
        sqlite3_reset(m_selObjects);
    }

    std::size_t objectIdFromName(const std::string& name)
    {
        sqlite3_bind_text(m_selObjectByName, 1, name.c_str(), name.length(), NULL);
        std::size_t retval = 0;
        if (sqlite3_step(m_selObjectByName) == SQLITE_ROW)
            retval = sqlite3_column_int64(m_selObjectByName, 0);
        sqlite3_reset(m_selObjectByName);
        return retval;
    }

};

}

#endif // CSQLITE_HPP
