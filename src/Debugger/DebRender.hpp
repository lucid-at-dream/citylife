#ifndef _DebRender_HPP
#define _DebRender_HPP

#include "SystemBase.hpp"
#include "GIMSGeometry.hpp"
#include <cairomm/cairomm.h>
#include <gtk/gtk.h>
#include <cmath>
#include <functional>

using namespace GIMSGEOMETRY;

class DebRenderer {

  private:
    GtkWidget *window;
    GtkWidget *darea;

    void init ( int argc, char *argv[] );

  public:
    std::function<void(cairo_t *)> renderCallback;

    void  mainloop       ();
    void  render         ( cairo_t *, GtkWidget *);
    void  renderGeometry ( cairo_t *cr, GIMSGeometry *g );
    void  renderPoint    ( cairo_t *cr, GIMSPoint * );
    void  renderEdge     ( cairo_t *cr, GIMSEdge *  );
    void  renderBBox     ( cairo_t *cr, GIMSBoundingBox *box );
    int   mainloop       ( int argc, char *argv[] );
          DebRenderer    (std::function<void(cairo_t *)>);
         ~DebRenderer    ();
};

extern DebRenderer *renderer;
gboolean on_draw_event(GtkWidget *, cairo_t *, gpointer);

#endif
