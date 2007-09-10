/* ************************************************************************** */
/* Ce fichier comprend toutes les opérations concernant le traitement	      */
/* des fichiers								      */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2006 Benjamin Drieu (bdrieu@april.org)	      */
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
#include "fichiers_gestion.h"
#include "./menu.h"
#include "./fenetre_principale.h"
#include "./erreur.h"
#include "./dialog.h"
#include "./utils_file_selection.h"
#include "./gsb_account.h"
#include "./gsb_currency_config.h"
#include "./gsb_data_account.h"
#include "./gsb_data_archive_store.h"
#include "./gsb_data_category.h"
#include "./gsb_data_payment.h"
#include "./gsb_data_scheduled.h"
#include "./gsb_data_transaction.h"
#include "./gsb_file_config.h"
#include "./gsb_file_load.h"
#include "./gsb_file_save.h"
#include "./gsb_file_util.h"
#include "./navigation.h"
#include "./gsb_real.h"
#include "./gsb_status.h"
#include "./gsb_transactions_list.h"
#include "./traitement_variables.h"
#include "./main.h"
#include "./accueil.h"
#include "./utils_files.h"
#include "./utils_str.h"
#include "./parametres.h"
#include "./affichage_liste.h"
#include "./gsb_file_config.h"
#include "./utils_file_selection.h"
#include "./fenetre_principale.h"
#include "./include.h"
#include "./structures.h"
#include "./gsb_data_account.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void ajoute_new_file_liste_ouverture ( gchar *path_fichier );
static gchar * demande_nom_enregistrement ( void );
static gboolean enregistrement_backup ( void );
/*END_STATIC*/




gchar *nom_fichier_backup;



/*START_EXTERN*/
extern gchar *dernier_chemin_de_travail;
extern GtkWidget *main_hpaned ;
extern GtkWidget *main_vbox;
extern gint max;
extern GtkWidget * navigation_tree_view;
extern gsize nb_derniers_fichiers_ouverts ;
extern gint nb_max_derniers_fichiers_ouverts ;
extern gchar *nom_fichier_comptes;
extern GtkWidget *notebook_general;
extern GSList *scheduled_transactions_taken;
extern GSList *scheduled_transactions_to_take;
extern GtkTreeSelection * selection;
extern gchar **tab_noms_derniers_fichiers_ouverts ;
extern GtkWidget *table_etat ;
extern gchar *titre_fichier;
extern GtkWidget *tree_view_vbox;
extern GtkWidget *window;
extern GtkWidget *window_vbox_principale;
/*END_EXTERN*/



/**
 * Called by menu, close the last file and open a new one
 * 
 * \param none
 * 
 * \return FALSE
 * */
gboolean new_file ( void )
{
    kind_account type_de_compte;
    gint account_number;

    /*   si la fermeture du fichier en cours se passe mal, on se barre */

    if ( !fermer_fichier () )
	return FALSE;

    init_variables ();

    type_de_compte = gsb_account_ask_account_type ();

    if ( type_de_compte == -1 )
	return FALSE;

    /*     création de la 1ère devise */

    if ( ! gsb_currency_config_add_currency ( NULL, NULL ) )
	return FALSE;
    /* xxx voir ici, devrait utiliser la fonction new_compte */
    account_number = gsb_data_account_new (type_de_compte);
    if ( account_number == -1 )
	return FALSE;

    /* set the default method of payment */
    gsb_data_payment_create_default (account_number);

    /* FIXME: hardcoded!  Yes, first account is always numbered 1 and
     * first currencty too, but be cleaner. */
    gsb_data_account_set_currency ( 1, 1 );

    init_gui_new_file ();
    init_variables_new_file ();

    /* on se met sur l'onglet de propriétés du compte */
    mise_a_jour_accueil ( TRUE );
    gsb_gui_navigation_set_selection ( GSB_HOME_PAGE, -1, NULL );

    modification_fichier ( TRUE );
    return FALSE;
}



/**
 * Init various variables upon new file creation.
 */
void init_variables_new_file ( void )
{
    /* FIXME: remove this. */
    etat.largeur_auto_colonnes = 1;

    titre_fichier = _("My accounts");

    /* Create initial lists. */
    gsb_data_category_create_default_category_list ();
}



/**
 * Initialize user interface part when a new accounts file is created.
 */
void init_gui_new_file ( void )
{
    GtkWidget * tree_view_widget;

    /* dégrise les menus nécessaire */
    
    menus_sensitifs ( TRUE );

    /*     récupère l'organisation des colonnes  */
    recuperation_noms_colonnes_et_tips ();

    /* Create main widget. */
    gtk_box_pack_start ( GTK_BOX ( window_vbox_principale), create_main_widget(),
			 TRUE, TRUE, 0 );

    /* Create transaction list. */
    tree_view_widget = gsb_transactions_list_make_gui_list ();
    gtk_box_pack_start ( GTK_BOX ( tree_view_vbox ),
			 tree_view_widget,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( tree_view_widget );

    navigation_change_account ( GINT_TO_POINTER ( gsb_gui_navigation_get_current_account () ) );

    /* Display accounts in menus */
    gsb_menu_update_accounts_in_menus ();

    /* Affiche le nom du fichier de comptes dans le titre de la fenetre */
    affiche_titre_fenetre();

    gtk_notebook_set_page ( GTK_NOTEBOOK( notebook_general ), GSB_HOME_PAGE );

    gtk_widget_show ( notebook_general );
}



void ouvrir_fichier ( void )
{
    GtkWidget *selection_fichier;
    GtkFileFilter * filter;

    selection_fichier = file_selection_new ( _("Open an accounts file"),
					     FILE_SELECTION_MUST_EXIST);
    gtk_window_set_position ( GTK_WINDOW ( selection_fichier ), GTK_WIN_POS_MOUSE);

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name ( filter, _("Grisbi files (*.gsb)") );
    gtk_file_filter_add_pattern ( filter, "*.gsb" );
    gtk_file_chooser_add_filter ( GTK_FILE_CHOOSER ( selection_fichier ), filter );
    gtk_file_chooser_set_filter ( GTK_FILE_CHOOSER ( selection_fichier ), filter );

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name ( filter, _("All files") );
    gtk_file_filter_add_pattern ( filter, "*" );
    gtk_file_chooser_add_filter ( GTK_FILE_CHOOSER ( selection_fichier ), filter );

    switch ( gtk_dialog_run ( GTK_DIALOG (selection_fichier)))
    {
	case GTK_RESPONSE_OK:
	    if ( fermer_fichier() )
	    {
		gtk_widget_hide ( selection_fichier );
		nom_fichier_comptes = file_selection_get_filename ( GTK_FILE_CHOOSER ( selection_fichier ) ) ;
		dernier_chemin_de_travail = file_selection_get_last_directory ( GTK_FILE_CHOOSER ( selection_fichier),
										TRUE );
		gsb_file_open_file (nom_fichier_comptes);
	    }
	    break;
      default:
	  break;
    }
    gtk_widget_destroy ( selection_fichier );

}
/* ************************************************************************************************************ */

/* ************************************************************************************************************ */
void ouverture_fichier_par_menu ( gpointer null,
				  gint no_fichier )
{
    /*   si la fermeture du fichier courant se passe mal, on se barre */

    if ( !fermer_fichier() )
	return;

    nom_fichier_comptes = tab_noms_derniers_fichiers_ouverts[no_fichier];

    gsb_file_open_file (nom_fichier_comptes);
}
/* ************************************************************************************************************ */



/**
 * open a new grisbi file, don't check anything about another opened file that must
 * have been done before
 * 
 * \para filename the name of the file
 * 
 * \return TRUE ok, FALSE problem
 * */
gboolean gsb_file_open_file ( gchar *filename )
{
    gint i;
    GSList *list_tmp;
    GtkWidget *main_widget;

    devel_debug ( g_strdup_printf ("gsb_file_open_file : %s",
				   filename ));

    if ( !filename
	 ||
	 !strlen (filename))
	return FALSE;

    gsb_status_wait ( TRUE );
    gsb_status_message ( _("Loading accounts") );

    /* try to load the file */

    if ( gsb_file_load_open_file (filename))
    {
	/* the file has been opened succesfully */

	/* we make a backup if necessary */
	if ( etat.sauvegarde_demarrage )
	{
	    gchar *backup_filename;
	    gchar **tab_char;

	    gsb_status_message ( _("Autosaving") );

	    backup_filename = my_strdup ( filename );

	    /* we get only the name of the file, not the path */

	    tab_char = g_strsplit ( backup_filename,
				    C_DIRECTORY_SEPARATOR,
				    0);
	    i=0;
	    while ( tab_char[i] )
		i++;

	    backup_filename = g_strconcat ( my_get_gsb_file_default_dir(),
				C_DIRECTORY_SEPARATOR,
#ifndef _WIN32
                                ".",
#endif
				tab_char [i-1],
				".bak",
				NULL );

	    g_strfreev ( tab_char );
	    gsb_file_save_save_file ( backup_filename,
				      etat.compress_backup,
				      FALSE );
	    g_free (backup_filename);
	}
    }
    else
    {
	/* Loading failed.  If the saving function at opening is set,
	 * we ask to load the last file */

	gsb_status_message ( _("Failed to load accounts") );

	if ( etat.sauvegarde_demarrage )
	{
	    gchar *backup_filename;
	    gchar **tab_char;

	    gsb_status_message ( _("Loading backup") );

	    /* create the name of the backup */
	    i=0;
	    tab_char = g_strsplit ( filename, C_DIRECTORY_SEPARATOR, 0);
	    while ( tab_char[i] )
		i++;
	    backup_filename = g_strconcat ( my_get_gsb_file_default_dir(),
					    C_DIRECTORY_SEPARATOR,
					    tab_char [i-1],
					    ".bak",
					    NULL );
	    g_strfreev ( tab_char );

	    /* try to load the backup */

	    if ( gsb_file_load_open_file ( backup_filename ) )
	    {
		/* the backup loaded succesfully */

		dialogue_error_hint ( _("Grisbi was unable to load file.  However, Grisbi loaded a backup file instead but all changes made since this backup were possibly lost."),
				      g_strdup_printf ( _("Error loading file '%s'"), filename) );
		g_free (backup_filename);
	    }
	    else
	    {
		/* the loading backup failed */

		dialogue_error_hint ( _("Grisbi was unable to load file.  Additionally, Grisbi was unable to load a backup file instead."),
				      g_strdup_printf ( _("Error loading file '%s'"), filename) );
		g_free (backup_filename);
		gsb_status_stop_wait ( TRUE );
		return FALSE;
	    }
	}
	else
	{
	    gsb_status_stop_wait ( TRUE );
	    return FALSE;
	}
    }

    /* ok, here the file or backup is loaded */
    gsb_status_message ( _("Checking schedulers"));

    /* the the name in the last opened files */
    ajoute_new_file_liste_ouverture ( filename );

    /* get the names of the columns */
    recuperation_noms_colonnes_et_tips ();

    /* we show and update the menus */
    menus_sensitifs ( TRUE );

    /* we make the main window */
    gsb_status_message ( _("Creating main window"));
    main_widget = create_main_widget();

    /* check the amounts of all the accounts */
    gsb_status_message ( _("Checking amounts"));
    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
	i = gsb_data_account_get_no_account ( list_tmp -> data );

	gsb_data_account_calculate_current_and_marked_balances (i);

	/* set the minimum balances to be shown or not
	 * if we are already under the minimum, we will show nothing */

	gsb_data_account_set_mini_balance_authorized_message ( i,
							       gsb_real_cmp ( gsb_data_account_get_current_balance (i),
									      gsb_data_account_get_mini_balance_authorized (i)) == -1 );
	gsb_data_account_set_mini_balance_wanted_message ( i,
							   gsb_real_cmp ( gsb_data_account_get_current_balance (i),
									  gsb_data_account_get_mini_balance_wanted (i)) == -1 );
	list_tmp = list_tmp -> next;
    }

    /* set the name of the file in the window title
     * and in the menu of the main window, so main_widget must
     * have been created */
    affiche_titre_fenetre();

    gsb_status_message ( _("Creating interface"));
    gsb_menu_update_view_menu (gsb_gui_navigation_get_current_account ());
    gsb_menu_update_accounts_in_menus ();

    /* append that window to the main window */
    gtk_box_pack_start ( GTK_BOX (window_vbox_principale), main_widget, TRUE, TRUE, 0 );
    gtk_widget_show ( main_widget );

    /* create the archives store data, ie the transaction wich will replace the archive in
     * the list of transactions */
    gsb_data_archive_store_create_list ();

    /* create and fill the gui transactions list */
    gtk_box_pack_start ( GTK_BOX ( tree_view_vbox ),
			 gsb_transactions_list_make_gui_list (),
			 TRUE,
			 TRUE,
			 0 );

    /* update the main page */
    mise_a_jour_accueil (TRUE);

    modification_fichier ( FALSE );

    gsb_status_message ( _("Done") );
    gsb_status_stop_wait ( TRUE );

    /* go to the home page */
    gsb_gui_navigation_set_selection ( GSB_HOME_PAGE, -1, NULL );    

    /* set the focus to the selection tree at left */
    gtk_widget_grab_focus (navigation_tree_view);

    return TRUE;
}


/**
 * Perform the "Save as" feature.
 *
 * \return TRUE on success.  FALSE otherwise.
 */
gboolean gsb_save_file_as ( void )
{
    return enregistrement_fichier ( -2 );
}


/* ************************************************************************************************************ */
/* Fonction appelé lorsqu'on veut enregistrer le fichier ( fin de prog, fermeture fichier ... ) */
/* enregistre automatiquement ou demande */
/* si origine = -1 : provient de la fonction fermeture_grisbi */
/* si origine = -2 : provient de enregistrer sous */
/* retour : TRUE si tout va bien, cad on ne veut pas enregistrer ou c'est enregistré */
/* ************************************************************************************************************ */
gboolean enregistrement_fichier ( gint origine )
{
    gint etat_force, result;
    gchar *nouveau_nom_enregistrement;

    devel_debug (g_strdup_printf ( "enregistrement_fichier from %d", origine ));

    etat_force = 0;

    if ( ( ! etat.modification_fichier && origine != -2 ) ||
	 ! gsb_data_account_get_accounts_amount () )
    {
	notice_debug ( "nothing done in enregistrement_fichier" );
	return ( TRUE );
    }

    /* si le fichier de comptes n'a pas de nom ou si on enregistre sous un nouveau nom */
    /*     c'est ici */

    if ( !nom_fichier_comptes || origine == -2 )
	nouveau_nom_enregistrement = demande_nom_enregistrement ();
    else
	nouveau_nom_enregistrement = nom_fichier_comptes;

    if ( !nouveau_nom_enregistrement )
	return FALSE;

    /*     on vérifie que le fichier n'est pas locké */

    if ( etat.fichier_deja_ouvert
	 &&
	 !etat.force_enregistrement
	 &&
	 origine != -2 )
    {
	dialogue_error_hint ( g_strdup_printf( _("Grisbi was unable to save this file because it is locked.  Please save it with another name or activate the \"%s\" option in preferences."),
					       _("Force saving of locked files" ) ),
			      g_strdup_printf( _("Can not save file \"%s\""), 
					       nom_fichier_comptes ) );
	return ( FALSE );
    }

    /*   on a maintenant un nom de fichier */
    /*     et on sait qu'on peut sauvegarder */

    gsb_status_message ( _("Saving file") );

    result = gsb_file_save_save_file ( nouveau_nom_enregistrement,
				       etat.compress_file,
				       FALSE );

    if ( result )
    {
	/* 	l'enregistrement s'est bien passé, */
	/* 	on délock le fichier (l'ancien ou le courant) */
	    
	gsb_file_util_modify_lock ( FALSE );

	nom_fichier_comptes = nouveau_nom_enregistrement;

	/* 	... et locke le nouveau */

	gsb_file_util_modify_lock ( TRUE );

	/* 	dans tout les cas, le fichier n'était plus ouvert à l'ouverture */

	etat.fichier_deja_ouvert = 0;
	modification_fichier ( FALSE );
	affiche_titre_fenetre ();
	ajoute_new_file_liste_ouverture ( nom_fichier_comptes );
    }

    /*     on enregistre la backup si nécessaire */

    enregistrement_backup();

    gsb_status_message ( _("Done") );

    return ( result );
}
/* ************************************************************************************************************ */



/* ************************************************************************************************************ */
/* cette fonction est appelée pour proposer d'enregistrer si le fichier est modifié */
/* elle n'enregistre pas, elle retourne juste le choix de l'utilisateur */
/* retourne : */
/* GTK_RESPONSE_OK : veut enregistrer */
/* GTK_RESPONSE_NO : veut pas enregistrer */
/* autre gint : annuler */
/* ************************************************************************************************************ */
gint question_fermer_sans_enregistrer ( void )
{
    gchar * hint, * message = "";
    gint result;
    GtkWidget *dialog;

    /*     si le fichier n'est pas modifié on renvoie qu'on ne veut pas enregistrer */

    if ( !etat.modification_fichier )
	return GTK_RESPONSE_NO;
    
    if ( etat.sauvegarde_auto && 
	 ( !etat.fichier_deja_ouvert || etat.force_enregistrement ) &&
	 nom_fichier_comptes )
      return GTK_RESPONSE_OK;

    /*     si le fichier était déjà locké et que force enregistrement n'est pas mis, */
    /*     on prévient ici */

    dialog = gtk_message_dialog_new ( GTK_WINDOW (window), 
				      GTK_DIALOG_DESTROY_WITH_PARENT,
				      GTK_MESSAGE_WARNING, 
				      GTK_BUTTONS_NONE,
				      " " );
    if ( etat.fichier_deja_ouvert
	 &&
	 !etat.force_enregistrement )
    {
	hint = _("Save locked files?");
	message = g_strdup_printf ( _("The document '%s' is locked but modified. If you want to save it, you must cancel and save it with another name or activate the \"%s\" option in setup."),
				    (nom_fichier_comptes ? g_path_get_basename(nom_fichier_comptes) : _("unnamed")),
				    _("Force saving of locked files"));
	gtk_dialog_add_buttons ( GTK_DIALOG(dialog),
				 _("Close without saving"), GTK_RESPONSE_NO,
				 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				 NULL );
	gtk_dialog_set_default_response ( GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL ); 
    }
    else
    {
	hint = g_strdup_printf (_("Save changes to document '%s' before closing?"),
				(nom_fichier_comptes ? g_path_get_basename(nom_fichier_comptes) : _("unnamed")));
	gtk_dialog_add_buttons ( GTK_DIALOG(dialog),
				 _("Close without saving"), GTK_RESPONSE_NO,
				 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				 GTK_STOCK_SAVE, GTK_RESPONSE_OK,
				 NULL );
	gtk_dialog_set_default_response ( GTK_DIALOG(dialog), GTK_RESPONSE_OK ); 
    }

    message = g_strconcat ( message, 
			    _("If you close without saving, all of your changes will be discarded."),
			    NULL );
    
    gtk_label_set_markup ( GTK_LABEL ( GTK_MESSAGE_DIALOG(dialog)->label ), 
			   make_hint ( hint, message ) );

    gtk_window_set_modal ( GTK_WINDOW ( dialog ), TRUE );

    result = gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy ( dialog );

    return result;
}
/* ************************************************************************************************************ */



/* ************************************************************************************************************ */
/* cette fonction est appelée lors de l'enregistrement, s'il n'y a pas de nom */
/* ou si on fait un enregistrement sous */
/* elle renvoie le nouveau nom */
/* ************************************************************************************************************ */
gchar * demande_nom_enregistrement ( void )
{
    gchar *nouveau_nom;
    GtkWidget *fenetre_nom;
    gint resultat;

    fenetre_nom = file_selection_new ( _("Name the accounts file"),
				       FILE_SELECTION_IS_SAVE_DIALOG);
    gtk_window_set_modal ( GTK_WINDOW ( fenetre_nom ),
			   TRUE );

    if ( ! nom_fichier_comptes )
    {
	gtk_file_chooser_set_current_name ( GTK_FILE_CHOOSER ( fenetre_nom ),
					    g_strconcat ( titre_fichier, ".gsb", NULL ) );
    }
    else
    {
	/* FIXME: select existing file */
    }

    resultat = gtk_dialog_run ( GTK_DIALOG ( fenetre_nom ));

    switch ( resultat )
    {
	case GTK_RESPONSE_OK :
	    nouveau_nom = file_selection_get_filename ( GTK_FILE_CHOOSER ( fenetre_nom ));

	    gtk_widget_destroy ( GTK_WIDGET ( fenetre_nom ));

	    /* Les vérifications d'usage ont  été faite par la boite de dialogue*/

	    break;

	default :
	    gtk_widget_destroy ( GTK_WIDGET ( fenetre_nom ));
	    return NULL;
    }

    if ( ! g_strrstr ( nouveau_nom, "." ) )
    {
	nouveau_nom = g_strconcat ( nouveau_nom, ".gsb", NULL );
    }

    return nouveau_nom;
}
/* ************************************************************************************************************ */





/* ************************************************************************************************************ */
gboolean fermer_fichier ( void )
{
    gint result;

    devel_debug ( "fermer_fichier" );


    if ( !gsb_data_account_get_accounts_amount () )
	return ( TRUE );


    /*     on propose d'enregistrer */

    result = question_fermer_sans_enregistrer();

    switch ( result )
    {
	case GTK_RESPONSE_OK:

	    /* 	    on a choisi d'enregistrer, si va pas, on s'en va */

	    if ( !enregistrement_fichier (-1) )
		return ( FALSE );

	case GTK_RESPONSE_NO :

	    /* 	    l'enregistrement s'est bien passé ou on a choisi de ne pas enregistrer */

	    /*     on enregistre la config */

	     gsb_file_config_save_config();

	    /* si le fichier n'était pas déjà ouvert, met à 0 l'ouverture */

	    if ( !etat.fichier_deja_ouvert
		 &&
		 gsb_data_account_get_accounts_amount ()
		 &&
		 nom_fichier_comptes )
		gsb_file_util_modify_lock ( FALSE );

	    /* libère les opérations de tous les comptes */

	    g_slist_free ( gsb_data_transaction_get_transactions_list ());
	    g_slist_free ( gsb_data_transaction_get_complete_transactions_list ());
	    g_slist_free ( gsb_data_account_get_list_accounts () );

	    /* libère les échéances */

	    g_slist_free ( gsb_data_scheduled_get_scheduled_list () );
	    g_slist_free ( scheduled_transactions_to_take );
	    g_slist_free ( scheduled_transactions_taken );
	    gtk_widget_destroy ( main_vbox );

	    init_variables ();

	    affiche_titre_fenetre();

	    menus_sensitifs ( FALSE );

	    main_hpaned = NULL;
	    table_etat = NULL;

	    return ( TRUE );

	default :
	    return FALSE;
    }
}
/* ************************************************************************************************************ */




/* ************************************************************************************************************ */
/* Fonction appelée une fois qu'on a nommé le fichier de compte */
/* met juste le titre dans la fenetre principale */
/* ************************************************************************************************************ */
void affiche_titre_fenetre ( void )
{
    gchar **parametres = NULL;
    gchar *titre = NULL;
    gint i=0;

    devel_debug ( "affiche_titre_fenetre" );

    if ( titre_fichier && strlen(titre_fichier) )
      titre = titre_fichier;
    else if ( nom_fichier_comptes )
    {
	parametres = g_strsplit ( nom_fichier_comptes, C_DIRECTORY_SEPARATOR, 0);
	while ( parametres[i] )
	  i++;
	titre = my_strdup(parametres [i-1]);
	g_strfreev ( parametres );
    }
    else
    {
      titre = g_strconcat ( "<", _("unnamed"), ">", NULL );
    }

    titre = g_strconcat ( titre, " - ", _("Grisbi"), NULL );
    gtk_window_set_title ( GTK_WINDOW ( window ), titre );
}



/* ************************************************************************************************************ */
/* Fonction enregistrement_backup */
/* appelée si necessaire au début de l'enregistrement */
/* ************************************************************************************************************ */

gboolean enregistrement_backup ( void )
{
    gboolean retour;

    if ( !nom_fichier_backup || !strlen(nom_fichier_backup) )
	return FALSE;

    gsb_status_message ( _("Saving backup") );

    retour = gsb_file_save_save_file( nom_fichier_backup,
				      etat.compress_backup,
				      FALSE );

    gsb_status_message ( _("Done") );

    return ( retour );
}
/* ************************************************************************************************************ */


/* ************************************************************************************************************ */
void ajoute_new_file_liste_ouverture ( gchar *path_fichier )
{
    gint i;
    gint position;
    gchar *dernier;

    devel_debug ( g_strdup_printf ("ajoute_new_file_liste_ouverture : %s", path_fichier ));

    if ( !nb_max_derniers_fichiers_ouverts ||
	 ! path_fichier)
	return;

    if ( nb_derniers_fichiers_ouverts < 0 )
	nb_derniers_fichiers_ouverts = 0;

    /* ALAIN-FIXME */
    /* si on n'a pas un chemin absolu, on n'enregistre pas ce fichier
       dans la liste. Du moins jusqu'à ce que quelqu'un trouve un moyen
       pour récupérer le chemein absolu */

    if ( !g_path_is_absolute ( nom_fichier_comptes ) )
    {
	/*     gchar *tmp_name, *tmp_name2;
	       tmp_name = realpath(nom_fichier_comptes, tmp_name2);

	       dialogue(tmp_name);
	       free(tmp_name);
	       dialogue(tmp_name2);
	       free(tmp_name2);*/

	return;
    }

    /* on commence par vérifier si ce fichier n'est pas dans les nb_derniers_fichiers_ouverts noms */

    position = 0;

    for ( i=0 ; i<nb_derniers_fichiers_ouverts ; i++ )
	if ( !strcmp ( path_fichier,
		       tab_noms_derniers_fichiers_ouverts[i] ))
	{
	    /* 	si ce fichier est déjà le dernier ouvert, on laisse tomber */

	    if ( !i )
		return;

	    position = i;
	}

    efface_derniers_fichiers_ouverts();

    if ( position )
    {
	/*       le fichier a été trouvé, on fait juste une rotation */

	for ( i=position ; i>0 ; i-- )
	    tab_noms_derniers_fichiers_ouverts[i] = tab_noms_derniers_fichiers_ouverts[i-1];
	if ( path_fichier )
	    tab_noms_derniers_fichiers_ouverts[0] = my_strdup ( path_fichier );
	else
	    tab_noms_derniers_fichiers_ouverts[0] = my_strdup ( "<no file>" );

	affiche_derniers_fichiers_ouverts();

	return;
    }

    /*   le fichier est nouveau, on décale tout d'un cran et on met le nouveau à 0 */

    /*   si on est déjà au max, c'est juste un décalage avec perte du dernier */
    /* on garde le ptit dernier dans le cas contraire */

    if ( nb_derniers_fichiers_ouverts )
	dernier = tab_noms_derniers_fichiers_ouverts[nb_derniers_fichiers_ouverts-1];
    else
	dernier = NULL;

    for ( i= nb_derniers_fichiers_ouverts - 1 ; i>0 ; i-- )
	tab_noms_derniers_fichiers_ouverts[i] = tab_noms_derniers_fichiers_ouverts[i-1];

    if ( nb_derniers_fichiers_ouverts < nb_max_derniers_fichiers_ouverts )
    {
	tab_noms_derniers_fichiers_ouverts = realloc ( tab_noms_derniers_fichiers_ouverts,
						       ( ++nb_derniers_fichiers_ouverts ) * sizeof ( gpointer ));
	tab_noms_derniers_fichiers_ouverts[nb_derniers_fichiers_ouverts-1] = dernier;
    }

    tab_noms_derniers_fichiers_ouverts[0] = my_strdup ( path_fichier );


    affiche_derniers_fichiers_ouverts();
}
/* ************************************************************************************************************ */


/****************************************************************************/
void remove_file_from_last_opened_files_list ( gchar * nom_fichier )
{
    gint i, j;

    efface_derniers_fichiers_ouverts();

    for ( i = 0 ; i < nb_derniers_fichiers_ouverts ; i++ )
    {
	if ( ! strcmp (nom_fichier, tab_noms_derniers_fichiers_ouverts[i]) )
	{
	    nb_derniers_fichiers_ouverts--;

	    for ( j = i; ( j + 1 ) < nb_derniers_fichiers_ouverts; j++ )
	    {
		tab_noms_derniers_fichiers_ouverts[j] = tab_noms_derniers_fichiers_ouverts[j+1];
		
	    }
	}
    }
    affiche_derniers_fichiers_ouverts();
}
/****************************************************************************/



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
