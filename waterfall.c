/* waterfall.c */
/* gordon@gjcp.net */

#include <gtk/gtk.h>
#include <gtk/gtkadjustment.h>
#include <gtk/gtkmain.h>
#include <stdlib.h>
#include <string.h>

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
static void sdr_waterfall_size_allocate(GtkWidget *widget, GtkAllocation *allocation);
static void sdr_waterfall_send_configure(GtkWidget *widget);

static void sdr_waterfall_class_init (SDRWaterfallClass *class) {
    GtkWidgetClass *widget_class;
    widget_class = GTK_WIDGET_CLASS(class);
    GObjectClass *gobject_class = G_OBJECT_CLASS (class);
    
    widget_class->expose_event = sdr_waterfall_expose;
    
    gobject_class->set_property = sdr_waterfall_set_property;
    gobject_class->get_property = sdr_waterfall_get_property;
    widget_class->size_allocate = sdr_waterfall_size_allocate;

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
    // stub
}

static void sdr_waterfall_size_allocate(GtkWidget *widget, GtkAllocation *allocation) {
	SDRWaterfall *wf = SDR_WATERFALL(widget);
	gint width, height, oldsize, newsize;
	
	guchar *newpixels;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(SDR_IS_WATERFALL(widget));
	g_return_if_fail(allocation != NULL);

	width = allocation->width;
	height = allocation->height;


	widget->allocation = *allocation;
	widget->allocation.width = width;
	widget->allocation.height = height;
	
	
	newsize = width * height * 4;
	oldsize = wf->pixelsize;
	
	//newpixels = g_new0(guchar, newsize);
    
    /*
    if (wf->pixels) {
        // scale old image to new one (copy for now)
        memcpy(wf->pixels, newpixels, MIN(oldsize, newsize));
        free(wf->pixels);
        wf->pixels = NULL;
        wf->pixelsize = newsize;        
    }
    */
	if (!wf->pixels) {
       	newpixels = g_new0(guchar, newsize);
        wf->pixels = newpixels;
	}
	
	if (GTK_WIDGET_REALIZED(widget)) {
    	gdk_window_move_resize(widget->window,
	        allocation->x, allocation->y,
	        allocation->width, allocation->height);
        sdr_waterfall_send_configure(widget);

	}

    printf("wf->pixels = %x\n", wf->pixels);
    printf("sdr_waterfall_size_allocate: %d pixels\n", width*height);
}

static void sdr_waterfall_send_configure(GtkWidget *widget) {
	GdkEventConfigure event;

	event.type = GDK_CONFIGURE;
	event.window = widget->window;
	event.send_event = TRUE;
	event.x = widget->allocation.x;
	event.y = widget->allocation.y;
	event.width = widget->allocation.width;
	event.height = widget->allocation.height;
  
	gtk_widget_event(widget, (GdkEvent*)&event);
}


GtkWidget *sdr_waterfall_new(GtkAdjustment *tuning, GtkAdjustment *lp_tune, GtkAdjustment *hp_tune) {
    // call this with three Adjustments, for tuning, lowpass filter and highpass filter
    // the tuning Adjustment should have its upper and lower bounds set to half the sample rate
    return g_object_new(
        SDR_TYPE_WATERFALL,
        "tuning", tuning,
        "lp_tune", lp_tune,
        "hp_tune", hp_tune,
        NULL
    );
}

void sdr_waterfall_update(GtkWidget *widget, guchar row[]) {
    SDRWaterfall *wf = SDR_WATERFALL(widget);
    int width = widget->allocation.width;
    int height = MIN(widget->allocation.height, 150);
    memmove(wf->pixels + 2048 * wf->pixelrow, row, 2048);    // FIXME
    wf->pixelrow++;
    if (wf->pixelrow > height) wf->pixelrow=0;
    gtk_widget_queue_draw(widget);
}

static gboolean sdr_waterfall_expose(GtkWidget *widget, GdkEventExpose *event) {

    SDRWaterfall *wf = SDR_WATERFALL(widget);
    int width = widget->allocation.width;
    //int height = widget->allocation.height;
        int height = MIN(widget->allocation.height, 150);
    cairo_t *cr = gdk_cairo_create (widget->window);
    cairo_surface_t *pix;
    
    float tuning = GTK_ADJUSTMENT(wf->tuning)->value;
    
    // scale in pixels per Hz
    float pixscale = width/(GTK_ADJUSTMENT(wf->tuning)->upper*2);
    int cursor = (tuning + GTK_ADJUSTMENT(wf->tuning)->upper) * pixscale;
    int lowpass = GTK_ADJUSTMENT(wf->lp_tune)->value * pixscale;
    int highpass = GTK_ADJUSTMENT(wf->hp_tune)->value * pixscale;
    
    // black background, will be replaced with waterfall pixels
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_fill(cr);
    
    pix = cairo_image_surface_create_for_data (wf->pixels,
        CAIRO_FORMAT_RGB24,
        512, //width,
        height,
        (2048));
    
    cairo_set_source_surface (cr, pix, 0, 0);
    cairo_paint(cr);
    
    // pink cursor
    cairo_set_source_rgba(cr, 1 ,0.5, 0.5, 0.75); // pink for cursor
    cairo_move_to(cr, cursor, 0);
    cairo_line_to(cr, cursor, height);
    cairo_stroke(cr);
    
    // white filter band
    cairo_set_source_rgba(cr, 1,1,1,0.25);
    cairo_rectangle(cr, cursor-lowpass, 0, lowpass-highpass, height);
    cairo_fill(cr);
    //cairo_set_source_rgba(cr, 1, 1, 1, 0.75);

    cairo_set_source_rgba(cr, 1,1,0,0.5);
    cairo_move_to(cr, 0, wf->pixelrow);
    cairo_line_to(cr, width, wf->pixelrow);
    cairo_set_line_width(cr, 0.5);
    cairo_stroke(cr);
    
    
    cairo_destroy (cr);
    return FALSE;
}

