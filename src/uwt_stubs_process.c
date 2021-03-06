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

#include "uwt_stubs_process.h"

CAMLprim value
uwt_disable_stdio_inheritance_na(value unit){
  (void) unit;
  uv_disable_stdio_inheritance();
  return Val_unit;
}

static char **
caml_string_array_to_c_array(value p, int * e)
{
  const size_t len = Wosize_val(p);
  if ( len == 0 ){
    *e = UV_EINVAL;
    return NULL;
  }
  char ** env = malloc( (len + 1) * sizeof(char*) );
  if ( !env ){
    *e = UV_ENOMEM;
    return NULL;
  }
  size_t i;
  for ( i = 0; i < len; i++ ){
    value s = Field(p,i);
    if ( !uwt_is_safe_string(s) ){
      *e = UV_ECHARSET;
      free(env);
      return NULL;
    }
    env[i] = String_val(s);
  }
  env[len] = NULL;
  *e = 0;
  return env;
}

static struct handle *
get_handle(value o_s)
{
  if ( !Is_block(o_s) || Wosize_val(o_s) != 4 ){
    return NULL;
  }
  struct handle * s = Handle_val(o_s);
  if ( HANDLE_IS_INVALID(s) ){
    return NULL;
  }
  return s;
}

static void
spawn_exit_cb(uv_process_t*t, int64_t exit_status, int term_signal)
{
  HANDLE_CB_INIT(t);
  value exn = Val_unit;
  struct handle * h = t->data;
  if ( h ){
    if ( h->cb_read != CB_INVALID && h->cb_listen != CB_INVALID ){
      const int o_signal = uwt__rev_convert_signal_number(term_signal);
      value callback = GET_CB_VAL(h->cb_read);
      value process = GET_CB_VAL(h->cb_listen);
      uwt__gr_unregister(&h->cb_read);
      uwt__gr_unregister(&h->cb_listen);
      exn=caml_callback3_exn(callback,
                             process,
                             Val_long(exit_status),
                             Val_long(o_signal));
    }
    if ( h->in_use_cnt ){
      --h->in_use_cnt;
    }
  }
  HANDLE_CB_RET(exn);
}

CAMLprim value
uwt_spawn(value p1, value p2, value p3, value p4)
{
  INIT_LOOP_RESULT(loop,Field(p1,0));
  uv_loop_t * l = &loop->loop;
  CAMLparam4(p1,p2,p3,p4);
  CAMLlocal2(ret,op);
  unsigned int i;
  int erg = UV_UWT_EFATAL;
  bool spawn_called = false;
  uv_process_options_t t;
  uv_stdio_container_t stdio[3];
  struct handle * handle;

  GR_ROOT_ENLARGE();
  ret = caml_alloc(1,Error_tag);
  op = uwt__handle_create(UV_PROCESS,loop);
  handle = Handle_val(op);
  handle->close_executed = 1;

  /* below: now further caml allocations */
  memset(&t, 0, sizeof t);
  memset(&stdio, 0, sizeof stdio);

  value tmp = Field(p1,1);

  for ( i = 0; i < 3; ++i ){
    value cur = Field(tmp,i);
    if ( cur == Val_unit ){
      stdio[i].flags = UV_IGNORE;
      stdio[i].data.stream = NULL;
    }
    else {
      cur = Field(cur,0);
      struct handle * h;
      const int tag = Tag_val(cur);
      cur = Field(cur,0);
      if ( tag == 0 ){
        stdio[i].flags = UV_INHERIT_FD;
        stdio[i].data.fd = FD_VAL(cur);
      }
      else {
        h = get_handle(cur);
        if ( h == NULL ){
          erg = UV_EBADF;
          goto error_end;
        }
        stdio[i].data.stream = (uv_stream_t*)h->handle;
        if ( tag == 1 ){
          if ( i == 0 ){
            stdio[i].flags = UV_CREATE_PIPE | UV_READABLE_PIPE;
          }
          else {
            stdio[i].flags = UV_CREATE_PIPE | UV_WRITABLE_PIPE;
          }
        }
        else {
          assert( tag == 2 || tag == 3 );
          stdio[i].flags = UV_INHERIT_STREAM;
        }
      }
    }
  }
  t.exit_cb = spawn_exit_cb;

  tmp = Field(p4,0) ;
  if ( !uwt_is_safe_string(tmp) ){
    erg = UV_ECHARSET;
    goto error_end;
  }
  t.file = String_val(tmp);

  tmp = Field(p4,1);
  if ( tmp == Atom(0) || Wosize_val(tmp) == 0 ){
    t.args = NULL;
  }
  else {
    t.args = caml_string_array_to_c_array(tmp,&erg);
    if ( t.args == NULL ){
      goto error_end;
    }
  }

  tmp = Field(p2,0);
  if ( Wosize_val(tmp) == 0 ){
    t.env =  NULL;
  }
  else {
    t.env = caml_string_array_to_c_array(tmp,&erg);
    if ( !t.env ){
      goto error_end;
    }
  }
  tmp = Field(p2,1);
  if ( tmp == Val_unit ){
    t.cwd = NULL;
  }
  else {
    tmp = Field(tmp,0);
    if ( !uwt_is_safe_string(tmp) ){
      erg = UV_ECHARSET;
      goto error_end;
    }
    t.cwd = String_val(tmp);
  }
  t.flags = Long_val(Field(p1,3));
  t.stdio_count = 3;
  t.stdio = stdio;
  t.uid = Long_val(Field(Field(p1,2),0));
  t.gid = Long_val(Field(Field(p1,2),1));

  spawn_called = true;
  handle->close_executed = 0;
  erg = uv_spawn(l, (uv_process_t*)handle->handle, &t);
  if ( erg < 0 ){
    /* uv_process_init is called internally first, see also:
       https://groups.google.com/forum/message/raw?msg=libuv/DUBr8DtzsWk/hw11ob9sPZ4J */
    handle->finalize_called = 1;
    uwt__handle_finalize_close(handle);
  }
  else {
    for ( i = 0; i < 3; ++i ){
      if ( (stdio[i].flags & UV_CREATE_PIPE) != 0 ){
        struct handle * h = stdio[i].data.stream->data;
        h->initialized = 1;
      }
    }
  }
error_end:
  if ( t.args ){
    free(t.args);
  }
  if ( t.env ){
    free(t.env);
  }
  if ( erg < 0 ){
    if ( spawn_called == false ){
      uwt__free_mem_uv_handle_t(handle);
      uwt__free_struct_handle(handle);
    }
    Field(op,1) = 0;
    Field(ret,0) = Val_uwt_error(erg);
  }
  else {
    if ( Is_block(p3) ){
      uwt__gr_register(&handle->cb_read,Field(p3,0));
      uwt__gr_register(&handle->cb_listen,op);
    }
    handle->initialized = 1;
    ++handle->in_use_cnt;
    Tag_val(ret) = Ok_tag;
    Store_field(ret,0,op);
  }
  CAMLreturn(ret);
}

CAMLprim value
uwt_pid_na(value o_h)
{
  HANDLE_NINIT_NA(h,o_h);
  uv_process_t * p = (uv_process_t *)h->handle;
  return (Val_long(p->pid));
}

CAMLprim value
uwt_process_kill_na(value o_h,value o_sig)
{
  HANDLE_NINIT_NA(h,o_h);
  uv_process_t * p = (uv_process_t *)h->handle;
  int signum = uwt__convert_signal_number(Long_val(o_sig));
  int ret = uv_process_kill(p,signum);
  return (VAL_UWT_UNIT_RESULT(ret));
}

CAMLprim value
uwt_kill_na(value o_pid,value o_sig)
{
  int signum = uwt__convert_signal_number(Long_val(o_sig));
  int ret = uv_kill(Long_val(o_pid),signum);
  return (VAL_UWT_UNIT_RESULT(ret));
}
