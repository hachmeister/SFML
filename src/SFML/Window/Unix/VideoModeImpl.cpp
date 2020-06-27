////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2019 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/VideoModeImpl.hpp>
#include <SFML/Window/Unix/Display.hpp>
#include <SFML/System/Err.hpp>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <algorithm>


namespace sf
{
namespace priv
{
////////////////////////////////////////////////////////////
std::vector<VideoMode> VideoModeImpl::getFullscreenModes()
{
    std::vector<VideoMode> modes;

    // Open a connection with the X server
    Display* display = OpenDisplay();
    if (display)
    {
        // Retrieve the default screen number
        int screen = DefaultScreen(display);

        // Check if the XRandR extension is present
        int version;
        if (XQueryExtension(display, "RANDR", &version, &version, &version))
        {
            // Get the current configuration
            XRRScreenConfiguration* config = XRRGetScreenInfo(display, RootWindow(display, screen));
            if (config)
            {
                // Get Screen resources
                XRRScreenResources* res = XRRGetScreenResources(display, RootWindow(display, screen));
                if (res)
                {
                    RROutput output = XRRGetOutputPrimary(display, RootWindow(display, screen));
                    XRROutputInfo* output_info = XRRGetOutputInfo(display, res, output);
                    if (output_info)
                    {
                        // Get the list of supported depths
                        int nbDepths = 0;
                        int* depths = XListDepths(display, screen, &nbDepths);
                        if (depths && (nbDepths > 0))
                        {
                            // Combine depths and sizes to fill the array of supported modes
                            for (int i = 0; i < nbDepths; ++i)
                            {
                                for (int j = 0; j < output_info->nmode; ++j)
                                {
                                    RRMode jmode = output_info->modes[j];

                                    for (int k = 0; k < res->nmode; ++k)
                                    {
                                        const XRRModeInfo *info = &res->modes[k];

                                        if (jmode == info->id)
                                        {
                                            // Convert to VideoMode
                                            VideoMode mode(info->width, info->height, depths[i]);

                                            Rotation currentRotation;
                                            XRRConfigRotations(config, &currentRotation);

                                            if (currentRotation == RR_Rotate_90 || currentRotation == RR_Rotate_270)
                                                std::swap(mode.width, mode.height);

                                            // Add it only if it is not already in the array
                                            if (std::find(modes.begin(), modes.end(), mode) == modes.end())
                                                modes.push_back(mode);
                                        }
                                    }
                                }
                            }

                            // Free the array of depths
                            XFree(depths);
                        }

                        // Free Output info
                        XRRFreeOutputInfo(output_info);
                    }

                    // Free Screen resources
                    XRRFreeScreenResources(res);
                }

                // Free the configuration instance
                XRRFreeScreenConfigInfo(config);
            }
            else
            {
                // Failed to get the screen configuration
                err() << "Failed to retrieve the screen configuration while trying to get the supported video modes" << std::endl;
            }
        }
        else
        {
            // XRandr extension is not supported: we cannot get the video modes
            err() << "Failed to use the XRandR extension while trying to get the supported video modes" << std::endl;
        }

        // Close the connection with the X server
        CloseDisplay(display);
    }
    else
    {
        // We couldn't connect to the X server
        err() << "Failed to connect to the X server while trying to get the supported video modes" << std::endl;
    }

    return modes;
}


////////////////////////////////////////////////////////////
VideoMode VideoModeImpl::getDesktopMode()
{
    VideoMode desktopMode;

    // Open a connection with the X server
    Display* display = OpenDisplay();
    if (display)
    {
        // Retrieve the default screen number
        int screen = DefaultScreen(display);

        // Check if the XRandR extension is present
        int version;
        if (XQueryExtension(display, "RANDR", &version, &version, &version))
        {
            // Get the current configuration
            XRRScreenConfiguration* config = XRRGetScreenInfo(display, RootWindow(display, screen));
            if (config)
            {
                // Get Screen resources
                XRRScreenResources* res = XRRGetScreenResources(display, RootWindow(display, screen));
                if (res)
                {
                    RROutput output = XRRGetOutputPrimary(display, RootWindow(display, screen));
                    XRROutputInfo* output_info = XRRGetOutputInfo(display, res, output);
                    if (output_info)
                    {
                        RRMode mode = output_info->modes[0];

                        for (int k = 0; k < res->nmode; ++k)
                        {
                            const XRRModeInfo *info = &res->modes[k];

                            if (mode == info->id)
                            {
                                desktopMode = VideoMode(info->width, info->height, DefaultDepth(display, screen));

                                Rotation currentRotation;
                                XRRConfigRotations(config, &currentRotation);

                                if (currentRotation == RR_Rotate_90 || currentRotation == RR_Rotate_270)
                                    std::swap(desktopMode.width, desktopMode.height);
                            }
                        }

                        // Free Output info
                        XRRFreeOutputInfo(output_info);
                    }

                    // Free Screen resources
                    XRRFreeScreenResources(res);
                }

                // Free the configuration instance
                XRRFreeScreenConfigInfo(config);
            }
            else
            {
                // Failed to get the screen configuration
                err() << "Failed to retrieve the screen configuration while trying to get the desktop video modes" << std::endl;
            }
        }
        else
        {
            // XRandr extension is not supported: we cannot get the video modes
            err() << "Failed to use the XRandR extension while trying to get the desktop video modes" << std::endl;
        }

        // Close the connection with the X server
        CloseDisplay(display);
    }
    else
    {
        // We couldn't connect to the X server
        err() << "Failed to connect to the X server while trying to get the desktop video modes" << std::endl;
    }

    return desktopMode;
}

} // namespace priv

} // namespace sf
