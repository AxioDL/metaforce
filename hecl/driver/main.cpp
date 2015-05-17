#include <stdio.h>
#include <HECLDatabase.hpp>

#define TESTDATA "TESTING 123"

int main(int argc, char** argv)
{
    printf("Hello World!\n");
    HECLDatabase::IDatabase* db = HECLDatabase::NewDatabase(HECLDatabase::IDatabase::LOOSE,
                                                            HECLDatabase::IDatabase::READWRITE,
                                                            "test.db");
    db->addDataBlob("TestObj", (void*)TESTDATA, sizeof(TESTDATA));
    delete db;
    return 0;
}
