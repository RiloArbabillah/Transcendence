//	main.cpp
//	Transcendence macOS - Full game entry point

#include "Platform/AppCore.h"

#include <string>

int main(int argc, char* argv[])
{
    std::string sCmdLine;
    for (int i = 1; i < argc; ++i)
    {
        if (!sCmdLine.empty())
            sCmdLine += ' ';

        const char *pArg = argv[i];
        bool bNeedsQuotes = false;
        for (const char *p = pArg; *p; ++p)
            if (*p == ' ' || *p == '\t' || *p == '"')
            {
                bNeedsQuotes = true;
                break;
            }

        if (bNeedsQuotes)
        {
            sCmdLine += '"';
            for (const char *p = pArg; *p; ++p)
            {
                if (*p == '"')
                    sCmdLine += '"';

                sCmdLine += *p;
            }
            sCmdLine += '"';
        }
        else
            sCmdLine += pArg;
    }

    return App_Run(sCmdLine.empty() ? nullptr : sCmdLine.c_str());
}