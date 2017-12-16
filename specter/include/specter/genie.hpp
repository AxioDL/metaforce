#ifndef __SPECTER_GENIE_HPP__
#define __SPECTER_GENIE_HPP__

#if __specter__
#define SPECTER_PROPERTY(n, d) \
    [[using specter: name(n), description(d)]]
#define SPECTER_ENUM(n, d, et) \
    [[using specter: name(n), description(d), enum_type(et)]]
#else
#define SPECTER_PROPERTY(n, d)
#define SPECTER_ENUM(n, d, et)
#endif


#endif //__SPECTER_GENIE_HPP__
