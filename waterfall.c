/* waterfall.c */
/* gordon@gjcp.net */

#include <gtk/gtk.h>
#include <gtk/gtkadjustment.h>
#include <gtk/gtkmain.h>
#include "waterfall.h"

static GtkWidgetClass *parent_class = NULL;
G_DEFINE_TYPE (SDRWaterfall, sdr_waterfall, GTK_TYPE_DRAWING_AREA);

enum {
    PROP_0,
    PROP_TUNING,
    PROP_LP_TUNE,
    PROP_HP_TUNE
};


static gboolean sdr_waterfall_expose(GtkWidget *wf, GdkEventExpose *event);
static void sdr_waterfall_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);
static void sdr_waterfall_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec);

static void sdr_waterfall_class_init (SDRWaterfallClass *class) {
    GtkWidgetClass *widget_class;
    widget_class = GTK_WIDGET_CLASS(class);
    GObjectClass *gobject_class = G_OBJECT_CLASS (class);
    
    widget_class->expose_event = sdr_waterfall_expose;
    
    gobject_class->set_property = sdr_waterfall_set_property;
    gobject_class->get_property = sdr_waterfall_get_property;

    g_object_class_install_property (gobject_class,
        PROP_TUNING,
        g_param_spec_object ("tuning",
		"Tuning",
		"The GtkAdjustment for the VFO tuning",
        GTK_TYPE_ADJUSTMENT,
        G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
      
    g_object_class_install_property (gobject_class,
        PROP_LP_TUNE,
        g_param_spec_object ("lp_tune",
		"Lowpass Tune",
		"The GtkAdjustment for the lowpass tuning",
        GTK_TYPE_ADJUSTMENT,
        G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
        PROP_HP_TUNE,
        g_param_spec_object ("hp_tune",
		"Highpass Tune",
		"The GtkAdjustment for the highpass tuning",
        GTK_TYPE_ADJUSTMENT,
        G_PARAM_READWRITE | G_PARAM_CONSTRUCT));   
  
}

static void sdr_waterfall_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec) {
    SDRWaterfall *wf = SDR_WATERFALL(object);
    switch(property_id) {
        case PROP_TUNING:
            wf->tuning = g_value_get_object(value);
            break;
        case PROP_LP_TUNE:
            wf->lp_tune = g_value_get_object(value);
            break;
        case PROP_HP_TUNE:
            wf->hp_tune = g_value_get_object(value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;            
    }
}   
/* case PROP_ADJUSTMENT:
      gtk_range_set_adjustment (range, g_value_get_object (value));
      break;
*/

static void sdr_waterfall_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec) {

    SDRWaterfall *wf = SDR_WATERFALL(object);
    switch(property_id) {
        case PROP_TUNING:
            g_value_set_object (value, wf->tuning);
            break;
        case PROP_LP_TUNE:
            g_value_set_object (value, wf->lp_tune);
            break;
        case PROP_HP_TUNE:
            g_value_set_object (value, wf->hp_tune);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;            
    }
}


static void sdr_waterfall_init (SDRWaterfall *wf) {
}

GtkWidget *sdr_waterfall_new(GtkAdjustment *tuning, GtkAdjustment *lp_tune, GtkAdjustment *hp_tune) {
    return g_object_new(
        SDR_TYPE_WATERFALL,
        "tuning", tuning,
        "lp_tune", lp_tune,
        "hp_tune", hp_tune,
        NULL
    );
}

static gboolean sdr_waterfall_expose(GtkWidget *wf, GdkEventExpose *event) {
        cairo_t *cr;

        /* get a cairo_t */
        cr = gdk_cairo_create (wf->window);

        cairo_destroy (cr);

        return FALSE;


    return FALSE;
}
