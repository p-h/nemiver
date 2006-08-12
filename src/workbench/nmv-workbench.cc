/*
 *This file is part of the Nemiver project
 *
 *Nemiver is free software; you can redistribute
 *it and/or modify it under the terms of
 *the GNU General Public License as published by the
 *Free Software Foundation; either version 2,
 *or (at your option) any later version.
 *
 *Nemiver is distributed in the hope that it will
 *be useful, but WITHOUT ANY WARRANTY;
 *without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *See the GNU General Public License for more details.
 *
 *You should have received a copy of the
 *GNU General Public License along with Goupil;
 *see the file COPYING.
 *If not, write to the Free Software Foundation,
 *Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *See COPYRIGHT file copyright information.
 */

#include <vector>
#include "nmv-exception.h"
#include "nmv-plugin.h"
#include "nmv-ui-utils.h"
#include "nmv-i-workbench.h"
#include "nmv-i-perspective.h"
#include "nmv-env.h"

using namespace std ;
using namespace nemiver ;
using namespace nemiver::common ;

namespace nemiver {

class Workbench : public IWorkbench {
    struct Priv ;
    SafePtr<Priv> m_priv ;

    Workbench (const Workbench&) ;
    Workbench& operator= (const Workbench&) ;


private:

    //************************
    //<slots (signal callbacks)>
    //************************
    void on_quit_menu_item_action () ;
    //************************
    //</slots (signal callbacks)>
    //************************

    void init_glade () ;
    void init_actions () ;
    void init_menubar () ;
    void init_toolbar () ;
    void init_body () ;
    void add_perspective_toolbars (IPerspective *a_perspective,
                                   list<Gtk::Toolbar*> &a_tbs) ;
    void add_perspective_body (IPerspective *a_perspective, Gtk::Widget *a_body) ;
    void select_perspective (IPerspective *a_perspective) ;

    void do_init () {}
    void shut_down () ;

public:
    Workbench () ;
    virtual ~Workbench () ;
    void get_info (Info &a_info) const ;
    void do_init (Gtk::Main &a_main) ;
    Glib::RefPtr<Gtk::ActionGroup> get_default_action_group () ;
    Glib::RefPtr<Gtk::ActionGroup> get_debugger_ready_action_group ();
    Gtk::Widget& get_menubar ();
    Gtk::Notebook& get_toolbar_container ();
    Gtk::Window& get_root_window () ;
    Glib::RefPtr<Gtk::UIManager>& get_ui_manager ()  ;
    IPerspective* get_perspective (const UString &a_name) ;
    map<UString, UString>& get_properties () ;
    sigc::signal<void>& shutting_down_signal () ;
};//end class Workbench

struct Workbench::Priv {
    bool initialized ;
    Gtk::Main *main ;
    Glib::RefPtr<Gtk::ActionGroup> default_action_group ;
    Glib::RefPtr<Gtk::UIManager> ui_manager ;
    Glib::RefPtr<Gnome::Glade::Xml> glade ;
    SafePtr <Gtk::Window> root_window ;
    Gtk::Widget *menubar ;
    Gtk::Notebook *toolbar_container;
    Gtk::Notebook *bodies_container ;
    PluginManagerSafePtr plugin_manager ;
    list<IPerspectiveSafePtr> perspectives ;
    map<IPerspective*, int> toolbars_index_map ;
    map<IPerspective*, int> bodies_index_map ;
    map<UString, UString> properties ;
    sigc::signal<void> shutting_down_signal ;

    Priv () :
        initialized (false),
        main (NULL),
        root_window (NULL),
        menubar (NULL),
        toolbar_container (NULL)
    {}
};//end Workbench::Priv

#ifndef CHECK_WB_INIT
#define CHECK_WB_INIT THROW_IF_FAIL(m_priv->initialized) ;
#endif

//****************
//private methods
//****************

//*********************
//signal slots methods
//*********************
void
Workbench::on_quit_menu_item_action ()
{
    shut_down () ;
}

Workbench::Workbench ()
{
    m_priv = new Priv ();
}

Workbench::~Workbench ()
{
    m_priv = NULL ;
}

void
Workbench::get_info (Info &a_info) const
{
    static Info s_info ("workbench",
                        "The workbench of Nemiver",
                        "1.0") ;
    a_info = s_info ;
}

void
Workbench::do_init (Gtk::Main &a_main)
{

    DynamicModule::Loader *loader = get_module_loader () ;
    THROW_IF_FAIL (loader) ;

    DynamicModuleManager *dynmod_manager = loader->get_dynamic_module_manager () ;
    THROW_IF_FAIL (dynmod_manager) ;

    m_priv->main = &a_main ;

    init_glade () ;
    init_actions () ;
    init_menubar () ;
    init_toolbar () ;
    init_body () ;

    m_priv->initialized = true ;

    m_priv->plugin_manager  = new PluginManager (*dynmod_manager) ;

    NEMIVER_TRY
        m_priv->plugin_manager->load_plugins () ;

        map<UString, PluginSafePtr>::const_iterator plugin_iter ;
        IPerspectiveSafePtr perspective ;
        Plugin::EntryPointSafePtr entry_point ;
        list<Gtk::Toolbar*> toolbars ;
        IWorkbenchSafePtr thiz (this, true) ;

        //**************************************************************
        //store the list of perspectives we may have loaded as plugins,
        //and init each of them.
        //**************************************************************
        for (plugin_iter = m_priv->plugin_manager->plugins_map ().begin () ;
             plugin_iter != m_priv->plugin_manager->plugins_map ().end ();
             ++plugin_iter) {
            if (plugin_iter->second && plugin_iter->second->entry_point_ptr ()) {
                entry_point = plugin_iter->second->entry_point_ptr () ;
                perspective = entry_point.do_dynamic_cast<IPerspective> () ;
                if (perspective) {
                    m_priv->perspectives.push_front (perspective) ;
                    perspective->do_init (thiz) ;
                    perspective->edit_workbench_menu () ;
                    toolbars.clear () ;
                    perspective->get_toolbars (toolbars) ;
                    add_perspective_toolbars (perspective, toolbars) ;
                    add_perspective_body (perspective, perspective->get_body ()) ;
                }
            }
        }

        if (!m_priv->perspectives.empty ()) {
            select_perspective (*m_priv->perspectives.begin ()) ;
        }
    NEMIVER_CATCH
}

void
Workbench::shut_down ()
{
    shutting_down_signal ().emit () ;
    m_priv->main->quit () ;
}

Glib::RefPtr<Gtk::ActionGroup>
Workbench::get_default_action_group ()
{
    CHECK_WB_INIT ;
    THROW_IF_FAIL (m_priv) ;
    return m_priv->default_action_group ;
}

Gtk::Widget&
Workbench::get_menubar ()
{
    CHECK_WB_INIT ;
    THROW_IF_FAIL (m_priv && m_priv->menubar) ;
    return *m_priv->menubar ;
}

Gtk::Notebook&
Workbench::get_toolbar_container ()
{
    CHECK_WB_INIT ;
    THROW_IF_FAIL (m_priv && m_priv->toolbar_container) ;
    return *m_priv->toolbar_container ;
}

Gtk::Window&
Workbench::get_root_window ()
{
    CHECK_WB_INIT ;
    THROW_IF_FAIL (m_priv && m_priv->root_window) ;

    return *m_priv->root_window ;
}

Glib::RefPtr<Gtk::UIManager>&
Workbench::get_ui_manager ()
{
    CHECK_WB_INIT ;
    THROW_IF_FAIL (m_priv && m_priv->ui_manager) ;
    return m_priv->ui_manager ;
}

IPerspective*
Workbench::get_perspective (const UString &a_name)
{
    list<IPerspectiveSafePtr>::const_iterator iter ;
    for (iter = m_priv->perspectives.begin ();
         iter != m_priv->perspectives.end ();
         ++iter) {
        if ((*iter)->descriptor ()->name () == a_name) {
            return iter->get () ;
        }
    }
    LOG_ERROR ("could not find perspective: '" << a_name << "'") ;
    return NULL;
}

map<UString, UString>&
Workbench::get_properties ()
{
    return m_priv->properties ;
}

sigc::signal<void>&
Workbench::shutting_down_signal ()
{
    THROW_IF_FAIL (m_priv) ;

    return m_priv->shutting_down_signal ;
}

void
Workbench::init_glade ()
{
    THROW_IF_FAIL (m_priv) ;
    UString file_path = env::build_path_to_glade_file ("workbench.glade") ;
    m_priv->glade = Gnome::Glade::Xml::create (file_path) ;
    THROW_IF_FAIL (m_priv->glade) ;

    m_priv->root_window =
       env::get_widget_from_glade<Gtk::Window> (m_priv->glade, "workbench");
    m_priv->root_window->hide () ;
}

void
Workbench::init_actions ()
{
    Gtk::StockID nil_stock_id ("") ;
    sigc::slot<void> nil_slot ;
    static ui_utils::ActionEntry s_default_action_entries [] = {
        {
            "FileMenuAction",
            nil_stock_id,
            "_File",
            "",
            nil_slot
        }
        ,
        {
            "QuitMenuItemAction",
            Gtk::Stock::QUIT,
            "_Quit",
            "Quit the application",
            sigc::mem_fun (*this, &Workbench::on_quit_menu_item_action)
        }
        ,
        {
            "HelpMenuAction",
            nil_stock_id,
            "_Help",
            "",
            nil_slot
        }
        ,
        {
            "AboutMenuItemAction",
            Gtk::Stock::ABOUT,
            "_About",
            "",
            nil_slot
        }
    };

    m_priv->default_action_group =
        Gtk::ActionGroup::create ("default-action-group") ;
    int num_default_actions =
         sizeof (s_default_action_entries)/sizeof (ui_utils::ActionEntry) ;
    ui_utils::add_action_entries_to_action_group (s_default_action_entries,
                                                  num_default_actions,
                                                  m_priv->default_action_group) ;
}

void
Workbench::init_menubar ()
{
    THROW_IF_FAIL (m_priv && m_priv->default_action_group) ;

    m_priv->ui_manager = Gtk::UIManager::create () ;

    UString file_path = env::build_path_to_menu_file ("menubar.xml") ;
    m_priv->ui_manager->insert_action_group (m_priv->default_action_group) ;

    m_priv->ui_manager->add_ui_from_file (file_path) ;

    m_priv->menubar = m_priv->ui_manager->get_widget ("/MenuBar") ;
    THROW_IF_FAIL (m_priv->menubar) ;

    Gtk::Box *menu_container =
        env::get_widget_from_glade<Gtk::Box> (m_priv->glade, "menucontainer");
    menu_container->pack_start (*m_priv->menubar) ;
    menu_container->show_all () ;
}

void
Workbench::init_toolbar ()
{
    m_priv->toolbar_container =
        env::get_widget_from_glade<Gtk::Notebook> (m_priv->glade,
                                                   "toolbarcontainer") ;
}

void
Workbench::init_body ()
{
    m_priv->bodies_container =
        env::get_widget_from_glade<Gtk::Notebook> (m_priv->glade,
                                                   "bodynotebook") ;
}

void
Workbench::add_perspective_toolbars (IPerspective *a_perspective,
                                     list<Gtk::Toolbar*> &a_tbs)
{
    if (a_tbs.empty ()) {return ;}

    SafePtr<Gtk::Box> box (Gtk::manage (new Gtk::VBox)) ;
    list<Gtk::Toolbar*>::const_iterator iter ;

    for (iter = a_tbs.begin (); iter != a_tbs.end () ; ++iter) {
        box->pack_start (**iter) ;
    }

    box->show_all () ;
    m_priv->toolbars_index_map [a_perspective] =
                    m_priv->toolbar_container->insert_page (*box, -1) ;

    box.release () ;
}

void
Workbench::add_perspective_body (IPerspective *a_perspective,
                                 Gtk::Widget *a_body)
{
    if (!a_body || !a_perspective) {return;}

    m_priv->bodies_index_map[a_perspective] =
        m_priv->bodies_container->insert_page (*a_body, -1);
}

void
Workbench::select_perspective (IPerspective *a_perspective)
{
    THROW_IF_FAIL (m_priv) ;
    THROW_IF_FAIL (m_priv->toolbar_container) ;
    THROW_IF_FAIL (m_priv->bodies_container) ;

    map<IPerspective*, int>::const_iterator iter, nil ;
    int toolbar_index=0, body_index=0 ;

    nil = m_priv->toolbars_index_map.end () ;
    iter = m_priv->toolbars_index_map.find (a_perspective) ;
    if (iter != nil) {
        toolbar_index = iter->second ;
    }

    nil = m_priv->bodies_index_map.end () ;
    iter = m_priv->bodies_index_map.find (a_perspective) ;
    if (iter != nil) {
        body_index = iter->second ;
    }

    m_priv->toolbar_container->set_current_page (toolbar_index) ;

    m_priv->bodies_container->set_current_page (body_index) ;
}

//the dynmod initial factory.
extern "C" {
bool
NEMIVER_API nemiver_common_create_dynamic_module_instance (void **a_new_instance)
{
    *a_new_instance = new nemiver::Workbench () ;
    return (*a_new_instance != 0) ;
}

}//end extern C

}//end namespace nemiver

