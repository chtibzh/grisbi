/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	     2005 Benjamin Drieu (bdrieu@april.org)	      */
/* 			http://www.grisbi.org				      */
/*                                                                            */
/*  This program is free software; you can redistribute it and/or modify      */
/*  it under the terms of the GNU General Public License as published by      */
/*  the Free Software Foundation; either version 2 of the License, or         */
/*  (at your option) any later version.                                       */
/*                                                                            */
/*  This program is distributed in the hope that it will be useful,           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*  GNU General Public License for more details.                              */
/*                                                                            */
/*  You should have received a copy of the GNU General Public License         */
/*  along with this program; if not, write to the Free Software               */
/*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                            */
/* ************************************************************************** */


#include "include.h"

/*START_INCLUDE*/
#include "etats_csv.h"
#include "dialog.h"
#include "gsb_data_report.h"
#include "navigation.h"
#include "utils_str.h"
#include "utils_files.h"
#include "etats_config.h"
#include "include.h"
#include "etats_csv.h"
#include "etats_affiche.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void csv_attach_hsep ( int x, int x2, int y, int y2);
static void csv_attach_label ( gchar * text, gdouble properties, int x, int x2, int y, int y2, 
			  enum alignement align, gpointer ope );
static void csv_attach_vsep ( int x, int x2, int y, int y2);
static gint csv_finish ();
static gint csv_initialise (GSList * opes_selectionnees, gchar * filename );
static void csv_safe ( gchar * text ) ;
/*END_STATIC*/


struct struct_etat_affichage csv_affichage = {
    csv_initialise,
    csv_finish,
    csv_attach_hsep,
    csv_attach_vsep,
    csv_attach_label,
};

FILE * csv_out;
gint csv_lastcol = 0;
gint csv_lastline = 1;


/*START_EXTERN*/
extern gint nb_colonnes;
/*END_EXTERN*/



/**
 * Backend function that is responsible for printing a label at a
 * specific place of the report, using text attibutes.
 *
 * \param text Text to print
 * \param properties bit field for text properties.  Can be:
 *        - TEXT_BOLD: text will be printed in bold
 *        - TEXT_ITALIC: text will be printed in intalic
 *        - TEXT_HUGE: text will be have a huge face
 *        - TEXT_LARGE: text will be have a large face
 *        - TEXT_SMALL: text will be have a small face
 * \param x left position of the label
 * \param y top position of the label
 * \param x2 right position of the label
 * \param y2 bottom position of the label
 * \param alignment how the text will be aligned in the cell
 * \param ope a pointer to a transaction to link to (not used as csv
 *            backend is not interactive)
 */
void csv_attach_label ( gchar * text, gdouble properties, int x, int x2, int y, int y2, 
			  enum alignement align, gpointer ope )
{
    int pad, realsize, realcolumns, i;
    gint current_report_number;

    current_report_number = gsb_gui_navigation_get_current_report ();

    if ( !text )
	text = "";

    if ( y >= csv_lastline )
    {
	csv_lastcol = 0;
	csv_lastline = y2;
	fprintf ( csv_out, "\n" );
    }

    for ( pad = csv_lastcol ; pad < x ; pad ++ )
	fprintf ( csv_out, "," );

    realsize = (x2 - x);
    if ( realsize > 1 )
    {
	if ( gsb_data_report_get_show_report_transactions (current_report_number))
	{
	    realsize /= 2;
	    if ( x == 0 )
		realsize ++;
	}
    }
    if ( gsb_data_report_get_show_report_transactions (current_report_number))
	realcolumns = (float)((nb_colonnes / 2) + 1);
    else 
	realcolumns = nb_colonnes;

    fprintf ( csv_out, "\"" );
    csv_safe ( g_strstrip ( my_strdup ( text ) ) );
    fprintf ( csv_out, "\"" );

    for ( x++; x < x2 ; x ++ )
	fprintf ( csv_out, "," );

    csv_lastcol = x2;
}



/**
 * Backend function that is responsible for printing a vertical
 * separator (i.e. a vertical line).
 *
 * \param x left position of the separator
 * \param y top position of the separator
 * \param x2 right position of the separator
 * \param y2 bottom position of the separator
 */
void csv_attach_vsep ( int x, int x2, int y, int y2)
{
}



/**
 * Backend function that is responsible for printing an horizontal
 * separator (i.e. an horizontal line).
 *
 * \param x left position of the separator
 * \param y top position of the separator
 * \param x2 right position of the separator
 * \param y2 bottom position of the separator
 */
void csv_attach_hsep ( int x, int x2, int y, int y2)
{
}



/**
 * Initialization function for the csv backend.
 *
 * \return TRUE on succes, FALSE otherwise.
 */
gint csv_initialise (GSList * opes_selectionnees, gchar * filename )
{
    GtkWidget * file_selector;
    gfloat colwidth, real_width;
    int i;

    g_return_val_if_fail ( filename, FALSE );

    unlink ( filename );	/* We don't care if this fails, this
				 * is just to guarantee next will
				 * not fail if file is existing.  This
				 * will help against file races. */
    csv_out = utf8_fopen ( filename, "w+x" );
    if ( ! csv_out )
    {
	dialogue_error ( g_strdup_printf (_("Unable to open file '%s'"), filename ));
	return FALSE;
    }

    return TRUE;
}



/**
 * Destructor function for the csv backend.
 *
 * \return TRUE on success, FALSE otherwise.
 */
gint csv_finish ()
{
    fclose (csv_out);

    return 1;
}



/** 
 * Print a csv safe string into the csv_out file descriptor.  All chars
 * that cannot be printed via csv are converted to their csv
 * equivalent (i.e. backslashes are escaped).
 *
 * \param text Text to print.
 */
void csv_safe ( gchar * text ) 
{
    if ( ! text || ! strlen(text))
	return;

    for ( ; * text; text ++ )
    {
	switch ( * text )
	{
	    case '"':
		fprintf ( csv_out, "\\" );

	    default:
		fprintf ( csv_out, "%c", *text );
		break;
	}
    }
}

/* Local variables: */
/* c-basic-offset: 4 */
/* End: */
