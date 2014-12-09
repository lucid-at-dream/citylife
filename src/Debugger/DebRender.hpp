#ifndef _DebRender_HPP
#define _DebRender_HPP

#include "SystemBase.hpp"
#include "GIMSGeometry.hpp"
#include <cairomm/cairomm.h>
#include <cairo.h>
#include <gtkmm.h>
#include <cmath>
#include <functional>
#include <list>

using namespace std;
using namespace GIMSGEOMETRY;

class DebugRenderable {
  public:
    virtual void debugRender( Cairo::RefPtr<Cairo::Context> ) = 0;
    virtual void onClick(double x, double y) = 0;
};

class DebRenderer {

  private:
    Gtk::Window *window;
    Gtk::DrawingArea *darea;
    
    
    double scalex,
           scaley,
           translatex,
           translatey;
    
    int dragx,
        dragy;

    void init ( int argc, char *argv[] );

  public:
    unsigned int renderCount;
    DebugRenderable *renderCallback;
    double zoom;
    double panx, pany;

    void  renderSvg      (const char *filename, double width, double height);
    void  clickEvent     ();
    void  dragBegin      ();
    void  dragEnd        ();
    void  scheduleRedraw ();
    void  setScale       (double x, double y);
    void  setTranslation (double x, double y);
    void  mainloop       ();
    void  render         ( Cairo::RefPtr<Cairo::Context>  );
    void  renderGeometry ( Cairo::RefPtr<Cairo::Context>, GIMSGeometry * );
    void  renderPoint    ( Cairo::RefPtr<Cairo::Context>, GIMSPoint * );
    void  renderEdge     ( Cairo::RefPtr<Cairo::Context>, GIMSEdge *  );
    void  renderBBox     ( Cairo::RefPtr<Cairo::Context>, GIMSBoundingBox * );
    void  renderFilledBBox ( Cairo::RefPtr<Cairo::Context>, GIMSBoundingBox * );
    int   mainloop       ( int argc, char *argv[] );
          DebRenderer    ();
          DebRenderer    (DebugRenderable *);
         ~DebRenderer    ();
};

extern DebRenderer *renderer;
bool on_draw_event(const ::Cairo::RefPtr< ::Cairo::Context> &);
bool on_scroll_event(GdkEventScroll* event);

#endif
