#include "CGuiTextSupport.hpp"
#include "CSimplePool.hpp"

namespace urde
{

CGuiTextSupport::CGuiTextSupport(TResId fontId, const CGuiTextProperties& props,
                                 const zeus::CColor& col1, const zeus::CColor& col2,
                                 const zeus::CColor& col3, int, int, CSimplePool* store)
{
    store->GetObj({SBIG('FONT'), fontId});
}

}
