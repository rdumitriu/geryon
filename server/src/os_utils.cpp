
#include "os_utils.hpp"
#include "log.hpp"

namespace geryon { namespace server {



void * openDynamicLibrary(const char * path) {    
    void * moduleHandler;
    #if defined(G_HAS_WIN)
        moduleHandler = LoadLibraryA(path);
    #else
        moduleHandler = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
    #endif
    if(!moduleHandler) {
        LOG(geryon::util::Log::WARNING) << "Cannot load dynamic library from path :" << path;
        #if defined(G_HAS_WIN)
            //::TODO::
        #else
            LOG(geryon::util::Log::WARNING) << "Reason :" << dlerror();
        #endif
        return 0;
    }
    return moduleHandler;
}

void closeDynamicLibrary(void * moduleHandler) {
    #if defined(G_HAS_WIN)
        FreeLibrary(reinterpret_cast<HINSTANCE>(moduleHandler));
    #else
        dlclose(moduleHandler);
    #endif
}

} }  /* namespace */
