/* -*- Mode: C++; indent-tabs-mode:nil; c-basic-offset:4;-*- */

/*Copyright (c) 2005-2006 Dodji Seketeli
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS",
 * WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#ifndef __NEMIVER_DYNAMIC_MODULE_H__
#define __NEMIVER_DYNAMIC_MODULE_H__

#include <vector>
#include <list>
#include <map>
#include <string>
#include <gmodule.h>
#include "nmv-api-macros.h"
#include "nmv-object.h"
#include "nmv-ustring.h"
#include "nmv-safe-ptr-utils.h"
#include "nmv-exception.h"

/// \file
/// the declaration of the DynamicModule class.
extern "C" {

/// \brief the factory of the DynamicModule.
///
/// Implementors of the loadable module must implement this function.
/// This function must then instanciate a DynamicModule,
/// set \em a_new_inst to that new DynamicModule, and return true in case
/// of success. Otherwise, it must return false.
bool nemiver_common_create_dynamic_module_instance (void **a_new_instance) ;

}

namespace nemiver {
namespace common {

class DynamicModule ;
typedef SafePtr<DynamicModule, ObjectRef, ObjectUnref> DynamicModuleSafePtr ;

class DynamicModuleManager ;
typedef SafePtr<DynamicModuleManager,
                ObjectRef,
                ObjectUnref> DynamicModuleManagerSafePtr ;

/// \brief The base class for loadable modules
///
/// A loadable module is a dynamic library that contains an instance
/// of DynamicModule.
class NEMIVER_API DynamicModule : public Object {

public:

    struct Info {
        UString module_name ;
        UString module_description ;
        UString module_version ;

        Info (const UString &a_name,
              const UString &a_desc,
              const UString &a_version) :
            module_name (a_name),
            module_description (a_desc),
            module_version (a_version)
        {}

    };//end struct info

    struct Config : public Object {
        std::vector<UString> custom_library_search_paths ;
        UString library_name ;
        virtual ~Config () {};
    };//end stuct ModuleConfig

    typedef SafePtr<Config, ObjectRef, ObjectUnref> ConfigSafePtr ;

    class NEMIVER_API Loader : public Object {
        struct Priv ;
        SafePtr<Priv> m_priv ;
        friend class DynamicModuleManager ;

        //non copyable
        Loader (const Loader &) ;
        Loader& operator= (const Loader &) ;

        void set_dynamic_module_manager (DynamicModuleManager *a_mgr) ;


    public:
        Loader () ;
        virtual ~Loader () ;
        std::vector<UString>& config_search_paths () ;
        virtual DynamicModule::ConfigSafePtr module_config
                                                    (const std::string &a_name) ;
        virtual UString build_library_path (const UString &a_module_name,
                                            const UString &a_lib_name) ;
        virtual ConfigSafePtr parse_module_config_file (const UString &a_path) ;
        virtual UString module_library_path (const UString &a_name) ;
        virtual GModule* load_library_from_path (const UString &a_path) ;
        virtual GModule* load_library_from_module_name (const UString &a_name) ;
        virtual DynamicModuleSafePtr create_dynamic_module_instance
                                                            (GModule *a_module) ;

        virtual DynamicModuleSafePtr load (const UString &a_name) ;
        virtual DynamicModuleSafePtr load_from_path
                                            (const UString &a_lib_path) ;
        DynamicModuleManager* get_dynamic_module_manager () ;

    };//end class Loader
    typedef SafePtr<Loader, ObjectRef, ObjectUnref> LoaderSafePtr ;

protected:
    //must be instanciated by a factory
    DynamicModule () ;

private:
    struct Priv ;
    SafePtr<Priv> m_priv ;
    friend bool gpil_common_create_loadable_module_instance (void **a_new_inst);
    friend class Loader ;
    friend class DynamicModuleManager ;

    //non copyable
    DynamicModule (const DynamicModule &) ;
    DynamicModule& operator= (const DynamicModule &) ;

    void set_real_library_path (const UString &a_path) ;

    void set_module_loader (Loader *a_loader) ;


public:

    /// \brief get the module loader class
    /// \return the module loader
    Loader* get_module_loader () ;

    /// \brief gets the path of the library this module has been instanciated
    /// from.
    /// \return the path of the library the module has been instanciated from.
    const UString& get_real_library_path () const ;

    /// \brief destructor
    virtual ~DynamicModule () ;

    /// \brief get module info
    virtual void get_info (Info &a_info) const = 0;

    /// \brief module init routinr
    virtual void do_init () = 0;
};//end class DynamicModule

class NEMIVER_API ModuleRegistry : public Object {
    struct Priv ;
    SafePtr<Priv> m_priv ;
    //non coyable ;
    ModuleRegistry (const ModuleRegistry &) ;
    ModuleRegistry& operator= (const ModuleRegistry &) ;

public:
    ModuleRegistry () ;
    virtual ~ModuleRegistry () ;
    GModule* get_library_from_cache (const UString a_name) ;
    void put_library_into_cache (const UString a_name,
                                 GModule *a_module) ;
};//end class ModuleRegistry


class NEMIVER_API DynamicModuleManager : public Object {
    struct Priv ;
    SafePtr<Priv> m_priv ;

public:

    DynamicModuleManager () ;
    virtual ~DynamicModuleManager () ;

    DynamicModuleSafePtr load (const UString &a_name,
                               DynamicModule::Loader &) ;

    DynamicModuleSafePtr load (const UString &a_name) ;

    DynamicModuleSafePtr load_from_path (const UString &a_library_path,
                                         DynamicModule::Loader &) ;

    DynamicModuleSafePtr load_from_path (const UString &a_library_path) ;

    template <class T> SafePtr<T,
                               ObjectRef,
                               ObjectUnref> load (const UString &a_name,
                                                  DynamicModule::Loader &) ;
    template <class T> SafePtr<T,
                               ObjectRef,
                               ObjectUnref> load (const UString &a_name) ;
    ModuleRegistry& module_registry () ;
    DynamicModule::LoaderSafePtr& module_loader () ;
    void module_loader (DynamicModule::LoaderSafePtr &a_loader) ;
};//end class DynamicModuleManager


template <class T>
SafePtr<T, ObjectRef, ObjectUnref>
DynamicModuleManager::load (const UString &a_name)
{
    return load<T> (a_name, *module_loader ()) ;
}

template <class T>
SafePtr<T, ObjectRef, ObjectUnref>
DynamicModuleManager::load (const UString &a_name,
                            DynamicModule::Loader &a_loader)
{
    DynamicModuleSafePtr module (load (a_name, a_loader)) ;
    typedef SafePtr<T, ObjectRef, ObjectUnref> TSafePtr ;
    TSafePtr result ;
    if (!module) {THROW (UString ("failed to load module '") + a_name);};
    result.reset (dynamic_cast<T*> (module.get ()), true);
    if (!result) {THROW ("module is not of the expected type'");}
    return result ;
}

}//end namespace common
}//end namespace nemiver

#endif// __NEMIVER_DYNAMIC_MODULE_H__

