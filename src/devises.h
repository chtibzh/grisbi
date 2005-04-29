#ifndef _DEVISES_H
#define _DEVISES_H (1)


/** Columns numbers for currencies list  */
enum currency_list_column {
    COUNTRY_NAME_COLUMN = 0,
    CURRENCY_NAME_COLUMN,
    CURRENCY_ISO_CODE_COLUMN,
    CURRENCY_NICKNAME_COLUMN,
    CONTINENT_NAME_COLUMN,
    NUM_CURRENCIES_COLUMNS,
};

/* START_INCLUDE_H */
#include "structures.h"
/* END_INCLUDE_H */


/* START_DECLARATION */
void ajout_devise ( GtkWidget *widget );
GtkWidget *creation_option_menu_devises ( gint devise_cachee, GSList *liste_tmp );
void demande_taux_de_change ( struct struct_devise *devise_compte,
			      struct struct_devise *devise ,
			      gint une_devise_compte_egale_x_devise_ope,
			      gdouble taux_change,
			      gdouble frais_change,
			      gboolean force );
struct struct_devise * find_currency_from_iso4217_list ( gchar * currency_name );
GtkWidget *onglet_devises ( void );
GtkWidget *tab_display_totals ( void );
/* END_DECLARATION */
#endif
