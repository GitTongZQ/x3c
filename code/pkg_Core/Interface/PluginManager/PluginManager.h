/*! \file PluginManager.h
 *  \brief Define plugin manager wrapper class
 *  \author Zhang Yun Gui, X3 C++ PluginFramework
 *  \date   2010.10.28
 */
#ifndef X3_MANAGER_PLUGINMANAGER_H_
#define X3_MANAGER_PLUGINMANAGER_H_

#include "Ix_ObjectFactory.h"
#include "Ix_PluginLoader.h"

/*! \ingroup _GROUP_PLUGIN_CORE_
 *  \brief Plugin manager wrapper class
 *  \see   Ix_ObjectFactory, Ix_PluginLoader
 */
class CPluginManager
{
public:
    CPluginManager()
    {
        wcscpy_s(m_filename, MAX_PATH, L"PluginManagerX3" PLNEXT);
    }

    ~CPluginManager()
    {
        Unload();
    }

    //! Unload plugin manager and plugins
    void Unload(bool all = false)
    {
        if (all || Handle())
        {
            Ix_PluginLoader* pLoader = GetPluginLoader();
            if (pLoader != NULL)
            {
                pLoader->UnloadPlugins();
            }
        }
        if (Handle() != NULL)
        {
            FreeLibrary(Handle());
            Handle() = NULL;
            Factory() = NULL;
        }
    }

    //! Load plugin manager.
    /*!
        \param subdir PluginManagerX3's path, absolute path or relative to instance.
        \param instance used if subdir is a relative path.
        \return true if plugin manager is loaded.
    */
    bool LoadPluginManager(const wchar_t* subdir, HMODULE instance = NULL)
    {
        GetModuleFileNameW(instance, m_filename, MAX_PATH);
        PathRemoveFileSpecW(m_filename);
        PathAppendW(m_filename, subdir);
        PathAppendW(m_filename, L"PluginManagerX3" PLNEXT);

        if (GetModuleHandleW(m_filename) || Handle())
        {
            return true;
        }

        Handle() = LoadLibraryW(m_filename);

        return Handle() && GetObjectFactory();
    }

    //! Load plugin manager and core plugins.
    /*!
        \param subdir plugin path, absolute path or relative to instance.
        \param instance used if subdir is a relative path.
        \return true if any plugin is loaded.
    */
    bool LoadCorePlugins(const wchar_t* subdir, HMODULE instance = NULL)
    {
        if (!LoadPluginManager(subdir, instance))
        {
            return false;
        }

        Ix_PluginLoader* pLoader = GetPluginLoader();
        if (pLoader)
        {
            long n = pLoader->LoadPluginFiles(subdir,
                L"LogManager.plugin" PLNEXT
                L",LogWriter.plugin" PLNEXT
                L",ConfigXml.plugin" PLNEXT
                L",StringTable.plugin" PLNEXT
                L",ChangeManager.plugin" PLNEXT
                L",FileUtility.plugin" PLNEXT
                L",TextUtility.plugin" PLNEXT
                L",PluginManagerEx.plugin" PLNEXT, instance);

            if (pLoader->InitializePlugins() > 0 && n > 0)
            {
                return true;
            }
        }

        return false;
    }

    //! Return the object creator.
    Ix_ObjectFactory* GetObjectFactory()
    {
        typedef Ix_ObjectFactory* (*FUNC)();
        Ix_ObjectFactory* p = Factory();

        if (!p)
        {
            HMODULE hdll = Handle() ? Handle() : GetModuleHandleW(m_filename);
            FUNC pfn = (FUNC)GetProcAddress(hdll, "x3GetRegisterBank");

            p = pfn ? (*pfn)() : NULL;
            if (p && Handle())
            {
                Factory() = p;
            }
        }

        return p;
    }

    //! Return the plugin loading object.
    Ix_PluginLoader* GetPluginLoader()
    {
        return dynamic_cast<Ix_PluginLoader*>(GetObjectFactory());
    }

    static HMODULE& Handle()
    {
        static HMODULE s_dll = NULL;
        return s_dll;
    }

    static Ix_ObjectFactory*& Factory()
    {
        static Ix_ObjectFactory* s_factory = NULL;
        return s_factory;
    }

private:
    CPluginManager(const CPluginManager&);
    void operator=(const CPluginManager&);

    wchar_t     m_filename[MAX_PATH];
};

#endif // X3_MANAGER_PLUGINMANAGER_H_
