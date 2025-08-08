#include "setgui.h"
#include "settings.h"
#include "update.h"
#include "actionqueue.h"

/* What we want to call our menu entry */
#define BRAIN_MENU_NAME "Ryan2" 

Boolean opt[14]; /* Menu options         */

setting *stngs;

#ifndef WIN32
	GtkWidget *menu_top;      /* The top of the menu  */

	void menuAboutActivate(GtkMenuItem *menuItem, gpointer user_data);
	void dialogAboutOK(GtkWidget *widget, gpointer user_data);
	void menuOptionsActivate(GtkMenuItem *menuItem, gpointer user_data);
	void checkUpdate(GtkMenuItem *menuItem, gpointer user_data);
#endif	

void setOptions(unsigned short *ops);



#ifndef WIN32
	GtkWidget *entry1;
	int settingsarray[] = {0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080, 0x0100, 0x0200, 0x0400};
#endif



int change_tracker = 0;



#ifndef WIN32
void setGui(GtkWidget *menu_bar) {

	int i;
 GtkWidget *brain_menu;   /* The submenu the items live in  */
 GtkWidget *restrictions_menu;
 GtkWidget *restrictionssub_menu;
  GtkWidget *item;         /* Item to create and add to menu */
  
	for (i=0;i < 14;i++) {
		opt[i] = 0;
	}
  /* Create our menu */
  menu_top = gtk_menu_item_new_with_label(BRAIN_MENU_NAME);
  gtk_widget_ref(menu_top);
  gtk_widget_show(menu_top);
  gtk_container_add(GTK_CONTAINER(menu_bar), menu_top); /* Append it here */
  brain_menu = gtk_menu_new();
  gtk_widget_ref(brain_menu);
  gtk_object_set_data_full (GTK_OBJECT(menu_top), "About", brain_menu, (GtkDestroyNotify) gtk_widget_unref);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_top), brain_menu);
  

  /* About */
  item = gtk_menu_item_new_with_label ("About");
  gtk_widget_ref(item);
  gtk_object_set_data_full (GTK_OBJECT(menu_top), "About", item, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (item);
  gtk_container_add(GTK_CONTAINER(brain_menu), item);
  
  gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(menuAboutActivate), NULL);

  /* Create Restrictions Menu */
  restrictions_menu = gtk_menu_item_new_with_label("Restrictions");
  gtk_widget_ref(restrictions_menu);
  gtk_widget_show(restrictions_menu);
  gtk_container_add(GTK_CONTAINER(brain_menu), restrictions_menu); /* Append it here */
  restrictionssub_menu = gtk_menu_new();
  gtk_widget_ref(restrictionssub_menu);
  gtk_object_set_data_full (GTK_OBJECT(restrictions_menu), "About", restrictionssub_menu, (GtkDestroyNotify) gtk_widget_unref);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(restrictions_menu),restrictionssub_menu);
  

  /* Seperator */
  item = gtk_menu_item_new ();
  gtk_widget_ref(menu_top);
  gtk_object_set_data_full (GTK_OBJECT(menu_top), "item", item, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(item);
  gtk_container_add(GTK_CONTAINER(brain_menu), item);

  /* Option 1 */
  item = gtk_check_menu_item_new_with_label("Attack Pillboxes");
  gtk_widget_ref(item);
  gtk_object_set_data_full(GTK_OBJECT(restrictions_menu), "option1", item, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(item);
  gtk_container_add(GTK_CONTAINER(restrictionssub_menu), item);
 
  gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(menuOptionsActivate), &opt[0]);
  
  /* Option 2 */
  item = gtk_check_menu_item_new_with_label("Build Pillboxes");
  gtk_widget_ref(item);
  gtk_object_set_data_full(GTK_OBJECT(restrictions_menu), "option2", item, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(item);
  gtk_container_add(GTK_CONTAINER(restrictionssub_menu), item);
  gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(menuOptionsActivate), &opt[1]);
  
  /* Option 3 */
  item = gtk_check_menu_item_new_with_label("Pickup Pillboxes");
  gtk_widget_ref(item);
  gtk_object_set_data_full(GTK_OBJECT(restrictions_menu), "option3", item, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(item);
  gtk_container_add(GTK_CONTAINER(restrictionssub_menu), item);
  gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(menuOptionsActivate), &opt[2]);

  /* Option 4 */
  item = gtk_check_menu_item_new_with_label("Lay Mines");
  gtk_widget_ref(item);
  gtk_object_set_data_full(GTK_OBJECT(restrictions_menu), "option4", item, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(item);
  gtk_container_add(GTK_CONTAINER(restrictionssub_menu), item);
  gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(menuOptionsActivate), &opt[3]);

  /* Option 5 */
  item = gtk_check_menu_item_new_with_label("Build Roads");
  gtk_widget_ref(item);
  gtk_object_set_data_full(GTK_OBJECT(restrictions_menu), "option5", item, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(item);
  gtk_container_add(GTK_CONTAINER(restrictionssub_menu), item);
  gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(menuOptionsActivate), &opt[4]);

  /* Option 6 */
  item = gtk_check_menu_item_new_with_label("Attack Tanks");
  gtk_widget_ref(item);
  gtk_object_set_data_full(GTK_OBJECT(restrictions_menu), "option6", item, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(item);
  gtk_container_add(GTK_CONTAINER(restrictionssub_menu), item);
  gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(menuOptionsActivate), &opt[5]);

  /* Option 7 */
  item = gtk_check_menu_item_new_with_label("Assasin");
  gtk_widget_ref(item);
  gtk_object_set_data_full(GTK_OBJECT(menu_top), "option7", item, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(item);
  gtk_container_add(GTK_CONTAINER(brain_menu), item);
  gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(menuOptionsActivate), &opt[6]);

  /* Option 8 */
  item = gtk_check_menu_item_new_with_label("Base Guard");
  gtk_widget_ref(item);
  gtk_object_set_data_full(GTK_OBJECT(menu_top), "option8", item, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(item);
  gtk_container_add(GTK_CONTAINER(brain_menu), item);
  gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(menuOptionsActivate), &opt[7]);

  /* Option 9 */
  item = gtk_check_menu_item_new_with_label("Architect");
  gtk_widget_ref(item);
  gtk_object_set_data_full(GTK_OBJECT(menu_top), "option9", item, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(item);
  gtk_container_add(GTK_CONTAINER(brain_menu), item);
  gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(menuOptionsActivate), &opt[8]);

  /* Option 10 */
  item = gtk_check_menu_item_new_with_label("Pillbox Hunter");
  gtk_widget_ref(item);
  gtk_object_set_data_full(GTK_OBJECT(menu_top), "option10", item, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(item);
  gtk_container_add(GTK_CONTAINER(brain_menu), item);
  gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(menuOptionsActivate), &opt[8]);

  /* Option 10 */
  item = gtk_check_menu_item_new_with_label("Explorer");
  gtk_widget_ref(item);
  gtk_object_set_data_full(GTK_OBJECT(menu_top), "option11", item, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(item);
  gtk_container_add(GTK_CONTAINER(brain_menu), item);
  gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(menuOptionsActivate), &opt[9]);
  
  item = gtk_menu_item_new_with_label ("Check for Update");
  gtk_widget_ref(item);
  gtk_object_set_data_full (GTK_OBJECT(menu_top), "Check for Update", item, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (item);
  gtk_container_add(GTK_CONTAINER(brain_menu), item);
  
  gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(checkUpdate), NULL);


}


/* Callback for OK button click */
void dialogAboutOK(GtkWidget *widget, gpointer user_data) {
  gtk_widget_destroy(GTK_WIDGET(user_data));
}



void dialogUpdateOK(GtkWidget *widget, gpointer user_data) {
  gtk_widget_destroy(GTK_WIDGET(user_data));
}


void checkUpdate(GtkMenuItem *menuItem, gpointer user_data) {

  GtkWidget *dialogUpdate;
  GtkWidget *vbox1;
  GtkWidget *lblTitle;
  GtkWidget *lblCaption;
  GtkWidget *button;
  
	char outmessage[5000];
	doUpdateCheck(outmessage, 5000);
	printf("%s\n", outmessage);

  dialogUpdate = gtk_window_new(GTK_WINDOW_DIALOG);
  gtk_object_set_data(GTK_OBJECT(dialogUpdate), "dialogUpdate", dialogUpdate);
  gtk_container_set_border_width(GTK_CONTAINER (dialogUpdate), 15);
  gtk_window_set_title(GTK_WINDOW(dialogUpdate), "About");
  gtk_window_set_position(GTK_WINDOW(dialogUpdate), GTK_WIN_POS_CENTER);
  gtk_window_set_modal(GTK_WINDOW(dialogUpdate), TRUE);
  gtk_window_set_policy(GTK_WINDOW(dialogUpdate), FALSE, FALSE, FALSE);

  vbox1 = gtk_vbox_new(FALSE, 0);
  gtk_widget_ref(vbox1);
  gtk_object_set_data_full(GTK_OBJECT(dialogUpdate), "vbox1", vbox1, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(vbox1);
  gtk_container_add(GTK_CONTAINER(dialogUpdate), vbox1);
  lblTitle = gtk_label_new ("Ryan2 " VERSION);
  gtk_widget_ref (lblTitle);
  gtk_object_set_data_full(GTK_OBJECT(dialogUpdate), "lblTitle", lblTitle, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(lblTitle);
  gtk_box_pack_start(GTK_BOX(vbox1), lblTitle, FALSE, FALSE, 0);
  

  lblCaption = gtk_label_new(outmessage);
  gtk_widget_ref (lblCaption);
  gtk_object_set_data_full(GTK_OBJECT(dialogUpdate), "lblCaption", lblCaption, (GtkDestroyNotify) gtk_widget_unref);

  gtk_widget_show (lblCaption);
  gtk_box_pack_start (GTK_BOX (vbox1), lblCaption, FALSE, TRUE, 9);
  gtk_label_set_justify (GTK_LABEL (lblCaption), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (lblCaption), 7.45058e-09, 0.5);

  button = gtk_button_new_with_label ("OK");
  gtk_widget_ref (button);
  gtk_object_set_data_full (GTK_OBJECT (dialogUpdate), "button", button, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox1), button, FALSE, FALSE, 0);
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  gtk_widget_grab_focus (button);
  gtk_widget_grab_default (button);
  gtk_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(dialogUpdateOK), dialogUpdate);

  /* Show it */
  gtk_widget_show(dialogUpdate);
	
}

/* Build and display the about box */
void menuAboutActivate(GtkMenuItem *menuItem, gpointer user_data) {
  GtkWidget *dialogAbout;
  GtkWidget *vbox1;
  GtkWidget *lblTitle;
  GtkWidget *entry1;
  GtkWidget *button;
  GtkWidget *scrolledwindow1;

  dialogAbout = gtk_window_new(GTK_WINDOW_DIALOG);
  gtk_object_set_data(GTK_OBJECT(dialogAbout), "dialogAbout", dialogAbout);
  gtk_container_set_border_width(GTK_CONTAINER (dialogAbout), 15);
  gtk_window_set_title(GTK_WINDOW(dialogAbout), "About");
  gtk_window_set_position(GTK_WINDOW(dialogAbout), GTK_WIN_POS_CENTER);
  gtk_window_set_modal(GTK_WINDOW(dialogAbout), TRUE);
  gtk_window_set_policy(GTK_WINDOW(dialogAbout), FALSE, FALSE, FALSE);

  vbox1 = gtk_vbox_new(FALSE, 0);
  gtk_widget_ref(vbox1);
  gtk_object_set_data_full(GTK_OBJECT(dialogAbout), "vbox1", vbox1, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(vbox1);
  gtk_container_add(GTK_CONTAINER(dialogAbout), vbox1);
  lblTitle = gtk_label_new ("Ryan2  " VERSION);
  gtk_widget_ref (lblTitle);
  gtk_object_set_data_full(GTK_OBJECT(dialogAbout), "lblTitle", lblTitle, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(lblTitle);
  gtk_box_pack_start(GTK_BOX(vbox1), lblTitle, FALSE, FALSE, 0);


  scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_ref (scrolledwindow1);
  gtk_object_set_data_full (GTK_OBJECT (dialogAbout), "scrolledwindow1", scrolledwindow1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (scrolledwindow1);
  gtk_container_add (GTK_CONTAINER (dialogAbout), scrolledwindow1);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  entry1 = gtk_text_new (NULL, NULL);
  gtk_widget_ref (entry1);
  gtk_object_set_data_full (GTK_OBJECT (entry1), "entry1", entry1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (entry1);
  gtk_container_add (GTK_CONTAINER (scrolledwindow1), entry1);
  gtk_text_set_editable (GTK_TEXT (entry1), FALSE);
  gtk_widget_set_usize (entry1, 391, 250);
  gtk_text_insert (GTK_TEXT (entry1), NULL, NULL, NULL,
                   ABOUTMESSAGE, -1);
  
  gtk_box_pack_start (GTK_BOX (vbox1), scrolledwindow1, FALSE, TRUE, 9);
  gtk_label_set_justify (GTK_LABEL (scrolledwindow1), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (scrolledwindow1), 7.45058e-09, 0.5);


  button = gtk_button_new_with_label ("OK");
  gtk_widget_ref (button);
  gtk_object_set_data_full (GTK_OBJECT (dialogAbout), "button", button, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (vbox1), button, FALSE, FALSE, 0);
  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  gtk_widget_grab_focus (button);
  gtk_widget_grab_default (button);
  gtk_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(dialogAboutOK), dialogAbout);
  
  

  
  
  



  /* Show it */
  gtk_widget_show(dialogAbout);
}



void menuOptionsActivate(GtkMenuItem *menuItem, gpointer user_data) {
	Boolean *item; /* The item (opt1,2 or 3) we are going to change */
	int i;

	item = (Boolean *) user_data;
	for (i=0;i < 14;i++) {
		if (item == &opt[i]) {
			if (*item) {
				settings_setoptions(stngs, settingsarray[i], 1);
				if (i == 7) {
					settings_setoptions(stngs, ATTACK_PILLBOXES, 1);
					settings_setoptions(stngs, EXPLORER, 1);
				}
			} else {
				settings_setoptions(stngs, settingsarray[i], 0);
				if (i == 7) {
					settings_setoptions(stngs, ATTACK_PILLBOXES, 0);
					settings_setoptions(stngs, EXPLORER, 0);
				}
			}
		}
	}
	printf("%d\n", settings_getoptions(stngs, ATTACK_PILLBOXES));
	*item ^= 1;
}



void menu_destroy() {
	  gtk_widget_destroy(menu_top);
}
#endif


void setsettings(setting *stn) {
	stngs = stn;
}

