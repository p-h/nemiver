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
#include "config.h"
#include <glibmm.h>
#include "nmv-libxml-utils.h"
#include "nmv-exception.h"
#include "nmv-plugin.h"
#include "nmv-env.h"

namespace nemiver {
namespace common {

struct Plugin::EntryPoint::Loader::Priv {
    UString plugin_path;
};//end struct Plugin::EntryPoint::Loader::Priv

Plugin::EntryPoint::Loader::Loader (const UString &a_plugin_path) :
    m_priv (new Plugin::EntryPoint::Loader::Priv)
{
    THROW_IF_FAIL (m_priv);

    config_search_paths ().clear ();
    if (!Glib::file_test (a_plugin_path, Glib::FILE_TEST_IS_DIR)) {
        THROW ("Couldn't find directory '" + a_plugin_path + "'");
    }
    config_search_paths ().push_back (a_plugin_path);
    m_priv->plugin_path = a_plugin_path;
}

Plugin::EntryPoint::Loader::~Loader ()
{
    LOG_D ("delete", "destructor-domain");
}

const UString&
Plugin::EntryPoint::Loader::plugin_path ()
{
    THROW_IF_FAIL (m_priv);
    return m_priv->plugin_path;
}

//**********************
//<class Plugin::EntryPoint>
//**********************
struct Plugin::EntryPoint::Priv {
    bool is_activated;
    Plugin::EntryPoint::LoaderSafePtr entry_point_loader;
    DescriptorSafePtr descriptor;

    Priv () :
        is_activated (false)
    {
    }
};//end Plugin::EntryPoint::Priv

Plugin::EntryPoint::LoaderSafePtr
Plugin::EntryPoint::plugin_entry_point_loader ()
{
    return m_priv->entry_point_loader;
}

void
Plugin::EntryPoint::plugin_entry_point_loader
                        (Plugin::EntryPoint::LoaderSafePtr &a_loader)
{
    THROW_IF_FAIL (m_priv);
    m_priv->entry_point_loader = a_loader;
}

Plugin::EntryPoint::EntryPoint (DynamicModuleSafePtr &a_module) :
    DynModIface (a_module),
    m_priv (new Plugin::EntryPoint::Priv)
{
}

Plugin::EntryPoint::EntryPoint (DynamicModule *a_module) :
    DynModIface (a_module),
    m_priv (new Plugin::EntryPoint::Priv)
{
}

Plugin::EntryPoint::~EntryPoint ()
{
    LOG_D ("delete", "destructor-domain");
}

void
Plugin::EntryPoint::activate (bool a_activate,
                              ObjectSafePtr &a_ctxt)
{
    if (a_ctxt) {}
    THROW_IF_FAIL (m_priv);
    m_priv->is_activated = a_activate;
}

bool
Plugin::EntryPoint::is_activated ()
{
    THROW_IF_FAIL (m_priv);
    return m_priv->is_activated;
}

const UString&
Plugin::EntryPoint::plugin_path ()
{
    THROW_IF_FAIL (plugin_entry_point_loader ());

    return plugin_entry_point_loader ()->plugin_path ();
}

bool
Plugin::EntryPoint::build_absolute_resource_path
                            (const UString &a_relative_resource_path,
                             string &a_absolute_path)
{
    string relative_path = Glib::locale_from_utf8 (a_relative_resource_path);
    string plugin_dir_path = Glib::locale_from_utf8 (plugin_path ());
    string absolute_path = Glib::build_filename (plugin_dir_path, relative_path);
    bool result (false);
    if (Glib::file_test (absolute_path,
                         Glib::FILE_TEST_IS_REGULAR
                         | Glib::FILE_TEST_EXISTS)) {
        result = true;
        a_absolute_path = absolute_path;
    }
    return result;
}

Plugin::DescriptorSafePtr
Plugin::EntryPoint::descriptor ()
{
    THROW_IF_FAIL (m_priv);
    return m_priv->descriptor;
}

void
Plugin::EntryPoint::descriptor (Plugin::DescriptorSafePtr &a_desc)
{
    THROW_IF_FAIL (m_priv);
    m_priv->descriptor = a_desc;
}


//**********************
//</class Plugin::Entry>
//**********************

//**********************
//<class Plugin>
//**********************

struct Plugin::Priv {
    Plugin::EntryPointSafePtr entry_point;
    Plugin::DescriptorSafePtr descriptor;
    DynamicModuleManager &module_manager;

    Priv (Plugin::DescriptorSafePtr &a_desc,
          DynamicModuleManager &a_manager) :
        descriptor (a_desc),
        module_manager (a_manager)
    {}

};//end Plugin::Priv

void
Plugin::load_entry_point ()
{
    THROW_IF_FAIL (m_priv && m_priv->descriptor);

    try {
        EntryPoint::LoaderSafePtr loader
                (new EntryPoint::Loader (m_priv->descriptor->plugin_path ()));
        m_priv->entry_point =
            m_priv->module_manager.load_iface<Plugin::EntryPoint>
                                (m_priv->descriptor->entry_point_module_name (),
                                 m_priv->descriptor->entry_point_interface_name(),
                                 *loader);
        THROW_IF_FAIL (m_priv->entry_point);
        LOG_REF_COUNT (m_priv->entry_point,
                       m_priv->descriptor->entry_point_interface_name ());

        LOG_REF_COUNT (loader, "plugin-entry-point-loader");
        m_priv->entry_point->plugin_entry_point_loader (loader);
        LOG_REF_COUNT (loader, "plugin-loader");

        m_priv->entry_point->descriptor (m_priv->descriptor);
    } catch (...) {
        THROW ("failed to load plugin entry point '"
               + m_priv->descriptor->entry_point_module_name ()
               + " for plugin '" + m_priv->descriptor->name () + "'");
    }
}

Plugin::Plugin (Plugin::DescriptorSafePtr &a_desc,
                DynamicModuleManager &a_manager) :
    m_priv (new Plugin::Priv (a_desc, a_manager))
{
    THROW_IF_FAIL (m_priv);
    THROW_IF_FAIL (a_desc);
    THROW_IF_FAIL (Glib::file_test (a_desc->plugin_path (),
                                    Glib::FILE_TEST_IS_DIR));
    load_entry_point ();
}

Plugin::~Plugin ()
{
    LOG_D ("delete", "destructor-domain");
}

Plugin::DescriptorSafePtr
Plugin::descriptor ()
{
    THROW_IF_FAIL (m_priv && m_priv->descriptor);
    return m_priv->descriptor;
}

void
Plugin::descriptor (const Plugin::DescriptorSafePtr &a_desc)
{
    THROW_IF_FAIL (m_priv);
    m_priv->descriptor = a_desc;
}

Plugin::EntryPoint&
Plugin::entry_point ()
{
    THROW_IF_FAIL (m_priv && m_priv->entry_point);
    return *m_priv->entry_point;
}

Plugin::EntryPointSafePtr
Plugin::entry_point_ptr ()
{
    THROW_IF_FAIL (m_priv && m_priv->entry_point);
    return m_priv->entry_point;
}

void
Plugin::activate (bool a_activate,
                  ObjectSafePtr &a_ctxt)
{
    entry_point ().activate (a_activate, a_ctxt);
}

bool
Plugin::is_activated ()
{
    return entry_point ().is_activated ();
}

//**********************
//</class Plugin>
//**********************

//***********************
//<class PluginManager>
//**********************

struct PluginManager::Priv {
    vector<UString> plugins_search_path;
    map<UString, UString> deps_descs_loaded;
    map<UString, PluginSafePtr> plugins_map;
    DynamicModuleManager &module_manager;

    Priv (DynamicModuleManager &a_in) :
        module_manager (a_in)
    {}
};//end struct PluginManagerPriv

PluginManager::PluginManager (DynamicModuleManager &a_in) :
    m_priv (new PluginManager::Priv (a_in))
{
    plugins_search_path ().push_back (env::get_system_plugins_dir ());
}

PluginManager::~PluginManager ()
{
    LOG_D ("delete", "destructor-domain");
}

UString
PluginManager::find_plugin_path_from_name (const UString &a_name)
{
    UString result;

    vector<UString>::const_iterator cur_path;
    string path;
    for (cur_path = plugins_search_path ().begin ();
         cur_path != plugins_search_path ().end ();
         ++cur_path) {
        path = Glib::build_filename (Glib::locale_from_utf8 (*cur_path),
                                     Glib::locale_from_utf8 (a_name));
        if (Glib::file_test (path, Glib::FILE_TEST_IS_DIR)) {
            result = Glib::locale_to_utf8 (path);
            break;
        }
    }
    return result;
}

bool
PluginManager::parse_descriptor (const UString &a_path,
                                 Plugin::DescriptorSafePtr &a_out)
{
    Plugin::DescriptorSafePtr desc (new Plugin::Descriptor);

    if (a_path == "") {
        THROW ("Got path \"\" to modules config file");
    }


    using libxmlutils::XMLTextReaderSafePtr;
    using libxmlutils::XMLCharSafePtr;

    XMLTextReaderSafePtr reader;
    XMLCharSafePtr xml_str;

    reader.reset (xmlNewTextReaderFilename (a_path.c_str ()));
    if (!reader) {
        LOG_ERROR ("could not create xml reader");
        return false;
    }

    UString dirname =
        Glib::locale_to_utf8
                (Glib::path_get_dirname (Glib::locale_from_utf8 (a_path)));

    desc->plugin_path (dirname);

    if (!goto_next_element_node_and_check (reader, "plugindescriptor")) {
        THROW ("first element node should be 'plugindescriptor'");
    }
    xml_str.reset (xmlTextReaderGetAttribute (reader.get (),
                                             (const xmlChar*)"autoactivate"));
    UString autoactivate = xml_str.get ();

    if (autoactivate == "yes") {
        desc->auto_activate (true);
    } else {
        desc->auto_activate (false);
    }

    desc->can_deactivate (true);
    if (desc->auto_activate ()) {
        xml_str.reset (xmlTextReaderGetAttribute (reader.get (),
                                                  (const xmlChar*) "candeactivate"));
        UString candeactivate = xml_str.get ();
        if (candeactivate == "no") {
            desc->can_deactivate (false);
        }
    }

    if (!goto_next_element_node_and_check (reader, "name")) {
        THROW ("expected element 'name', got: "
               + UString (xmlTextReaderConstName (reader.get ())));
    }

    xml_str.reset (xmlTextReaderReadString (reader.get ()));

    desc->name (xml_str.get ());

    if (!goto_next_element_node_and_check (reader, "version")) {
        THROW ("expected element 'version', got: "
               + UString (xmlTextReaderConstName (reader.get ())));
    }

    xml_str.reset (xmlTextReaderReadString (reader.get ()));
    desc->version (xml_str.get ());

    if (!goto_next_element_node_and_check (reader, "entrypoint")) {
        THROW ("expected element 'entrypoint', got: "
               + UString (xmlTextReaderConstName (reader.get ())));
    }

    if (!goto_next_element_node_and_check (reader, "modulename")) {
        THROW ("expected element 'modulename', got: "
               + UString (xmlTextReaderConstName (reader.get ())));
    }

    xml_str.reset (xmlTextReaderReadString (reader.get ()));
    desc->entry_point_module_name (xml_str.get ());

    if (!goto_next_element_node_and_check (reader, "interfacename")) {
        THROW ("expected element 'interfacename', got: "
               + UString (xmlTextReaderConstName (reader.get ())));
    }

    xml_str.reset (xmlTextReaderReadString (reader.get ()));
    desc->entry_point_interface_name (xml_str.get ());

    if (!goto_next_element_node_and_check (reader, "dependencies")) {
        THROW ("expected element 'dependencies', got: "
               + UString (xmlTextReaderConstName (reader.get ())));
    }

    UString name;
    for (;;) {
        if (!goto_next_element_node_and_check (reader, "plugin")) {
            break;
        }
        if (!goto_next_element_node_and_check (reader, "name")) {
            THROW ("expected element 'name', got: "
                   + UString (xmlTextReaderConstName (reader.get ())));
        }
        xml_str.reset (xmlTextReaderReadString (reader.get ()));
        name = xml_str.get ();
        if (!goto_next_element_node_and_check (reader, "version")) {
            THROW ("expected element 'version', got: "
                   + UString (xmlTextReaderConstName (reader.get ())));
        }
        xml_str.reset (xmlTextReaderReadString (reader.get ()));
        desc->dependencies ()[name] = xml_str.get ();
    }

    a_out = desc;
    return true;
}

const UString&
PluginManager::descriptor_name ()
{
    static UString s_descriptor_name = "plugin-descriptor.xml";
    return s_descriptor_name;
}

bool
PluginManager::load_descriptor_from_plugin_path (const UString &a_plugin_path,
                                                 Plugin::DescriptorSafePtr &a_in)
{
    vector<string> path_elements;
    path_elements.push_back (Glib::locale_from_utf8 (a_plugin_path));
    path_elements.push_back (descriptor_name ());
    string path = Glib::build_filename (path_elements);
    if (!Glib::file_test (path, Glib::FILE_TEST_IS_REGULAR)) {
        return false;
    }
    return parse_descriptor (Glib::locale_to_utf8 (path), a_in);
}

bool
PluginManager::load_descriptor_from_plugin_name (const UString &a_name,
                                                 Plugin::DescriptorSafePtr &a_out)
{
    THROW_IF_FAIL (a_name != "");

    UString path = find_plugin_path_from_name (a_name);
    if (path == "") {return false;}

    return load_descriptor_from_plugin_path (path, a_out);
}

bool
PluginManager::load_dependant_descriptors
                                (const Plugin::Descriptor &a_desc,
                                 vector<Plugin::DescriptorSafePtr> &a_descs)
{
    bool result (true);
    map<UString, UString>::const_iterator cur_dep;
    Plugin::DescriptorSafePtr desc;
    for (cur_dep = a_desc.dependencies ().begin ();
         cur_dep != a_desc.dependencies ().end ();
         ++cur_dep) {
        if (load_descriptor_from_plugin_name (cur_dep->first, desc)
            && desc) {
            a_descs.push_back (desc);
            result = true;
        } else {
            LOG_ERROR ("Could not load plugin dependency: " + cur_dep->first);
            result = false;
            break;
        }
    }
    return result;
}

bool
PluginManager::load_dependant_descriptors_recursive
                                (const Plugin::Descriptor &a_desc,
                                 vector<Plugin::DescriptorSafePtr> &a_full_deps)
{
    vector<Plugin::DescriptorSafePtr> deps;

    if (!load_dependant_descriptors (a_desc, deps)) {
        LOG_ERROR ("failed to load direct dependent descriptors of module '"
                   + a_desc.name () + "'");
        return false;
    }

    if (deps.empty ()) {return true;}

    vector<Plugin::DescriptorSafePtr>::iterator it;
    vector<Plugin::DescriptorSafePtr> sub_deps;
    for (it = deps.begin ();
         it != deps.end ();
         ++it) {
        //if an asked dependency has been loaded already, don't load it
        if (m_priv->deps_descs_loaded.find ((*it)->name ())
            == m_priv->deps_descs_loaded.end ()) {
            m_priv->deps_descs_loaded[(*it)->name ()] = "";
        } else {
            continue;
        }
        if (!load_dependant_descriptors_recursive ((**it), sub_deps)) {
            LOG_ERROR ("failed to load deep dependent descriptors of module '"
                       + a_desc.name () + "'");
            return false;
        }
        a_full_deps.push_back (*it);
        if (sub_deps.empty ()) {
            a_full_deps.insert (a_full_deps.end (),
                                sub_deps.begin (),
                                sub_deps.end ());
            sub_deps.clear ();
        }
    }
    return true;
}

PluginSafePtr
PluginManager::load_plugin_from_path (const UString &a_plugin_path,
                                      vector<PluginSafePtr> &a_deps)
{
    PluginSafePtr result;
    string path (Glib::locale_from_utf8 (a_plugin_path)); ;

    if (!Glib::file_test (path, Glib::FILE_TEST_IS_DIR)) {
        LOG_ERROR ("directory '" + a_plugin_path + "does not exist");
        return result;
    }

    Plugin::DescriptorSafePtr descriptor;
    if (!load_descriptor_from_plugin_path (a_plugin_path,
                                           descriptor)
        || !descriptor) {
        LOG_ERROR ("couldn't load plugin descriptor for "
                   "plugin at path '" + a_plugin_path + "'");
        return result;
    }

    vector<Plugin::DescriptorSafePtr> dependant_descs;
    if (!load_dependant_descriptors_recursive (*descriptor, dependant_descs)) {
        LOG_ERROR ("couldn't load plugin descriptor dependances "
                   "for root descriptor '" + descriptor->name () + "' at '"
                   + descriptor->plugin_path () + "'");
        return result;
    }

    UString ppath;
    vector<Plugin::DescriptorSafePtr>::iterator cur_desc;
    vector<PluginSafePtr> dependances;
    PluginSafePtr plugin;
    for (cur_desc = dependant_descs.begin ();
         cur_desc != dependant_descs.end ();
         ++cur_desc) {
        try {
            THROW_IF_FAIL (*cur_desc);
            ppath = (*cur_desc)->plugin_path ();
            if (plugins_map ().find (ppath) != plugins_map ().end ()) {
                plugin = plugins_map ()[ppath];
            } else {
                plugin.reset (new Plugin (*cur_desc, m_priv->module_manager));
                plugins_map ()[ppath] = plugin;
            }
        } catch (...) {
            LOG_ERROR ("Failed to load dependant plugin '"
                       + (*cur_desc)->name () + "'");
            return result;
        }
        dependances.push_back (plugin);
    }

    try {
        ppath = descriptor->plugin_path ();
        if (plugins_map ().find (ppath) != plugins_map ().end ()) {
            plugin = plugins_map ()[ppath];
        } else {
            plugin.reset (new Plugin (descriptor, m_priv->module_manager));
            plugins_map ()[ppath] = plugin;
        }
    } catch (...) {
        LOG_ERROR ("failed to load plugin '" + descriptor->name () + "'");
        return result;
    }

    a_deps = dependances;
    result = plugin;
    if (result) {
        LOG_D ("plugin '"
                << a_plugin_path
                << "' refcount: "
                << (int) result->get_refcount (),
                "refcount-domain");
    }
    LOG_D ("loaded plugin from path " << Glib::locale_from_utf8 (a_plugin_path),
            "plugin-loading-domain");
    return result;
}

PluginSafePtr
PluginManager::load_plugin_from_name (const UString &a_name,
                                      vector<PluginSafePtr> &a_deps)
{
    PluginSafePtr result;

    vector<UString>::const_iterator cur_path;
    vector<string> path_elements;
    string plugin_path;
    for (cur_path = plugins_search_path ().begin ();
         cur_path != plugins_search_path ().end ();
         ++cur_path)
    {
        path_elements.clear ();
        path_elements.push_back (Glib::locale_from_utf8 (*cur_path));
        path_elements.push_back (Glib::locale_from_utf8 (a_name));
        plugin_path = Glib::build_filename (path_elements);
        if (!Glib::file_test (plugin_path, Glib::FILE_TEST_IS_DIR)) {
            continue;
        }
        try {
            result = load_plugin_from_path (Glib::locale_to_utf8 (plugin_path),
                                            a_deps);
        } catch (...) {}

        if (result) {
            LOG_D ("plugin '"
                    << a_name
                    << "' refcount: "
                    << (int) result->get_refcount (),
                    "refcount-domain");
            break;
        }
    }
    LOG_D ("loaded plugin " << Glib::locale_from_utf8 (a_name),
           "plugin-loading-domain");
    return result;
}

vector<UString>&
PluginManager::plugins_search_path ()
{
    return m_priv->plugins_search_path;
}

bool
PluginManager::load_plugins ()
{
    PluginSafePtr plugin;
    vector<PluginSafePtr> dependances;

    vector<UString>::const_iterator cur_dir;
    string path;
    for (cur_dir = plugins_search_path ().begin ();
         cur_dir != plugins_search_path ().end ();
         ++cur_dir) {
        try {
            Glib::Dir opened_dir (*cur_dir);
            for (Glib::DirIterator it = opened_dir.begin ();
                 it != opened_dir.end ();
                 ++it) {
                path = Glib::build_filename (*cur_dir, *it);
                if (plugins_map ().find (Glib::locale_to_utf8 (path))
                    != plugins_map ().end ()) {
                    continue;
                }
                plugin = load_plugin_from_path (Glib::locale_to_utf8 (path),
                                                dependances);
                if (plugin) {
                    LOG_D ("plugin '"
                            << path
                            << "' put in  map. Refcount: "
                            << (int) plugin->get_refcount (),
                            "refcount-domain");
                }
            }
        } catch (...) {
            continue;
        }
    }
    return true;
}

map<UString, PluginSafePtr>&
PluginManager::plugins_map ()
{
    return m_priv->plugins_map;
}

//***********************
//</class PluginManager>
//**********************

}//end namespace common
}//end namespace nemiver

