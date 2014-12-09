#include "DebRender.hpp"

bool on_drag_begin(GdkEventButton* event) {
    printf("began drag\n");
    renderer->dragBegin ( );
    return false;
}

bool on_drag_end(GdkEventButton *event) {
    printf("end drag\n");
    double prev_panx = renderer->panx,
           prev_pany = renderer->pany;
    renderer->dragEnd( );
    if( prev_panx == renderer->panx && prev_pany == renderer->pany )
        renderer->clickEvent();
    
    renderer->scheduleRedraw();
    return false;
}

bool on_scroll_event(GdkEventScroll* event) {
    double zinc = fabs( 0.1 * renderer->zoom );
    if( event->direction == GDK_SCROLL_UP ) {
        renderer->zoom += zinc;
    } else if ( event->direction == GDK_SCROLL_DOWN ) {
        renderer->zoom -= zinc;
    }
    renderer->scheduleRedraw();
    return false;
}

bool on_draw_event(const ::Cairo::RefPtr< ::Cairo::Context> &cr) {
    cr->translate( -200 * (renderer->zoom-1), -200 * (renderer->zoom-1) );
    cr->scale( renderer->zoom, renderer->zoom );
    cr->translate( renderer->panx, renderer->pany );

    renderer->render(cr);
    return false;
}

void DebRenderer::dragBegin (){
    int x, y;
    darea->get_pointer ( x, y );
    dragx = x;
    dragy = y;
}

void DebRenderer::dragEnd (){
    int x, y;
    darea->get_pointer ( x, y );
    int relx = x - dragx,
        rely = y - dragy;
    panx += relx / renderer->zoom;
    pany += rely / renderer->zoom;
}

void DebRenderer::clickEvent(){
    int p_x, p_y;
    darea->get_pointer ( p_x, p_y );

    double rootx = (p_x/zoom) + ((200 * (zoom-1)) / zoom) - panx,
           rooty = (p_y/zoom) + ((200 * (zoom-1)) / zoom) - pany;

    double x = rootx/scalex - translatex,
           y = rooty/scaley - translatey;

    renderCallback->onClick(x,y);
}

void DebRenderer::scheduleRedraw () {
    this->darea->queue_draw_area (0, 0, 400, 400);
}


void DebRenderer::renderSvg (const char *filename, double width, double height){
    cairo_surface_t * surface = cairo_svg_surface_create (filename, width, height);
    Cairo::Context *cr = new Cairo::Context(cairo_create(surface));
    Cairo::RefPtr<Cairo::Context> cc_ptr = Cairo::RefPtr<Cairo::Context>(cr);

    this->render(cc_ptr);

    cc_ptr->show_page();
    cairo_surface_flush(surface);
    cairo_surface_finish(surface);
    cairo_surface_destroy(surface);
}

void DebRenderer::render ( Cairo::RefPtr<Cairo::Context> cr ) {
    this->renderCount += 1;

    cr->set_source_rgb(0, 0, 0);
    cr->set_line_width(1.0/zoom);

    this->renderCallback->debugRender(cr);

    cr->stroke();
}


void DebRenderer::renderGeometry( Cairo::RefPtr<Cairo::Context> cr, GIMSGeometry *g ){
    if ( g->type == POINT ) {
        renderPoint( cr, (GIMSPoint *)g );
        cr->stroke();
        return;

    } else if ( g->type == EDGE ) {
        renderEdge( cr, (GIMSEdge *)g );
        cr->stroke();
        return;

    } else if ( g->type == BOUNDINGBOX ) {
        renderBBox( cr, (GIMSBoundingBox *)g );
        cr->stroke();
        return;

    } else if ( g->type == MIXEDLIST ) {
        for( std::list<GIMSGeometry *>::iterator it = ((GIMSGeometryList *)g)->list->begin();
             it != ((GIMSGeometryList *)g)->list->end(); it++ ) {
            renderGeometry( cr, *it );
        }
    } else if ( g->type == POLYGON ){
        GIMSPolygon *p = (GIMSPolygon *)g;
        renderGeometry(cr, p->externalRing);
        for(list<GIMSGeometry *>::iterator it = p->internalRings->list->begin();
            it != p->internalRings->list->end(); it++)
            renderGeometry(cr, *it);
    }else{
        fprintf(stderr, "tryed to render unsupported geometry type. Quiting.\n");
    }
}

void DebRenderer::renderPoint ( Cairo::RefPtr<Cairo::Context> cr, GIMSPoint *p ) {
    double x = (p->x+translatex)*scalex,
           y = (p->y+translatey)*scaley;

    cr->arc(x, y, 3.0/zoom, 0., 2*M_PI);
    cr->stroke_preserve();
    cr->fill();
}

void DebRenderer::renderEdge ( Cairo::RefPtr<Cairo::Context> cr, GIMSEdge *e ) {
    cr->move_to((e->p1->x + translatex)*scalex, (e->p1->y + translatey)*scaley);
    cr->line_to((e->p2->x + translatex)*scalex, (e->p2->y + translatey)*scaley);
}

void DebRenderer::renderFilledBBox ( Cairo::RefPtr<Cairo::Context> cr, GIMSBoundingBox *box ) {
    renderBBox(cr, box);
    cr->stroke_preserve();
}

void DebRenderer::renderBBox ( Cairo::RefPtr<Cairo::Context> cr, GIMSBoundingBox *box ) {
    //printf("rendering box\n");
    cr->move_to((box->lowerLeft->x + translatex)*scalex,  (box->lowerLeft->y + translatey)*scaley );
    cr->line_to((box->lowerLeft->x + translatex)*scalex,  (box->upperRight->y + translatey)*scaley);
    cr->line_to((box->upperRight->x + translatex)*scalex, (box->upperRight->y + translatey)*scaley);
    cr->line_to((box->upperRight->x + translatex)*scalex, (box->lowerLeft->y + translatey)*scaley );
    cr->line_to((box->lowerLeft->x + translatex)*scalex,  (box->lowerLeft->y + translatey)*scaley );
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

    darea->signal_draw().connect( sigc::ptr_fun( &on_draw_event ) );

    darea->add_events(Gdk::SCROLL_MASK);
    darea->add_events(Gdk::BUTTON_PRESS_MASK);
    darea->add_events(Gdk::BUTTON_RELEASE_MASK);
    darea->signal_scroll_event().connect( sigc::ptr_fun( &on_scroll_event ) );

    darea->signal_button_press_event().connect( sigc::ptr_fun( &on_drag_begin ) );
    darea->signal_button_release_event().connect( sigc::ptr_fun( &on_drag_end ) );

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
    this->zoom = 1.0;
    this->panx = this->pany = 0;
    this->renderCount = 0;
}
DebRenderer::DebRenderer(){
    this->zoom = 1.0;
    this->panx = this->pany = 0;
    this->renderCount = 0;
}
DebRenderer::~DebRenderer(){
}

DebRenderer *renderer = NULL;
