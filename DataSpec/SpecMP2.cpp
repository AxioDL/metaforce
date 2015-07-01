#include "SpecBase.hpp"

static HECL::Database::DataSpecEntry SpecMP2
(
    _S("MP2"),
    _S("Data specification for original Metroid Prime 2 engine"),
    [](HECL::Database::DataSpecTool tool) -> HECL::Database::IDataSpec* {}
);
