#include "CUVElement.hpp"

namespace pshag
{

CUVEAnimTexture::CUVEAnimTexture(TToken<CTexture>&& tex, CIntElement* a, CIntElement* b,
                                 CIntElement* c, CIntElement* d, CIntElement* e, bool f)
: x4_tex(std::move(tex)), x24_loop(f), x28_cycleFrameRate(e)
{
    a->GetValue(0, x10_tileW);
    delete a;
    b->GetValue(0, x14_tileH);
    delete b;
    c->GetValue(0, x18_strideW);
    delete c;
    d->GetValue(0, x1c_strideH);
    delete d;

    int width = x4_tex.GetObj()->GetWidth();
    int height = x4_tex.GetObj()->GetHeight();
    float widthF = width;
    float heightF = height;
    int xTiles = std::max(1, width / x18_strideW);
    int yTiles = std::max(1, height / x1c_strideH);
    x20_tiles = xTiles * yTiles;
    x2c_uvElems.reserve(x20_tiles);
    for (int y=yTiles-1 ; y>=0 ; --y)
    {
        for (int x=0 ; x<xTiles ; ++x)
        {
            int px = x18_strideW * x;
            int px2 = px + x10_tileW;
            int py = x1c_strideH * y;
            int py2 = py + x14_tileH;
            x2c_uvElems.push_back({px / widthF, py / heightF, px2 / widthF, py2 / heightF});
        }
    }
}

void CUVEAnimTexture::GetValueUV(int frame, SUVElementSet& valOut) const
{
    int cv;
    x28_cycleFrameRate->GetValue(frame, cv);
    float cvf = cv / float(x20_tiles);
    cvf = frame / cvf;

    int tile = cvf;
    if (x24_loop)
    {
        if (cvf >= x20_tiles)
            tile = int(cvf) % x20_tiles;
    }
    else
    {
        if (cvf >= x20_tiles)
            tile = x20_tiles - 1;
    }

    valOut = x2c_uvElems[tile];
}

}
