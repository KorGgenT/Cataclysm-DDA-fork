#include "map_viewer.h"

#include "mapgen_factory.h"

creator::map_viewer_window::map_viewer_window( QWidget *parent, Qt::WindowFlags flags )
    : QMainWindow( parent, flags )
{
    map_json.resize( QSize( 800, 600 ) );
    map_json.setReadOnly( true );

    time = new QTimer( this );
    connect( time, SIGNAL( timeout() ), this, SLOT( Render() ) );
    time->start( 1000 / 60 );

    renderer = 0;
    win = 0;
    position = 0;
    dir = 1;
}

creator::map_viewer_window::~map_viewer_window()
{
    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( win );

    delete time;

    renderer = 0;
    win = 0;
    time = 0;
}

void creator::map_viewer_window::SDL_init()
{
    set_window( SDL_CreateWindowFrom( (void *)centralWidget()->winId() ) );
    set_renderer( SDL_CreateRenderer( get_window(), -1, SDL_RENDERER_ACCELERATED ) );
}

void creator::map_viewer_window::Render()
{
    // Basic square bouncing animation
    SDL_Rect spos;
    spos.h = 100;
    spos.w = 100;
    spos.y = height() / 2 - 50;
    spos.x = position;

    SDL_SetRenderDrawColor( renderer, 0, 0, 0, 255 );
    SDL_RenderFillRect( renderer, 0 );
    SDL_SetRenderDrawColor( renderer, 0xFF, 0x0, 0x0, 0xFF );
    SDL_RenderFillRect( renderer, &spos );
    SDL_RenderPresent( renderer );

    if( position >= width() - 100 )
        dir = 0;
    else if( position <= 0 )
        dir = 1;

    if( dir )
        position += 5;
    else
        position -= 5;
}

void creator::map_viewer_window::set_window( SDL_Window *ref )
{
    win = ref;
}

void creator::map_viewer_window::set_renderer( SDL_Renderer *ref )
{
    renderer = ref;
}

SDL_Window *creator::map_viewer_window::get_window()
{
    return win;
}

SDL_Renderer *creator::map_viewer_window::get_renderer()
{
    return renderer;
}
