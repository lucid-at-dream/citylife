#include "DebRender.hpp"

gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    renderer->render(cr, widget);
    return FALSE;
}

void DebRenderer::render(cairo_t *cr, GtkWidget *widget) {
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 0.5);

    this->renderCallback(cr);

    cairo_stroke(cr);
}

void DebRenderer::renderGeometry( cairo_t *cr, GIMSGeometry *g ){
    if ( g->type == POINT ) {
        renderPoint( cr, (GIMSPoint *)g );
        return;

    } else if ( g->type == EDGE ) {
        renderEdge( cr, (GIMSEdge *)g );
        return;

    } else if ( g->type == BOUNDINGBOX ) {
        renderBBox( cr, (GIMSBoundingBox *)g );
        return;

    } else if ( g->type == MIXEDLIST ) {
        for( std::list<GIMSGeometry *>::iterator it = ((GIMSGeometryList *)g)->list->begin();
             it != ((GIMSGeometryList *)g)->list->end(); it++ ) {
            renderGeometry( cr, *it );
        }
    } else {
        fprintf(stderr, "tryed to render unsupported geometry type. Quiting.\n");
        exit(-1);
    }
}

void DebRenderer::renderPoint ( cairo_t *cr, GIMSPoint *p ) {
    //cairo_set_source_rgb(cr, 0.69, 0.19, 0);
    cairo_move_to(cr, p->x, p->y);
    cairo_line_to(cr, p->x, p->y);
}

void DebRenderer::renderEdge ( cairo_t *cr, GIMSEdge *e ) {
    cairo_move_to(cr, e->p1->x, e->p1->y);
    cairo_line_to(cr, e->p2->x, e->p2->y);
}

void DebRenderer::renderBBox ( cairo_t *cr, GIMSBoundingBox *box ) {
    cairo_move_to(cr, box->lowerLeft->x,  box->lowerLeft->y );
    cairo_line_to(cr, box->lowerLeft->x,  box->upperRight->y);
    cairo_line_to(cr, box->upperRight->x, box->upperRight->y);
    cairo_line_to(cr, box->upperRight->x, box->lowerLeft->y );
    cairo_line_to(cr, box->lowerLeft->x,  box->lowerLeft->y );
}

void DebRenderer::init( int argc, char *argv[] ){
    gtk_init ( &argc, &argv );

    this->window = gtk_window_new ( GTK_WINDOW_TOPLEVEL );
    this->darea = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(this->window), this->darea);

    gtk_widget_add_events(this->window, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(G_OBJECT(this->darea), "draw",
        G_CALLBACK(on_draw_event), NULL);
    g_signal_connect(this->window, "destroy",
        G_CALLBACK(gtk_main_quit), NULL);
    /*g_signal_connect(this->window, "button-press-event", 
        G_CALLBACK(clicked), NULL);
    */
    gtk_window_set_position(GTK_WINDOW(this->window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(this->window), 500, 500); 
    gtk_window_set_title(GTK_WINDOW(this->window), "GIMS Debug Visualization Tool");

    gtk_widget_show_all(this->window);
}

int DebRenderer::mainloop ( int argc, char *argv[] ) {
    this->init(argc, argv);
    gtk_main();
    return 0;
}

DebRenderer::DebRenderer (std::function<void(cairo_t *)>) {
    this->renderCallback = renderCallback;
}

DebRenderer::~DebRenderer(){
}

DebRenderer *renderer = NULL;
