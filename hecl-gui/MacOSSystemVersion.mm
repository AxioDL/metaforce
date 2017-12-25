#include "MacOSSystemVersion.hpp"
#include <CoreFoundation/CoreFoundation.h>
#include <Foundation/Foundation.h>

void GetMacOSSystemVersion(int& major, int& minor, int& patch)
{
    major = 0;
    minor = 0;
    patch = 0;

    id pInfo = [NSProcessInfo processInfo];
    if ([pInfo respondsToSelector:@selector(operatingSystemVersion)])
    {
        NSOperatingSystemVersion version = [pInfo operatingSystemVersion];
        major = version.majorVersion;
        minor = version.minorVersion;
        patch = version.patchVersion;
    }
    else
    {
        major = 10;
        if (kCFCoreFoundationVersionNumber >= kCFCoreFoundationVersionNumber10_9_2)
        {
            minor = 9;
            patch = 2;
        }
        else if (kCFCoreFoundationVersionNumber >= kCFCoreFoundationVersionNumber10_9_1)
        {
            minor = 9;
            patch = 1;
        }
        else if (kCFCoreFoundationVersionNumber >= kCFCoreFoundationVersionNumber10_9)
        {
            minor = 9;
            patch = 0;
        }
        else if (kCFCoreFoundationVersionNumber >= kCFCoreFoundationVersionNumber10_8_4)
        {
            minor = 8;
            patch = 4;
        }
        else if (kCFCoreFoundationVersionNumber >= kCFCoreFoundationVersionNumber10_8_3)
        {
            minor = 8;
            patch = 3;
        }
        else if (kCFCoreFoundationVersionNumber >= kCFCoreFoundationVersionNumber10_8_2)
        {
            minor = 8;
            patch = 2;
        }
        else if (kCFCoreFoundationVersionNumber >= kCFCoreFoundationVersionNumber10_8_1)
        {
            minor = 8;
            patch = 1;
        }
        else if (kCFCoreFoundationVersionNumber >= kCFCoreFoundationVersionNumber10_8)
        {
            minor = 8;
            patch = 0;
        }
        else if (kCFCoreFoundationVersionNumber >= kCFCoreFoundationVersionNumber10_7_5)
        {
            minor = 7;
            patch = 5;
        }
        else if (kCFCoreFoundationVersionNumber >= kCFCoreFoundationVersionNumber10_7_4)
        {
            minor = 7;
            patch = 4;
        }
        else if (kCFCoreFoundationVersionNumber >= kCFCoreFoundationVersionNumber10_7_3)
        {
            minor = 7;
            patch = 3;
        }
        else if (kCFCoreFoundationVersionNumber >= kCFCoreFoundationVersionNumber10_7_2)
        {
            minor = 7;
            patch = 2;
        }
        else if (kCFCoreFoundationVersionNumber >= kCFCoreFoundationVersionNumber10_7_1)
        {
            minor = 7;
            patch = 1;
        }
        else if (kCFCoreFoundationVersionNumber >= kCFCoreFoundationVersionNumber10_7)
        {
            minor = 7;
            patch = 0;
        }
        else if (kCFCoreFoundationVersionNumber >= kCFCoreFoundationVersionNumber10_6_8)
        {
            minor = 6;
            patch = 8;
        }
        else if (kCFCoreFoundationVersionNumber >= kCFCoreFoundationVersionNumber10_6_7)
        {
            minor = 6;
            patch = 7;
        }
        else if (kCFCoreFoundationVersionNumber >= kCFCoreFoundationVersionNumber10_6_6)
        {
            minor = 6;
            patch = 6;
        }
        else if (kCFCoreFoundationVersionNumber >= kCFCoreFoundationVersionNumber10_6_5)
        {
            minor = 6;
            patch = 5;
        }
        else if (kCFCoreFoundationVersionNumber >= kCFCoreFoundationVersionNumber10_6_4)
        {
            minor = 6;
            patch = 4;
        }
        else if (kCFCoreFoundationVersionNumber >= kCFCoreFoundationVersionNumber10_6_3)
        {
            minor = 6;
            patch = 3;
        }
        else if (kCFCoreFoundationVersionNumber >= kCFCoreFoundationVersionNumber10_6_2)
        {
            minor = 6;
            patch = 2;
        }
        else if (kCFCoreFoundationVersionNumber >= kCFCoreFoundationVersionNumber10_6_1)
        {
            minor = 6;
            patch = 1;
        }
        else if (kCFCoreFoundationVersionNumber >= kCFCoreFoundationVersionNumber10_6)
        {
            minor = 6;
            patch = 0;
        }
        else if (kCFCoreFoundationVersionNumber >= kCFCoreFoundationVersionNumber10_5)
        {
            minor = 5;
            patch = 0;
        }
        else if (kCFCoreFoundationVersionNumber >= kCFCoreFoundationVersionNumber10_4)
        {
            minor = 4;
            patch = 0;
        }
        else if (kCFCoreFoundationVersionNumber >= kCFCoreFoundationVersionNumber10_3)
        {
            minor = 3;
            patch = 0;
        }
        else if (kCFCoreFoundationVersionNumber >= kCFCoreFoundationVersionNumber10_2)
        {
            minor = 2;
            patch = 0;
        }
        else if (kCFCoreFoundationVersionNumber >= kCFCoreFoundationVersionNumber10_1)
        {
            minor = 1;
            patch = 0;
        }
        else if (kCFCoreFoundationVersionNumber >= kCFCoreFoundationVersionNumber10_0)
        {
            minor = 0;
            patch = 0;
        }
    }
}
