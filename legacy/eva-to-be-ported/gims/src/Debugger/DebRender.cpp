#include "DebRender.hpp"

bool on_drag_begin(GdkEventButton *event)
{
    renderer.dragBegin();
    return false;
}

bool on_drag_end(GdkEventButton *event)
{
    double prev_panx = renderer.panx, prev_pany = renderer.pany;
    renderer.dragEnd();
    if (prev_panx == renderer.panx && prev_pany == renderer.pany)
        renderer.clickEvent();

    renderer.scheduleRedraw();
    return false;
}

bool on_scroll_event(GdkEventScroll *event)
{
    double zinc = fabs(0.1 * renderer.zoom);
    if (event->direction == GDK_SCROLL_UP)
    {
        renderer.zoom += zinc;
    }
    else if (event->direction == GDK_SCROLL_DOWN)
    {
        renderer.zoom -= zinc;
    }
    renderer.scheduleRedraw();
    return false;
}

bool on_draw_event(const ::Cairo::RefPtr< ::Cairo::Context> &cr)
{
    cr->translate(-200 * (renderer.zoom - 1), -200 * (renderer.zoom - 1));
    cr->scale(renderer.zoom, renderer.zoom);
    cr->translate(renderer.panx, renderer.pany);

    renderer.render(cr);
    return false;
}

void DebRenderer::dragBegin()
{
    int x, y;
    darea->get_pointer(x, y);
    dragx = x;
    dragy = y;
}

void DebRenderer::dragEnd()
{
    int x, y;
    darea->get_pointer(x, y);
    int relx = x - dragx, rely = y - dragy;
    panx += relx / renderer.zoom;
    pany += rely / renderer.zoom;
}

void DebRenderer::clickEvent()
{
    int p_x, p_y;
    darea->get_pointer(p_x, p_y);

    double rootx = (p_x / zoom) + ((200 * (zoom - 1)) / zoom) - panx, rooty = (p_y / zoom) + ((200 * (zoom - 1)) / zoom) - pany;

    double x = rootx / scalex - translatex, y = rooty / scaley - translatey;

    renderCallback->onClick(x, y);
}

void DebRenderer::scheduleRedraw()
{
    this->darea->queue_draw_area(0, 0, 400, 400);
}

void DebRenderer::renderSvg(const char *filename, double width, double height)
{
    cairo_surface_t *surface = cairo_svg_surface_create(filename, width, height);
    Cairo::Context *cr = new Cairo::Context(cairo_create(surface));
    Cairo::RefPtr<Cairo::Context> cc_ptr = Cairo::RefPtr<Cairo::Context>(cr);

    this->render(cc_ptr);

    cc_ptr->show_page();
    cairo_surface_flush(surface);
    cairo_surface_finish(surface);
    cairo_surface_destroy(surface);
}

void DebRenderer::render(Cairo::RefPtr<Cairo::Context> cr)
{
    this->renderCount += 1;

    cr->set_source_rgb(0, 0, 0);
    cr->set_line_width(1.0 / zoom);

    this->renderCallback->debugRender(cr);

    cr->stroke();
}

void DebRenderer::renderGeometry(Cairo::RefPtr<Cairo::Context> cr, GIMS_Geometry *g)
{
    if (g == NULL)
        return;

    switch (g->type)
    {
    case BOUNDINGBOX:
        renderBBox(cr, (GIMS_BoundingBox *)g);
        break;
    case POINT:
        renderPoint(cr, (GIMS_Point *)g);
        break;
    case LINESTRING:
        renderLineString(cr, (GIMS_LineString *)g);
        break;
    case RING:
        renderRing(cr, (GIMS_Ring *)g);
        break;
    case POLYGON:
        renderPolygon(cr, (GIMS_Polygon *)g);
        break;
    case MULTIPOINT:
        renderMultiPoint(cr, (GIMS_MultiPoint *)g);
        break;
    case MULTILINESTRING:
        renderMultiLineString(cr, (GIMS_MultiLineString *)g);
        break;
    case MULTIPOLYGON: //not supported yet
        renderMultiPolygon(cr, (GIMS_MultiPolygon *)g);
        break;
    case GEOMETRYCOLLECTION:
        renderGeometryCollection(cr, (GIMS_GeometryCollection *)g);
        break;
    case LINESEGMENT:
        renderLineSegment(cr, (GIMS_LineSegment *)g);
        break;
    default:
        fprintf(stderr, "tryed to render a geometry with unknown type. Not Rendering!\n");
    };
}

void DebRenderer::renderPoint(Cairo::RefPtr<Cairo::Context> cr, GIMS_Point *p)
{
    if (p == NULL)
        return;
    double x = (p->x + translatex) * scalex, y = (p->y + translatey) * scaley;
    cr->stroke();
    cr->arc(x, y, 2.5 / zoom, 0.0, 2 * M_PI);
    cr->stroke_preserve();
    cr->fill();
    cr->stroke();
}

void DebRenderer::renderLineString(Cairo::RefPtr<Cairo::Context> cr, GIMS_LineString *ls)
{
    if (ls == NULL)
        return;
    cr->move_to((ls->list[0]->x + translatex) * scalex, (ls->list[0]->y + translatey) * scaley);
    for (int i = 1; i < ls->size; i++)
        cr->line_to((ls->list[i]->x + translatex) * scalex, (ls->list[i]->y + translatey) * scaley);
}

void DebRenderer::renderFilledBBox(Cairo::RefPtr<Cairo::Context> cr, GIMS_BoundingBox *box)
{
    if (box == NULL)
        return;
    renderBBox(cr, box);
    cr->stroke_preserve();
    cr->fill();
}

void DebRenderer::renderBBox(Cairo::RefPtr<Cairo::Context> cr, GIMS_BoundingBox *box)
{
    if (box == NULL)
        return;
    cr->move_to((box->lowerLeft->x + translatex) * scalex, (box->lowerLeft->y + translatey) * scaley);
    cr->line_to((box->lowerLeft->x + translatex) * scalex, (box->upperRight->y + translatey) * scaley);
    cr->line_to((box->upperRight->x + translatex) * scalex, (box->upperRight->y + translatey) * scaley);
    cr->line_to((box->upperRight->x + translatex) * scalex, (box->lowerLeft->y + translatey) * scaley);
    cr->line_to((box->lowerLeft->x + translatex) * scalex, (box->lowerLeft->y + translatey) * scaley);
}

void DebRenderer::renderRing(Cairo::RefPtr<Cairo::Context> cr, GIMS_Ring *ring)
{
    if (ring == NULL)
        return;
    cr->move_to((ring->list[0]->x + translatex) * scalex, (ring->list[0]->y + translatey) * scaley);
    for (int i = 1; i < ring->size; i++)
        cr->line_to((ring->list[i]->x + translatex) * scalex, (ring->list[i]->y + translatey) * scaley);
    cr->line_to((ring->list[0]->x + translatex) * scalex, (ring->list[0]->y + translatey) * scaley);
}

void DebRenderer::renderApproximation(Cairo::RefPtr<Cairo::Context> cr, GIMS_Approximation *appr)
{
    if (appr == NULL)
        return;

    GIMS_Point **hull = appr->hull;
    int N = appr->N;

    if (N <= 2)
        return;

    cr->set_source_rgb(255, 0, 0);
    cr->move_to((hull[0]->x + translatex) * scalex, (hull[0]->y + translatey) * scaley);
    for (int i = 1; i < N; i++)
    {
        cr->line_to((hull[i]->x + translatex) * scalex, (hull[i]->y + translatey) * scaley);
    }
    cr->line_to((hull[0]->x + translatex) * scalex, (hull[0]->y + translatey) * scaley);
    cr->stroke();
    cr->set_source_rgb(0, 0, 0);
}

void DebRenderer::renderPolygon(Cairo::RefPtr<Cairo::Context> cr, GIMS_Polygon *g)
{
    if (g == NULL)
        return;
    this->renderMultiLineString(cr, g->externalRing);
    cr->stroke();
    cr->set_source_rgb(255, 0, 0);
    this->renderMultiLineString(cr, g->internalRings);
    cr->stroke();
    cr->set_source_rgb(0, 0, 0);
}

void DebRenderer::renderMultiPoint(Cairo::RefPtr<Cairo::Context> cr, GIMS_MultiPoint *g)
{
    if (g == NULL)
        return;
    for (int i = 0; i < g->size; i++)
        this->renderPoint(cr, g->list[i]);
}

void DebRenderer::renderMultiLineString(Cairo::RefPtr<Cairo::Context> cr, GIMS_MultiLineString *g)
{
    if (g == NULL)
        return;
    for (int i = 0; i < g->size; i++)
        this->renderLineString(cr, g->list[i]);
}

void DebRenderer::renderMultiPolygon(Cairo::RefPtr<Cairo::Context> cr, GIMS_MultiPolygon *g)
{
    if (g == NULL)
        return;
    for (int i = 0; i < g->size; i++)
        this->renderPolygon(cr, g->list[i]);
}

void DebRenderer::renderGeometryCollection(Cairo::RefPtr<Cairo::Context> cr, GIMS_GeometryCollection *g)
{
    if (g == NULL)
        return;
    for (int i = 0; i < g->size; i++)
        this->renderGeometry(cr, g->list[i]);
}

void DebRenderer::renderLineSegment(Cairo::RefPtr<Cairo::Context> cr, GIMS_LineSegment *g)
{
    if (g == NULL)
        return;
    cr->move_to((g->p1->x + translatex) * scalex, (g->p1->y + translatey) * scaley);
    cr->line_to((g->p2->x + translatex) * scalex, (g->p2->y + translatey) * scaley);
}

void DebRenderer::setScale(double x, double y)
{
    scalex = x;
    scaley = y;
}

void DebRenderer::setTranslation(double x, double y)
{
    translatex = x;
    translatey = y;
}

void DebRenderer::init(int argc, char *argv[])
{
    Gtk::Main kit(argc, argv);

    window = new Gtk::Window();
    darea = new Gtk::DrawingArea();
    window->add(*((Gtk::Widget *)darea));

    darea->signal_draw().connect(sigc::ptr_fun(&on_draw_event));

    darea->add_events(Gdk::SCROLL_MASK);
    darea->add_events(Gdk::BUTTON_PRESS_MASK);
    darea->add_events(Gdk::BUTTON_RELEASE_MASK);
    darea->signal_scroll_event().connect(sigc::ptr_fun(&on_scroll_event));

    darea->signal_button_press_event().connect(sigc::ptr_fun(&on_drag_begin));
    darea->signal_button_release_event().connect(sigc::ptr_fun(&on_drag_end));

    window->set_position(Gtk::WindowPosition::WIN_POS_CENTER);
    window->set_default_size(400, 400);
    darea->set_valign(Gtk::Align::ALIGN_CENTER);
    darea->set_halign(Gtk::Align::ALIGN_CENTER);
    darea->set_hexpand(true);
    darea->set_vexpand(true);
    darea->set_size_request(400, 400);
    window->set_title("GIMS_ Debug Visualization Tool");

    window->show_all();
}

int DebRenderer::mainloop(int argc, char *argv[])
{
    this->init(argc, argv);
    gtk_main();
    return 0;
}

DebRenderer::DebRenderer(DebugRenderable *renderCallback)
{
    this->renderCallback = renderCallback;
    this->zoom = 1.0;
    this->panx = this->pany = 0;
    this->renderCount = 0;
}
DebRenderer::DebRenderer()
{
    this->zoom = 1.0;
    this->panx = this->pany = 0;
    this->renderCount = 0;
}
DebRenderer::~DebRenderer()
{
}

DebRenderer renderer;
