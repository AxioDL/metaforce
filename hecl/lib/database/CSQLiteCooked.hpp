#ifndef CSQLITECOOKED_HPP
#define CSQLITECOOKED_HPP

#include <sqlite3.h>
#include <stdexcept>
#include <functional>

#include "HECLDatabase.hpp"
#include "sqlite_hecl_vfs.h"

namespace HECLDatabase
{

static const char* skCOOKEDDBINIT =
"PRAGMA foreign_keys = ON;\n"
"CREATE TABLE IF NOT EXISTS cgrps("
    "grpid INTEGER PRIMARY KEY," /* Unique group identifier (from main DB) */
    "offset UNSIGNED INTEGER," /* Group-blob offset within package */
    "compLen UNSIGNED INTEGER," /* Compressed blob-length */
    "decompLen UNSIGNED INTEGER);\n" /* Decompressed blob-length */
"CREATE TABLE IF NOT EXISTS cobjs("
    "objid INTEGER PRIMARY KEY," /* Unique object identifier (from main DB) */
    "type4cc UNSIGNED INTEGER," /* Type FourCC as claimed by first project class in dataspec */
    "loosegrp REFERENCES cgrps(grpid) ON DELETE SET NULL DEFAULT NULL);\n" /* single-object group of ungrouped object */
"CREATE TABLE IF NOT EXISTS cgrplinks("
    "grpid REFERENCES cgrps(grpid) ON DELETE CASCADE," /* Group ref */
    "objid REFERENCES cobjs(objid) ON DELETE CASCADE," /* Object ref */
    "offset UNSIGNED INTEGER," /* Offset within decompressed group-blob */
    "decompLen UNSIGNED INTEGER," /* Decompressed object length */
    "UNIQUE (grpid, objid) ON CONFLICT IGNORE);\n"
"CREATE INDEX IF NOT EXISTS grpidx ON cgrplinks(grpid);\n";

#define PREPSTMT(stmtSrc, outVar)\
if (sqlite3_prepare_v2(m_db, stmtSrc, 0, &outVar, NULL) != SQLITE_OK)\
{\
    throw std::runtime_error(sqlite3_errmsg(m_db));\
    sqlite3_close(m_db);\
    return;\
}

class CSQLiteCooked
{
    sqlite3* m_db;

public:
    CSQLiteCooked(const char* path, bool readonly)
    {
        /* Open database connection */
        int errCode = 0;
        if ((errCode = sqlite3_open_v2(path, &m_db, SQLITE_OPEN_READONLY,
                                       "hecl_memlba")) != SQLITE_OK)
        {
            throw std::runtime_error(sqlite3_errstr(errCode));
            sqlite3_close(m_db);
            return;
        }

        /* Execute bootstrap statements */
        char* errMsg = NULL;
        sqlite3_exec(m_db, skCOOKEDDBINIT, NULL, NULL, &errMsg);
        if (errMsg)
        {
            throw std::runtime_error(errMsg);
            sqlite3_free(errMsg);
            sqlite3_close(m_db);
            return;
        }

        /* Precompile statements */

    }

    ~CSQLiteCooked()
    {
        sqlite3_close(m_db);
    }

};

}

#endif // CSQLITE_HPP
