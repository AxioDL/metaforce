#include "SpecBase.hpp"

static HECL::Database::DataSpecEntry SpecMP3
(
    _S("MP3"),
    _S("Data specification for original Metroid Prime 3 engine"),
    [](HECL::Database::DataSpecTool tool) -> HECL::Database::IDataSpec* {return nullptr;}
);

