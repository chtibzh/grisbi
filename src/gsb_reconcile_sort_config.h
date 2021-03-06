#ifndef _GSB_RECONCILE_SORT_CONFIG_H
#define _GSB_RECONCILE_SORT_CONFIG_H (1)

#include <gtk/gtk.h>

enum reconciliation_sort_columns {
    RECONCILIATION_SORT_NAME_COLUMN = 0,
    RECONCILIATION_SORT_VISIBLE_COLUMN,
    RECONCILIATION_SORT_SORT_COLUMN,
    RECONCILIATION_SORT_SPLIT_NEUTRAL_COLUMN,
    RECONCILIATION_SORT_ACCOUNT_COLUMN,
    RECONCILIATION_SORT_TYPE_COLUMN,
    RECONCILIATION_SORT_SENSITIVE_COLUMN,
    NUM_RECONCILIATION_SORT_COLUMNS
};


/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
GtkWidget *	gsb_reconcile_sort_config_create	(void);
void		gsb_reconcile_sort_config_fill		(void);
/* END_DECLARATION */
#endif
