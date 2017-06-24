#include "CollisionUtil.hpp"
#include "CCollisionInfo.hpp"
#include "CCollisionInfoList.hpp"

namespace urde
{
namespace CollisionUtil
{

bool LineIntersectsOBBox(const zeus::COBBox& obb, const zeus::CMRay& ray, float& d)
{
    const zeus::CVector3f transXf = obb.transform.toMatrix4f().vec[0].toVec3f();
    return RayAABoxIntersection(ray.getInvUnscaledTransformRay(obb.transform), {-obb.extents, obb.extents},
                                transXf, d);
}

u32 RayAABoxIntersection(const zeus::CMRay& ray, const zeus::CAABox& box, const zeus::CVector3f&, float& d)
{

    return 0;
}

u32 RaySphereIntersection_Double(const zeus::CSphere&, const zeus::CVector3f &, const zeus::CVector3f &, double &)
{
    return 0;
}

bool RaySphereIntersection(const zeus::CSphere& sphere, const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                           float mag, float& T, zeus::CVector3f& point)
{
    zeus::CVector3f rayToSphere = sphere.position - pos;
    float magSq = rayToSphere.magSquared();
    float dirDot = rayToSphere.dot(dir);
    float radSq = sphere.radius * sphere.radius;
    if (dirDot < 0.f && magSq > radSq)
        return false;
    float intersectSq = radSq - (magSq - dirDot * dirDot);
    if (intersectSq < 0.f)
        return false;
    T = magSq > radSq ? dirDot - std::sqrt(intersectSq) : dirDot + std::sqrt(intersectSq);
    if (T < mag || mag == 0.f)
    {
        point = pos + T * dir;
        return true;
    }
    return false;
}

void FilterOutBackfaces(const zeus::CVector3f& vec, const CCollisionInfoList& in, CCollisionInfoList& out)
{
    if (vec.canBeNormalized())
    {
        zeus::CVector3f norm = vec.normalized();
        for (const CCollisionInfo& info : in)
        {
            if (info.GetNormalLeft().dot(norm) < 0.001f)
                out.Add(info, false);
        }
    }
    else
    {
        out = in;
    }
}

void FilterByClosestNormal(const zeus::CVector3f& norm, const CCollisionInfoList& in, CCollisionInfoList& out)
{
    float maxDot = -1.1f;
    int idx = -1;
    int i=0;
    for (const CCollisionInfo& info : in)
    {
        float dot = info.GetNormalLeft().dot(norm);
        if (dot > maxDot)
        {
            maxDot = dot;
            idx = i;
        }
        ++i;
    }

    if (idx != -1)
        out.Add(in.GetItem(i), false);
}

static const zeus::CVector3f AABBNormalTable[] =
{
    {-1.f, 0.f, 0.f},
    {1.f, 0.f, 0.f},
    {0.f, -1.f, 0.f},
    {0.f, 1.f, 0.f},
    {0.f, 0.f, -1.f},
    {0.f, 0.f, 1.f}
};

bool AABoxAABoxIntersection(const zeus::CAABox& aabb0, const CMaterialList& list0,
                            const zeus::CAABox& aabb1, const CMaterialList& list1,
                            CCollisionInfoList& infoList)
{
    zeus::CAABox boolAABB = aabb0.booleanIntersection(aabb1);
    if (boolAABB.invalid())
        return false;

    /* TODO: Finish */

    if (!infoList.GetCount())
    {
        infoList.Add(CCollisionInfo(boolAABB, list0, list1, AABBNormalTable[4], -AABBNormalTable[4]), false);
        infoList.Add(CCollisionInfo(boolAABB, list0, list1, AABBNormalTable[5], -AABBNormalTable[5]), false);
    }

    return true;
}

bool AABoxAABoxIntersection(const zeus::CAABox& aabb0, const zeus::CAABox& aabb1)
{
    return aabb0.intersects(aabb1);
}

/* http://fileadmin.cs.lth.se/cs/Personal/Tomas_Akenine-Moller/code/tribox2.txt */
/********************************************************/
/* AABB-triangle overlap test code                      */
/* by Tomas Akenine-Möller                              */
/* Function: int triBoxOverlap(float boxcenter[3],      */
/*          float boxhalfsize[3],float triverts[3][3]); */
/* History:                                             */
/*   2001-03-05: released the code in its first version */
/*   2001-06-18: changed the order of the tests, faster */
/*                                                      */
/* Acknowledgement: Many thanks to Pierre Terdiman for  */
/* suggestions and discussions on how to optimize code. */
/* Thanks to David Hunt for finding a ">="-bug!         */
/********************************************************/

#define FINDMINMAX(x0,x1,x2,min,max) \
    min = max = x0;   \
    if (x1<min) min = x1;\
    if (x1>max) max = x1;\
    if (x2<min) min = x2;\
    if (x2>max) max = x2;

static bool planeBoxOverlap(const zeus::CVector3f& normal, float d, const zeus::CVector3f& maxbox)
{
    zeus::CVector3f vmin, vmax;
    for (int q=0 ; q<=2 ; q++)
    {
        if (normal[q] > 0.0f)
        {
            vmin[q] = -maxbox[q];
            vmax[q] = maxbox[q];
        }
        else
        {
            vmin[q] = maxbox[q];
            vmax[q] = -maxbox[q];
        }
    }
    if (normal.dot(vmin) + d > 0.0f) return false;
    if (normal.dot(vmax) + d >= 0.0f) return true;

    return false;
}


/*======================== X-tests ========================*/
#define AXISTEST_X01(a, b, fa, fb)             \
    p0 = a*v0.y - b*v0.z;                    \
    p2 = a*v2.y - b*v2.z;                    \
    if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
    rad = fa * boxhalfsize.y + fb * boxhalfsize.z;   \
    if(min>rad || max<-rad) return false;

#define AXISTEST_X2(a, b, fa, fb)              \
    p0 = a*v0.y - b*v0.z;                    \
    p1 = a*v1.y - b*v1.z;                    \
    if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
    rad = fa * boxhalfsize.y + fb * boxhalfsize.z;   \
    if(min>rad || max<-rad) return false;

/*======================== Y-tests ========================*/
#define AXISTEST_Y02(a, b, fa, fb)             \
    p0 = -a*v0.x + b*v0.z;                   \
    p2 = -a*v2.x + b*v2.z;                       \
    if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
    rad = fa * boxhalfsize.x + fb * boxhalfsize.z;   \
    if(min>rad || max<-rad) return false;

#define AXISTEST_Y1(a, b, fa, fb)              \
    p0 = -a*v0.x + b*v0.z;                   \
    p1 = -a*v1.x + b*v1.z;                       \
    if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
    rad = fa * boxhalfsize.x + fb * boxhalfsize.z;   \
    if(min>rad || max<-rad) return false;

/*======================== Z-tests ========================*/

#define AXISTEST_Z12(a, b, fa, fb)             \
    p1 = a*v1.x - b*v1.y;                    \
    p2 = a*v2.x - b*v2.y;                    \
    if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;} \
    rad = fa * boxhalfsize.x + fb * boxhalfsize.y;   \
    if(min>rad || max<-rad) return false;

#define AXISTEST_Z0(a, b, fa, fb)              \
    p0 = a*v0.x - b*v0.y;                \
    p1 = a*v1.x - b*v1.y;                    \
    if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
    rad = fa * boxhalfsize.x + fb * boxhalfsize.y;   \
    if(min>rad || max<-rad) return false;

bool TriBoxOverlap(const zeus::CVector3f& boxcenter, const zeus::CVector3f& boxhalfsize,
                   const zeus::CVector3f& trivert0, const zeus::CVector3f& trivert1,
                   const zeus::CVector3f& trivert2)
{

    /*    use separating axis theorem to test overlap between triangle and box */
    /*    need to test for overlap in these directions: */
    /*    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle */
    /*       we do not even need to test these) */
    /*    2) normal of the triangle */
    /*    3) crossproduct(edge from tri, {x,y,z}-directin) */
    /*       this gives 3x3=9 more tests */
    zeus::CVector3f v0, v1, v2;
    float min, max, d, p0, p1, p2, rad, fex, fey, fez;
    zeus::CVector3f normal, e0, e1, e2;

    /* This is the fastest branch on Sun */
    /* move everything so that the boxcenter is in (0,0,0) */
    v0 = trivert0 - boxcenter;
    v1 = trivert1 - boxcenter;
    v2 = trivert2 - boxcenter;

    /* compute triangle edges */
    e0 = v1 - v0;      /* tri edge 0 */
    e1 = v2 - v1;      /* tri edge 1 */
    e2 = v0 - v2;      /* tri edge 2 */

    /* Bullet 3:  */
    /*  test the 9 tests first (this was faster) */
    fex = std::fabs(e0.x);
    fey = std::fabs(e0.y);
    fez = std::fabs(e0.z);
    AXISTEST_X01(e0.z, e0.y, fez, fey);
    AXISTEST_Y02(e0.z, e0.x, fez, fex);
    AXISTEST_Z12(e0.y, e0.x, fey, fex);

    fex = std::fabs(e1.x);
    fey = std::fabs(e1.y);
    fez = std::fabs(e1.z);
    AXISTEST_X01(e1.z, e1.y, fez, fey);
    AXISTEST_Y02(e1.z, e1.x, fez, fex);
    AXISTEST_Z0(e1.y, e1.x, fey, fex);

    fex = std::fabs(e2.x);
    fey = std::fabs(e2.y);
    fez = std::fabs(e2.z);
    AXISTEST_X2(e2.z, e2.y, fez, fey);
    AXISTEST_Y1(e2.z, e2.x, fez, fex);
    AXISTEST_Z12(e2.y, e2.x, fey, fex);

    /* Bullet 1: */
    /*  first test overlap in the {x,y,z}-directions */
    /*  find min, max of the triangle each direction, and test for overlap in */
    /*  that direction -- this is equivalent to testing a minimal AABB around */
    /*  the triangle against the AABB */

    /* test in X-direction */
    FINDMINMAX(v0.x, v1.x, v2.x, min, max);
    if (min>boxhalfsize.x || max<-boxhalfsize.x) return false;

    /* test in Y-direction */
    FINDMINMAX(v0.y, v1.y, v2.y, min, max);
    if (min>boxhalfsize.y || max<-boxhalfsize.y) return false;

    /* test in Z-direction */
    FINDMINMAX(v0.z, v1.z, v2.z, min, max);
    if (min>boxhalfsize.z || max<-boxhalfsize.z) return false;

    /* Bullet 2: */
    /*  test if the box intersects the plane of the triangle */
    /*  compute plane equation of triangle: normal*x+d=0 */
    normal = e0.cross(e1);
    d = -normal.dot(v0);  /* plane eq: normal.x+d=0 */
    if (!planeBoxOverlap(normal, d, boxhalfsize)) return false;

    return true;   /* box and triangle overlaps */
}

}
}
