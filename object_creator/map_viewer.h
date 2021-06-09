#ifndef CATA_OBJECT_CREATOR_MAP_VIEWER_H
#define CATA_OBJECT_CREATOR_MAP_VIEWER_H

#include <QtCore/qtimer.h>
#include <QtWidgets/qmainwindow.h>
#include <QtWidgets/qtextedit.h>
#include <QtWidgets/qwidget.h>
#include <SDL2/SDL.h>

namespace creator
{
class map_viewer_window : public QMainWindow
{
    Q_OBJECT
    public:
        map_viewer_window( QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags() );

        ~map_viewer_window();

        void SDL_init();
        void set_window( SDL_Window *win );
        void set_renderer( SDL_Renderer *renderer );
        SDL_Window *get_window();
        SDL_Renderer *get_renderer();

    private:
        QTextEdit map_json;

        QWidget *mapper;
        SDL_Window *win;
        SDL_Renderer *renderer;
        QTimer *time;
        int position;
        int dir;
    Q_SLOTS
        void Render();

};
}

#endif
