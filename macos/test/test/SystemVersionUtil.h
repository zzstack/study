#pragma once

class SystemVersionUtil
{
private:
    bool isOperatingSystemAtLeastVersion_1()
    {
        if ([[NSProcessInfo processInfo] isOperatingSystemAtLeastVersion:(NSOperatingSystemVersion){.majorVersion = 10, .minorVersion = 12, .patchVersion = 0}])
        {
            return true;
        }
        return false;
    }
private:
    bool isOperatingSystemAtLeastVersion_2()
    {
        if (floor(NSAppKitVersionNumber) <= NSAppKitVersionNumber10_9)
        {
            TRACED(“macOS 10.9 or earlier");
            return false;
        }
        else if (floor(NSAppKitVersionNumber) <= NSAppKitVersionNumber10_10)
        {
            TRACED(“macOS 10.10");
            return false;
        }
        else if (floor(NSAppKitVersionNumber) <= NSAppKitVersionNumber10_10_Max)
        {
            TRACED(“macOS 10.10.x");
            return false;
        }
        else if (floor(NSAppKitVersionNumber) <= NSAppKitVersionNumber10_11)
        {
            TRACED(“macOS 10.11");
            return false;
        }
        else if (floor(NSAppKitVersionNumber) <= NSAppKitVersionNumber10_12)
        {
            TRACED(“macOS 10.12 or 10.12.x");
            return true;
        }
        else
        {
            TRACEW("macOS 10.13 or later");
            return true;
        }
        return false;
    }
private:
    bool isOperatingSystemAtLeastVersion_3()
    {
        if (@available(macOS 10.12, *)) {
            return true;
        }
        return false;
    }
};
