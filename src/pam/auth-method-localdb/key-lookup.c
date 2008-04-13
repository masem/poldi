/* key-lookup.c - Lookup keys for localdb authentication
   Copyright (C) 2004, 2005, 2007, 2008 g10 Code GmbH
 
   This file is part of Poldi.
  
   Poldi is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
  
   Poldi is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

#include <config.h>

#include <gpg-error.h>
#include <gcrypt.h>

#include "util/util.h"
#include "util/defs.h"
#include "util/support.h"

#include "key-lookup.h"



/* This functions construct a new C-string containing the absolute
   path for the file, which is to expected to contain the public key
   for the card identified by SERIALNO.  Returns proper error
   code.  */
static gpg_error_t
key_filename_construct (char **filename, const char *serialno)
{
  char *path;

  path = make_filename (POLDI_KEY_DIRECTORY, serialno, NULL);
  *filename = path;

  return 0;
}

/* Lookup the key belonging to the card specified by SERIALNO.
   Returns a proper error code.  */
gpg_error_t
key_lookup_by_serialno (const char *serialno, gcry_sexp_t *key)
{
  gcry_sexp_t key_sexp;
  char *key_string;
  char *key_path;
  gpg_error_t err;

  key_path = NULL;
  key_string = NULL;

  err = key_filename_construct (&key_path, serialno);
  if (err)
    {
      log_error ("Error: failed to construct key file path "
		 "for serial number `%s': %s\n",
		 serialno, gpg_strerror (err));
      goto out;
    }

  err = file_to_string (key_path, &key_string);
  if ((! err) && (! key_string))
    err = gpg_error (GPG_ERR_NO_PUBKEY);
  if (err)
    {
      log_error ("Error: failed to retrieve key from key file `%s': %s\n",
		 key_path, gpg_strerror (err));
      goto out;
    }

  err = string_to_sexp (&key_sexp, key_string);
  if (err)
    {
      log_error ("Error: failed to convert key "
		 "from `%s' into S-Expression: %s\n",
		 key_path, gpg_strerror (err));
      goto out;
    }

  *key = key_sexp;

 out:

  free (key_path);
  free (key_string);

  return err;
}

