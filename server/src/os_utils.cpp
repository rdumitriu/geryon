
#include "os_utils.hpp"
#include "log.hpp"

namespace geryon { namespace server {



void * openDynamicLibrary(const char * path) {    
    void * moduleHandler;
    #if defined(_WIN32) || defined(_WIN64)
        moduleHandler = LoadLibraryA(path);
    #else
        moduleHandler = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
    #endif
    if(!moduleHandler) {
        LOG(geryon::util::Log::WARNING) << "Cannot load dynamic library from path :" << path;
        #if defined(_WIN32) || defined(_WIN64)
            //::TODO::
        #else
            LOG(geryon::util::Log::WARNING) << "Reason '" << dlerror();
        #endif
        return NULL;
    }
    return moduleHandler;
}

void closeDynamicLibrary(void * moduleHandler) {
    #if defined(_WIN32) || defined(_WIN64)
        FreeLibrary(reinterpret_cast<HINSTANCE>(moduleHandler));
    #else
        dlclose(moduleHandler);
    #endif
}

} }  /* namespace */
