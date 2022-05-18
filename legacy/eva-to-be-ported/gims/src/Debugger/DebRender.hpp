#ifndef _DebRender_HPP
#define _DebRender_HPP

#include "Geometry.hpp"
#include "SystemBase.hpp"
#include <cairo.h>
#include <cairomm/cairomm.h>
#include <cmath>
#include <functional>
#include <gtkmm.h>
#include <list>

using namespace std;
using namespace GIMS_GEOMETRY;

class DebugRenderable
{
public:
    virtual void debugRender(Cairo::RefPtr<Cairo::Context>) = 0;
    virtual void onClick(double x, double y) = 0;
};

class DebRenderer
{
private:
    Gtk::Window *window;
    Gtk::DrawingArea *darea;

    double scalex, scaley, translatex, translatey;

    int dragx, dragy;

    void init(int argc, char *argv[]);

public:
    unsigned int renderCount;
    DebugRenderable *renderCallback;
    double zoom;
    double panx, pany;

    void clickEvent();
    void dragBegin();
    void dragEnd();
    void scheduleRedraw();
    void setScale(double x, double y);
    void setTranslation(double x, double y);
    void render(Cairo::RefPtr<Cairo::Context>);
    void renderSvg(const char *filename, double width, double height);

    void renderApproximation(Cairo::RefPtr<Cairo::Context>, GIMS_Approximation *);
    void renderGeometry(Cairo::RefPtr<Cairo::Context>, GIMS_Geometry *);
    void renderBBox(Cairo::RefPtr<Cairo::Context>, GIMS_BoundingBox *);
    void renderPoint(Cairo::RefPtr<Cairo::Context>, GIMS_Point *);
    void renderLineString(Cairo::RefPtr<Cairo::Context>, GIMS_LineString *);
    void renderRing(Cairo::RefPtr<Cairo::Context>, GIMS_Ring *);
    void renderPolygon(Cairo::RefPtr<Cairo::Context>, GIMS_Polygon *);
    void renderMultiPoint(Cairo::RefPtr<Cairo::Context>, GIMS_MultiPoint *);
    void renderMultiLineString(Cairo::RefPtr<Cairo::Context>, GIMS_MultiLineString *);
    void renderMultiPolygon(Cairo::RefPtr<Cairo::Context>, GIMS_MultiPolygon *);
    void renderGeometryCollection(Cairo::RefPtr<Cairo::Context>, GIMS_GeometryCollection *);
    void renderLineSegment(Cairo::RefPtr<Cairo::Context>, GIMS_LineSegment *);
    void renderFilledBBox(Cairo::RefPtr<Cairo::Context>, GIMS_BoundingBox *);

    int mainloop(int argc, char *argv[]);
    DebRenderer();
    DebRenderer(DebugRenderable *);
    ~DebRenderer();
};

extern DebRenderer renderer;
bool on_draw_event(const ::Cairo::RefPtr< ::Cairo::Context> &);
bool on_scroll_event(GdkEventScroll *event);

#endif
