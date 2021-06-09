#include "creator_main_window.h"

#include "spell_window.h"
#include "enum_conversions.h"
#include "map_viewer.h"
#include "translations.h"

#include <QtWidgets/qapplication.h>
#include <QtWidgets/qmainwindow.h>
#include <QtWidgets/qpushbutton.h>

namespace io
{
    // *INDENT-OFF*
    template<>
    std::string enum_to_string<creator::jsobj_type>( creator::jsobj_type data )
    {
        switch( data ) {
        case creator::jsobj_type::SPELL: return "Spell";
        case creator::jsobj_type::LAST: break;
        }
        debugmsg( "Invalid valid_target" );
        abort();
    }
    // *INDENT-ON*

} // namespace io

int creator::main_window::execute( QApplication &app )
{
    QMainWindow title_menu;

    spell_window spell_editor( &title_menu );
    map_viewer_window map_editor( &title_menu );

    QPushButton spell_button( _( "Spell Creator" ), &title_menu );
    spell_button.resize( QSize( 100, 20 ) );
    QPushButton map_editor_button( _( "Map Editor" ), &title_menu );
    map_editor_button.resize( QSize( 100, 20 ) );
    map_editor_button.move( QPoint( 0, 20 ) );

    QObject::connect( &spell_button, &QPushButton::released,
    [&]() {
        title_menu.hide();
        spell_editor.show();
    } );

    QObject::connect( &map_editor_button, &QPushButton::released,
        [&]() {
        title_menu.hide();
        map_editor.show();
    } );

    title_menu.show();
    spell_button.show();

    return app.exec();
}
