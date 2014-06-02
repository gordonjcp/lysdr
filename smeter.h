/*  lysdr Software Defined Radio
	(C) 2010-2011 Gordon JC Pearce MM0YEQ and others
	
	smeter.h
	
	This file is part of lysdr.

	lysdr is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	any later version.

	lysdr is distributed in the hope that it will be useful, but
	WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with lysdr.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef __SMETER_H
#define __SMETER_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _SDRSMeter			SDRSMeter;
typedef struct _SDRSMeterClass	   SDRSMeterClass;
typedef struct _SDRSMeterPrivate	 SDRSMeterPrivate;

struct _SDRSMeter {
	GtkDrawingArea parent;
	gdouble level;
};

struct _SDRSMeterClass {
	GtkDrawingAreaClass parent_class;
};

struct _SDRSMeterPrivate {
};

#define SDR_TYPE_SMETER			 (sdr_smeter_get_type ())
#define SDR_SMETER(obj)			 (G_TYPE_CHECK_INSTANCE_CAST ((obj), SDR_TYPE_SMETER, SDRSMeter))
#define SDR_SMETER_CLASS(obj)	   (G_TYPE_CHECK_CLASS_CAST ((obj), SDR_SMETER,  SDRSMeterClass))
#define SDR_IS_SMETER(obj)		  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SDR_TYPE_SMETER))
#define SDR_IS_SMETER_CLASS(obj)	(G_TYPE_CHECK_CLASS_TYPE ((obj), SDR_TYPE_SMETER))
#define SDR_SMETER_GET_CLASS		(G_TYPE_INSTANCE_GET_CLASS ((obj), SDR_TYPE_SMETER, SDRSMeterClass))
#define SDR_SMETER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), SDR_TYPE_SMETER, SDRSMeterPrivate))

G_END_DECLS

GtkWidget *sdr_smeter_new();
void sdr_smeter_set_level(SDRSMeter *sm, gdouble value);
GType sdr_smeter_get_type(void);

#endif /* __SMETER_H */

/* vim: set noexpandtab ai ts=4 sw=4 tw=4: */
