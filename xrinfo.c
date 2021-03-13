#include <stdio.h> 
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>

#include <GL/gl.h>
#include <GL/glext.h>
#include <X11/Xlib.h>
#include <GL/glx.h>

#ifdef USE_VULKAN
#include <vulkan/vulkan.h>
#endif

#define XR_USE_PLATFORM_XLIB
#define XR_USE_GRAPHICS_API_OPENGL
#ifdef USE_VULKAN
#define XR_USE_GRAPHICS_API_VULKAN
#endif 
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

static PFN_xrGetOpenGLGraphicsRequirementsKHR F_xrGetOpenGLGraphicsRequirementsKHR = NULL;

static void check_result(const char* func, XrInstance inst, XrResult result)
{
    if(!XR_SUCCEEDED(result))
    {
        char buf[XR_MAX_RESULT_STRING_SIZE];
        xrResultToString(inst, result, buf);
        printf("Call to '%s' failed: %s\n", func, buf);
        exit(1);
    }
}

#define ASSERT_OK(inst, fn) check_result(#fn, inst, fn)

const char* enabled_extensions[] = {
    XR_KHR_OPENGL_ENABLE_EXTENSION_NAME,
#ifdef USE_VULKAN
    XR_KHR_VULKAN_ENABLE_EXTENSION_NAME,
#endif
};

int main()
{
    printf("Built against OpenXR loader version %u.%u.%u\n\n", XR_VERSION_MAJOR(XR_CURRENT_API_VERSION), 
        XR_VERSION_MINOR(XR_CURRENT_API_VERSION),
        XR_VERSION_PATCH(XR_CURRENT_API_VERSION));
    
    /******************/
    /* List API Layers */
    uint32_t propCount = 0;
    xrEnumerateApiLayerProperties(0, &propCount, NULL);

    XrApiLayerProperties* props = malloc(sizeof(XrApiLayerProperties) * propCount);
    uint32_t outCount = 0;
    xrEnumerateApiLayerProperties(propCount, &outCount, props);
    
    printf("Supported API Layers\n");
    printf("--------------------\n");
    if(propCount == 0)
        printf("  No supported API layers\n");
    for(int i = 0; i < propCount; i++) {
        XrApiLayerProperties p = props[i];
        printf("  %-40s (v%u): %s\n", p.layerName, p.layerVersion, p.description);
    }
    /******************/
 
    /******************/
    /* Enum instance extensions */
    uint32_t extCount = 0;
    xrEnumerateInstanceExtensionProperties(NULL, 0, &extCount, NULL);

    XrExtensionProperties* extensions = malloc(sizeof(XrExtensionProperties) * extCount);
    memset(extensions, 0, sizeof(XrExtensionProperties) * extCount);
    for(int i = 0; i < extCount; i++)
        extensions[i].type = XR_TYPE_EXTENSION_PROPERTIES;
    xrEnumerateInstanceExtensionProperties(NULL, extCount, &extCount, extensions);

    printf("\nSupported Instance Extensions\n");
    printf("-----------------------------\n");
    if(extCount == 0)
        printf("  No supported instance extensions\n");
    for(int i = 0; i < extCount; i++) {
        XrExtensionProperties e = extensions[i];
        printf("  %-40s (v%u)\n", e.extensionName, e.extensionVersion);
    }
    /******************/
    
    /******************/
    XrApplicationInfo appInfo;
    memset(&appInfo, 0, sizeof(appInfo));
    appInfo.apiVersion = XR_CURRENT_API_VERSION;
    strcpy(appInfo.applicationName, "xrinfo");
    strcpy(appInfo.engineName, "xrinfo");
    appInfo.engineVersion = 1;
    appInfo.applicationVersion = 1;
    
    XrInstanceCreateInfo createInfo;
    memset(&createInfo, 0, sizeof(createInfo));
    createInfo.applicationInfo = appInfo;
    createInfo.type = XR_TYPE_INSTANCE_CREATE_INFO;
    createInfo.enabledExtensionCount = sizeof(enabled_extensions) / sizeof(enabled_extensions[0]);
    createInfo.enabledExtensionNames = enabled_extensions;
    
    XrInstance instance = 0;
    if(xrCreateInstance(&createInfo, &instance) != XR_SUCCESS) {
        printf("Failed to create XrInstance!!!\n");
        exit(1);
    }
    /******************/

    /******************/
    /* Resolve some symbols here, cool */
    ASSERT_OK(instance, xrGetInstanceProcAddr(instance, "xrGetOpenGLGraphicsRequirementsKHR", (PFN_xrVoidFunction*)&F_xrGetOpenGLGraphicsRequirementsKHR));
    /******************/


    /******************/
    /* System info stuff */
    XrSystemGetInfo sysInfo;
    XrSystemId sysId = 0;
    memset(&sysInfo, 0, sizeof(sysInfo));
    sysInfo.type = XR_TYPE_SYSTEM_GET_INFO;
    sysInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

    xrGetSystem(instance, &sysInfo, &sysId);
    
    XrSystemProperties sysProps;
    memset(&sysProps, 0, sizeof(XrSystemProperties));
    sysProps.type = XR_TYPE_SYSTEM_PROPERTIES;
    
    ASSERT_OK(instance, xrGetSystemProperties(instance, sysId, &sysProps));
    
    printf("\nSystem Info\n");
    printf("-------------\n");
    printf("  System Name: %s\n", sysProps.systemName);
    printf("  Vendor ID: %u\n", sysProps.vendorId);
    printf("  Graphics Props:\n");
    printf("   Swapchain max W: %u\n", sysProps.graphicsProperties.maxSwapchainImageWidth);
    printf("   Swapchain max H: %u\n", sysProps.graphicsProperties.maxSwapchainImageHeight);
    printf("   Max layers: %u\n", sysProps.graphicsProperties.maxLayerCount);
    printf("  Tracking:\n");
    printf("   Orientation: %s\n", sysProps.trackingProperties.orientationTracking ? "true": "false");
    printf("   Position: %s\n", sysProps.trackingProperties.positionTracking ? "true" : "false");
    
    /******************/

    /******************/
    /* OpenGL stuffs */
    XrGraphicsRequirementsOpenGLKHR glRequirements;
    memset(&glRequirements, 0, sizeof(glRequirements));
    glRequirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR;

    F_xrGetOpenGLGraphicsRequirementsKHR(instance, sysId, &glRequirements);

    printf("\nOpenGL Requirements:\n");
    printf("------------------------\n");
    printf("  Min OpenGL version: %u.%u\n", XR_VERSION_MAJOR(glRequirements.minApiVersionSupported),
            XR_VERSION_MINOR(glRequirements.minApiVersionSupported));
    printf("  Max OpenGL version: %u.%u\n", XR_VERSION_MAJOR(glRequirements.maxApiVersionSupported),
            XR_VERSION_MINOR(glRequirements.maxApiVersionSupported));

    /******************/

    /******************/
    /* Cleanup */
    xrDestroyInstance(instance);

    free(extensions);
    free(props);
}
