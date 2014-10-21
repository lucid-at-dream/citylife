#include "DebRender.hpp"

bool on_draw_event(const ::Cairo::RefPtr< ::Cairo::Context> &cr) {
    renderer->render(cr);
    return false;
}

void DebRenderer::render(Cairo::RefPtr<Cairo::Context> cr) {
    cr->set_source_rgb(0, 0, 0);
    cr->set_line_width(1.0);

    printf("rendering debug info\n");
    this->renderCallback->debugRender(cr);

    cr->stroke();
}

void DebRenderer::renderGeometry( Cairo::RefPtr<Cairo::Context> cr, GIMSGeometry *g ){
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

void DebRenderer::renderPoint ( Cairo::RefPtr<Cairo::Context> cr, GIMSPoint *p ) {
    //cr->set_source_rgb(cr, 0.69, 0.19, 0);
    printf("rendering point\n");
    cr->move_to((p->x+translatex)*scalex, (p->y+translatey)*scaley);
    cr->line_to((p->x+translatex)*scalex, (p->y+translatey)*scaley);
    cr->stroke();
}

void DebRenderer::renderEdge ( Cairo::RefPtr<Cairo::Context> cr, GIMSEdge *e ) {
    printf("rendering edge\n");
    cr->move_to((e->p1->x + translatex)*scalex, (e->p1->y + translatey)*scaley);
    cr->line_to((e->p2->x + translatex)*scalex, (e->p2->y + translatey)*scaley);
    cr->stroke();
}

void DebRenderer::renderBBox ( Cairo::RefPtr<Cairo::Context> cr, GIMSBoundingBox *box ) {
    printf("rendering box\n");
    cr->move_to((box->lowerLeft->x + translatex)*scalex,  (box->lowerLeft->y + translatey)*scaley );
    cr->line_to((box->lowerLeft->x + translatex)*scalex,  (box->upperRight->y + translatey)*scaley);
    cr->line_to((box->upperRight->x + translatex)*scalex, (box->upperRight->y + translatey)*scaley);
    cr->line_to((box->upperRight->x + translatex)*scalex, (box->lowerLeft->y + translatey)*scaley );
    cr->line_to((box->lowerLeft->x + translatex)*scalex,  (box->lowerLeft->y + translatey)*scaley );
    cr->stroke();
}

void DebRenderer::setScale (double x, double y) {
    scalex = x;
    scaley = y;
}

void DebRenderer::setTranslation (double x, double y) {
    translatex = x;
    translatey = y;
}

void DebRenderer::init( int argc, char *argv[] ){
    Gtk::Main kit(argc, argv);

    window = new Gtk::Window();
    darea = new Gtk::DrawingArea();
    window->add( *((Gtk::Widget *)darea) );

    //gtk_widget_add_events(window, GDK_BUTTON_PRESS_MASK);
    darea->signal_draw().connect(
        sigc::ptr_fun(&on_draw_event )
    );

    window->set_position( Gtk::WindowPosition::WIN_POS_CENTER );
    window->set_default_size(400, 400);
    darea->set_valign(Gtk::Align::ALIGN_CENTER);
    darea->set_halign(Gtk::Align::ALIGN_CENTER);
    darea->set_hexpand(true);
    darea->set_vexpand(true);
    darea->set_size_request(400, 400);
    window->set_title("GIMS Debug Visualization Tool");

    window->show_all();
}

int DebRenderer::mainloop ( int argc, char *argv[] ) {
    this->init(argc, argv);
    gtk_main();
    return 0;
}

DebRenderer::DebRenderer (DebugRenderable *renderCallback) {
    this->renderCallback = renderCallback;
}
DebRenderer::DebRenderer(){
}
DebRenderer::~DebRenderer(){
}

DebRenderer *renderer = NULL;
