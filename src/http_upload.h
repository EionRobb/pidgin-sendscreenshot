 /*
  *  Pidgin SendScreenshot plugin - http upload header -
  *
  * This program is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation; either version 2 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
  *
  *
  * --  Raoul Berger <contact@raoulito.info>
  *
  */

#ifndef __HTTP_UPLOAD_H__
#define __HTTP_UPLOAD_H__ 1

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#ifndef ENABLE_UPLOAD
#error "***** ENABLE_UPLOAD is not defined ! *****"
#endif

#include "main.h"

#define CLEAR_HOST_PARAM_DATA(host_data)			\
  host_data->is_inside = FALSE;					\
  host_data->quit_handlers = FALSE;				\
  if (host_data->extra_names || host_data->extra_values) {	\
    guint i;							\
    for (i = 0; i < host_data->extra_names->len; i++) {		\
      gchar *extra_name, *extra_value;				\
      extra_name =					  	\
	g_array_index (host_data->extra_names, gchar *, i);	\
      extra_value=						\
	g_array_index (host_data->extra_values, gchar *, i);	\
      g_free (extra_name);					\
      g_free (extra_value);					\
      }								\
    g_array_free (host_data->extra_names, TRUE);		\
    g_array_free (host_data->extra_values, TRUE);		\
    host_data->extra_names = NULL;				\
    host_data->extra_values = NULL;				\
  }								\
  if (host_data->xml_hosts_version) {	  			\
    g_free (host_data->xml_hosts_version);			\
    host_data->xml_hosts_version = NULL;			\
  }								\
  if (host_data->host_names) {					\
    guint i;							\
    for (i = 0; i < host_data->host_names->len; i++) {		\
      gchar *host_name =					\
	g_array_index (host_data->host_names, gchar *, i);	\
      g_free (host_name);					\
    }								\
    g_array_free (host_data->host_names, TRUE);			\
    host_data->host_names = NULL;				\
  }

#define CLEAR_HOST_PARAM_DATA_FULL(host_data)			\
 if (host_data->selected_hostname) {				\
    g_free (host_data->selected_hostname);			\
    host_data->selected_hostname = NULL;			\
  }								\
  if (host_data->form_action){					\
    g_free (host_data->form_action);				\
    host_data->form_action = NULL;				\
  }								\
  if (host_data->file_input_name) {				\
    g_free (host_data->file_input_name);			\
    host_data->file_input_name = NULL;				\
  }								\
  if (host_data->regexp) {					\
    g_free (host_data->regexp);					\
    host_data->regexp = NULL;					\
  }								\
  if (host_data->html_response) {					\
    g_free (host_data->html_response);				\
    host_data->html_response = NULL;					\
  }								\
  CLEAR_HOST_PARAM_DATA(host_data)

#define PLUGIN_ERR_ON_LINE gettext_noop ("Error on line %d char %d:\n")
#define PLUGIN_XML_STUFF_MISSING  _("Something's missing for server \'%s\':")
#define PLUGIN_PLEASE_REFER_TO _("Please refer to %s to learn how to add your own server.")

#define PLUGIN_UPLOAD_BAD_REGEXP_ERROR _("Bad regular expression: '%s'!")

#define PLUGIN_UPLOAD_FETCHURL_ERROR _("Cannot fetch the URL of your screenshot !\n"\
				       "Regular expression doesn't match (see debug window).\n"\
				       "Most likeley '%s' has been updated...\n")

#define PLUGIN_UPLOAD_XML_ERROR _("Incorrect xml host config file")

#define PLUGIN_LOAD_XML_ERROR _("%s.\n\n"\
				"This file contains a list of image hosting providers.\n"\
				"Please connect to %s to in order to fetch it.")

#define PLUGIN_HOST_DISABLED_ERROR _("You haven't selected any image hosting provider yet.\n"\
				     "Please select one from the configure dialog.")

#define PLUGIN_PARSE_XML_ERROR _("Failed to parse the contents of:\n%s")


#define PLUGIN_PARSE_XML_MISSING_HOST _("Selected host not met !")
#define PLUGIN_PARSE_XML_MISSING_ACTION _("No server-side form handler !")
#define PLUGIN_PARSE_XML_MISSING_INPUT  _("No file-select control name !")
#define PLUGIN_PARSE_XML_MISSING_REGEXP _("No regular expression to extract html link !")

#define PLUGIN_PARSE_XML_ERRMSG_INVELEM _(PLUGIN_ERR_ON_LINE"\"%s\" is not a valid element name!\n")
#define PLUGIN_PARSE_XML_ERRMSG_INVATTR _(PLUGIN_ERR_ON_LINE"\"%s\" is not a valid attribute for element \"%s\"!\n")
#define PLUGIN_PARSE_XML_ERRMSG_MISSATTR _(PLUGIN_ERR_ON_LINE"element \"%s\" is missing an attribute !\n")
#define PLUGIN_PARSE_XML_ERRMSG_MISSATTRVAL _(PLUGIN_ERR_ON_LINE"a \"value\" attribute is missing !\n")



#define MAX_URL_SIZE 2500       /* this should be enough, right ? */


#define SEND_AS_HTML_LINK_TXT _("as a link (_HTML upload)")
#define SEND_AS_HTML_URL_TXT _("as a URL (_HTML upload)")
#define MORE_INFO _("More info")

/* decls, prototypes... whatever they are called : */

void http_upload_prepare (PurplePlugin * plugin);

#endif

/* end of http_upload.h */
