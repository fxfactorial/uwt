/* Libuv bindings for OCaml
 * http://github.com/fdopen/uwt
 * Copyright (C) 2015-2016 Andreas Hauptmann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, with linking exceptions;
 * either version 2.1 of the License, or (at your option) any later
 * version. See COPYING file for details.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef __UWT_STUBS_PIPE_H
#define __UWT_STUBS_PIPE_H

#include "uwt_stubs_handle.h"

#ifdef __cplusplus
extern "C" {
#endif

UWT_EXTERN3(uwt_pipe_open);
UWT_EXTERN2(uwt_pipe_init);
UWT_EXTERN2(uwt_pipe_bind_na);
UWT_EXTERN1(uwt_pipe_getsockname);
UWT_EXTERN1(uwt_pipe_getpeername);
UWT_EXTERN2(uwt_pipe_pending_instances_na);
UWT_EXTERN1(uwt_pipe_pending_count_na);
UWT_EXTERN1(uwt_pipe_pending_type_na);
UWT_EXTERN3(uwt_pipe_connect);

#ifdef __cplusplus
}
#endif

#endif /* __UWT_STUBS_PIPE_H */
